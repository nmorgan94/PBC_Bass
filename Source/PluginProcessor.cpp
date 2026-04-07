#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
namespace
{
constexpr auto twoPi = juce::MathConstants<float>::twoPi;

float makeBipolarSaw (float phase)
{
    return (2.0f * phase) - 1.0f;
}
}

AudioPluginAudioProcessor::ReeseVoice::ReeseVoice (AudioPluginAudioProcessor& ownerProcessor)
    : owner (ownerProcessor)
{
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    ampEnvelopeParameters.attack = 0.01f;
    ampEnvelopeParameters.decay = 0.08f;
    ampEnvelopeParameters.sustain = 0.85f;
    ampEnvelopeParameters.release = 0.2f;
    ampEnvelope.setParameters (ampEnvelopeParameters);
}

bool AudioPluginAudioProcessor::ReeseVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<ReeseSound*> (sound) != nullptr;
}

void AudioPluginAudioProcessor::ReeseVoice::startNote (int midiNoteNumber, float velocity,
                                                       juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    level = velocity;
    baseFrequencyHz = (float) juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    oscPhaseA = 0.0f;
    oscPhaseB = 0.5f;
    subPhase = 0.0f;
    pitchWheelMoved (currentPitchWheelPosition);
    ampEnvelope.noteOn();
}

void AudioPluginAudioProcessor::ReeseVoice::stopNote (float, bool allowTailOff)
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

void AudioPluginAudioProcessor::ReeseVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchBendSemitones = juce::jmap ((float) newPitchWheelValue, 0.0f, 16383.0f, -2.0f, 2.0f);
}

void AudioPluginAudioProcessor::ReeseVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    juce::ignoreUnused (controllerNumber, newControllerValue);
}

void AudioPluginAudioProcessor::ReeseVoice::prepare (double sampleRate, int samplesPerBlock, int outputChannels)
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
    updateFilter();
}

float AudioPluginAudioProcessor::ReeseVoice::getDetunedFrequencyHz() const
{
    const auto detuneCents = owner.getFloatParam ("detune");
    const auto bendRatio = std::pow (2.0f, pitchBendSemitones / 12.0f);
    const auto detuneRatio = std::pow (2.0f, (detuneCents * 0.5f) / 1200.0f);
    return baseFrequencyHz * bendRatio * detuneRatio;
}

void AudioPluginAudioProcessor::ReeseVoice::updateFilter()
{
    filter.setCutoffFrequency (owner.getFloatParam ("cutoff"));
    filter.setResonance (owner.getFloatParam ("resonance"));
}

float AudioPluginAudioProcessor::ReeseVoice::renderSample()
{
    const auto frequencyA = baseFrequencyHz * std::pow (2.0f, pitchBendSemitones / 12.0f);
    const auto frequencyB = getDetunedFrequencyHz();
    const auto subFrequency = 0.5f * frequencyA;
    const auto sampleRateFloat = (float) currentSampleRate;

    oscPhaseA += frequencyA / sampleRateFloat;
    oscPhaseB += frequencyB / sampleRateFloat;
    subPhase += subFrequency / sampleRateFloat;
    lfoPhase += owner.getFloatParam ("lfoRate") / sampleRateFloat;

    oscPhaseA -= std::floor (oscPhaseA);
    oscPhaseB -= std::floor (oscPhaseB);
    subPhase -= std::floor (subPhase);
    lfoPhase -= std::floor (lfoPhase);

    const auto sawA = makeBipolarSaw (oscPhaseA);
    const auto sawB = makeBipolarSaw (oscPhaseB);
    const auto sub = std::sin (twoPi * subPhase) * owner.getFloatParam ("sub");
    const auto lfo = std::sin (twoPi * lfoPhase) * owner.getFloatParam ("lfoDepth");
    const auto dry = (0.5f * (sawA + sawB)) + sub;
    const auto driven = std::tanh (dry * (1.0f + owner.getFloatParam ("drive") * 6.0f));
    const auto envelope = ampEnvelope.getNextSample();

    const auto modulatedCutoff = juce::jlimit (40.0f, 18000.0f,
                                               owner.getFloatParam ("cutoff") * (1.0f + (0.35f * lfo)));
    filter.setCutoffFrequency (modulatedCutoff);
    return filter.processSample (0, driven * envelope * level);
}

void AudioPluginAudioProcessor::ReeseVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    updateFilter();

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

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new ReeseVoice (*this));

    synth.addSound (new ReeseSound());
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("output", "Output",
                                                                    juce::NormalisableRange<float> (-24.0f, 6.0f, 0.1f), -6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("detune", "Detune",
                                                                    juce::NormalisableRange<float> (0.0f, 40.0f, 0.01f), 12.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sub", "Sub",
                                                                    juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cutoff", "Cutoff",
                                                                    juce::NormalisableRange<float> (40.0f, 18000.0f, 0.01f, 0.35f), 1800.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("resonance", "Resonance",
                                                                    juce::NormalisableRange<float> (0.1f, 1.2f, 0.001f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("drive", "Drive",
                                                                    juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.15f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoRate", "LFO Rate",
                                                                    juce::NormalisableRange<float> (0.05f, 20.0f, 0.001f, 0.35f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfoDepth", "LFO Depth",
                                                                    juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.25f));

    return { params.begin(), params.end() };
}

float AudioPluginAudioProcessor::getFloatParam (const juce::StringRef& paramID) const
{
    if (auto* value = apvts.getRawParameterValue (paramID))
        return value->load();

    jassertfalse;
    return 0.0f;
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<ReeseVoice*> (synth.getVoice (i)))
            voice->prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void AudioPluginAudioProcessor::releaseResources()
{
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    const auto outputGain = juce::Decibels::decibelsToGain (getFloatParam ("output"));
    const auto driveTrim = juce::jmap (getFloatParam ("drive"), 1.0f, 0.75f);
    buffer.applyGain (outputGain * driveTrim);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
