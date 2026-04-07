#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    configureSlider (controls[0], "output", "Output");
    configureSlider (controls[1], "detune", "Detune");
    configureSlider (controls[2], "sub", "Sub");
    configureSlider (controls[3], "cutoff", "Cutoff");
    configureSlider (controls[4], "resonance", "Resonance");
    configureSlider (controls[5], "drive", "Drive");
    configureSlider (controls[6], "lfoRate", "LFO Rate");
    configureSlider (controls[7], "lfoDepth", "LFO Depth");

    setSize (720, 420);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() = default;

void AudioPluginAudioProcessorEditor::configureSlider (SliderWithAttachment& sliderControl,
                                                       const juce::String& paramID,
                                                       const juce::String& labelText)
{
    sliderControl.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderControl.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    sliderControl.slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    sliderControl.slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible (sliderControl.slider);

    sliderControl.label.setText (labelText, juce::dontSendNotification);
    sliderControl.label.setJustificationType (juce::Justification::centred);
    sliderControl.label.attachToComponent (&sliderControl.slider, false);
    addAndMakeVisible (sliderControl.label);

    sliderControl.attachment = std::make_unique<SliderAttachment> (processorRef.apvts, paramID, sliderControl.slider);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 18, 24));

    auto bounds = getLocalBounds().reduced (20);

    g.setColour (juce::Colours::darkorange);
    g.setFont (juce::FontOptions (28.0f).withStyle ("Bold"));
    g.drawText ("PBC Bass", bounds.removeFromTop (30), juce::Justification::centredLeft);

    auto panelBounds = bounds.reduced (8, 2);
    g.setColour (juce::Colours::grey.withAlpha (0.35f));
    g.drawRoundedRectangle (panelBounds.toFloat(), 12.0f, 1.5f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);
    bounds.removeFromTop (30);

    auto panelArea = bounds.reduced (8, 2);
    auto area = panelArea.reduced (12);

    constexpr int columns = 4;
    const auto rowHeight = area.getHeight() / 2;
    const auto columnWidth = area.getWidth() / columns;

    for (int i = 0; i < (int) controls.size(); ++i)
    {
        const auto row = i / columns;
        const auto column = i % columns;

        auto cell = juce::Rectangle<int> (area.getX() + (column * columnWidth),
                                          area.getY() + (row * rowHeight),
                                          columnWidth,
                                          rowHeight).reduced (10);

        controls[(size_t) i].label.setBounds (cell.removeFromTop (24));
        controls[(size_t) i].slider.setBounds (cell);
    }
}
