/**
 * @file MainComponent_B.cpp
 * @brief Implements the graphical interface, real-time audio processing,
 *        MIDI handling, preset management, and participant interaction logging
 *        for Synthesiser Design B.
 *
 * The component provides an eight-voice polyphonic synthesiser intended for
 * comparative user evaluation. Perceptually labelled controls expose musically
 * meaningful parameters while abstracting lower-level synthesis terminology.
 * Participant interactions and task-completion times are written to a unique
 * session log to support subsequent analysis.
 */
#include "MainComponent_B.h"

 //==============================================================================
 // Component initialisation and experimental interface configuration
MainComponent::MainComponent()
{
    setLookAndFeel(&customLookAndFeel);
    setSize(1200, 850);

    // Allocate a fixed pool of synthesis voices for polyphonic playback. The
    // fixed-size visualisation buffer avoids allocation during audio rendering.
    voices.resize(8);
    waveformBuffer.resize(512, 0.0f);

    // Generate a unique session filename so that separate participant logs are
    // retained rather than overwritten.
    auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");

    logFile = juce::File::getSpecialLocation(
        juce::File::userDocumentsDirectory)
        .getChildFile("SynthStudy_B_" + timestamp + ".txt");

    logAction("===== SESSION STARTED =====");

    addAndMakeVisible(powerButton);
    powerButton.setButtonText("Sound On");
    powerButton.setToggleState(false, juce::dontSendNotification);
    powerButton.onClick = [this] { noteOn = powerButton.getToggleState(); };

    addAndMakeVisible(presetSelector);
    presetSelector.addItem("Init Patch", 1);
    presetSelector.addItem("Bass", 2);
    presetSelector.addItem("Lead", 3);
    presetSelector.addItem("Pad", 4);
    presetSelector.addItem("Pluck", 5);
    presetSelector.setSelectedId(1);
    presetSelector.onChange = [this] { applyPreset(presetSelector.getSelectedId()); };

    addAndMakeVisible(initButton);
    initButton.setButtonText("Reset");
    initButton.onClick = [this]
        {
            initPatch();
            presetSelector.setSelectedId(1, juce::dontSendNotification);
        };

    //==========================================================================
    // Experimental task logging
    //
    // Each task represents a perceptual sound-design objective. Start time,
    // completion time, duration, and parameter interactions are recorded to
    // support analysis of participant behaviour.

    addAndMakeVisible(taskSelector);

    taskSelector.addItem("Task 1 - Brightness", 1);
    taskSelector.addItem("Task 2 - Warmth", 2);
    taskSelector.addItem("Task 3 - Movement", 3);
    taskSelector.addItem("Task 4 - Distance", 4);
    taskSelector.addItem("Task 5 - Pad", 5);

    taskSelector.setSelectedId(1, juce::dontSendNotification);

    taskSelector.onChange = [this]
        {
            currentTaskStartTime = juce::Time::getCurrentTime();
            taskIsRunning = true;

            logAction("");
            logAction("==================================================");
            logAction("STARTED " + getCurrentTaskName());
            logAction("Start Time: " + currentTaskStartTime.formatted("%H:%M:%S"));
            logAction("==================================================");
        };

    addAndMakeVisible(taskFinishedButton);

    taskFinishedButton.setButtonText("Task Finished");

    taskFinishedButton.onClick = [this]
        {
            auto finishTime = juce::Time::getCurrentTime();

            logAction("FINISHED " + getCurrentTaskName());
            logAction("Finish Time: " + finishTime.formatted("%H:%M:%S"));

            if (taskIsRunning)
            {
                auto durationSeconds = (int)(finishTime.toMilliseconds() - currentTaskStartTime.toMilliseconds()) / 1000;

                int minutes = durationSeconds / 60;
                int seconds = durationSeconds % 60;

                logAction("Duration: "
                    + juce::String(minutes)
                    + " min "
                    + juce::String(seconds)
                    + " sec");

                taskIsRunning = false;
            }
            else
            {
                logAction("Duration: unavailable - task start was not recorded");
            }

            logAction("==================================================");
            logAction("");
        };

    setupRotary(brightnessSlider, "Brightness\n(Cutoff)", 200.0, 18000.0, 1.0, 8000.0,
        "Higher values allow more high frequencies through, making the sound brighter.");

    setupRotary(sizeSlider, "Size\n(Detune)", 0.0, 25.0, 0.1, 5.0,
        "Two oscillators slightly detuned against each other create a thicker, wider sound.");

    setupRotary(distanceSlider, "Distance\n(Reverb)", 0.0, 1.0, 0.01, 0.0,
        "Simulates acoustic space. More reverb makes the sound feel further away.");

    setupRotary(movementRateSlider, "Rate\n(LFO)", 0.1, 20.0, 0.1, 5.0,
        "How fast the sound wobbles or pulses.");

    setupRotary(movementDepthSlider, "Depth\n(LFO)", 0.0, 1.0, 0.01, 0.0,
        "How extreme the wobble or pulse effect is.");

    setupFader(attackSlider, "Attack", 0.001, 2.0, 0.001, 0.05,
        "How quickly the sound reaches full volume. Short attack = punchy.");

    setupFader(decaySlider, "Decay", 0.001, 2.0, 0.001, 0.2,
        "How quickly the sound drops to the sustain level after the initial punch.");

    setupFader(sustainSlider, "Sustain", 0.0, 1.0, 0.01, 0.7,
        "The steady volume level while you hold the key down.");

    setupFader(releaseSlider, "Release", 0.001, 2.0, 0.001, 0.4,
        "How long the sound takes to fade out after you let go of the key.");

    addAndMakeVisible(presetExplanationLabel);
    presetExplanationLabel.setFont(juce::FontOptions(15.0f, juce::Font::italic));
    presetExplanationLabel.setColour(juce::Label::textColourId, juce::Colours::lightgoldenrodyellow);
    presetExplanationLabel.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(whatChangedLabel);
    whatChangedLabel.setFont(juce::FontOptions(16.0f));
    whatChangedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    whatChangedLabel.setJustificationType(juce::Justification::topLeft);
    whatChangedLabel.setText("", juce::dontSendNotification);

    // Discover MIDI devices at runtime. ComboBox item ID 1 is reserved for the
    // explicit "No MIDI Input" option, so hardware device IDs begin at 2.
    addAndMakeVisible(midiInputList);
    midiInputList.addItem("No MIDI Input", 1);

    auto midiInputs = juce::MidiInput::getAvailableDevices();

    for (int i = 0; i < midiInputs.size(); ++i)
        midiInputList.addItem(midiInputs[i].name, i + 2);

    midiInputList.setSelectedId(1);

    midiInputList.onChange = [this, midiInputs]
        {
            midiInput.reset();

            auto selectedId = midiInputList.getSelectedId();

            if (selectedId > 1)
            {
                auto deviceIndex = selectedId - 2;

                if (deviceIndex >= 0 && deviceIndex < midiInputs.size())
                    midiInput = juce::MidiInput::openDevice(midiInputs[deviceIndex].identifier, this);

                if (midiInput != nullptr)
                    midiInput->start();
            }
        };

    keyboardState.addListener(this);
    keyboardComponent.setAvailableRange(24, 108);
    addAndMakeVisible(keyboardComponent);

    // The application requires no audio input and produces stereo output. The
    // timer refreshes the oscilloscope independently of the audio callback.
    setAudioChannels(0, 2);
    startTimerHz(60);

    // Suppress interaction feedback while the initial patch is loaded so that
    // programmatic setup is not treated as participant input.
    isInitialising = true;
    initPatch();
    isInitialising = false;

    whatChangedLabel.setText("", juce::dontSendNotification);
}

