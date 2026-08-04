/**
 * @file CustomLookAndFeel.cpp
 * @brief Implements the custom JUCE LookAndFeel used throughout the
 *        synthesiser interface.
 *
 * The interface adopts a hardware-inspired aesthetic using procedural
 * drawing techniques rather than bitmap assets. This approach provides a
 * consistent visual appearance across platforms while allowing controls to
 * scale without dependence on external image resources.
 */

#include "CustomLookAndFeel.h"

 /**
  * Configures the shared colour palette used by the application's controls.
  *
  * Centralising these values ensures consistency across sliders, labels,
  * buttons, and combo boxes while simplifying subsequent theme changes.
  */
CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::lightgrey);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

    setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

    setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(20, 24, 28));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(35, 45, 50));
    setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    setColour(juce::TextButton::textColourOnId, juce::Colours::cyan);

    setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(18, 24, 28));
    setColour(juce::ComboBox::outlineColourId, juce::Colour::fromRGB(90, 105, 110));
    setColour(juce::ComboBox::textColourId, juce::Colours::lightgrey);
    setColour(juce::ComboBox::arrowColourId, juce::Colours::lightgrey);
}

/**
 * Renders a custom rotary control inspired by analogue synthesiser hardware.
 *
 * The knob is constructed procedurally using gradients, shadows, calibration
 * marks, and a rotating pointer, removing the need for external image assets.
 */
void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y,
    int width, int height,
    float sliderPos,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    auto area = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height)
        .reduced(5.0f);

    auto radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.5f;
    auto centre = area.getCentre();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    auto knobBounds = juce::Rectangle<float>(
        centre.x - radius,
        centre.y - radius,
        radius * 2.0f,
        radius * 2.0f
    );

    // Draw evenly spaced calibration marks around the control to provide
    // visual feedback across the available parameter range.
    auto tickRadiusOuter = radius + 7.0f;
    auto tickRadiusInner = radius + 3.0f;

    g.setColour(juce::Colours::cyan.withAlpha(0.75f));

    constexpr int numTicks = 21;
    for (int i = 0; i < numTicks; ++i)
    {
        auto t = (float)i / (float)(numTicks - 1);
        auto tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);

        auto p1 = centre + juce::Point<float>(
            std::cos(tickAngle - juce::MathConstants<float>::halfPi) * tickRadiusInner,
            std::sin(tickAngle - juce::MathConstants<float>::halfPi) * tickRadiusInner
        );

        auto p2 = centre + juce::Point<float>(
            std::cos(tickAngle - juce::MathConstants<float>::halfPi) * tickRadiusOuter,
            std::sin(tickAngle - juce::MathConstants<float>::halfPi) * tickRadiusOuter
        );

        g.drawLine({ p1, p2 }, i % 5 == 0 ? 1.6f : 1.0f);
    }

    // Render a soft shadow beneath the knob to increase depth perception.
    g.setColour(juce::Colours::black.withAlpha(0.70f));
    g.fillEllipse(knobBounds.translated(3.0f, 5.0f));

    // Simulate a metallic outer ring using a vertical gradient.
    juce::ColourGradient outerGradient(
        juce::Colour::fromRGB(230, 230, 225),
        knobBounds.getCentreX(),
        knobBounds.getY(),
        juce::Colour::fromRGB(55, 55, 58),
        knobBounds.getCentreX(),
        knobBounds.getBottom(),
        false
    );

    g.setGradientFill(outerGradient);
    g.fillEllipse(knobBounds);

    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.drawEllipse(knobBounds, 1.5f);

    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.drawEllipse(knobBounds.reduced(2.0f), 1.0f);

    // Draw the recessed centre section to reinforce the hardware aesthetic.
    auto innerBounds = knobBounds.reduced(radius * 0.23f);

    juce::ColourGradient innerGradient(
        juce::Colour::fromRGB(48, 48, 50),
        innerBounds.getCentreX(),
        innerBounds.getY(),
        juce::Colour::fromRGB(5, 5, 7),
        innerBounds.getCentreX(),
        innerBounds.getBottom(),
        false
    );

    g.setGradientFill(innerGradient);
    g.fillEllipse(innerBounds);

    g.setColour(juce::Colours::white.withAlpha(0.18f));
    g.drawEllipse(innerBounds.reduced(1.0f), 1.0f);

    // Rotate the position indicator according to the current slider value.
    juce::Path pointer;

    auto pointerLength = radius * 0.72f;
    auto pointerThickness = 3.0f;

    pointer.addRoundedRectangle(
        -pointerThickness * 0.5f,
        -radius + 7.0f,
        pointerThickness,
        pointerLength,
        1.5f
    );

    pointer.applyTransform(
        juce::AffineTransform::rotation(angle).translated(centre.x, centre.y)
    );

    g.setColour(juce::Colours::cyan.withAlpha(0.95f));
    g.fillPath(pointer);

    g.setColour(juce::Colours::cyan.withAlpha(0.35f));
    g.strokePath(pointer, juce::PathStrokeType(2.0f));
}

