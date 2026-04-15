#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class ADSRVisualizer : public juce::Component,
                       private juce::Timer
{
public:
    ADSRVisualizer(juce::AudioProcessorValueTreeState& apvtsToUse)
        : apvts(apvtsToUse)
    {
        startTimerHz(30); 
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient bgGradient(
            juce::Colour(0xff0f1419).withAlpha(0.3f), bounds.getX(), bounds.getY(),
            juce::Colour(0xff1a1f2e).withAlpha(0.2f), bounds.getX(), bounds.getBottom(),
            false);
        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(bounds, 8.0f);
        
        float attack = getParameterValue("attack");
        float decay = getParameterValue("decay");
        float sustain = getParameterValue("sustain");
        float release = getParameterValue("release");
        
        drawEnvelope(g, bounds.reduced(10.0f), attack, decay, sustain, release);
    }

private:
    void timerCallback() override
    {
        repaint();
    }

    float getParameterValue(const juce::String& paramID) const
    {
        if (auto* param = apvts.getRawParameterValue(paramID))
            return param->load();
        return 0.0f;
    }

    void drawEnvelope(juce::Graphics& g, juce::Rectangle<float> bounds, 
                     float attack, float decay, float sustain, float release)
    {
        juce::Path envelopePath;
        
        const float width = bounds.getWidth();
        const float height = bounds.getHeight();
        const float startX = bounds.getX();
        const float startY = bounds.getBottom();
        const float sustainDuration = 0.3f; // Fixed sustain duration for display
        
        // Normalize time values for visualization
        const float totalTime = attack + decay + sustainDuration + release; 
        const float timeScale = width / totalTime;
        
        // Calculate x positions
        const float attackX = attack * timeScale;
        const float decayX = attackX + (decay * timeScale);
        const float sustainX = decayX + (sustainDuration * timeScale);
        const float releaseX = sustainX + (release * timeScale);
        
        envelopePath.startNewSubPath(startX, startY);
        
        // Attack phase - curve up to peak
        const float peakY = startY - height;
        envelopePath.lineTo(startX + attackX, peakY);
        
        // Decay phase - curve down to sustain level
        const float sustainY = startY - (height * sustain);
        envelopePath.lineTo(startX + decayX, sustainY);
        
        // Sustain phase - flat line
        envelopePath.lineTo(startX + sustainX, sustainY);
        
        // Release phase - curve down to zero
        envelopePath.lineTo(startX + releaseX, startY);
        
        // Draw filled area under the curve
        juce::Path fillPath = envelopePath;
        fillPath.closeSubPath(); 
        
        juce::ColourGradient fillGradient(
            juce::Colour(0xff00d9ff).withAlpha(0.15f), startX, peakY,
            juce::Colour(0xff00d9ff).withAlpha(0.05f), startX, startY,
            false);
        g.setGradientFill(fillGradient);
        g.fillPath(fillPath);
        
        // Draw the envelope line with glow effect
        g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.3f));
        g.strokePath(envelopePath, juce::PathStrokeType(3.0f));
        
        g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.8f));
        g.strokePath(envelopePath, juce::PathStrokeType(1.5f));
    }

    juce::AudioProcessorValueTreeState& apvts;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRVisualizer)
};