/**
 * Releases audio, MIDI, timer, and LookAndFeel resources when the component is
 * destroyed.
 */
MainComponent::~MainComponent()
{
    logAction("===== SESSION ENDED =====");
    setLookAndFeel(nullptr);

    // Detach callback-producing resources before shutting down the audio device
    // to avoid references to a partially destroyed component.
    stopTimer();
    keyboardState.removeListener(this);
    midiInput.reset();
    shutdownAudio();

}

//==============================================================================
/**
 * Configures a rotary parameter control and registers it for interaction
 * logging and perceptual feedback.
 *
 * @param slider      Slider instance to configure.
 * @param name        User-facing perceptual parameter name.
 * @param min         Minimum parameter value.
 * @param max         Maximum parameter value.
 * @param interval    Quantisation interval.
 * @param defaultVal  Initial parameter value.
 * @param tooltip     Plain-language explanation shown to the participant.
 */
void MainComponent::setupRotary(juce::Slider& slider,
    const juce::String& name,
    double min,
    double max,
    double interval,
    double defaultVal,
    const juce::String& tooltip)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 75, 22);
    slider.setRange(min, max, interval);
    slider.setValue(defaultVal);
    slider.setName(name);
    auto* s = &slider;

    // Record only the final drag position rather than every intermediate value.
    // This reduces log volume while preserving each deliberate parameter choice.
    slider.onDragEnd = [this, s]
        {
            if (!isInitialising)
            {
                logAction(
                    getCurrentTaskName()
                    + " | Adjusted "
                    + s->getName().replace("\n", " ")
                    + " -> "
                    + juce::String(s->getValue(), 2));
            }
        };
    slider.setTooltip(tooltip);
    slider.addListener(this);
    addAndMakeVisible(slider);
}

/**
 * Configures a vertical parameter control and registers it for interaction
 * logging and perceptual feedback.
 *
 * @param slider      Slider instance to configure.
 * @param name        User-facing perceptual parameter name.
 * @param min         Minimum parameter value.
 * @param max         Maximum parameter value.
 * @param interval    Quantisation interval.
 * @param defaultVal  Initial parameter value.
 * @param tooltip     Plain-language explanation shown to the participant.
 */
void MainComponent::setupFader(juce::Slider& slider,
    const juce::String& name,
    double min,
    double max,
    double interval,
    double defaultVal,
    const juce::String& tooltip)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 22);
    slider.setRange(min, max, interval);
    slider.setValue(defaultVal);
    slider.setName(name);
    auto* s = &slider;

    // Record only the final drag position rather than every intermediate value.
    // This reduces log volume while preserving each deliberate parameter choice.
    slider.onDragEnd = [this, s]
        {
            if (!isInitialising)
            {
                logAction(
                    getCurrentTaskName()
                    + " | Adjusted "
                    + s->getName().replace("\n", " ")
                    + " -> "
                    + juce::String(s->getValue(), 2));
            }
        };
    slider.setTooltip(tooltip);
    slider.addListener(this);
    addAndMakeVisible(slider);
}

//==============================================================================
/**
 * Appends a timestamped interaction event to the current evaluation log.
 *
 * The file is created lazily when required. Logging remains outside the
 * real-time synthesis path so that file I/O is never performed by the audio
 * callback.
 *
 * @param action Human-readable description of the recorded event.
 */
