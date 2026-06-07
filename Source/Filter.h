#pragma once

class Filter
{
public:
    void setCutoff(float newCutoff)
    {
        cutoff = newCutoff;
    }

    void setResonance(float newResonance)
    {
        resonance = newResonance;
    }

    void reset()
    {
        lowPassState = 0.0f;
        bandPassState = 0.0f;
    }

    float process(float input)
    {
        auto feedback = resonance * bandPassState;
        auto filteredInput = input - feedback;

        lowPassState = lowPassState + cutoff * bandPassState;
        bandPassState = bandPassState + cutoff * (filteredInput - lowPassState);

        return lowPassState;
    }

private:
    float cutoff = 1.0f;
    float resonance = 0.0f;

    float lowPassState = 0.0f;
    float bandPassState = 0.0f;
};