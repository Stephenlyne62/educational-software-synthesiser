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

    juce::TextButton savePresetButton;
    juce::TextButton loadPresetButton;

    void savePreset();
    void loadPreset();

    juce::TextButton bassPresetButton;
    juce::TextButton leadPresetButton;
    juce::TextButton padPresetButton;
    juce::TextButton pluckPresetButton;

    juce::ComboBox lfoTargetSelector;
    juce::Label lfoTargetLabel;

    juce::Slider frequencySlider;
    juce::Label frequencyLabel;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    juce::Slider lfoRateSlider;
    juce::Slider lfoDepthSlider;

    juce::Label lfoRateLabel;
    juce::Label lfoDepthLabel;

    LFO lfo;

    juce::ComboBox waveformSelector;
    juce::Label waveformLabel;

    juce::Slider filterSlider;
    juce::Label filterLabel;

    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;

    juce::ToggleButton powerButton;

    juce::ComboBox midiInputList;
    juce::Label midiInputLabel;

    juce::Slider detuneSlider;
    juce::Label detuneLabel;

    std::vector<Voice> voices;

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent
    {
        keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard
    };

    std::unique_ptr<juce::MidiInput> midiInput;

    bool noteOn = false;

    std::vector<float> waveformBuffer;
    int waveformBufferIndex = 0;

    void setFrequencyFromMidiNote(int midiNoteNumber);
    Voice* findFreeVoice();

    void applyPreset(int presetNumber);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};