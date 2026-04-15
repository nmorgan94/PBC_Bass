#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CustomLookAndFeel.h"

class TriangleSelectorLookAndFeel : public CustomLookAndFeel
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider& slider) override
    {
        juce::ignoreUnused(sliderPosProportional, rotaryStartAngle, rotaryEndAngle);
        
        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(10.0f);
        auto centre = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        
        const int selectedValue = (int)slider.getValue();
        
        // Calculate rotation angle based on selected value (0, 1, 2)
        // Value 0 = -120 degrees, Value 1 = 0 degrees, Value 2 = 120 degrees
        float angle = (selectedValue - 1) * (juce::MathConstants<float>::pi * 2.0f / 3.0f);
        
        juce::Path triangle;
        float triangleSize = radius * 0.8f;
        triangle.addTriangle(
            0.0f, -triangleSize,           // Top point
            -triangleSize * 0.6f, triangleSize * 0.5f,   // Bottom left
            triangleSize * 0.6f, triangleSize * 0.5f     // Bottom right
        );
        
        // Apply rotation and translation
        triangle.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
        
        // Draw triangle with glow effect
        g.setColour(juce::Colour(CYAN).withAlpha(0.3f));
        g.fillPath(triangle);
        
        g.setColour(juce::Colour(CYAN));
        g.strokePath(triangle, juce::PathStrokeType(2.0f));
        
        // Draw subtle circle outline to show the rotation area
        g.setColour(juce::Colour(BORDER_BLUE).withAlpha(0.3f));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);
        
        // Center dot
        g.setColour(juce::Colour(CYAN));
        g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
    }
};
