#pragma once

#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"

class Voice
{
public:
    void setSampleRate(double newSampleRate)
    {
        oscillatorA.setSampleRate(newSampleRate);
        oscillatorB.setSampleRate(newSampleRate);
        envelope.setSampleRate(newSampleRate);
    }

    void setEnvelopeParameters(float attack, float decay, float sustain, float release)
    {
        envelope.setParameters(attack, decay, sustain, release);
    }

    void setFilterCutoff(float cutoff)
    {
        filter.setCutoff(cutoff);
    }

    void startNote(int newMidiNoteNumber, double frequency)
    {
        midiNoteNumber = newMidiNoteNumber;
        active = true;

        oscillatorA.setFrequency(frequency);
        oscillatorB.setFrequency(frequency * 1.005);

        filter.reset();
        envelope.noteOn();
    }

    void stopNote()
    {
        envelope.noteOff();
    }

    void setWaveform(int waveform)
    {
        oscillatorA.setWaveform(waveform);
        oscillatorB.setWaveform(waveform);
    }

    float getNextSample()
    {
        auto envelopeValue = envelope.getNextValue();

        if (!envelope.isActive())
        {
            active = false;
            midiNoteNumber = -1;
            return 0.0f;
        }

        auto sampleA = oscillatorA.getNextSample();
        auto sampleB = oscillatorB.getNextSample();

        auto mixedSample = ((sampleA + sampleB) * 0.5f) * envelopeValue;

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

private:
    Oscillator oscillatorA;
    Oscillator oscillatorB;
    Envelope envelope;
    Filter filter;

    bool active = false;
    int midiNoteNumber = -1;
};