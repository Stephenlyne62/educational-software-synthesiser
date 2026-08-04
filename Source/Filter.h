#pragma once

#include <cmath>
#include <algorithm>

class Filter
{
public:

    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateCoefficients();
    }

    void setCutoffHz(float newCutoffHz)
    {
        if (newCutoffHz < 20.0f)   newCutoffHz = 20.0f;
        if (newCutoffHz > 18000.0f) newCutoffHz = 18000.0f;
        cutoffHz = newCutoffHz;
        updateCoefficients();
    }

    void setResonance(float newResonance)
    {
        if (newResonance < 0.0f)  newResonance = 0.0f;
        if (newResonance > 0.95f) newResonance = 0.95f;
        resonance = newResonance;
    }

    void reset()
    {
        lowPassState = 0.0f;
        bandPassState = 0.0f;
    }

    float process(float input)
    {
        float feedback = resonance * bandPassState;
        float filteredInput = input - feedback;

        lowPassState = lowPassState + coeff * (filteredInput - lowPassState);
        bandPassState = bandPassState + coeff * (filteredInput - lowPassState - bandPassState);

        return lowPassState;
    }

private:

    double sampleRate = 44100.0;
    float  cutoffHz = 8000.0f;
    float  resonance = 0.0f;
    float  coeff = 0.9f;

    float lowPassState = 0.0f;
    float bandPassState = 0.0f;

    void updateCoefficients()
    {
        if (sampleRate <= 0.0) return;

        const double pi = 3.14159265358979323846;
        double omega = 2.0 * pi * (double)cutoffHz / sampleRate;
        double c = 1.0 - std::exp(-omega);

        if (c < 0.001)  c = 0.001;
        if (c > 0.9999) c = 0.9999;

        coeff = (float)c;
    }
};