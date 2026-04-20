#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr juce::uint32 CYAN = 0xff00d9ff;
    static constexpr juce::uint32 MAGENTA = 0xffff006e;
    static constexpr juce::uint32 LIGHT_BLUE = 0xffa0b0ff;
    static constexpr juce::uint32 DARK_BLUE_1 = 0xff1a1f2e;
    static constexpr juce::uint32 DARK_BLUE_2 = 0xff0f1419;
    static constexpr juce::uint32 BORDER_BLUE = 0xff2a3f5f;
    static constexpr juce::uint32 KNOB_OUTER = 0xff3f2a1f;
    static constexpr juce::uint32 KNOB_LIGHT = 0xff6f4a3a;
    static constexpr juce::uint32 KNOB_DARK = 0xff4f2a1a;
    static constexpr juce::uint32 KNOB_HIGHLIGHT = 0xff9f7a5a;

    CustomLookAndFeel()
    {
        setColour(juce::Label::textColourId, juce::Colour(LIGHT_BLUE));
        
        setColour(juce::ComboBox::textColourId, juce::Colour(LIGHT_BLUE));
        setColour(juce::ComboBox::arrowColourId, juce::Colour(CYAN));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(DARK_BLUE_1));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(BORDER_BLUE));
        
        setColour(juce::TextButton::buttonColourId, juce::Colour(DARK_BLUE_1));
        setColour(juce::TextButton::textColourOffId, juce::Colour(CYAN));
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

            juce::ColourGradient gradient(juce::Colour(CYAN), centre.x - arcRadius, centre.y,
                                         juce::Colour(MAGENTA), centre.x + arcRadius, centre.y,
                                         false);
            g.setGradientFill(gradient);
            g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));

            // Add glow effect to value arc
            g.setColour(juce::Colour(CYAN).withAlpha(0.3f));
            g.strokePath(valueArc, juce::PathStrokeType(lineW + 0.5f, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }

        auto knobRadius = radius * 0.8f;
        
        // Outer ring
        g.setColour(juce::Colour(BORDER_BLUE));
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
        g.setColour(juce::Colour(CYAN).withAlpha(0.5f));
        g.fillPath(pointer);
        
        // Pointer solid
        g.setColour(juce::Colour(CYAN));
        pointer = juce::Path();
        pointer.addRectangle(-pointerThickness * 0.3f, -pointerLength, pointerThickness * 0.6f, pointerLength * 0.6f);
        pointer.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
        g.fillPath(pointer);

        // Center dot
        g.setColour(juce::Colour(CYAN));
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

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                     int, int, int, int, juce::ComboBox& box) override
    {
        auto cornerSize = 4.0f;
        juce::Rectangle<int> boxBounds(0, 0, width, height);

        // Background with gradient
        juce::ColourGradient bgGradient(juce::Colour(DARK_BLUE_1), 0, 0,
                                        juce::Colour(DARK_BLUE_2), 0, (float)height,
                                        false);
        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

        // Border
        g.setColour(juce::Colour(BORDER_BLUE));
        g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

        // Arrow area
        juce::Rectangle<int> arrowZone(width - 20, 0, 20, height);
        juce::Path path;
        path.startNewSubPath((float)arrowZone.getX() + 6.0f, (float)arrowZone.getCentreY() - 2.0f);
        path.lineTo((float)arrowZone.getCentreX(), (float)arrowZone.getCentreY() + 3.0f);
        path.lineTo((float)arrowZone.getRight() - 6.0f, (float)arrowZone.getCentreY() - 2.0f);

        g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(box.isEnabled() ? 0.9f : 0.2f));
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        // Popup background
        juce::ColourGradient bgGradient(juce::Colour(DARK_BLUE_1), 0, 0,
                                        juce::Colour(DARK_BLUE_2), 0, (float)height,
                                        false);
        g.setGradientFill(bgGradient);
        g.fillAll();

        // Border
        g.setColour(juce::Colour(BORDER_BLUE));
        g.drawRect(0, 0, width, height, 1);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                          bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                          const juce::Drawable* icon, const juce::Colour*) override
    {
        if (isSeparator)
        {
            auto r = area.reduced(5, 0);
            r.removeFromTop(juce::roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));

            g.setColour(juce::Colour(BORDER_BLUE).withAlpha(0.3f));
            g.fillRect(r.removeFromTop(1));
        }
        else
        {
            auto textColour = juce::Colour(LIGHT_BLUE);

            auto r = area.reduced(1);

            if (isHighlighted && isActive)
            {
                g.setColour(juce::Colour(CYAN).withAlpha(0.2f));
                g.fillRect(r);

                g.setColour(juce::Colour(CYAN).withAlpha(0.5f));
                g.drawRect(r, 1);

                textColour = juce::Colour(CYAN);
            }

            r.reduce(juce::jmin(5, area.getWidth() / 20), 0);

            auto font = getPopupMenuFont();

            auto maxFontHeight = (float)r.getHeight() / 1.3f;

            if (font.getHeight() > maxFontHeight)
                font.setHeight(maxFontHeight);

            g.setFont(font);

            auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight)).toFloat();

            if (icon != nullptr)
            {
                icon->drawWithin(g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
                r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
            }
            else if (isTicked)
            {
                auto tick = getTickShape(1.0f);
                g.setColour(textColour);
                g.fillPath(tick, tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 5, 0).toFloat(), true));
            }

            if (hasSubMenu)
            {
                auto arrowH = 0.6f * getPopupMenuFont().getAscent();

                auto x = static_cast<float>(r.removeFromRight((int)arrowH).getX());
                auto halfH = static_cast<float>(r.getCentreY());

                juce::Path path;
                path.startNewSubPath(x, halfH - arrowH * 0.5f);
                path.lineTo(x + arrowH * 0.6f, halfH);
                path.lineTo(x, halfH + arrowH * 0.5f);

                g.setColour(textColour);
                g.strokePath(path, juce::PathStrokeType(2.0f));
            }

            r.removeFromRight(3);
            g.setColour(textColour);

            g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

            if (shortcutKeyText.isNotEmpty())
            {
                auto f2 = font;
                f2.setHeight(f2.getHeight() * 0.75f);
                f2.setHorizontalScale(0.95f);
                g.setFont(f2);

                g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
            }
        }
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(orbitronRegular().withHeight(14.0f));
    }

    juce::Font getPopupMenuFont() override
    {
        return juce::Font(orbitronRegular().withHeight(14.0f));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = 3.0f;
        const bool isOn = button.getToggleState();
        
        // Background gradient
        if (isOn)
        {
            juce::ColourGradient gradient(juce::Colour(CYAN).withAlpha(0.3f), 0, 0,
                                         juce::Colour(CYAN).withAlpha(0.15f), 0, bounds.getHeight(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, cornerSize);
            
            // Border
            g.setColour(juce::Colour(CYAN));
            g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
        }
        else
        {
            juce::ColourGradient gradient(juce::Colour(DARK_BLUE_1), 0, 0,
                                         juce::Colour(DARK_BLUE_2), 0, bounds.getHeight(), false);
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(bounds, cornerSize);
            
            // Border
            g.setColour(juce::Colour(BORDER_BLUE));
            g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
        }
        
        // Highlight on hover/down
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
        {
            g.setColour(juce::Colour(CYAN).withAlpha(0.1f));
            g.fillRoundedRectangle(bounds, cornerSize);
        }
        
        // Text
        g.setFont(orbitronBold().withHeight(9.0f));
        g.setColour(isOn ? juce::Colour(CYAN) : juce::Colour(LIGHT_BLUE).withAlpha(0.6f));
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
    }

};
