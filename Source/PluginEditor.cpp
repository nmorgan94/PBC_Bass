#include "PluginEditor.h"
#include "ui/CustomLookAndFeel.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setLookAndFeel(&customLookAndFeel);

    configureSlider (controls[0], "output", "OUTPUT");
    configureSlider (controls[1], "detune", "DETUNE");
    configureSlider (controls[2], "sub", "SUB");
    configureSlider (controls[3], "cutoff", "CUTOFF");
    configureSlider (controls[4], "resonance", "RESONANCE");
    configureSlider (controls[5], "drive", "DRIVE");
    configureSlider (controls[6], "lfoRate", "LFO RATE");
    configureSlider (controls[7], "lfoDepth", "LFO DEPTH");
    configureSlider (controls[8], "glideTime", "GLIDE");

    setSize (720, 420);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::configureSlider (SliderWithAttachment& sliderControl,
                                                       const juce::String& paramID,
                                                       const juce::String& labelText)
{
    sliderControl.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderControl.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    sliderControl.slider.setMouseDragSensitivity(150);
    addAndMakeVisible (sliderControl.slider);

    sliderControl.label.setText (labelText, juce::dontSendNotification);
    sliderControl.label.setJustificationType (juce::Justification::centred);
    sliderControl.label.setFont(CustomLookAndFeel::orbitronBold().withHeight(12.0f));
    sliderControl.label.attachToComponent (&sliderControl.slider, false);
    addAndMakeVisible (sliderControl.label);

    sliderControl.attachment = std::make_unique<SliderAttachment> (processorRef.apvts, paramID, sliderControl.slider);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bgGradient(juce::Colour(0xff0a0e1a), 0, 0,
                                    juce::Colour(0xff151925), 0, (float)getHeight(),
                                    false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    auto bounds = getLocalBounds().reduced (3);

    auto titleBounds = bounds.removeFromTop(40);
    
    // Calculate panel bounds first to align title
    auto panelBounds = bounds.reduced (1, 1);
    auto titleX = panelBounds.getX();
    
    // Title with mixed fonts and glow
    auto titleArea = titleBounds.withX(titleX).toFloat();
    
    juce::AttributedString titleGlow;
    titleGlow.setJustification(juce::Justification::centredLeft);
    titleGlow.append("SampleRealm: ", CustomLookAndFeel::orbitronBold().withPointHeight(28.0f), juce::Colour(0xff00d9ff).withAlpha(0.3f));
    titleGlow.append("Reece", CustomLookAndFeel::orbitronRegular().withPointHeight(28.0f), juce::Colour(0xffff006e).withAlpha(0.3f));
    titleGlow.draw(g, titleArea.translated(0, 1));
    
    // Main title
    juce::AttributedString titleText;
    titleText.setJustification(juce::Justification::centredLeft);
    titleText.append("SampleRealm: ", CustomLookAndFeel::orbitronBold().withPointHeight(28.0f), juce::Colour(0xff00d9ff));
    titleText.append("Reece", CustomLookAndFeel::orbitronRegular().withPointHeight(28.0f), juce::Colour(0xffff006e));
    titleText.draw(g, titleArea);

    // Main panel with depth
    
    // Outer glow
    g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.08f));
    g.fillRoundedRectangle(panelBounds.toFloat().expanded(2.0f), 14.0f);
    
    // Panel background with gradient
    juce::ColourGradient panelGradient(juce::Colour(0xff1a1f2e).withAlpha(0.6f),
                                       panelBounds.getX(), panelBounds.getY(),
                                       juce::Colour(0xff0f1419).withAlpha(0.8f),
                                       panelBounds.getX(), panelBounds.getBottom(),
                                       false);
    g.setGradientFill(panelGradient);
    g.fillRoundedRectangle(panelBounds.toFloat(), 12.0f);
    
    // Panel border with gradient
    juce::ColourGradient borderGradient(juce::Colour(0xff2a3f5f),
                                        panelBounds.getX(), panelBounds.getY(),
                                        juce::Colour(0xff1a2f4f),
                                        panelBounds.getX(), panelBounds.getBottom(),
                                        false);
    g.setGradientFill(borderGradient);
    g.drawRoundedRectangle(panelBounds.toFloat(), 12.0f, 2.0f);

    // Inner highlight
    g.setColour(juce::Colour(0xff3a4f6f).withAlpha(0.3f));
    g.drawRoundedRectangle(panelBounds.toFloat().reduced(2.0f), 10.0f, 1.0f);

    // Section dividers
    auto dividerArea = panelBounds.reduced(20, 15);
    const auto rowHeight = dividerArea.getHeight() / 3;
    
    for (int i = 1; i < 3; ++i)
    {
        auto y = dividerArea.getY() + (i * rowHeight);
        
        // Divider glow
        g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.1f));
        g.drawLine(dividerArea.getX(), y, dividerArea.getRight(), y, 2.0f);
        
        // Divider line
        g.setColour(juce::Colour(0xff2a3f5f).withAlpha(0.5f));
        g.drawLine(dividerArea.getX(), y, dividerArea.getRight(), y, 1.0f);
    }

    // Corner accents
    auto drawCornerAccent = [&](float x, float y, bool flipX, bool flipY)
    {
        juce::Path accent;
        accent.startNewSubPath(0, 15);
        accent.lineTo(0, 0);
        accent.lineTo(15, 0);
        
        auto transform = juce::AffineTransform::translation(x, y);
        if (flipX) transform = transform.scaled(-1, 1, x, y);
        if (flipY) transform = transform.scaled(1, -1, x, y);
        
        accent.applyTransform(transform);
        
        g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.4f));
        g.strokePath(accent, juce::PathStrokeType(2.0f));
    };
    
    auto cornerInset = 25.0f;
    drawCornerAccent(panelBounds.getX() + cornerInset, panelBounds.getY() + cornerInset, false, false);
    drawCornerAccent(panelBounds.getRight() - cornerInset, panelBounds.getY() + cornerInset, true, false);
    drawCornerAccent(panelBounds.getX() + cornerInset, panelBounds.getBottom() - cornerInset, false, true);
    drawCornerAccent(panelBounds.getRight() - cornerInset, panelBounds.getBottom() - cornerInset, true, true);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);
    bounds.removeFromTop (30);

    auto panelArea = bounds.reduced (8, 2);
    auto area = panelArea.reduced (12);

    constexpr int columns = 4;
    constexpr int rows = 3;
    const auto rowHeight = area.getHeight() / rows;
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