void MainComponent::logAction(const juce::String& action)
{
    if (logFile.existsAsFile() || logFile.create())
    {
        auto timestamp = juce::Time::getCurrentTime().formatted("%H:%M:%S");
        logFile.appendText(timestamp + " - " + action + "\n");
    }
}

/**
 * Converts a parameter change into immediate perceptual feedback.
 *
 * The new value is compared with its previously stored value so that the
 * interface can communicate both the direction of change and its expected
 * auditory consequence. This supports participants who may be unfamiliar with
 * conventional synthesis terminology.
 *
 * @param slider Slider that generated the change event.
 */
void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (isInitialising)
        return;

    const double value = slider->getValue();

    auto name = slider->getName();
    auto val = juce::String(value, 2);

    // Translate low-level parameter changes into perceptually oriented language.
    // The previous value is updated only after the direction is determined.
    if (slider == &brightnessSlider)
    {
        if (value > previousBrightness)
        {
            updateWhatChanged(
                "Brightness",
                "increased",
                "More high frequencies are passing through the filter, making the sound brighter.");
        }
        else
        {
            updateWhatChanged(
                "Brightness",
                "decreased",
                "Fewer high frequencies are passing through the filter, making the sound darker.");
        }

        previousBrightness = value;
    }
    else if (slider == &sizeSlider)
    {
        if (value > previousSize)
        {
            updateWhatChanged(
                "Size",
                "increased",
                "The two oscillators are further apart in pitch, creating a thicker and wider sound.");
        }
        else
        {
            updateWhatChanged(
                "Size",
                "decreased",
                "The two oscillators are closer together in pitch, creating a thinner and more focused sound.");
        }

        previousSize = value;
    }
    else if (slider == &distanceSlider)
    {
        if (value > previousDistance)
        {
            updateWhatChanged(
                "Distance",
                "increased",
                "More reverb is being added, making the sound feel further away.");
        }
        else
        {
            updateWhatChanged(
                "Distance",
                "decreased",
                "Less reverb is being added, making the sound feel closer and drier.");
        }

        previousDistance = value;
    }
    else if (slider == &movementRateSlider)
    {
        if (value > previousMovementRate)
        {
            updateWhatChanged(
                "Movement Rate",
                "increased",
                "The wobble or pulse is happening faster.");
        }
        else
        {
            updateWhatChanged(
                "Movement Rate",
                "decreased",
                "The wobble or pulse is happening more slowly.");
        }

        previousMovementRate = value;
    }
    else if (slider == &movementDepthSlider)
    {
        if (value > previousMovementDepth)
        {
            updateWhatChanged(
                "Movement Depth",
                "increased",
                "The wobble or pulse effect is stronger and more noticeable.");
        }
        else
        {
            updateWhatChanged(
                "Movement Depth",
                "decreased",
                "The wobble or pulse effect is more subtle.");
        }

        previousMovementDepth = value;
    }
    else if (slider == &attackSlider)
    {
        if (value > previousAttack)
        {
            updateWhatChanged(
                "Attack",
                "increased",
                "The sound fades in more slowly, making the start softer.");
        }
        else
        {
            updateWhatChanged(
                "Attack",
                "decreased",
                "The sound starts more quickly, making it feel more immediate and punchy.");
        }

        previousAttack = value;
    }
    else if (slider == &decaySlider)
    {
        if (value > previousDecay)
        {
            updateWhatChanged(
                "Decay",
                "increased",
                "The sound takes longer to fall after the initial hit.");
        }
        else
        {
            updateWhatChanged(
                "Decay",
                "decreased",
                "The sound falls away more quickly, making it shorter and more plucky.");
        }

        previousDecay = value;
    }
    else if (slider == &sustainSlider)
    {
        if (value > previousSustain)
        {
            updateWhatChanged(
                "Sustain",
                "increased",
                "The held part of the sound is louder and more continuous.");
        }
        else
        {
            updateWhatChanged(
                "Sustain",
                "decreased",
                "The held part of the sound is quieter, making the sound feel shorter or more percussive.");
        }

        previousSustain = value;
    }
    else if (slider == &releaseSlider)
    {
        if (value > previousRelease)
        {
            updateWhatChanged(
                "Release",
                "increased",
                "The sound fades out for longer after the key is released.");
        }
        else
        {
            updateWhatChanged(
                "Release",
                "decreased",
                "The sound stops more quickly after the key is released.");
        }

        previousRelease = value;
    }
}
/**
 * Updates the contextual feedback panel following a participant interaction.
 *
 * @param controlName Perceptual name of the modified control.
 * @param action      Direction of the modification, for example "increased".
 * @param result      Expected audible consequence of the modification.
 */
void MainComponent::updateWhatChanged(const juce::String& controlName,
    const juce::String& action,
    const juce::String& result)
{
    juce::String text;

    text << "What Changed?\n\n";
    text << "You " << action << " " << controlName << ".\n\n";
    text << "Result:\n" << result;

    whatChangedLabel.setText(text, juce::dontSendNotification);
}

//==============================================================================
/**
 * Restores the neutral reference patch used as a common starting condition.
 *
 * Values are assigned without sending slider notifications so that resetting
 * the synthesiser is treated as one application action rather than a sequence
 * of participant-generated parameter changes.
 */
