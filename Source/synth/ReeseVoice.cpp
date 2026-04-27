#include "ReeseVoice.h"
#include "../PluginProcessor.h"

//==============================================================================
namespace
{
constexpr auto twoPi = juce::MathConstants<float>::twoPi;

// Waveform generation functions
float makeBipolarSaw (float phase)
{
    return (2.0f * phase) - 1.0f;
}

float makeBipolarSquare (float phase)
{
    return (phase < 0.5f) ? -1.0f : 1.0f;
}

float makeBipolarTriangle (float phase)
{
    return (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
}

float generateWaveform (float phase, int waveType)
{
    switch (waveType)
    {
        case 0: return makeBipolarSquare (phase);
        case 1: return makeBipolarSaw (phase);
        case 2: return makeBipolarTriangle (phase);
        default: return makeBipolarSaw (phase);
    }
}
}

//==============================================================================
ReeseVoice::ReeseVoice (AudioPluginAudioProcessor& ownerProcessor)
    : owner (ownerProcessor)
{
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    filter2.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    updateEnvelope();
}

bool ReeseVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<ReeseSound*> (sound) != nullptr;
}

void ReeseVoice::startNote (int midiNoteNumber, float velocity,
                            juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    level = velocity;
    // Transpose down 2 octaves
    targetFrequencyHz = (float) juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber - 24);
    
    // If not in legato mode, or if this is the first note, jump to target frequency
    if (!isLegatoMode || currentFrequencyHz == 0.0f)
    {
        currentFrequencyHz = targetFrequencyHz;
        baseFrequencyHz = targetFrequencyHz;
    }
    // Otherwise keep currentFrequencyHz and baseFrequencyHz, renderSample will glide to targetFrequencyHz
    
    pitchWheelMoved (currentPitchWheelPosition);
    ampEnvelope.noteOn();
}

void ReeseVoice::setLegatoMode (bool shouldBeLegatoMode)
{
    isLegatoMode = shouldBeLegatoMode;
}

void ReeseVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnvelope.noteOff();
    }
    else
    {
        ampEnvelope.reset();
        clearCurrentNote();
    }
}

void ReeseVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchBendSemitones = juce::jmap ((float) newPitchWheelValue, 0.0f, 16383.0f, -2.0f, 2.0f);
}

void ReeseVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    juce::ignoreUnused (controllerNumber, newControllerValue);
}

void ReeseVoice::prepare (double sampleRate, int samplesPerBlock, int outputChannels)
{
    juce::ignoreUnused (samplesPerBlock, outputChannels);

    currentSampleRate = sampleRate;
    ampEnvelope.setSampleRate (sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) juce::jmax (samplesPerBlock, 1);
    spec.numChannels = 1;
    filter.reset();
    filter.prepare (spec);
    filter2.reset();
    filter2.prepare (spec);
    updateFilter();
}

float ReeseVoice::getDetunedFrequencyHz() const
{
    const auto detuneCents = owner.getFloatParam ("detune");
    const auto bendRatio = std::pow (2.0f, pitchBendSemitones / 12.0f);
    const auto detuneRatio = std::pow (2.0f, (detuneCents * 0.5f) / 1200.0f);
    return baseFrequencyHz * bendRatio * detuneRatio;
}

void ReeseVoice::updateFilter()
{
    const auto cutoff = owner.getFloatParam ("cutoff");
    const auto resonance = owner.getFloatParam ("resonance");
    
    filter.setCutoffFrequency (cutoff);
    filter.setResonance (resonance);
    filter2.setCutoffFrequency (cutoff);
    filter2.setResonance (resonance);
}

void ReeseVoice::updateEnvelope()
{
    ampEnvelopeParameters.attack = owner.getFloatParam ("attack");
    ampEnvelopeParameters.decay = owner.getFloatParam ("decay");
    ampEnvelopeParameters.sustain = owner.getFloatParam ("sustain");
    ampEnvelopeParameters.release = owner.getFloatParam ("release");
    ampEnvelope.setParameters (ampEnvelopeParameters);
}

float ReeseVoice::getLFORate() const
{
    const bool syncEnabled = owner.getBoolParam ("lfoSync");
    
    if (!syncEnabled)
        return owner.getFloatParam ("lfoRate");
    
    double bpm = owner.getCurrentBPM();
    if (bpm <= 0.0)
        bpm = 120.0;  // Default to 120 BPM if host doesn't provide tempo
    
    const int syncRateIndex = owner.getChoiceParam ("lfoSyncRate");
    
    // Beats per cycle for each timing option
    // T = Triplet (2/3 of normal), D = Dotted (1.5x normal)
    const float beatsPerCycle[] = {
        0.25f,                  // 1/16
        0.25f * 2.0f / 3.0f,    // 1/16T
        0.25f * 1.5f,           // 1/16D
        0.5f,                   // 1/8
        0.5f * 2.0f / 3.0f,     // 1/8T
        0.5f * 1.5f,            // 1/8D
        1.0f,                   // 1/4
        1.0f * 2.0f / 3.0f,     // 1/4T
        1.0f * 1.5f,            // 1/4D
        2.0f,                   // 1/2
        2.0f * 2.0f / 3.0f,     // 1/2T
        2.0f * 1.5f,            // 1/2D
        4.0f,                   // 1 Bar
        8.0f,                   // 2 Bars
        16.0f                   // 4 Bars
    };
    const float beats = beatsPerCycle[syncRateIndex];
    
    return static_cast<float>((bpm / 60.0) / beats);
}

