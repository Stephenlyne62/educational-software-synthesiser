#pragma once

#include <cmath>
#include <algorithm>

// =============================================================================
// Envelope.h — Exponential ADSR Envelope
//
// WHAT CHANGED FROM THE ORIGINAL:
//
// 1. EXPONENTIAL CURVES (most important change)
//    The original used linear ramps for all four stages. Human hearing perceives
//    loudness logarithmically, so a linear ramp sounds unnatural — it appears
//    to "jump" at the start of attack and "drag" at the end of decay/release.
//
//    This version uses an exponential approach: each stage moves toward its
//    target by multiplying the remaining distance by a coefficient per sample.
//    This produces the smooth, natural-sounding curves found in hardware
//    synthesisers and is the standard approach in professional audio software.
//
//    The coefficient is calculated from the time parameter using:
//        coeff = exp(-log(9.0) / (timeInSeconds * sampleRate))
//    This ensures the envelope reaches ~90% of its target in the specified time,
//    which matches the perceptual expectation of the parameter.
//
// 2. RETRIGGER SUPPORT
//    The original always reset to zero on noteOn, causing a click if a note
//    is retriggered while the envelope is still in its release phase. This
//    version retriggers from the current level, eliminating the click.
//
// 3. RELEASE FROM CURRENT LEVEL
//    The original released from the sustain level only. If the note was
//    released during attack or decay, the release would jump to sustain first,
//    causing an audible glitch. This version releases from whatever the
//    current level is at the moment noteOff() is called.
//
// 4. PARAMETER RANGES
//    Attack, Decay, Release: time in seconds (e.g. 0.001 to 5.0)
//    Sustain: level 0.0 to 1.0
// =============================================================================

class Envelope
{
public:
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        recalculateCoefficients();
    }

    void setParameters(float newAttack, float newDecay,
        float newSustain, float newRelease)
    {
        // Use std:: equivalents — no JUCE dependency needed in this header
        attack = std::max(0.001f, newAttack);
        decay = std::max(0.001f, newDecay);
        sustain = std::max(0.0f, std::min(1.0f, newSustain));
        release = std::max(0.001f, newRelease);
        recalculateCoefficients();
    }

    void noteOn()
    {
        // Retrigger from current level (no click on fast repeated notes)
        state = State::Attack;
        // Do NOT reset level to 0 — retrigger from wherever we are
    }

    void noteOff()
    {
        if (state != State::Idle)
        {
            state = State::Release;
            releaseStartLevel = level; // Release from current level, not sustain
        }
    }

    float getNextValue()
    {
        switch (state)
        {
        case State::Idle:
            level = 0.0f;
            break;

        case State::Attack:
            // Exponential approach toward 1.0
            level = 1.0f - (1.0f - level) * attackCoeff;

            if (level >= 0.999f)
            {
                level = 1.0f;
                state = State::Decay;
            }
            break;

        case State::Decay:
            // Exponential approach toward sustain level
            level = sustain + (level - sustain) * decayCoeff;

            if (level <= sustain + 0.0001f)
            {
                level = sustain;
                state = State::Sustain;
            }
            break;

        case State::Sustain:
            level = sustain;
            break;

        case State::Release:
            // Exponential approach toward 0.0 from wherever we started
            level = level * releaseCoeff;

            if (level <= 0.0001f)
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
    enum class State { Idle, Attack, Decay, Sustain, Release };

    State state = State::Idle;

    double sampleRate = 44100.0;

    float attack = 0.01f;
    float decay = 0.2f;
    float sustain = 0.7f;
    float release = 0.3f;

    float level = 0.0f;
    float releaseStartLevel = 0.0f;

    // Exponential coefficients — recalculated when parameters or sample rate change
    float attackCoeff = 0.0f;
    float decayCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    // Calculate per-sample exponential coefficient from a time in seconds.
    // The envelope reaches ~90% of its target in 'timeSeconds'.
    static float calcCoeff(float timeSeconds, double sr)
    {
        if (timeSeconds <= 0.0f || sr <= 0.0) return 0.0f;
        return (float)std::exp(-std::log(9.0) / ((double)timeSeconds * sr));
    }

    void recalculateCoefficients()
    {
        attackCoeff = calcCoeff(attack, sampleRate);
        decayCoeff = calcCoeff(decay, sampleRate);
        releaseCoeff = calcCoeff(release, sampleRate);
    }
};