void MainComponent::initPatch()
{
    logAction("Loaded Init Patch");

    presetExplanationLabel.setText(
        "Init Patch: A basic starting point with no special characteristics.",
        juce::dontSendNotification);

    // Reset synthesis parameters that are not directly exposed through the
    // perceptually labelled interface.
    currentWaveform = 1;
    currentNoise = 0.0f;
    currentResonance = 0.0f;
    currentLfoTarget = 1;

    brightnessSlider.setValue(8000.0, juce::dontSendNotification);
    sizeSlider.setValue(5.0, juce::dontSendNotification);

    attackSlider.setValue(0.05, juce::dontSendNotification);
    decaySlider.setValue(0.2, juce::dontSendNotification);
    sustainSlider.setValue(0.7, juce::dontSendNotification);
    releaseSlider.setValue(0.4, juce::dontSendNotification);

    movementRateSlider.setValue(5.0, juce::dontSendNotification);
    movementDepthSlider.setValue(0.0, juce::dontSendNotification);

    distanceSlider.setValue(0.0, juce::dontSendNotification);

    whatChangedLabel.setText("", juce::dontSendNotification);

    // Synchronise comparison values so that the next participant adjustment is
    // measured against the newly loaded patch.
    syncPreviousValues();
}

/**
 * Applies a predefined synthesis configuration.
 *
 * Presets provide recognisable auditory reference points while retaining the
 * same perceptually labelled controls used during the evaluation.
 *
 * @param presetNumber ComboBox identifier of the requested preset.
 */
void MainComponent::applyPreset(int presetNumber)
{
    juce::String presetName;

    // Configure waveform, filter, modulation, envelope, detuning, and reverb as
    // a coherent patch. Slider notifications are suppressed because preset
    // loading must not be interpreted as manual participant adjustment.
    switch (presetNumber)
    {
    case 1:
        initPatch();
        return;

    case 2:
        // Bass: harmonically rich waveform, low cutoff, and short envelope.
        presetName = "Bass";
        presetExplanationLabel.setText("Preset: Bass - A dark, thick sound. Low brightness and a short punchy envelope.", juce::dontSendNotification);

        currentWaveform = 2;
        currentResonance = 0.55f;
        currentNoise = 0.02f;
        currentLfoTarget = 3;

        brightnessSlider.setValue(800.0, juce::dontSendNotification);
        attackSlider.setValue(0.005, juce::dontSendNotification);
        decaySlider.setValue(0.15, juce::dontSendNotification);
        sustainSlider.setValue(0.65, juce::dontSendNotification);
        releaseSlider.setValue(0.12, juce::dontSendNotification);
        movementRateSlider.setValue(4.0, juce::dontSendNotification);
        movementDepthSlider.setValue(0.15, juce::dontSendNotification);
        sizeSlider.setValue(3.0, juce::dontSendNotification);
        distanceSlider.setValue(0.0, juce::dontSendNotification);
        break;

    case 3:
        // Lead: brighter spectrum with pitch modulation and moderate ambience.
        presetName = "Lead";
        presetExplanationLabel.setText("Preset: Lead - A bright, cutting sound with vibrato movement and slight distance.", juce::dontSendNotification);

        currentWaveform = 3;
        currentResonance = 0.35f;
        currentNoise = 0.0f;
        currentLfoTarget = 2;

        brightnessSlider.setValue(6000.0, juce::dontSendNotification);
        attackSlider.setValue(0.01, juce::dontSendNotification);
        decaySlider.setValue(0.2, juce::dontSendNotification);
        sustainSlider.setValue(0.85, juce::dontSendNotification);
        releaseSlider.setValue(0.25, juce::dontSendNotification);
        movementRateSlider.setValue(5.5, juce::dontSendNotification);
        movementDepthSlider.setValue(0.2, juce::dontSendNotification);
        sizeSlider.setValue(8.0, juce::dontSendNotification);
        distanceSlider.setValue(0.2, juce::dontSendNotification);
        break;

    case 4:
        // Pad: slow envelope, greater detuning, filter movement, and reverb.
        presetName = "Pad";
        presetExplanationLabel.setText("Preset: Pad - A soft, wide sound. Slow attack, large size, and heavy distance.", juce::dontSendNotification);

        currentWaveform = 4;
        currentResonance = 0.15f;
        currentNoise = 0.08f;
        currentLfoTarget = 3;

        brightnessSlider.setValue(2000.0, juce::dontSendNotification);
        attackSlider.setValue(1.2, juce::dontSendNotification);
        decaySlider.setValue(0.8, juce::dontSendNotification);
        sustainSlider.setValue(0.8, juce::dontSendNotification);
        releaseSlider.setValue(1.5, juce::dontSendNotification);
        movementRateSlider.setValue(0.8, juce::dontSendNotification);
        movementDepthSlider.setValue(0.25, juce::dontSendNotification);
        sizeSlider.setValue(14.0, juce::dontSendNotification);
        distanceSlider.setValue(0.6, juce::dontSendNotification);
        break;

    case 5:
        // Pluck: rapid transient, short decay, and low sustain.
        presetName = "Pluck";
        presetExplanationLabel.setText("Preset: Pluck - Fast attack and short decay create a plucky, percussive sound.", juce::dontSendNotification);

        currentWaveform = 1;
        currentResonance = 0.45f;
        currentNoise = 0.04f;
        currentLfoTarget = 1;

        brightnessSlider.setValue(5000.0, juce::dontSendNotification);
        attackSlider.setValue(0.001, juce::dontSendNotification);
        decaySlider.setValue(0.1, juce::dontSendNotification);
        sustainSlider.setValue(0.25, juce::dontSendNotification);
        releaseSlider.setValue(0.08, juce::dontSendNotification);
        movementRateSlider.setValue(8.0, juce::dontSendNotification);
        movementDepthSlider.setValue(0.1, juce::dontSendNotification);
        sizeSlider.setValue(1.0, juce::dontSendNotification);
        distanceSlider.setValue(0.1, juce::dontSendNotification);
        break;
    }

    logAction("Loaded Preset: " + presetName);
    whatChangedLabel.setText("What Changed?\nLoaded " + presetName + " preset.", juce::dontSendNotification);

    // Reset the comparison baseline to the selected preset values.
    syncPreviousValues();
}

