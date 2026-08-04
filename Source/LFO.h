/**
 * @file LFO.h
 * @brief Implements a sine-wave low-frequency oscillator (LFO) used for
 *        real-time parameter modulation.
 *
 * The oscillator generates a continuous sinusoidal waveform whose frequency
 * is substantially below the audible range. The resulting signal can be used
 * to modulate synthesis parameters such as amplitude, pitch, or filter cutoff
 * to create effects including tremolo, vibrato, and filter movement.
 */

#pragma once

#include <JuceHeader.h>

 /**
  * Generates a continuous sinusoidal low-frequency modulation signal.
  *
  * The LFO maintains its own phase accumulator and advances it once per
  * generated sample. Changing either the sample rate or modulation rate
  * automatically updates the phase increment required for accurate playback.
  */
class LFO
{
public:
    /**
     * Sets the audio sample rate used to calculate phase progression.
     *
     * @param newSampleRate Active audio sample rate in hertz.
     */
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateAngleDelta();
    }

    /**
     * Sets the modulation frequency of the LFO.
     *
     * @param newRate Oscillation frequency in hertz.
     */
    void setRate(double newRate)
    {
        rate = newRate;
        updateAngleDelta();
    }

    /**
     * Generates the next sample of the sinusoidal modulation waveform.
     *
     * The internal phase accumulator is wrapped after one complete cycle to
     * maintain numerical stability during continuous operation.
     *
     * @return Current LFO output in the range -1.0 to +1.0.
     */
    float getNextValue()
    {
        auto value = (float)std::sin(currentAngle);

        currentAngle += angleDelta;

        // Wrap the phase accumulator after one complete cycle.
        if (currentAngle >= 2.0 * juce::MathConstants<double>::pi)
            currentAngle -= 2.0 * juce::MathConstants<double>::pi;

        return value;
    }

private:
    //==========================================================================
    // Oscillator state

    double sampleRate = 44100.0;
    double rate = 5.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;

    /**
     * Recalculates the phase increment required for each generated sample.
     *
     * The phase increment is derived from the requested modulation frequency
     * and the current audio sample rate.
     */
    void updateAngleDelta()
    {
        angleDelta = rate * 2.0 * juce::MathConstants<double>::pi / sampleRate;
    }
};