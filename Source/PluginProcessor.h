#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    float getFloatParam (const juce::StringRef& paramID) const;
    int getIntParam (const juce::StringRef& paramID) const;
    bool getBoolParam (const juce::StringRef& paramID) const;
    int getChoiceParam (const juce::StringRef& paramID) const;
    
    // Peak level tracking
    float getPeakLevel() const { return peakLevel.load(); }
    bool isClipping() const { return isCurrentlyClipping.load(); }
    
    // Transport info
    double getCurrentBPM() const { return currentBPM.load(); }

private:
    struct ReeseSound final : public juce::SynthesiserSound
    {
        bool appliesToNote (int) override { return true; }
        bool appliesToChannel (int) override { return true; }
    };

    struct ReeseVoice final : public juce::SynthesiserVoice
    {
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

    juce::Synthesiser synth;
    int numNotesHeld { 0 };
    
    // Peak level tracking
    std::atomic<float> peakLevel { 0.0f };
    std::atomic<bool> isCurrentlyClipping { false };
    
    // Transport info tracking
    std::atomic<double> currentBPM { 0.0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