/**
 * Draws the custom vertical fader used for envelope controls.
 *
 * Slider styles other than LinearVertical fall back to JUCE's default
 * implementation so that only the controls required by this interface are
 * overridden.
 */
void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g,
    int x, int y,
    int width, int height,
    float sliderPos,
    float minSliderPos,
    float maxSliderPos,
    const juce::Slider::SliderStyle style,
    juce::Slider& slider)
{
    if (style != juce::Slider::LinearVertical)
    {
        juce::LookAndFeel_V4::drawLinearSlider(g,
            x, y,
            width, height,
            sliderPos,
            minSliderPos,
            maxSliderPos,
            style,
            slider);
        return;
    }

    auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);

    const float trackWidth = 9.0f;
    const float trackX = bounds.getCentreX() - trackWidth * 0.5f;
    const float trackY = bounds.getY() + 10.0f;
    const float trackH = bounds.getHeight() - 22.0f;

    juce::Rectangle<float> slot(trackX, trackY, trackWidth, trackH);

    // Draw a recessed background to create the impression of a physical slot.
    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.fillRoundedRectangle(slot.expanded(2.0f, 2.0f), 5.0f);

    // Render the main fader track using a subtle vertical gradient.
    juce::ColourGradient slotGradient(
        juce::Colour::fromRGB(5, 8, 10),
        slot.getCentreX(),
        slot.getY(),
        juce::Colour::fromRGB(20, 32, 36),
        slot.getCentreX(),
        slot.getBottom(),
        false
    );

    g.setGradientFill(slotGradient);
    g.fillRoundedRectangle(slot, 4.5f);

    g.setColour(juce::Colour::fromRGB(35, 65, 70).withAlpha(0.8f));
    g.drawRoundedRectangle(slot, 4.5f, 1.0f);

    // Draw evenly spaced reference marks to improve visual estimation of
    // parameter position.
    g.setColour(juce::Colours::lightgrey.withAlpha(0.35f));

    constexpr int numTicks = 11;
    for (int i = 0; i < numTicks; ++i)
    {
        float tickY = trackY + ((float)i / (float)(numTicks - 1)) * trackH;

        float major = (i % 5 == 0) ? 14.0f : 9.0f;
        float thickness = (i % 5 == 0) ? 1.3f : 1.0f;

        g.drawLine(trackX - major, tickY, trackX - 4.0f, tickY, thickness);
        g.drawLine(trackX + trackWidth + 4.0f, tickY,
            trackX + trackWidth + major, tickY, thickness);
    }

    // Position the movable fader cap according to the current slider value.
    const float capW = juce::jmin(46.0f, bounds.getWidth() - 4.0f);
    const float capH = 22.0f;
    const float capX = bounds.getCentreX() - capW * 0.5f;
    const float capY = juce::jlimit(trackY - capH * 0.25f,
        trackY + trackH - capH * 0.75f,
        sliderPos - capH * 0.5f);

    juce::Rectangle<float> cap(capX, capY, capW, capH);

    // Add depth beneath the fader cap using a translated shadow.
    g.setColour(juce::Colours::black.withAlpha(0.75f));
    g.fillRoundedRectangle(cap.translated(3.0f, 4.0f), 3.5f);

    // Simulate a metallic fader cap using gradient shading.
    juce::ColourGradient capGradient(
        juce::Colour::fromRGB(215, 215, 205),
        cap.getCentreX(),
        cap.getY(),
        juce::Colour::fromRGB(55, 55, 58),
        cap.getCentreX(),
        cap.getBottom(),
        false
    );

    g.setGradientFill(capGradient);
    g.fillRoundedRectangle(cap, 3.5f);

    // Add a darker lower edge to reinforce the three-dimensional appearance.
    auto lowerLip = cap.withTrimmedTop(capH * 0.55f);
    g.setColour(juce::Colour::fromRGB(12, 12, 14).withAlpha(0.85f));
    g.fillRoundedRectangle(lowerLip, 3.0f);

    // Draw the outer edge of the fader cap.
    g.setColour(juce::Colours::black.withAlpha(0.90f));
    g.drawRoundedRectangle(cap, 3.5f, 1.2f);

    // Highlight the centre of the cap with the application's accent colour.
    auto cyanStrip = juce::Rectangle<float>(
        cap.getX() + 8.0f,
        cap.getCentreY() - 2.0f,
        cap.getWidth() - 16.0f,
        4.0f
    );

    g.setColour(juce::Colours::cyan.withAlpha(0.95f));
    g.fillRoundedRectangle(cyanStrip, 2.0f);

    g.setColour(juce::Colours::cyan.withAlpha(0.35f));
    g.fillRoundedRectangle(cyanStrip.expanded(1.0f), 2.0f);

    // Add a subtle specular highlight to enhance the metallic finish.
    g.setColour(juce::Colours::white.withAlpha(0.30f));
    g.drawLine(cap.getX() + 4.0f,
        cap.getY() + 3.0f,
        cap.getRight() - 4.0f,
        cap.getY() + 3.0f,
        1.0f);
}

