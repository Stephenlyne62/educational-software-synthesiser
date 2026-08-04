/**
 * @file Envelope.h
 * @brief Implements a sample-by-sample ADSR envelope using exponential
 *        transitions between envelope stages.
 *
 * The envelope produces a normalised control signal across attack, decay,
 * sustain, and release stages. Exponential smoothing is used for the
 * time-varying stages to provide gradual transitions suitable for amplitude
 * and filter modulation.
 *
 * Attack, decay, and release parameters are expressed in seconds. Sustain is
 * represented as a normalised level between 0.0 and 1.0.
 */

#pragma once

#include <cmath>
#include <algorithm>

 /**
  * Implements a reusable ADSR control envelope.
  *
  * The class maintains an explicit state machine and generates one normalised
  * envelope value per call to getNextValue(). Parameter and sample-rate changes
  * trigger coefficient recalculation outside the sample-generation method.
  */
class Envelope
{
public:
    /**
     * Sets the active audio sample rate and recalculates all time-dependent
     * smoothing coefficients.
     *
     * @param newSampleRate Active sample rate in hertz.
     */
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        recalculateCoefficients();
    }

    /**
     * Updates the ADSR parameters and constrains them to supported ranges.
     *
     * @param newAttack  Attack time in seconds.
     * @param newDecay   Decay time in seconds.
     * @param newSustain Normalised sustain level.
     * @param newRelease Release time in seconds.
     */
    void setParameters(float newAttack, float newDecay,
        float newSustain, float newRelease)
    {
        // Constrain incoming parameters using standard-library functions,
        // keeping the envelope independent of JUCE-specific utility methods.
        attack = std::max(0.001f, newAttack);
        decay = std::max(0.001f, newDecay);
        sustain = std::max(0.0f, std::min(1.0f, newSustain));
        release = std::max(0.001f, newRelease);
        recalculateCoefficients();
    }

    /**
     * Starts or retriggers the attack stage.
     *
     * The existing envelope level is preserved to avoid the discontinuity
     * caused by resetting the signal abruptly to zero.
     */
    void noteOn()
    {
        // Retrigger from the current level rather than resetting the envelope.
        state = State::Attack;

        // Preserve the existing level so overlapping note events remain
        // continuous.
    }

    /**
     * Starts the release stage when the envelope is active.
     */
    void noteOff()
    {
        if (state != State::Idle)
        {
            state = State::Release;
            releaseStartLevel = level; // Preserve the level present when release begins
        }
    }

    /**
     * Advances the envelope state machine by one sample.
     *
     * @return Current normalised envelope level.
     */
    float getNextValue()
    {
        switch (state)
        {
        case State::Idle:
            level = 0.0f;
            break;

        case State::Attack:
            // Approach the maximum envelope level using exponential smoothing.
            level = 1.0f - (1.0f - level) * attackCoeff;

            if (level >= 0.999f)
            {
                level = 1.0f;
                state = State::Decay;
            }
            break;

        case State::Decay:
            // Transition exponentially from the peak level toward sustain.
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
            // Attenuate the current envelope level exponentially toward silence.
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

    /**
     * Indicates whether the envelope is currently outside the idle state.
     *
     * @return true while any envelope stage is active; otherwise false.
     */
    bool isActive() const
    {
        return state != State::Idle;
    }

private:
    //==========================================================================
    // Envelope state

    enum class State { Idle, Attack, Decay, Sustain, Release };

    State state = State::Idle;

    //==========================================================================
    // Timing and level parameters

    double sampleRate = 44100.0;

    float attack = 0.01f;
    float decay = 0.2f;
    float sustain = 0.7f;
    float release = 0.3f;

    float level = 0.0f;
    float releaseStartLevel = 0.0f;

    //==========================================================================
    // Precomputed exponential smoothing coefficients

    float attackCoeff = 0.0f;
    float decayCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    /**
     * Calculates the per-sample smoothing coefficient for an exponential
     * transition.
     *
     * After timeSeconds, approximately one ninth of the initial distance to
     * the target remains. Stage-completion thresholds are evaluated separately.
     *
     * @param timeSeconds Requested transition time in seconds.
     * @param sr          Active sample rate in hertz.
     * @return Per-sample exponential smoothing coefficient.
     */
    static float calcCoeff(float timeSeconds, double sr)
    {
        if (timeSeconds <= 0.0f || sr <= 0.0) return 0.0f;
        return (float)std::exp(-std::log(9.0) / ((double)timeSeconds * sr));
    }

    /**
     * Recalculates all smoothing coefficients after a parameter or sample-rate
     * change.
     */
    void recalculateCoefficients()
    {
        attackCoeff = calcCoeff(attack, sampleRate);
        decayCoeff = calcCoeff(decay, sampleRate);
        releaseCoeff = calcCoeff(release, sampleRate);
    }
};