float ReeseVoice::renderSample()
{
    // Apply glide smoothing only in legato mode
    if (isLegatoMode && std::abs (currentFrequencyHz - targetFrequencyHz) > 0.01f)
    {
        const auto glideTime = owner.getFloatParam ("glideTime");
        
        // Calculate glide coefficient (exponential smoothing)
        const auto glideCoeff = 1.0f - std::exp (-1.0f / (glideTime * (float) currentSampleRate));
        currentFrequencyHz += (targetFrequencyHz - currentFrequencyHz) * glideCoeff;
        baseFrequencyHz = currentFrequencyHz;
    }
    else
    {
        currentFrequencyHz = targetFrequencyHz;
        baseFrequencyHz = targetFrequencyHz;
    }
    
    const auto frequencyA = baseFrequencyHz * std::pow (2.0f, pitchBendSemitones / 12.0f);
    const auto frequencyB = getDetunedFrequencyHz();
    const auto subFrequency = 0.5f * frequencyA;
    const auto sampleRateFloat = (float) currentSampleRate;
    
    const auto unisonVoices = static_cast<size_t>(owner.getIntParam ("unisonVoices"));
    const int oscAWaveType = owner.getIntParam ("oscAWave");
    const int oscBWaveType = owner.getIntParam ("oscBWave");
    
    // Render unison oscillators
    float unisonSum = 0.0f;
    
    for (size_t i = 0; i < unisonVoices; ++i)
    {
        // Calculate detune offset for this voice (-50% to +50% of detune range)
        const float voiceDetune = (unisonVoices > 1)
            ? ((float) i / (float) (unisonVoices - 1) - 0.5f) * 2.0f
            : 0.0f;
        
        const auto unisonFreqA = frequencyA * std::pow (2.0f, voiceDetune * owner.getFloatParam ("detune") / 1200.0f);
        const auto unisonFreqB = frequencyB * std::pow (2.0f, voiceDetune * owner.getFloatParam ("detune") / 1200.0f);
        
        // Update phases for this unison voice
        unisonPhasesA[i] += unisonFreqA / sampleRateFloat;
        unisonPhasesB[i] += unisonFreqB / sampleRateFloat;
        
        unisonPhasesA[i] -= std::floor (unisonPhasesA[i]);
        unisonPhasesB[i] -= std::floor (unisonPhasesB[i]);
        
        const auto waveA = generateWaveform (unisonPhasesA[i], oscAWaveType);
        const auto waveB = generateWaveform (unisonPhasesB[i], oscBWaveType);
        
        unisonSum += (waveA + waveB) * 0.5f;
    }
    
    // Normalize by number of voices to prevent volume increase
    unisonSum /= (float) unisonVoices;

    subPhase += subFrequency / sampleRateFloat;
    lfoPhase += getLFORate() / sampleRateFloat;

    subPhase -= std::floor (subPhase);
    lfoPhase -= std::floor (lfoPhase);

    const auto sub = std::sin (twoPi * subPhase) * owner.getFloatParam ("sub");
    const auto lfo = std::sin (twoPi * lfoPhase) * owner.getFloatParam ("lfoDepth");
    const auto dry = unisonSum + sub;
    const auto driven = std::tanh (dry * (1.0f + owner.getFloatParam ("drive") * 6.0f));
    const auto envelope = ampEnvelope.getNextSample();

    const auto modulatedCutoff = juce::jlimit (40.0f, 18000.0f,
                                               owner.getFloatParam ("cutoff") * (1.0f + (0.35f * lfo)));
    filter.setCutoffFrequency (modulatedCutoff);
    filter2.setCutoffFrequency (modulatedCutoff);
    
    auto filtered = filter.processSample (0, driven * envelope * level);
    
    // Apply second stage if 24dB mode is enabled
    if (owner.getBoolParam ("filter24db"))
        filtered = filter2.processSample (0, filtered);
    
    return filtered;
}

void ReeseVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    updateFilter();
    updateEnvelope();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto value = renderSample();

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample (channel, startSample + sample, value);

        if (! ampEnvelope.isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}