/**
 * Draws the custom toggle button used by the synthesiser interface.
 *
 * A simulated status LED provides immediate visual feedback indicating
 * whether the associated control is active.
 */
void CustomLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();

    auto buttonBg = bounds.reduced(1.0f);

    juce::ColourGradient bg(
        juce::Colour::fromRGB(30, 34, 38),
        buttonBg.getCentreX(),
        buttonBg.getY(),
        juce::Colour::fromRGB(8, 10, 12),
        buttonBg.getCentreX(),
        buttonBg.getBottom(),
        false
    );

    g.setGradientFill(bg);
    g.fillRoundedRectangle(buttonBg, 5.0f);

    g.setColour(shouldDrawButtonAsHighlighted
        ? juce::Colours::cyan.withAlpha(0.4f)
        : juce::Colours::black.withAlpha(0.75f));

    g.drawRoundedRectangle(buttonBg, 5.0f, 1.2f);

    // Draw the recessed LED housing.
    const float ledRadius = 6.0f;
    const float ledX = 11.0f;
    const float ledY = bounds.getCentreY() - ledRadius;

    g.setColour(juce::Colour::fromRGB(2, 18, 20));
    g.fillEllipse(ledX, ledY, ledRadius * 2.0f, ledRadius * 2.0f);

    // Illuminate the indicator when the toggle is enabled.
    if (button.getToggleState())
    {
        g.setColour(juce::Colours::cyan.withAlpha(0.35f));
        g.fillEllipse(ledX - 5.0f,
            ledY - 5.0f,
            ledRadius * 2.0f + 10.0f,
            ledRadius * 2.0f + 10.0f);

        g.setColour(juce::Colours::cyan);
        g.fillEllipse(ledX, ledY, ledRadius * 2.0f, ledRadius * 2.0f);
    }

    g.setColour(juce::Colours::white.withAlpha(0.55f));
    g.fillEllipse(ledX + 2.5f, ledY + 2.0f, 3.5f, 3.5f);

    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::FontOptions(13.5f, juce::Font::bold));

    g.drawText(button.getButtonText().toUpperCase(),
        32,
        0,
        button.getWidth() - 34,
        button.getHeight(),
        juce::Justification::centredLeft,
        true);
}

