#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../ui/CustomLookAndFeel.h"

class TransportDisplay : public juce::Component
{
public:
    TransportDisplay() 
    { 
        setOpaque(false); 
    }

    void paint(juce::Graphics& g) override
    {
        g.setFont(CustomLookAndFeel::orbitronRegular().withPointHeight(9.0f));
        g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.4f));
        
        auto text = (bpm > 0.0) ? juce::String(bpm, 1) + " BPM" : "-- BPM";
        g.drawText(text, getLocalBounds(), juce::Justification::centredLeft);
    }

    void setBPM(double newBpm)
    {
        constexpr double epsilon = 0.01;  // Epsilon for floating point comparison
        
        if (std::abs(bpm - newBpm) > epsilon)
        {
            bpm = newBpm;
            repaint();
        }
    }

private:
    double bpm = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportDisplay)
};
