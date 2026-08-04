/**
 * @file CustomLookAndFeel.h
 * @brief Declares the custom JUCE LookAndFeel used by the synthesiser
 *        interface.
 *
 * The class centralises the visual presentation of rotary controls, vertical
 * faders, buttons, combo boxes, and labels. Separating rendering behaviour
 * from the main application component improves consistency and keeps interface
 * styling independent from synthesis and interaction logic.
 */

#pragma once

#include <JuceHeader.h>

 /**
  * Provides a hardware-inspired visual theme for the synthesiser interface.
  *
  * The class extends juce::LookAndFeel_V4 and overrides only the controls that
  * require application-specific rendering. Each method is responsible solely
  * for drawing; component behaviour and state management remain within the
  * associated JUCE controls.
  */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    /**
     * Initialises the shared colour palette used by the customised controls.
     */
    CustomLookAndFeel();

    /**
     * Draws a procedurally rendered rotary control with calibration marks,
     * metallic shading, and a value-dependent position indicator.
     */
    void drawRotarySlider(juce::Graphics& g,
        int x, int y,
        int width, int height,
        float sliderPos,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;

    /**
     * Draws the custom vertical fader used by the synthesiser envelope
     * controls. Unsupported slider styles are delegated to JUCE's default
     * LookAndFeel implementation.
     */
    void drawLinearSlider(juce::Graphics& g,
        int x, int y,
        int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const juce::Slider::SliderStyle style,
        juce::Slider& slider) override;

    /**
     * Draws a toggle control with a simulated status indicator that reflects
     * the current toggle state.
     */
    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    /**
     * Draws the shared background used by push buttons, including hover and
     * pressed-state feedback.
     */
    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    /**
     * Draws a combo box using the same hardware-inspired visual language as
     * the remaining synthesiser controls.
     */
    void drawComboBox(juce::Graphics& g,
        int width,
        int height,
        bool isButtonDown,
        int buttonX,
        int buttonY,
        int buttonW,
        int buttonH,
        juce::ComboBox& box) override;

    /**
     * Draws labels using the application's shared typography while preserving
     * JUCE's standard behaviour during text editing.
     */
    void drawLabel(juce::Graphics& g,
        juce::Label& label) override;
};