//==============================================================================
/**
 * Prepares sample-rate-dependent DSP components before audio rendering begins.
 *
 * @param samplesPerBlockExpected Expected host buffer size. The implementation
 *        does not require block-size-specific allocation.
 * @param sampleRate Active audio-device sample rate in hertz.
 */
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    lfo.setSampleRate(sampleRate);
    reverb.reset();

    // Propagate sample-rate and filter-envelope configuration to every voice so
    // that time-dependent behaviour remains consistent across audio devices.
    for (auto& voice : voices)
    {
        voice.setSampleRate(sampleRate);
        voice.setFilterEnvAmount(filterEnvAmount);
        voice.setFilterEnvelopeParameters(filterAttack, filterDecay, filterSustain, filterRelease);
    }
}

/**
 * Renders the next block of stereo audio.
 *
 * For each output sample, the method generates the current LFO value, maps it
 * to amplitude, pitch, or filter modulation, accumulates active voices,
 * normalises the combined signal, applies stereo reverb, and stores the dry
 * sample for oscilloscope visualisation. No dynamic allocation or file I/O is
 * intentionally performed in this real-time callback.
 *
 * @param bufferToFill Output buffer region supplied by JUCE.
 */
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Clear the host buffer while synthesis is disabled so that stale samples
    // cannot reach the output device.
    if (!powerButton.getToggleState())
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);

    auto* rightBuffer = bufferToFill.buffer->getNumChannels() > 1
        ? bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample)
        : nullptr;

    // Map the perceptual "Distance" control to room size and wet/dry balance.
    // A substantial dry component is retained to preserve source intelligibility.
    auto revAmt = (float)distanceSlider.getValue();

    reverbParameters.roomSize = revAmt;
    reverbParameters.wetLevel = revAmt * 0.35f;
    reverbParameters.dryLevel = 1.0f - (revAmt * 0.25f);
    reverbParameters.width = 1.0f;
    reverbParameters.damping = 0.4f;

    reverb.setParameters(reverbParameters);

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        float oscillatorValue = 0.0f;

        lfo.setRate(movementRateSlider.getValue());

        auto lfoValue = lfo.getNextValue();
        auto lfoNormalised = (lfoValue + 1.0f) * 0.5f;
        auto lfoDepth = (float)movementDepthSlider.getValue();

        float tremoloGain = 1.0f;
        float pitchModulationCents = 0.0f;
        float filterCutoff = (float)brightnessSlider.getValue();

        // The selected preset determines whether the shared LFO produces
        // tremolo, vibrato, or filter-cutoff modulation.
        if (currentLfoTarget == 1)
            tremoloGain = 1.0f - (lfoDepth * lfoNormalised);
        else if (currentLfoTarget == 2)
            pitchModulationCents = lfoValue * lfoDepth * 50.0f;
        else if (currentLfoTarget == 3)
            filterCutoff = juce::jlimit(200.0f, 18000.0f, filterCutoff + (lfoValue * lfoDepth * 4000.0f));

        // Sum independently enveloped voices after applying the current global
        // synthesis and modulation parameters.
        for (auto& voice : voices)
        {
            voice.setWaveform(currentWaveform);
            voice.setEnvelopeParameters((float)attackSlider.getValue(),
                (float)decaySlider.getValue(),
                (float)sustainSlider.getValue(),
                (float)releaseSlider.getValue());

            voice.setFilterCutoff(filterCutoff);
            voice.setFilterResonance(currentResonance);
            voice.setNoiseAmount(currentNoise);
            voice.setDetuneCents((float)sizeSlider.getValue());
            voice.setPitchModulationCents(pitchModulationCents);

            oscillatorValue += voice.getNextSample();
        }

        // Count active voices so that polyphonic passages can be level-normalised.
        int activeVoiceCount = 0;

        for (auto& voice : voices)
            if (voice.isActive())
                ++activeVoiceCount;

        if (activeVoiceCount > 0)
            oscillatorValue /= (float)activeVoiceCount;

        // Retain additional headroom before modulation and effects to reduce
        // clipping under resonant or highly detuned configurations.
        oscillatorValue *= 0.5f;

        auto rawSample = oscillatorValue * 0.4f * tremoloGain;

        float leftSample = rawSample;
        float rightSample = rawSample;

        reverb.processStereo(&leftSample, &rightSample, 1);

        leftBuffer[sample] = leftSample;

        if (rightBuffer != nullptr)
            rightBuffer[sample] = rightSample;

        // Store the pre-reverb signal so that the display represents the source
        // waveform rather than the effect tail.
        waveformBuffer[(size_t)waveformBufferIndex] = rawSample;
        waveformBufferIndex = (waveformBufferIndex + 1) % (int)waveformBuffer.size();
    }
}

void MainComponent::releaseResources()
{
}

//==============================================================================
/**
 * Forwards hardware MIDI events to JUCE's keyboard state so that physical and
 * on-screen keyboard input share the same note-handling path.
 */
void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    keyboardState.processNextMidiEvent(message);
}

/**
 * Assigns an incoming note to an available synthesis voice and initialises its
 * current timbral and envelope parameters.
 */
