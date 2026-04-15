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
        
        // Center dot
        g.setColour(juce::Colour(CYAN));
        g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
        
        drawWaveformSymbols(g, centre, radius);
    }

private:
    void drawWaveformSymbols(juce::Graphics& g, juce::Point<float> centre, float radius)
    {
        const float symbolRadius = radius * 1.25f;
        const float symbolSize = 8.5f;
        
        g.setColour(juce::Colour(CYAN).withAlpha(0.5f));
        
        // Draw waveform symbols at 120-degree intervals
        // Starting at bottom-left and going clockwise: Square, Saw, Triangle
        const float baseAngle = 5.0f * juce::MathConstants<float>::pi / 6.0f;  // 150° (bottom-left)
        const float angleStep = juce::MathConstants<float>::pi * 2.0f / 3.0f;  // 120°
        
        for (int i = 0; i < 3; ++i)
        {
            float angle = baseAngle + (i * angleStep);
            float x = centre.x + std::cos(angle) * symbolRadius;
            float y = centre.y + std::sin(angle) * symbolRadius;
            
            // i=0: 150° (bottom-left) = Square (value 0)
            // i=1: 270° = -90° (top) = Saw (value 1)
            // i=2: 390° = 30° (bottom-right) = Triangle (value 2)
            if (i == 0)
                drawSquareWave(g, x, y, symbolSize);
            else if (i == 1)
                drawSawWave(g, x, y, symbolSize);
            else
                drawTriangleWave(g, x, y, symbolSize);
        }
    }
    
    void drawSquareWave(juce::Graphics& g, float x, float y, float size)
    {
        juce::Path path;
        float halfSize = size * 0.5f;
        float height = size * 0.4f;
        
        path.startNewSubPath(x - halfSize, y + height);
        path.lineTo(x - halfSize, y - height);
        path.lineTo(x + halfSize, y - height);
        path.lineTo(x + halfSize, y + height);
        
        g.strokePath(path, juce::PathStrokeType(1.2f));
    }
    
    void drawSawWave(juce::Graphics& g, float x, float y, float size)
    {
        juce::Path path;
        float halfSize = size * 0.5f;
        float height = size * 0.4f;
        
        path.startNewSubPath(x - halfSize, y + height);
        path.lineTo(x, y - height);
        path.lineTo(x, y + height);
        path.lineTo(x + halfSize, y - height);
        path.lineTo(x + halfSize, y + height);
        
        g.strokePath(path, juce::PathStrokeType(1.2f));
    }
    
    void drawTriangleWave(juce::Graphics& g, float x, float y, float size)
    {
        juce::Path path;
        float halfSize = size * 0.5f;
        float height = size * 0.5f;
        
        path.startNewSubPath(x - halfSize, y + height);
        path.lineTo(x, y - height);
        path.lineTo(x + halfSize, y + height);
        
        g.strokePath(path, juce::PathStrokeType(1.2f));
    }
};

