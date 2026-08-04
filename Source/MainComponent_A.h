#pragma once

/*
    MainComponent_A.h

    Declares the primary user interface, audio processing components,
    MIDI handling and study logging functionality for the conventional
    parameter-based synthesiser interface (Version A).
*/

#include "CustomLookAndFeel.h"

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

    // GUI
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Audio callbacks
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // MIDI callbacks
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

    // Timer callback used for UI updates
    void timerCallback() override;

private:

    //==========================================================================
    // Application Look and Feel

    CustomLookAndFeel customLookAndFeel;

    double currentSampleRate = 0.0;

    //==========================================================================
    // User Interface Components

    // Oscillator controls
    juce::Slider   frequencySlider;
    juce::Slider   detuneSlider;
    juce::Slider   noiseSlider;
    juce::ComboBox waveformSelector;

    // Filter controls
    juce::Slider   filterSlider;
    juce::Slider   resonanceSlider;

    // Amplitude envelope controls
    juce::Slider   attackSlider;
    juce::Slider   decaySlider;
    juce::Slider   sustainSlider;
    juce::Slider   releaseSlider;

    // Filter envelope parameters.
    // Hidden in Version A to maintain DSP parity with Version B.
    float filterEnvAmount = 0.0f;
    float filterAttack = 0.01f;
    float filterDecay = 0.3f;
    float filterSustain = 0.0f;
    float filterRelease = 0.3f;

    // Low Frequency Oscillator (LFO)
    juce::Slider   lfoRateSlider;
    juce::Slider   lfoDepthSlider;
    juce::ComboBox lfoTargetSelector;

    // Reverb control
    juce::Slider reverbSlider;

    // Top bar controls
    juce::ComboBox   presetSelector;
    juce::TextButton initButton;
    juce::ToggleButton powerButton;

    //==========================================================================
    // MIDI Input and Waveform Display

    juce::ComboBox midiInputList;

    juce::MidiKeyboardState keyboardState;

    juce::MidiKeyboardComponent keyboardComponent
    {
        keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard
    };

    //==========================================================================
    // Audio Processing State

    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParameters;

    LFO lfo;

    std::unique_ptr<juce::MidiInput> midiInput;

    std::vector<Voice> voices;

    bool noteOn = false;

    // Circular buffer used by the oscilloscope display.
    std::vector<float> waveformBuffer;
    int waveformBufferIndex = 0;

    //==========================================================================
    // Study Logging and Task Timing

    juce::ComboBox taskSelector;
    juce::TextButton taskFinishedButton;

    juce::Time currentTaskStartTime;
    bool taskIsRunning = false;

    juce::File logFile;

    // Prevents participant interactions being logged while presets initialise.
    bool isInitialising = false;

    void logAction(const juce::String& action);
    juce::String getCurrentTaskName() const;

    //==========================================================================
    // Helper Functions

    void setupRotary(juce::Slider& slider,
        const juce::String& name,
        double min,
        double max,
        double interval,
        double defaultVal);

    void setupFader(juce::Slider& slider,
        const juce::String& name,
        double min,
        double max,
        double interval,
        double defaultVal);

    void setFrequencyFromMidiNote(int midiNoteNumber);

    Voice* findFreeVoice();

    void applyPreset(int presetNumber);

    void initPatch();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};