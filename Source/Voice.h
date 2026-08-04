/**
 * @file Voice.h
 * @brief Defines a single polyphonic synthesiser voice responsible for
 *        oscillator generation, envelope processing, filtering, and noise
 *        synthesis.
 *
 * Each Voice represents one independently playable note within the
 * polyphonic synthesiser. The class encapsulates all DSP required for a
 * single voice while exposing a simple interface for note allocation and
 * parameter updates from MainComponent.
 */

#pragma once

#include <cmath>

#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"

 /**
  * Represents one synthesiser voice within the fixed polyphony pool.
  *
  * Each voice contains two detuned oscillators, an amplitude ADSR envelope,
  * a filter ADSR envelope, a resonant filter, and optional white-noise
  * generation. Voice instances are reused by the allocator when new MIDI
  * notes are received.
  */
class Voice
{
public:
    /**
     * Propagates the host sample rate to every DSP component that performs
     * time-dependent processing.
     *
     * @param newSampleRate Active audio sample rate supplied by JUCE.
     */
    void setSampleRate(double newSampleRate)
    {
        oscillatorA.setSampleRate(newSampleRate);
        oscillatorB.setSampleRate(newSampleRate);
        ampEnvelope.setSampleRate(newSampleRate);
        filterEnvelope.setSampleRate(newSampleRate);
        filter.setSampleRate(newSampleRate);
    }

    /**
     * Configures the amplitude ADSR envelope applied to the generated signal.
     *
     * @param attack  Attack time in seconds.
     * @param decay   Decay time in seconds.
     * @param sustain Sustain level.
     * @param release Release time in seconds.
     */
    void setEnvelopeParameters(float attack, float decay, float sustain, float release)
    {
        ampEnvelope.setParameters(attack, decay, sustain, release);
    }

    /**
     * Configures the ADSR envelope used to modulate filter cutoff over time.
     *
     * @param attack  Attack time in seconds.
     * @param decay   Decay time in seconds.
     * @param sustain Sustain level.
     * @param release Release time in seconds.
     */
    void setFilterEnvelopeParameters(float attack, float decay, float sustain, float release)
    {
        filterEnvelope.setParameters(attack, decay, sustain, release);
    }

    //==========================================================================
    // Filter configuration

    void setFilterCutoff(float cutoffHz)
    {
        baseFilterCutoff = cutoffHz;
    }

    void setFilterResonance(float resonance)
    {
        filter.setResonance(resonance);
    }

    void setFilterEnvAmount(float amountHz)
    {
        filterEnvAmount = amountHz;
    }

    //==========================================================================
    // Oscillator pitch control

    void setDetuneCents(float newDetuneCents)
    {
        detuneCents = newDetuneCents;
        updateOscillatorFrequencies();
    }

    void setPitchModulationCents(float newPitchModulationCents)
    {
        pitchModulationCents = newPitchModulationCents;
        updateOscillatorFrequencies();
    }

    /**
     * Initialises the voice for a newly received MIDI note.
     *
     * Oscillator frequencies are updated before the amplitude and filter
     * envelopes are triggered. The filter state is reset to prevent residual
     * state from a previously allocated note affecting the new event.
     *
     * @param newMidiNoteNumber MIDI note number assigned to the voice.
     * @param frequency         Corresponding note frequency in hertz.
     */
    void startNote(int newMidiNoteNumber, double frequency)
    {
        midiNoteNumber = newMidiNoteNumber;
        baseFrequency = frequency;
        active = true;

        updateOscillatorFrequencies();

        filter.reset();
        ampEnvelope.noteOn();
        filterEnvelope.noteOn();
    }

    /**
     * Releases the current note by triggering the release stage of both the
     * amplitude and filter envelopes.
     */
    void stopNote()
    {
        ampEnvelope.noteOff();
        filterEnvelope.noteOff();
    }

    /**
     * Applies the selected waveform to both oscillators so that timbral
     * changes remain consistent across the detuned oscillator pair.
     *
     * @param waveform Identifier of the oscillator waveform to use.
     */
    void setWaveform(int waveform)
    {
        oscillatorA.setWaveform(waveform);
        oscillatorB.setWaveform(waveform);
    }

    /**
     * Generates one processed audio sample for the current voice.
     *
     * Processing order:
     *  1. Evaluate the amplitude and filter envelopes.
     *  2. Modulate the base filter cutoff using the filter envelope.
     *  3. Generate and combine the two oscillator signals.
     *  4. Blend the oscillator signal with optional white noise.
     *  5. Apply the amplitude envelope.
     *  6. Process the result through the resonant filter.
     *
     * @return One processed audio sample.
     */
    float getNextSample()
    {
        auto ampEnvValue = ampEnvelope.getNextValue();
        auto filterEnvValue = filterEnvelope.getNextValue();

        // Mark the voice as reusable only after the amplitude envelope has
        // completed its release stage.
        if (!ampEnvelope.isActive())
        {
            active = false;
            midiNoteNumber = -1;
            return 0.0f;
        }

        // Modulate the base cutoff frequency using the current filter-envelope
        // value and configured modulation depth.
        float currentCutoff = baseFilterCutoff + (filterEnvValue * filterEnvAmount);

        // Constrain the cutoff to the valid operating range of the filter.
        if (currentCutoff < 20.0f) currentCutoff = 20.0f;
        if (currentCutoff > 18000.0f) currentCutoff = 18000.0f;

        filter.setCutoffHz(currentCutoff);

        auto sampleA = oscillatorA.getNextSample();
        auto sampleB = oscillatorB.getNextSample();

        // Generate bipolar white noise and blend it with the oscillator signal
        // according to the current noise level.
        auto noiseSample = (random.nextFloat() * 2.0f) - 1.0f;

        auto oscillatorMix = (sampleA + sampleB) * 0.5f;
        auto mixedSample = ((oscillatorMix * (1.0f - noiseAmount))
            + (noiseSample * noiseAmount)) * ampEnvValue;

        return filter.process(mixedSample);
    }

    bool isActive() const
    {
        return active;
    }

    int getMidiNoteNumber() const
    {
        return midiNoteNumber;
    }

    void setNoiseAmount(float newNoiseAmount)
    {
        noiseAmount = newNoiseAmount;
    }

private:
    //==========================================================================
    // DSP modules

    Oscillator oscillatorA;
    Oscillator oscillatorB;
    Envelope ampEnvelope;
    Envelope filterEnvelope;
    Filter filter;

    //==========================================================================
    // Voice state

    bool active = false;
    int midiNoteNumber = -1;

    //==========================================================================
    // Oscillator parameters

    double baseFrequency = 440.0;
    float detuneCents = 5.0f;
    float pitchModulationCents = 0.0f;
    float noiseAmount = 0.0f;

    //==========================================================================
    // Filter parameters

    float baseFilterCutoff = 8000.0f;
    float filterEnvAmount = 0.0f;

    // Independent random-number source used for white-noise generation.
    juce::Random random;

    /**
     * Recalculates oscillator frequencies after a change to the base note,
     * detuning amount, or pitch modulation.
     *
     * Pitch offsets expressed in cents are converted into multiplicative
     * frequency ratios before being applied to the two oscillators.
     */
    void updateOscillatorFrequencies()
    {
        // Convert pitch offsets expressed in cents into frequency ratios.
        auto pitchRatio = std::pow(2.0, pitchModulationCents / 1200.0);
        auto detuneRatio = std::pow(2.0, detuneCents / 1200.0);

        oscillatorA.setFrequency(baseFrequency * pitchRatio);
        oscillatorB.setFrequency(baseFrequency * pitchRatio * detuneRatio);
    }
};
