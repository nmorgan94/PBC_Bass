#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include "synth/ReeseVoice.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
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
