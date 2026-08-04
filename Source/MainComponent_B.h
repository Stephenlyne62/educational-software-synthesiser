/**
 * @file MainComponent_B.h
 * @brief Declares the principal JUCE component for Synthesiser Design B.
 *
 * MainComponent coordinates the graphical interface, real-time audio
 * processing, MIDI input, polyphonic voice management, perceptual feedback,
 * preset handling, oscilloscope visualisation, and study-session logging.
 *
 * The class acts as the integration layer between the user interface and the
 * underlying DSP modules while keeping synthesis voices, modulation, effects,
 * and experimental instrumentation logically separated.
 */

#pragma once 

#include <JuceHeader.h>
#include <vector>

#include "CustomLookAndFeel.h"
#include "Oscillator.h"
#include "Voice.h"
#include "LFO.h"

 /**
  * Main application component for the perceptually oriented synthesiser.
  *
  * The component implements JUCE interfaces for audio rendering, MIDI input,
  * on-screen keyboard interaction, timed repainting, and slider event handling.
  * It also manages the fixed polyphony pool and records participant interactions
  * for later analysis.
  */
class MainComponent : public juce::AudioAppComponent,
    public juce::MidiInputCallback,
    public juce::MidiKeyboardStateListener,
    public juce::Timer,
    public juce::Slider::Listener
{
public:
    /**
     * Initialises the interface, audio system, MIDI controls, synthesis voices,
     * presets, interaction logging, and default patch state.
     */
    MainComponent();

    /**
     * Releases audio, MIDI, timer, listener, and LookAndFeel resources.
     */
    ~MainComponent() override;

    //==========================================================================
    // JUCE component rendering and layout

    /**
     * Draws the synthesiser body, control panels, labels, and oscilloscope.
     */
    void paint(juce::Graphics& g) override;

    /**
     * Assigns bounds to all child components using the study interface layout.
     */
    void resized() override;

    //==========================================================================
    // Real-time audio lifecycle

    /**
     * Prepares sample-rate-dependent DSP components before playback begins.
     */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /**
     * Generates the next block of stereo synthesis output.
     */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    /**
     * Releases resources allocated for audio playback.
     */
    void releaseResources() override;

    //==========================================================================
    // MIDI and keyboard interaction

    /**
     * Forwards incoming hardware MIDI messages to the shared keyboard state.
     */
    void handleIncomingMidiMessage(juce::MidiInput* source,
        const juce::MidiMessage& message) override;

    /**
     * Allocates and starts a synthesis voice for an incoming note event.
     */
    void handleNoteOn(juce::MidiKeyboardState* source,
        int midiChannel,
        int midiNoteNumber,
        float velocity) override;

    /**
     * Releases any active voices associated with the specified MIDI note.
     */
    void handleNoteOff(juce::MidiKeyboardState* source,
        int midiChannel,
        int midiNoteNumber,
        float velocity) override;

    /**
     * Requests periodic repainting of dynamic interface elements.
     */
    void timerCallback() override;

    /**
     * Produces direction-aware perceptual feedback following slider changes.
     */
    void sliderValueChanged(juce::Slider* slider) override;

private:
    //==========================================================================
    // Shared visual theme

    CustomLookAndFeel customLookAndFeel;

    //==========================================================================
    // Audio configuration

    double currentSampleRate = 0.0;

    //==========================================================================
    // Perceptually oriented sound-design controls
    //
    // Version B exposes user-facing perceptual concepts while abstracting the
    // underlying synthesis parameters.

    // Brightness is mapped to filter cutoff frequency.
    juce::Slider brightnessSlider;

    // Size is mapped to oscillator detuning.
    juce::Slider sizeSlider;

    // Punch is represented by the amplitude ADSR envelope.
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    // Movement is controlled through LFO rate and modulation depth.
    juce::Slider movementRateSlider;
    juce::Slider movementDepthSlider;

    // Distance is mapped to the reverb amount.
    juce::Slider distanceSlider;

    //==========================================================================
    // Preset-controlled synthesis parameters
    //
    // These technical parameters remain hidden from the participant in
    // Version B and are configured indirectly through the preset system.

    int currentWaveform = 1;
    float currentNoise = 0.0f;
    float currentResonance = 0.0f;
    int currentLfoTarget = 1;

    float filterEnvAmount = 0.0f;
    float filterAttack = 0.01f;
    float filterDecay = 0.3f;
    float filterSustain = 0.0f;
    float filterRelease = 0.3f;

    //==========================================================================
    // Contextual and educational feedback

    juce::Label presetExplanationLabel;
    juce::Label whatChangedLabel;

    //==========================================================================
    // Global interface controls

    juce::ComboBox presetSelector;
    juce::TextButton initButton;
    juce::ToggleButton powerButton;

    // Maintains the lifetime required for JUCE tooltip presentation.
    juce::TooltipWindow tooltipWindow;

    //==========================================================================
    // MIDI input, keyboard state, and visualisation

    juce::ComboBox midiInputList;

    juce::MidiKeyboardState keyboardState;

    juce::MidiKeyboardComponent keyboardComponent
    {
        keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard
    };

    //==========================================================================
    // DSP engine and voice management

    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParameters;

    LFO lfo;

    std::unique_ptr<juce::MidiInput> midiInput;
    std::vector<Voice> voices;

    // Tracks whether any note activity is currently present.
    bool noteOn = false;

    // Suppresses interaction feedback during programmatic parameter changes.
    bool isInitialising = false;

    // Circular sample buffer used by the oscilloscope display.
    std::vector<float> waveformBuffer;
    int waveformBufferIndex = 0;

    // Timestamped text file used to record participant interactions.
    juce::File logFile;

    //==========================================================================
    // Baseline values for direction-aware feedback
    //
    // These values allow the interface to determine whether a participant has
    // increased or decreased a perceptual parameter.

    double previousBrightness = 8000.0;
    double previousSize = 5.0;
    double previousDistance = 0.0;

    double previousMovementRate = 5.0;
    double previousMovementDepth = 0.0;

    double previousAttack = 0.05;
    double previousDecay = 0.2;
    double previousSustain = 0.7;
    double previousRelease = 0.4;

    //==========================================================================
    // Experimental task logging

    juce::ComboBox taskSelector;
    juce::TextButton taskFinishedButton;

    juce::Time currentTaskStartTime;
    bool taskIsRunning = false;

    /**
     * Converts the selected task identifier into the human-readable label
     * written to the study log.
     */
    juce::String getCurrentTaskName() const;

    //==========================================================================
    // Interface setup helpers

    /**
     * Configures a rotary control, tooltip, event listener, and interaction
     * logging callback.
     */
    void setupRotary(juce::Slider& slider,
        const juce::String& name,
        double min,
        double max,
        double interval,
        double defaultVal,
        const juce::String& tooltip);

    /**
     * Configures a vertical fader, tooltip, event listener, and interaction
     * logging callback.
     */
    void setupFader(juce::Slider& slider,
        const juce::String& name,
        double min,
        double max,
        double interval,
        double defaultVal,
        const juce::String& tooltip);

    //==========================================================================
    // Voice allocation and patch management

    /**
     * Returns an available synthesis voice or applies the fallback
     * voice-stealing policy when all voices are active.
     */
    Voice* findFreeVoice();

    /**
     * Loads the synthesis parameters associated with the selected preset.
     */
    void applyPreset(int presetNumber);

    /**
     * Restores the neutral reference patch used as the common starting state.
     */
    void initPatch();

    //==========================================================================
    // Participant feedback and study instrumentation

    /**
     * Updates the contextual panel with the direction and expected audible
     * consequence of a parameter change.
     */
    void updateWhatChanged(const juce::String& controlName,
        const juce::String& action,
        const juce::String& result);

    /**
     * Appends a timestamped event to the current study-session log.
     */
    void logAction(const juce::String& action);

    /**
     * Stores the current control values as the comparison baseline for future
     * direction-aware feedback.
     */
    void syncPreviousValues();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};