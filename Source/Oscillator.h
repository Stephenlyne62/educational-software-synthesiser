#pragma once

#include <JuceHeader.h>

class Oscillator
{
public:
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateAngleDelta();
    }

    void setFrequency(double newFrequency)
    {
        frequency = newFrequency;
        updateAngleDelta();
    }

    void setWaveform(int newWaveform)
    {
        waveform = newWaveform;
    }

    float getNextSample()
    {
        float oscillatorValue = 0.0f;

        auto phase = currentAngle / (2.0 * juce::MathConstants<double>::pi);

        while (phase >= 1.0)
            phase -= 1.0;

        switch (waveform)
        {
        case 1:
            oscillatorValue = (float)std::sin(currentAngle);
            break;

        case 2:
            oscillatorValue = phase < 0.5 ? 1.0f : -1.0f;
            break;

        case 3:
            oscillatorValue = (float)((2.0 * phase) - 1.0);
            break;

        case 4:
            oscillatorValue = (float)(4.0 * std::abs(phase - 0.5) - 1.0);
            break;

        default:
            oscillatorValue = (float)std::sin(currentAngle);
            break;
        }

        currentAngle += angleDelta;

        return oscillatorValue;
    }

private:
    double sampleRate = 44100.0;
    double frequency = 440.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;

    int waveform = 1;

    void updateAngleDelta()
    {
        angleDelta = frequency * 2.0 * juce::MathConstants<double>::pi / sampleRate;
    }
};