#pragma once

#include <JuceHeader.h>
#include <vector>

#include "Oscillator.h"
#include "Voice.h"
#include "LFO.h"

class MainComponent : public juce::AudioAppComponent,
    public juce::MidiInputCallback,
    public juce::MidiKeyboardStateListener,
    public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void handleIncomingMidiMessage(juce::MidiInput* source,
        const juce::MidiMessage& message) override;

    void handleNoteOn(juce::MidiKeyboardState* source,
        int midiChannel,
        int midiNoteNumber,
        float velocity) override;

    void handleNoteOff(juce::MidiKeyboardState* source,
        int midiChannel,
        int midiNoteNumber,
        float velocity) override;

    void timerCallback() override;

private:
    double currentSampleRate = 0.0;

    //==========================================================================
    // Oscillator / Waveform
    juce::ComboBox waveformSelector;
    juce::Label    waveformLabel;

    juce::Slider   frequencySlider;
    juce::Label    frequencyLabel;

    juce::Slider   detuneSlider;
    juce::Label    detuneLabel;

    juce::Slider   noiseSlider;
    juce::Label    noiseLabel;

    //==========================================================================
    // Volume
    juce::Slider   volumeSlider;
    juce::Label    volumeLabel;

    //==========================================================================
    // Filter
    juce::Slider   filterSlider;       // Cutoff in Hz (200–18000)
    juce::Label    filterLabel;

    juce::Slider   resonanceSlider;
    juce::Label    resonanceLabel;

    //==========================================================================
    // Amplitude Envelope (ADSR)
    juce::Slider   attackSlider;
    juce::Label    attackLabel;
    juce::Slider   decaySlider;
    juce::Label    decayLabel;
    juce::Slider   sustainSlider;
    juce::Label    sustainLabel;
    juce::Slider   releaseSlider;
    juce::Label    releaseLabel;

    //==========================================================================
    // Filter Envelope (second ADSR routed to filter cutoff)
    juce::Slider   filterEnvAmountSlider;   // Range: -8000 to +8000 Hz
    juce::Label    filterEnvAmountLabel;
    juce::Slider   filterAttackSlider;
    juce::Label    filterAttackLabel;
    juce::Slider   filterDecaySlider;
    juce::Label    filterDecayLabel;
    juce::Slider   filterSustainSlider;
    juce::Label    filterSustainLabel;
    juce::Slider   filterReleaseSlider;
    juce::Label    filterReleaseLabel;

    //==========================================================================
    // LFO
    juce::Slider   lfoRateSlider;
    juce::Label    lfoRateLabel;
    juce::Slider   lfoDepthSlider;
    juce::Label    lfoDepthLabel;
    juce::ComboBox lfoTargetSelector;
    juce::Label    lfoTargetLabel;

    //==========================================================================
    // Reverb
    juce::Slider   reverbSlider;
    juce::Label    reverbLabel;

    juce::Reverb            reverb;
    juce::Reverb::Parameters reverbParameters;

    //==========================================================================
    // LFO engine
    LFO lfo;

    //==========================================================================
    // Buttons
    juce::ToggleButton powerButton;

    juce::TextButton bassPresetButton;
    juce::TextButton leadPresetButton;
    juce::TextButton padPresetButton;
    juce::TextButton pluckPresetButton;

    juce::TextButton savePresetButton;
    juce::TextButton loadPresetButton;

    //==========================================================================
    // MIDI
    juce::ComboBox midiInputList;
    juce::Label    midiInputLabel;

    juce::MidiKeyboardState     keyboardState;
    juce::MidiKeyboardComponent keyboardComponent
    {
        keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard
    };

    std::unique_ptr<juce::MidiInput> midiInput;

    //==========================================================================
    // Voices and state
    std::vector<Voice> voices;

    bool noteOn = false;

    std::vector<float> waveformBuffer;
    int waveformBufferIndex = 0;

    //==========================================================================
    // Private helpers
    void setFrequencyFromMidiNote(int midiNoteNumber);
    Voice* findFreeVoice();
    void applyPreset(int presetNumber);
    void savePreset();
    void loadPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};