void MainComponent::handleNoteOn(juce::MidiKeyboardState* source,
    int midiChannel,
    int midiNoteNumber,
    float velocity)
{
    auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    // Select the first inactive voice; when all voices are occupied, the
    // allocation method applies its deterministic voice-stealing fallback.
    auto* voice = findFreeVoice();

    if (voice != nullptr)
    {
        voice->setEnvelopeParameters((float)attackSlider.getValue(),
            (float)decaySlider.getValue(),
            (float)sustainSlider.getValue(),
            (float)releaseSlider.getValue());

        voice->setFilterCutoff((float)brightnessSlider.getValue());
        voice->setFilterResonance(currentResonance);
        voice->setNoiseAmount(currentNoise);
        voice->setDetuneCents((float)sizeSlider.getValue());
        voice->startNote(midiNoteNumber, frequency);
    }

    noteOn = true;
}

/**
 * Releases every voice assigned to the specified MIDI note and updates the
 * component-level note state.
 */
void MainComponent::handleNoteOff(juce::MidiKeyboardState* source,
    int midiChannel,
    int midiNoteNumber,
    float velocity)
{
    for (auto& voice : voices)
        if (voice.getMidiNoteNumber() == midiNoteNumber)
            voice.stopNote();

    bool anyVoiceActive = false;

    for (auto& voice : voices)
        if (voice.isActive())
            anyVoiceActive = true;

    noteOn = anyVoiceActive;
}

/**
 * Returns the first inactive voice in the fixed polyphony pool.
 *
 * If all voices are active, voice zero is reused as a deterministic fallback;
 * the implementation does not track voice age for oldest-note stealing.
 */
Voice* MainComponent::findFreeVoice()
{
    for (auto& voice : voices)
        if (!voice.isActive())
            return &voice;

    return &voices[0];
}

//==============================================================================
/**
 * Requests regular repaints for the oscilloscope and other dynamic visual
 * elements. Audio generation remains independent of the GUI refresh rate.
 */
void MainComponent::timerCallback()
{
    repaint();
}

