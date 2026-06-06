#pragma once

class Envelope
{
public:
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
    }

    void setParameters(float newAttack, float newDecay, float newSustain, float newRelease)
    {
        attack = newAttack;
        decay = newDecay;
        sustain = newSustain;
        release = newRelease;
    }

    void noteOn()
    {
        state = State::Attack;
    }

    void noteOff()
    {
        state = State::Release;
    }

    float getNextValue()
    {
        switch (state)
        {
        case State::Idle:
            level = 0.0f;
            break;

        case State::Attack:
            level += 1.0f / ((float)sampleRate * attack);

            if (level >= 1.0f)
            {
                level = 1.0f;
                state = State::Decay;
            }
            break;

        case State::Decay:
            level -= (1.0f - sustain) / ((float)sampleRate * decay);

            if (level <= sustain)
            {
                level = sustain;
                state = State::Sustain;
            }
            break;

        case State::Sustain:
            level = sustain;
            break;

        case State::Release:
            level -= sustain / ((float)sampleRate * release);

            if (level <= 0.0f)
            {
                level = 0.0f;
                state = State::Idle;
            }
            break;
        }

        return level;
    }

    bool isActive() const
    {
        return state != State::Idle;
    }

private:
    enum class State
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    State state = State::Idle;

    double sampleRate = 44100.0;

    float attack = 0.01f;
    float decay = 0.2f;
    float sustain = 0.7f;
    float release = 0.3f;

    float level = 0.0f;
};