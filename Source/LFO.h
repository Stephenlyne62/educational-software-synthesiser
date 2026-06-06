#pragma once

#include <JuceHeader.h>

class LFO
{
public:
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateAngleDelta();
    }

    void setRate(double newRate)
    {
        rate = newRate;
        updateAngleDelta();
    }

    float getNextValue()
    {
        auto value = (float)std::sin(currentAngle);

        currentAngle += angleDelta;

        if (currentAngle >= 2.0 * juce::MathConstants<double>::pi)
            currentAngle -= 2.0 * juce::MathConstants<double>::pi;

        return value;
    }

private:
    double sampleRate = 44100.0;
    double rate = 5.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;

    void updateAngleDelta()
    {
        angleDelta = rate * 2.0 * juce::MathConstants<double>::pi / sampleRate;
    }
};