//==============================================================================
/**
 * Renders the synthesiser interface and oscilloscope.
 *
 * Decorative elements are drawn procedurally to provide a consistent,
 * hardware-inspired design without reliance on external image assets.
 */
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(9, 11, 13));

    const int woodW = 24;

    auto leftWood = juce::Rectangle<int>(0, 0, woodW, getHeight());
    auto rightWood = juce::Rectangle<int>(getWidth() - woodW, 0, woodW, getHeight());

    auto drawWood = [&](juce::Rectangle<int> r)
        {
            juce::ColourGradient woodGrad(
                juce::Colour::fromRGB(90, 48, 24),
                (float)r.getX(),
                0.0f,
                juce::Colour::fromRGB(38, 20, 10),
                (float)r.getRight(),
                0.0f,
                false);

            g.setGradientFill(woodGrad);
            g.fillRect(r);

            g.setColour(juce::Colour::fromRGB(135, 78, 38).withAlpha(0.45f));

            for (int y = 0; y < getHeight(); y += 18)
                g.drawLine((float)r.getX() + 4.0f, (float)y, (float)r.getRight() - 4.0f, (float)y + 12.0f, 1.0f);

            g.setColour(juce::Colours::black.withAlpha(0.8f));
            g.drawRect(r, 2);
        };

    drawWood(leftWood);
    drawWood(rightWood);

    auto body = juce::Rectangle<int>(woodW, 0, getWidth() - woodW * 2, getHeight());

    juce::ColourGradient bg(
        juce::Colour::fromRGB(18, 21, 24),
        0.0f,
        0.0f,
        juce::Colour::fromRGB(5, 6, 7),
        0.0f,
        (float)getHeight(),
        false);

    g.setGradientFill(bg);
    g.fillRect(body);

    auto drawPanel = [&](juce::Rectangle<int> r)
        {
            auto rf = r.toFloat();

            g.setColour(juce::Colours::black.withAlpha(0.65f));
            g.fillRoundedRectangle(rf.translated(3.0f, 4.0f), 5.0f);

            juce::ColourGradient panelGrad(
                juce::Colour::fromRGB(34, 37, 39),
                rf.getCentreX(),
                rf.getY(),
                juce::Colour::fromRGB(11, 13, 14),
                rf.getCentreX(),
                rf.getBottom(),
                false);

            g.setGradientFill(panelGrad);
            g.fillRoundedRectangle(rf, 5.0f);

            g.setColour(juce::Colours::white.withAlpha(0.12f));
            g.drawRoundedRectangle(rf.reduced(1.0f), 5.0f, 1.0f);

            g.setColour(juce::Colours::black.withAlpha(0.90f));
            g.drawRoundedRectangle(rf, 5.0f, 2.0f);

            // Draw recessed corner fasteners to reinforce the hardware-panel metaphor.
            g.setColour(juce::Colours::black.withAlpha(0.95f));
            g.fillEllipse((float)r.getX() + 10.0f, (float)r.getY() + 10.0f, 10.0f, 10.0f);
            g.fillEllipse((float)r.getRight() - 20.0f, (float)r.getY() + 10.0f, 10.0f, 10.0f);
            g.fillEllipse((float)r.getX() + 10.0f, (float)r.getBottom() - 20.0f, 10.0f, 10.0f);
            g.fillEllipse((float)r.getRight() - 20.0f, (float)r.getBottom() - 20.0f, 10.0f, 10.0f);

            g.setColour(juce::Colours::white.withAlpha(0.18f));
            g.drawEllipse((float)r.getX() + 11.0f, (float)r.getY() + 11.0f, 8.0f, 8.0f, 1.0f);
            g.drawEllipse((float)r.getRight() - 19.0f, (float)r.getY() + 11.0f, 8.0f, 8.0f, 1.0f);
            g.drawEllipse((float)r.getX() + 11.0f, (float)r.getBottom() - 19.0f, 8.0f, 8.0f, 1.0f);
            g.drawEllipse((float)r.getRight() - 19.0f, (float)r.getBottom() - 19.0f, 8.0f, 8.0f, 1.0f);
        };

    const int contentX = 44;
    const int gap = 18;

    juce::Rectangle<int> brightnessPanel(contentX, 125, 360, 250);
    juce::Rectangle<int> sizePanel(contentX + 360 + gap, 125, 320, 250);
    juce::Rectangle<int> punchPanel(contentX + 360 + gap + 320 + gap, 125, 410, 250);

    juce::Rectangle<int> movementPanel(contentX, 395, 360, 230);
    juce::Rectangle<int> distancePanel(contentX + 360 + gap, 395, 320, 230);
    juce::Rectangle<int> changedPanel(contentX + 360 + gap + 320 + gap, 395, 410, 230);

    drawPanel(brightnessPanel);
    drawPanel(sizePanel);
    drawPanel(punchPanel);
    drawPanel(movementPanel);
    drawPanel(distancePanel);

    auto changedF = changedPanel.toFloat();

    g.setColour(juce::Colours::black.withAlpha(0.75f));
    g.fillRoundedRectangle(changedF.translated(3.0f, 4.0f), 6.0f);

    g.setColour(juce::Colour::fromRGB(3, 10, 13));
    g.fillRoundedRectangle(changedF, 6.0f);

    g.setColour(juce::Colours::cyan.withAlpha(0.75f));
    g.drawRoundedRectangle(changedF, 6.0f, 1.5f);

    g.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    g.setColour(juce::Colours::cyan);

    g.drawFittedText(
        "SYNTH",
        -15,
        38,
        getWidth(),
        50,
        juce::Justification::centred,
        1);

    auto drawTwoLineTitle = [&](juce::String main, juce::String sub, juce::Rectangle<int> r)
        {
            g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
            g.setColour(juce::Colours::white);
            g.drawText(main.toUpperCase(), r.getX(), r.getY() + 20, r.getWidth(), 26, juce::Justification::centred);

            g.setFont(juce::FontOptions(15.0f));
            g.setColour(juce::Colours::lightgrey);
            g.drawText(sub, r.getX(), r.getY() + 48, r.getWidth(), 22, juce::Justification::centred);
        };

    drawTwoLineTitle("Brightness", "(Cutoff)", brightnessPanel);
    drawTwoLineTitle("Size", "(Detune)", sizePanel);
    drawTwoLineTitle("Punch", "(Envelope)", punchPanel);
    drawTwoLineTitle("Movement", "(LFO)", movementPanel);
    drawTwoLineTitle("Distance", "(Reverb)", distancePanel);

    g.setFont(juce::FontOptions(14.0f));
    g.setColour(juce::Colours::white);
    g.drawText("ATTACK", attackSlider.getX(), attackSlider.getY() - 24, attackSlider.getWidth(), 22, juce::Justification::centred);
    g.drawText("DECAY", decaySlider.getX(), decaySlider.getY() - 24, decaySlider.getWidth(), 22, juce::Justification::centred);
    g.drawText("SUSTAIN", sustainSlider.getX(), sustainSlider.getY() - 24, sustainSlider.getWidth(), 22, juce::Justification::centred);
    g.drawText("RELEASE", releaseSlider.getX(), releaseSlider.getY() - 24, releaseSlider.getWidth(), 22, juce::Justification::centred);

    g.drawText("RATE", movementRateSlider.getX(), movementRateSlider.getY() - 24, movementRateSlider.getWidth(), 22, juce::Justification::centred);
    g.drawText("DEPTH", movementDepthSlider.getX(), movementDepthSlider.getY() - 24, movementDepthSlider.getWidth(), 22, juce::Justification::centred);

    // Render a scrolling time-domain representation of the recent dry output.
    auto scopeArea = juce::Rectangle<int>(44, 637, getWidth() - 88, 93);
    auto sf = scopeArea.toFloat();

    g.setColour(juce::Colours::black.withAlpha(0.75f));
    g.fillRoundedRectangle(sf.translated(2.0f, 3.0f), 4.0f);

    g.setColour(juce::Colour::fromRGB(3, 8, 10));
    g.fillRoundedRectangle(sf, 4.0f);

    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawRoundedRectangle(sf, 4.0f, 1.0f);

    g.setColour(juce::Colours::cyan.withAlpha(0.12f));

    // Draw a fixed reference grid to aid interpretation of waveform amplitude
    // and temporal variation.
    for (int x = scopeArea.getX(); x < scopeArea.getRight(); x += 32)
        g.drawVerticalLine(x, (float)scopeArea.getY(), (float)scopeArea.getBottom());

    for (int y = scopeArea.getY(); y < scopeArea.getBottom(); y += 22)
        g.drawHorizontalLine(y, (float)scopeArea.getX(), (float)scopeArea.getRight());

    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.setColour(juce::Colours::cyan);
    g.drawText(
        "OSCILLOSCOPE",
        scopeArea.getX() + 38,
        scopeArea.getY() + 6,
        200,
        20,
        juce::Justification::left);

    g.setColour(juce::Colours::lightgrey.withAlpha(0.8f));
    g.drawText("1.0", scopeArea.getX() + 8, scopeArea.getY() + 8, 30, 18, juce::Justification::left);
    g.drawText("0.0", scopeArea.getX() + 8, scopeArea.getCentreY() - 9, 30, 18, juce::Justification::left);
    g.drawText("-1.0", scopeArea.getX() + 8, scopeArea.getBottom() - 26, 35, 18, juce::Justification::left);

    // Read the circular buffer from its oldest sample to its newest so that the
    // displayed waveform progresses chronologically from left to right.
    juce::Path waveformPath;
    auto centreY = (float)scopeArea.getCentreY();
    auto width = (float)scopeArea.getWidth();
    auto height = (float)scopeArea.getHeight();

    for (size_t i = 0; i < waveformBuffer.size(); ++i)
    {
        auto index = (waveformBufferIndex + (int)i) % (int)waveformBuffer.size();
        auto sampleValue = waveformBuffer[(size_t)index];

        auto x = scopeArea.getX() + ((float)i / (float)waveformBuffer.size()) * width;
        auto y = centreY - sampleValue * height * 1.8f;

        if (i == 0)
            waveformPath.startNewSubPath(x, y);
        else
            waveformPath.lineTo(x, y);
    }

    // Layer translucent strokes beneath the main trace to improve visibility
    // while retaining a narrow and precise waveform line.
    g.setColour(juce::Colours::cyan.withAlpha(0.18f));
    g.strokePath(waveformPath, juce::PathStrokeType(6.0f));

    g.setColour(juce::Colours::cyan.withAlpha(0.45f));
    g.strokePath(waveformPath, juce::PathStrokeType(3.0f));

    g.setColour(juce::Colours::cyan);
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
}

