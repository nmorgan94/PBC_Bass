#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CustomLookAndFeel.h"

//==============================================================================
class PeakMeter final : public juce::Component
{
public:
    PeakMeter() = default;
    
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        
        // Peak meter background
        g.setColour(juce::Colour(0xff1a1f2e).withAlpha(0.8f));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        // Peak meter border
        g.setColour(juce::Colour(0xff2a3f5f));
        g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
        
        // Peak meter fill
        auto meterFillArea = bounds.reduced(3);
        const float meterHeight = (float) meterFillArea.getHeight();
        const float fillHeight = peakLevel * meterHeight;
        
        if (fillHeight > 0.0f)
        {
            auto fillRect = meterFillArea.removeFromBottom((int)fillHeight).toFloat();
            
            // Gradient from cyan to magenta to red
            juce::ColourGradient meterGradient(
                juce::Colour(0xffff006e), fillRect.getX(), fillRect.getBottom(),
                juce::Colour(0xff00d9ff), fillRect.getX(), fillRect.getY(),
                false);
            
            // Add red zone at top
            if (peakLevel > 0.85f)
                meterGradient.addColour(0.15, juce::Colour(0xffff0000));
            
            g.setGradientFill(meterGradient);
            g.fillRoundedRectangle(fillRect, 2.0f);
            
            // Glow effect
            g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.3f));
            g.fillRoundedRectangle(fillRect.expanded(1.0f), 2.0f);
        }
        
        // Clip indicator - thin rectangle at top
        auto clipArea = bounds.removeFromTop(3).toFloat();
        
        if (isClipping)
        {
            // Red glow when clipping
            g.setColour(juce::Colour(0xffff0000).withAlpha(0.4f));
            g.fillRoundedRectangle(clipArea.expanded(0.0f, 1.0f), 1.0f);
            
            // Bright red bar
            g.setColour(juce::Colour(0xffff0000));
            g.fillRoundedRectangle(clipArea, 1.0f);
        }
        else
        {
            // Dim when not clipping
            g.setColour(juce::Colour(0xff3a0000).withAlpha(0.5f));
            g.fillRoundedRectangle(clipArea, 1.0f);
        }
    }
    
    void setPeakLevel (float level)
    {
        peakLevel = level;
        repaint();
    }
    
    void setClipping (bool clipping)
    {
        isClipping = clipping;
        repaint();
    }

private:
    float peakLevel { 0.0f };
    bool isClipping { false };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakMeter)
};
