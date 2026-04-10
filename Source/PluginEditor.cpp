#include "PluginEditor.h"

namespace UIConstants
{
    constexpr int COMBO_BOX_WIDTH = 200;
    constexpr int BUTTON_SIZE = 24;
    constexpr int SPACING = 4;
}

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), presetManager(p.apvts)
{
    setLookAndFeel(&customLookAndFeel);

    configureSlider (controls[0], "oscAWave", "OSC A");
    configureSlider (controls[1], "oscBWave", "OSC B");
    configureSlider (controls[2], "unisonVoices", "UNISON");
    configureSlider (controls[3], "drive", "DRIVE");
    configureSlider (controls[4], "detune", "DETUNE");
    configureSlider (controls[5], "sub", "SUB");
    configureSlider (controls[6], "cutoff", "CUTOFF");
    configureSlider (controls[7], "resonance", "RESONANCE");
    configureSlider (controls[8], "lfoRate", "LFO RATE");
    configureSlider (controls[9], "lfoDepth", "LFO DEPTH");
    configureSlider (controls[10], "glideTime", "GLIDE");
    configureSlider (controls[11], "output", "OUTPUT");

    presetComboBox.setTextWhenNothingSelected("Select Preset");
    presetComboBox.onChange = [this]()
    {
        auto selectedId = presetComboBox.getSelectedId();
        if (selectedId > 0)
        {
            auto presetName = presetComboBox.getItemText(selectedId - 1);
            presetManager.loadPreset(presetName);
        }
    };
    addAndMakeVisible(presetComboBox);
    
    savePresetButton.setButtonText("+");
    savePresetButton.onClick = [this]() { savePresetClicked(); };
    addAndMakeVisible(savePresetButton);

    deletePresetButton.setButtonText("-");
    deletePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::MAGENTA));
    deletePresetButton.onClick = [this]() { deletePresetClicked(); };
    addAndMakeVisible(deletePresetButton);
    
    addAndMakeVisible(peakMeter);
    
    updatePresetComboBox();
    
    startTimerHz (1000 / TIMER_INTERVAL_MS);

    setSize (720, 420);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    // Update peak level with decay
    const float newPeak = processorRef.getPeakLevel();
    const float decayRate = 0.95f;
    currentPeakLevel = juce::jmax (newPeak, currentPeakLevel * decayRate);
    
    if (processorRef.isClipping())
    {
        clipIndicatorActive = true;
        clipIndicatorTimer = CLIP_HOLD_TIME_MS;
    }
    else if (clipIndicatorTimer > 0)
    {
        clipIndicatorTimer -= TIMER_INTERVAL_MS;
        if (clipIndicatorTimer <= 0)
            clipIndicatorActive = false;
    }
    
    peakMeter.setPeakLevel(currentPeakLevel);
    peakMeter.setClipping(clipIndicatorActive);
}

void AudioPluginAudioProcessorEditor::updateSliderLabel (SliderWithAttachment& sliderControl, const juce::String& paramID)
{
    // Show waveform names for oscillator wave parameters
    if (paramID == "oscAWave" || paramID == "oscBWave")
    {
        const int waveValue = (int)sliderControl.slider.getValue();
        const char* waveNames[] = { "SAW", "SQUARE", "TRI" };
        sliderControl.label.setText(waveNames[waveValue], juce::dontSendNotification);
    }
    else
    {
        int decimals = sliderControl.slider.getNumDecimalPlacesToDisplay();
        sliderControl.label.setText(juce::String(sliderControl.slider.getValue(), decimals), juce::dontSendNotification);
    }
}

void AudioPluginAudioProcessorEditor::configureSlider (SliderWithAttachment& sliderControl,
                                                       const juce::String& paramID,
                                                       const juce::String& labelText)
{
    sliderControl.originalLabelText = labelText;
    
    sliderControl.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    sliderControl.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    sliderControl.slider.setMouseDragSensitivity(150);
    
    // Set up callbacks to track active slider
    sliderControl.slider.onDragStart = [this, &sliderControl, paramID]()
    {
        activeSlider = &sliderControl.slider;
        updateSliderLabel(sliderControl, paramID);
    };
    
    sliderControl.slider.onDragEnd = [this, &sliderControl]()
    {
        activeSlider = nullptr;
        sliderControl.label.setText(sliderControl.originalLabelText, juce::dontSendNotification);
    };
    
    sliderControl.slider.onValueChange = [this, &sliderControl, paramID]()
    {
        if (activeSlider == &sliderControl.slider)
            updateSliderLabel(sliderControl, paramID);
    };
    
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
    
    auto panelBounds = bounds;
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
    auto bounds = getLocalBounds().reduced (3);
    auto titleArea = bounds.removeFromTop(40);
    
    // Position preset controls and peak meter
    auto rightArea = titleArea.removeFromRight(titleArea.getWidth() / 2);
    
    auto meterArea = rightArea.removeFromRight(100).reduced(4, 10);
    peakMeter.setBounds(meterArea);
    
    // Add spacing
    rightArea.removeFromRight(8);
    
    auto presetControlsArea = rightArea;
    
    const int totalWidth = UIConstants::BUTTON_SIZE + UIConstants::SPACING +
                          UIConstants::COMBO_BOX_WIDTH + UIConstants::SPACING +
                          UIConstants::BUTTON_SIZE;
    
    // Center the controls
    auto controlsBounds = presetControlsArea.withSizeKeepingCentre(totalWidth, UIConstants::BUTTON_SIZE);
    
    savePresetButton.setBounds(controlsBounds.removeFromLeft(UIConstants::BUTTON_SIZE));
    controlsBounds.removeFromLeft(UIConstants::SPACING);

    presetComboBox.setBounds(controlsBounds.removeFromLeft(UIConstants::COMBO_BOX_WIDTH));
    controlsBounds.removeFromLeft(UIConstants::SPACING);
    
    deletePresetButton.setBounds(controlsBounds.removeFromLeft(UIConstants::BUTTON_SIZE));

    auto panelArea = bounds;
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


void AudioPluginAudioProcessorEditor::updatePresetComboBox()
{
    presetComboBox.clear();
    
    auto presetList = presetManager.getPresetList();
    
    for (int i = 0; i < presetList.size(); ++i)
    {
        presetComboBox.addItem(presetList[i], i + 1);
    }
}

void AudioPluginAudioProcessorEditor::savePresetClicked()
{
    presetManager.showSaveDialog(this, [this](bool success, juce::String presetName)
    {
        if (success)
        {
            updatePresetComboBox();
            
            // Select the newly saved preset
            for (int i = 0; i < presetComboBox.getNumItems(); ++i)
            {
                if (presetComboBox.getItemText(i) == presetName)
                {
                    presetComboBox.setSelectedId(i + 1, juce::dontSendNotification);
                    break;
                }
            }
        }
    });
}

void AudioPluginAudioProcessorEditor::deletePresetClicked()
{
    auto selectedId = presetComboBox.getSelectedId();
    
    if (selectedId > 0)
    {
        auto presetName = presetComboBox.getItemText(selectedId - 1);
        
        presetManager.showDeleteDialog(presetName, this, [this](bool success)
        {
            if (success)
            {
                updatePresetComboBox();
                presetComboBox.setSelectedId(0, juce::dontSendNotification);
            }
        });
    }
    else
    {
        juce::NativeMessageBox::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                                    "No Preset Selected",
                                                    "Please select a preset to delete.");
    }
}
