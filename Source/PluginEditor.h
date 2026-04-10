#pragma once

#include "PluginProcessor.h"
#include "ui/CustomLookAndFeel.h"
#include "ui/PeakMeter.h"
#include "PresetManager.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

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
    std::array<SliderWithAttachment, 10> controls;
    juce::Slider* activeSlider = nullptr;
    
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;
    PeakMeter peakMeter;
    
    void updatePresetComboBox();
    void savePresetClicked();
    void deletePresetClicked();
    
    // Peak meter state
    float currentPeakLevel { 0.0f };
    bool clipIndicatorActive { false };
    int clipIndicatorTimer { 0 };
    static constexpr int CLIP_HOLD_TIME_MS = 500;
    static constexpr int TIMER_INTERVAL_MS = 30;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
