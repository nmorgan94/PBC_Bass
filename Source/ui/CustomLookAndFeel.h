#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setColour(juce::Label::textColourId, juce::Colour(0xffa0b0ff));
    }

    [[nodiscard]] static juce::FontOptions orbitronRegular()
    {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::OrbitronRegular_ttf,
            BinaryData::OrbitronRegular_ttfSize);
        return juce::FontOptions(typeface);
    }
    
    [[nodiscard]] static juce::FontOptions orbitronBold()
    {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::OrbitronBold_ttf,
            BinaryData::OrbitronBold_ttfSize);
        return juce::FontOptions(typeface);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(10.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmin(8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;
        auto centre = bounds.getCentre();

        // Draw value arc with gradient
        if (sliderPosProportional > 0.0f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius,
                                  0.0f, rotaryStartAngle, toAngle, true);

            juce::ColourGradient gradient(juce::Colour(0xff00d9ff), centre.x - arcRadius, centre.y,
                                         juce::Colour(0xffff006e), centre.x + arcRadius, centre.y,
                                         false);
            g.setGradientFill(gradient);
            g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));

            // Add glow effect to value arc
            g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.3f));
            g.strokePath(valueArc, juce::PathStrokeType(lineW + 2.0f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }

        auto knobRadius = radius * 0.4f;
        
        // Outer ring
        g.setColour(juce::Colour(0xff2a3f5f));
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius,
                     knobRadius * 2.0f, knobRadius * 2.0f);

        // Inner gradient
        juce::ColourGradient knobGradient(juce::Colour(0xff3a4f6f), centre.x, centre.y - knobRadius,
                                         juce::Colour(0xff1a2f4f), centre.x, centre.y + knobRadius,
                                         false);
        g.setGradientFill(knobGradient);
        g.fillEllipse(centre.x - knobRadius * 0.9f, centre.y - knobRadius * 0.9f,
                     knobRadius * 1.8f, knobRadius * 1.8f);

        // Highlight
        g.setColour(juce::Colour(0xff5a7f9f).withAlpha(0.6f));
        g.fillEllipse(centre.x - knobRadius * 0.6f, centre.y - knobRadius * 0.8f,
                     knobRadius * 0.8f, knobRadius * 0.8f);

        // Draw pointer with glow
        juce::Path pointer;
        auto pointerLength = radius * 0.5f;
        auto pointerThickness = 3.0f;
        pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength * 0.6f);
        pointer.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));

        // Pointer glow
        g.setColour(juce::Colour(0xff00d9ff).withAlpha(0.5f));
        g.fillPath(pointer);
        
        // Pointer solid
        g.setColour(juce::Colour(0xff00d9ff));
        pointer = juce::Path();
        pointer.addRectangle(-pointerThickness * 0.3f, -pointerLength, pointerThickness * 0.6f, pointerLength * 0.6f);
        pointer.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
        g.fillPath(pointer);

        // Center dot
        g.setColour(juce::Colour(0xff00d9ff));
        g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll(label.findColour(juce::Label::backgroundColourId));

        if (!label.isBeingEdited())
        {
            auto alpha = label.isEnabled() ? 1.0f : 0.5f;
            auto font = getLabelFont(label);

            g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
            g.setFont(orbitronBold().withHeight(font.getHeight()));

            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

            g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                           juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
                           label.getMinimumHorizontalScale());

            g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
        }
        else if (label.isEnabled())
        {
            g.setColour(label.findColour(juce::Label::outlineColourId));
        }

        g.drawRect(label.getLocalBounds());
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(orbitronBold().withHeight(14.0f));
    }

};