/**
 * Renders the shared background used by push buttons.
 *
 * The gradient direction is inverted while the button is pressed to reinforce
 * the visual impression of physical button travel.
 */
void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

    auto top = shouldDrawButtonAsDown
        ? juce::Colour::fromRGB(12, 15, 18)
        : juce::Colour::fromRGB(35, 40, 44);

    auto bottom = shouldDrawButtonAsDown
        ? juce::Colour::fromRGB(35, 40, 44)
        : juce::Colour::fromRGB(8, 10, 12);

    juce::ColourGradient bg(
        top,
        bounds.getCentreX(),
        bounds.getY(),
        bottom,
        bounds.getCentreX(),
        bounds.getBottom(),
        false
    );

    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(shouldDrawButtonAsHighlighted
        ? juce::Colours::cyan.withAlpha(0.45f)
        : juce::Colours::black.withAlpha(0.8f));

    g.drawRoundedRectangle(bounds, 5.0f, 1.2f);

    g.setColour(juce::Colours::white.withAlpha(shouldDrawButtonAsDown ? 0.08f : 0.18f));
    g.drawLine(bounds.getX() + 4.0f,
        bounds.getY() + 2.0f,
        bounds.getRight() - 4.0f,
        bounds.getY() + 2.0f,
        1.0f);
}

/**
 * Draws the customised combo box used for preset, task, and MIDI selection.
 *
 * The default JUCE appearance is replaced with styling consistent with the
 * remainder of the synthesiser interface.
 */
void CustomLookAndFeel::drawComboBox(juce::Graphics& g,
    int width,
    int height,
    bool isButtonDown,
    int buttonX,
    int buttonY,
    int buttonW,
    int buttonH,
    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height)
        .reduced(1.0f);

    juce::ColourGradient bg(
        juce::Colour::fromRGB(30, 36, 40),
        bounds.getCentreX(),
        bounds.getY(),
        juce::Colour::fromRGB(8, 10, 12),
        bounds.getCentreX(),
        bounds.getBottom(),
        false
    );

    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(isButtonDown ? juce::Colours::cyan.withAlpha(0.55f)
        : juce::Colour::fromRGB(80, 95, 100));

    g.drawRoundedRectangle(bounds, 5.0f, 1.2f);

    // Draw the dropdown indicator using a simple vector path.
    juce::Path arrow;
    auto arrowArea = juce::Rectangle<float>((float)buttonX, (float)buttonY, (float)buttonW, (float)buttonH)
        .reduced(6.0f);

    auto cx = arrowArea.getCentreX();
    auto cy = arrowArea.getCentreY();

    arrow.startNewSubPath(cx - 5.0f, cy - 3.0f);
    arrow.lineTo(cx, cy + 3.0f);
    arrow.lineTo(cx + 5.0f, cy - 3.0f);

    g.setColour(juce::Colours::lightgrey);
    g.strokePath(arrow, juce::PathStrokeType(2.0f));
}

/**
 * Draws labels using the application's shared typography while preserving
 * JUCE's default rendering whenever a label enters text-edit mode.
 */
void CustomLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    if (label.isBeingEdited())
    {
        juce::LookAndFeel_V4::drawLabel(g, label);
        return;
    }

    auto alpha = label.isEnabled() ? 1.0f : 0.5f;
    auto area = label.getLocalBounds();

    g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
    g.setFont(getLabelFont(label));

    g.drawFittedText(label.getText(),
        area,
        label.getJustificationType(),
        1,
        label.getMinimumHorizontalScale());
}