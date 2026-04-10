#pragma once

#include "PluginProcessor.h"
#include "ui/CustomLookAndFeel.h"
#include "PresetManager.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    struct SliderWithAttachment
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
        juce::String originalLabelText;
    };

    void configureSlider (SliderWithAttachment& sliderControl, const juce::String& paramID, const juce::String& labelText);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    CustomLookAndFeel customLookAndFeel;
    PresetManager presetManager;
    std::array<SliderWithAttachment, 9> controls;
    juce::Slider* activeSlider = nullptr;
    
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;
    
    void updatePresetComboBox();
    void savePresetClicked();
    void deletePresetClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