/**
 * Assigns bounds to child components using the fixed 1200 x 850 evaluation
 * layout. A consistent arrangement ensures that each participant encounters
 * the same control positions during the study.
 */
void MainComponent::resized()
{
    const int contentX = 44;
    const int gap = 18;

    const int topY = 40;
    const int buttonW = 160;
    const int comboW = 280;
    const int controlH = 42;
    const int sideMargin = 52;
    const int gapTop = 20;

    // Position global application controls separately from the perceptual
    // sound-design panels.
    powerButton.setBounds(
        sideMargin,
        topY,
        buttonW,
        controlH);

    presetSelector.setBounds(
        powerButton.getRight() + gapTop,
        topY,
        comboW,
        controlH);

    initButton.setBounds(
        getWidth() - sideMargin - buttonW,
        topY,
        buttonW,
        controlH);

    midiInputList.setBounds(
        initButton.getX() - gapTop - comboW,
        topY,
        comboW,
        controlH);

    presetExplanationLabel.setBounds(
        52,
        79,
        getWidth() - 104,
        30);

    taskSelector.setBounds(
        getWidth() - 360,
        88,
        220,
        30);

    taskFinishedButton.setBounds(
        getWidth() - 130,
        84,
        100,
        30);

    // Align interactive controls with the panel geometry drawn in paint().
    brightnessSlider.setBounds(contentX + 115, 205, 130, 130);
    sizeSlider.setBounds(contentX + 360 + gap + 95, 205, 130, 130);

    attackSlider.setBounds(contentX + 360 + gap + 320 + gap + 45, 220, 72, 145);
    decaySlider.setBounds(contentX + 360 + gap + 320 + gap + 130, 220, 72, 145);
    sustainSlider.setBounds(contentX + 360 + gap + 320 + gap + 215, 220, 72, 145);
    releaseSlider.setBounds(contentX + 360 + gap + 320 + gap + 300, 220, 72, 145);

    movementRateSlider.setBounds(contentX + 65, 485, 110, 110);
    movementDepthSlider.setBounds(contentX + 205, 485, 110, 110);

    distanceSlider.setBounds(contentX + 360 + gap + 100, 475, 130, 130);

    whatChangedLabel.setBounds(
        contentX + 360 + gap + 320 + gap + 28,
        426,
        355,
        180);

    // Reserve the lower section for direct note input while preserving usable
    // key width across the selected MIDI range.
    keyboardComponent.setAvailableRange(24, 120);
    keyboardComponent.setKeyWidth(21.7f);
    keyboardComponent.setBounds(
        52,
        732,
        getWidth() - 104,
        100);
}

/**
 * Stores the current control state as the baseline for subsequent
 * direction-of-change feedback.
 */
void MainComponent::syncPreviousValues()
{
    previousBrightness = brightnessSlider.getValue();
    previousSize = sizeSlider.getValue();
    previousDistance = distanceSlider.getValue();

    previousMovementRate = movementRateSlider.getValue();
    previousMovementDepth = movementDepthSlider.getValue();

    previousAttack = attackSlider.getValue();
    previousDecay = decaySlider.getValue();
    previousSustain = sustainSlider.getValue();
    previousRelease = releaseSlider.getValue();
}

/**
 * Converts the selected task identifier into the label written to the
 * participant interaction log.
 *
 * @return Human-readable task label, or "Unknown Task" for an invalid ID.
 */
juce::String MainComponent::getCurrentTaskName() const
{
    switch (taskSelector.getSelectedId())
    {
    case 1: return "Task 1 - Brightness";
    case 2: return "Task 2 - Warmth";
    case 3: return "Task 3 - Movement";
    case 4: return "Task 4 - Distance";
    case 5: return "Task 5 - Pad";
    }

    return "Unknown Task";
}