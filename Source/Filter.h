#pragma once

class Filter
{
public:
    void setCutoff(float newCutoff)
    {
        cutoff = newCutoff;
    }

    void reset()
    {
        state = 0.0f;
    }

    float process(float input)
    {
        state = state + cutoff * (input - state);
        return state;
    }

private:
    float cutoff = 1.0f;
    float state = 0.0f;
};