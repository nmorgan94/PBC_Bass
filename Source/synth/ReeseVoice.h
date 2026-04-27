#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// Forward declaration
class AudioPluginAudioProcessor;

class ReeseSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class ReeseVoice final : public juce::SynthesiserVoice
{
public:
    explicit ReeseVoice (AudioPluginAudioProcessor& ownerProcessor);

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void prepare (double sampleRate, int samplesPerBlock, int outputChannels);
    void setLegatoMode (bool shouldBeLegatoMode);

private:
    float renderSample();
    float getDetunedFrequencyHz() const;
    void updateFilter();
    void updateEnvelope();
    float getLFORate() const;

    AudioPluginAudioProcessor& owner;
    juce::ADSR ampEnvelope;
    juce::ADSR::Parameters ampEnvelopeParameters;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::StateVariableTPTFilter<float> filter2;
    double currentSampleRate { 44100.0 };
    float level { 0.0f };
    float baseFrequencyHz { 110.0f };
    float targetFrequencyHz { 110.0f };
    float currentFrequencyHz { 0.0f };
    float pitchBendSemitones { 0.0f };
    float subPhase { 0.0f };
    float lfoPhase { 0.0f };
    bool isLegatoMode { false };
    
    std::array<float, 8> unisonPhasesA {};
    std::array<float, 8> unisonPhasesB {};
};
