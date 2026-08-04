#include "MainComponent_A.h"

//==============================================================================
MainComponent::MainComponent()
{
    setLookAndFeel(&customLookAndFeel);
    setSize(1200, 850);

    voices.resize(8);
    waveformBuffer.resize(512, 0.0f);

    //==========================================================================
    // Study logging used to record participant interactions and task timings.

    auto timestamp = juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S");

    logFile = juce::File::getSpecialLocation(
        juce::File::userDocumentsDirectory)
        .getChildFile("SynthStudy_A_" + timestamp + ".txt");

    logAction("===== SESSION STARTED =====");

    //==========================================================================
    // Top Bar

    addAndMakeVisible(powerButton);
    powerButton.setButtonText("Sound On");
    powerButton.setToggleState(false, juce::dontSendNotification);
    powerButton.onClick = [this]
        {
            noteOn = powerButton.getToggleState();

            logAction(powerButton.getToggleState()
                ? "Power ON"
                : "Power OFF");
        };

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
            logAction("Pressed Reset");
            initPatch();
            presetSelector.setSelectedId(1, juce::dontSendNotification);
        };

    //==========================================================================
    // Task Logger

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

    //==========================================================================
    // Oscillator
    setupRotary(detuneSlider, "Detune", 0.0, 25.0, 0.1, 5.0);
    setupRotary(noiseSlider, "Noise", 0.0, 1.0, 0.01, 0.0);

    waveformSelector.addItem("Sine", 1);
    waveformSelector.addItem("Square", 2);
    waveformSelector.addItem("Saw", 3);
    waveformSelector.addItem("Triangle", 4);
    waveformSelector.setSelectedId(1);
    // Hidden in Version A to ensure both interfaces expose the same functionality.

    //==========================================================================
    // Filter
    setupRotary(filterSlider, "Cutoff (Hz)", 200.0, 18000.0, 1.0, 8000.0);
    setupRotary(resonanceSlider, "Resonance", 0.0, 0.95, 0.01, 0.0);

    //==========================================================================
    // Amplitude Envelope (Faders)
    setupFader(attackSlider, "Attack", 0.001, 2.0, 0.001, 0.05);
    setupFader(decaySlider, "Decay", 0.001, 2.0, 0.001, 0.2);
    setupFader(sustainSlider, "Sustain", 0.0, 1.0, 0.01, 0.7);
    setupFader(releaseSlider, "Release", 0.001, 2.0, 0.001, 0.4);

    //==========================================================================
    // LFO
    setupRotary(lfoRateSlider, "Rate", 0.1, 20.0, 0.1, 5.0);
    setupRotary(lfoDepthSlider, "Depth", 0.0, 1.0, 0.01, 0.0);

    lfoTargetSelector.addItem("Volume", 1);
    lfoTargetSelector.addItem("Pitch", 2);
    lfoTargetSelector.addItem("Filter", 3);
    lfoTargetSelector.setSelectedId(1);

    //==========================================================================
    // Reverb
    setupRotary(reverbSlider, "Reverb", 0.0, 1.0, 0.01, 0.0);

    //==========================================================================
    // MIDI
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
            logAction("Selected MIDI Input: " + midiInputList.getText());
            if (selectedId > 1)
            {
                auto deviceIndex = selectedId - 2;
                if (deviceIndex >= 0 && deviceIndex < midiInputs.size())
                    midiInput = juce::MidiInput::openDevice(midiInputs[deviceIndex].identifier, this);
                if (midiInput != nullptr) midiInput->start();
            }
        };

    keyboardState.addListener(this);
    addAndMakeVisible(keyboardComponent);

    setAudioChannels(0, 2);
    startTimerHz(60);

    initPatch();
}

MainComponent::~MainComponent()
{
    logAction("===== SESSION ENDED =====");
    setLookAndFeel(nullptr);

    stopTimer();
    keyboardState.removeListener(this);
    midiInput.reset();
    shutdownAudio();
}

void MainComponent::setupRotary(juce::Slider& slider, const juce::String& name, double min, double max, double interval, double defaultVal)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider.setRange(min, max, interval);
    slider.setValue(defaultVal);
    slider.setName(name);

    auto* s = &slider;

    slider.onDragEnd = [this, s]
        {
            if (!isInitialising)
            {
                logAction(
                    getCurrentTaskName()
                    + " | Adjusted "
                    + s->getName()
                    + " -> "
                    + juce::String(s->getValue(), 2));
            }
        };

    addAndMakeVisible(slider);
}

void MainComponent::setupFader(juce::Slider& slider, const juce::String& name, double min, double max, double interval, double defaultVal)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider.setRange(min, max, interval);
    slider.setValue(defaultVal);
    slider.setName(name);

    auto* s = &slider;

    slider.onDragEnd = [this, s]
        {
            if (!isInitialising)
            {
                logAction(
                    getCurrentTaskName()
                    + " | Adjusted "
                    + s->getName()
                    + " -> "
                    + juce::String(s->getValue(), 2));
            }
        };

    addAndMakeVisible(slider);
}

void MainComponent::initPatch()
{
    juce::ScopedValueSetter<bool> svs(isInitialising, true);
    logAction("Loaded Init Patch");

    waveformSelector.setSelectedId(1);
    detuneSlider.setValue(5.0, juce::dontSendNotification);
    noiseSlider.setValue(0.0, juce::dontSendNotification);

    filterSlider.setValue(8000.0, juce::dontSendNotification);
    resonanceSlider.setValue(0.0, juce::dontSendNotification);

    attackSlider.setValue(0.05, juce::dontSendNotification);
    decaySlider.setValue(0.2, juce::dontSendNotification);
    sustainSlider.setValue(0.7, juce::dontSendNotification);
    releaseSlider.setValue(0.4, juce::dontSendNotification);

    lfoRateSlider.setValue(5.0, juce::dontSendNotification);
    lfoDepthSlider.setValue(0.0, juce::dontSendNotification);

    reverbSlider.setValue(0.0, juce::dontSendNotification);
}

void MainComponent::applyPreset(int presetNumber)
{
    juce::ScopedValueSetter<bool> svs(isInitialising, true);
    juce::String presetName;

    switch (presetNumber)
    {
    case 1: initPatch(); break;
    case 2: // Bass
        presetName = "Bass";
        waveformSelector.setSelectedId(2);
        filterSlider.setValue(800.0);
        resonanceSlider.setValue(0.55);
        noiseSlider.setValue(0.02);
        attackSlider.setValue(0.005);
        decaySlider.setValue(0.15);
        sustainSlider.setValue(0.65);
        releaseSlider.setValue(0.12);
        lfoRateSlider.setValue(4.0);
        lfoDepthSlider.setValue(0.15);
        lfoTargetSelector.setSelectedId(3);
        detuneSlider.setValue(3.0);
        reverbSlider.setValue(0.0);
        break;
    case 3: // Lead
        presetName = "Lead";
        waveformSelector.setSelectedId(3);
        filterSlider.setValue(6000.0);
        resonanceSlider.setValue(0.35);
        noiseSlider.setValue(0.00);
        attackSlider.setValue(0.01);
        decaySlider.setValue(0.2);
        sustainSlider.setValue(0.85);
        releaseSlider.setValue(0.25);
        lfoRateSlider.setValue(5.5);
        lfoDepthSlider.setValue(0.2);
        lfoTargetSelector.setSelectedId(2);
        detuneSlider.setValue(8.0);
        reverbSlider.setValue(0.2);
        break;
    case 4: // Pad
        presetName = "Pad";
        waveformSelector.setSelectedId(4);
        filterSlider.setValue(2000.0);
        resonanceSlider.setValue(0.15);
        noiseSlider.setValue(0.08);
        attackSlider.setValue(1.2);
        decaySlider.setValue(0.8);
        sustainSlider.setValue(0.8);
        releaseSlider.setValue(1.5);
        lfoRateSlider.setValue(0.8);
        lfoDepthSlider.setValue(0.25);
        lfoTargetSelector.setSelectedId(3);
        detuneSlider.setValue(14.0);
        reverbSlider.setValue(0.6);
        break;
    case 5: // Pluck
        presetName = "Pluck";
        waveformSelector.setSelectedId(1);
        filterSlider.setValue(5000.0);
        resonanceSlider.setValue(0.45);
        noiseSlider.setValue(0.04);
        attackSlider.setValue(0.001);
        decaySlider.setValue(0.1);
        sustainSlider.setValue(0.25);
        releaseSlider.setValue(0.08);
        lfoRateSlider.setValue(8.0);
        lfoDepthSlider.setValue(0.1);
        lfoTargetSelector.setSelectedId(1);
        detuneSlider.setValue(1.0);
        reverbSlider.setValue(0.1);
        break;
    }

    if (presetName.isNotEmpty())
        logAction("Loaded Preset: " + presetName);
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    lfo.setSampleRate(sampleRate);
    reverb.reset();

    for (auto& voice : voices)
    {
        voice.setSampleRate(sampleRate);
        voice.setFilterEnvAmount(filterEnvAmount);
        voice.setFilterEnvelopeParameters(filterAttack, filterDecay, filterSustain, filterRelease);
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (!powerButton.getToggleState())
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getNumChannels() > 1
        ? bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample) : nullptr;

    auto revAmt = (float)reverbSlider.getValue();
    reverbParameters.roomSize = revAmt;
    reverbParameters.wetLevel = revAmt * 0.35f;
    reverbParameters.dryLevel = 1.0f - (revAmt * 0.25f);
    reverbParameters.width = 1.0f;
    reverbParameters.damping = 0.4f;
    reverb.setParameters(reverbParameters);

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        float oscillatorValue = 0.0f;

        lfo.setRate(lfoRateSlider.getValue());
        auto lfoValue = lfo.getNextValue();
        auto lfoNormalised = (lfoValue + 1.0f) * 0.5f;
        auto lfoDepth = (float)lfoDepthSlider.getValue();
        auto selectedTarget = lfoTargetSelector.getSelectedId();

        float tremoloGain = 1.0f;
        float pitchModulationCents = 0.0f;
        float filterCutoff = (float)filterSlider.getValue();

        if (selectedTarget == 1) tremoloGain = 1.0f - (lfoDepth * lfoNormalised);
        else if (selectedTarget == 2) pitchModulationCents = lfoValue * lfoDepth * 50.0f;
        else if (selectedTarget == 3) filterCutoff = juce::jlimit(200.0f, 18000.0f, filterCutoff + (lfoValue * lfoDepth * 4000.0f));

        for (auto& voice : voices)
        {
            voice.setWaveform(waveformSelector.getSelectedId());
            voice.setEnvelopeParameters((float)attackSlider.getValue(), (float)decaySlider.getValue(),
                (float)sustainSlider.getValue(), (float)releaseSlider.getValue());
            voice.setFilterCutoff(filterCutoff);
            voice.setFilterResonance((float)resonanceSlider.getValue());
            voice.setNoiseAmount((float)noiseSlider.getValue());
            voice.setDetuneCents((float)detuneSlider.getValue());
            voice.setPitchModulationCents(pitchModulationCents);

            oscillatorValue += voice.getNextSample();
        }

        int activeVoiceCount = 0;
        for (auto& v : voices) if (v.isActive()) ++activeVoiceCount;
        if (activeVoiceCount > 0) oscillatorValue /= (float)activeVoiceCount;

        oscillatorValue *= 0.5f;
        auto rawSample = oscillatorValue * 0.4f * tremoloGain; // Prevent excessive output levels.

        float leftSample = rawSample;
        float rightSample = rawSample;
        reverb.processStereo(&leftSample, &rightSample, 1);

        leftBuffer[sample] = leftSample;
        if (rightBuffer != nullptr) rightBuffer[sample] = rightSample;

        waveformBuffer[(size_t)waveformBufferIndex] = rawSample;
        waveformBufferIndex = (waveformBufferIndex + 1) % (int)waveformBuffer.size();
    }
}

void MainComponent::releaseResources() {}

//==============================================================================
void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    keyboardState.processNextMidiEvent(message);
}

void MainComponent::handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    auto* voice = findFreeVoice();

    if (voice != nullptr)
    {
        voice->setEnvelopeParameters((float)attackSlider.getValue(), (float)decaySlider.getValue(),
            (float)sustainSlider.getValue(), (float)releaseSlider.getValue());
        voice->setFilterCutoff((float)filterSlider.getValue());
        voice->setFilterResonance((float)resonanceSlider.getValue());
        voice->setNoiseAmount((float)noiseSlider.getValue());
        voice->setDetuneCents((float)detuneSlider.getValue());
        voice->startNote(midiNoteNumber, frequency);
    }
    noteOn = true;
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity)
{
    for (auto& voice : voices)
        if (voice.getMidiNoteNumber() == midiNoteNumber) voice.stopNote();

    bool anyVoiceActive = false;
    for (auto& voice : voices) if (voice.isActive()) anyVoiceActive = true;
    noteOn = anyVoiceActive;
}

Voice* MainComponent::findFreeVoice()
{
    for (auto& voice : voices) if (!voice.isActive()) return &voice;
    return &voices[0];
}

//==============================================================================
void MainComponent::timerCallback() { repaint(); }

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(9, 11, 13));

    const int woodW = 24;
    const int contentX = 52;
    const int gap = 18;

    // Wooden side panels
    auto drawWood = [&](juce::Rectangle<int> r)
        {
            juce::ColourGradient woodGrad(
                juce::Colour::fromRGB(90, 48, 24), (float)r.getX(), 0.0f,
                juce::Colour::fromRGB(38, 20, 10), (float)r.getRight(), 0.0f, false);

            g.setGradientFill(woodGrad);
            g.fillRect(r);

            g.setColour(juce::Colour::fromRGB(135, 78, 38).withAlpha(0.45f));
            for (int y = 0; y < getHeight(); y += 18)
                g.drawLine((float)r.getX() + 4.0f, (float)y,
                    (float)r.getRight() - 4.0f, (float)y + 12.0f, 1.0f);

            g.setColour(juce::Colours::black.withAlpha(0.8f));
            g.drawRect(r, 2);
        };

    drawWood({ 0, 0, woodW, getHeight() });
    drawWood({ getWidth() - woodW, 0, woodW, getHeight() });

    juce::ColourGradient bg(
        juce::Colour::fromRGB(18, 21, 24), 0.0f, 0.0f,
        juce::Colour::fromRGB(5, 6, 7), 0.0f, (float)getHeight(), false);

    g.setGradientFill(bg);
    g.fillRect(woodW, 0, getWidth() - woodW * 2, getHeight());

    // Main synthesiser panels
    auto drawPanel = [&](juce::Rectangle<int> r)
        {
            auto rf = r.toFloat();

            g.setColour(juce::Colours::black.withAlpha(0.65f));
            g.fillRoundedRectangle(rf.translated(3.0f, 4.0f), 5.0f);

            juce::ColourGradient panelGrad(
                juce::Colour::fromRGB(34, 37, 39), rf.getCentreX(), rf.getY(),
                juce::Colour::fromRGB(11, 13, 14), rf.getCentreX(), rf.getBottom(), false);

            g.setGradientFill(panelGrad);
            g.fillRoundedRectangle(rf, 5.0f);

            g.setColour(juce::Colours::white.withAlpha(0.12f));
            g.drawRoundedRectangle(rf.reduced(1.0f), 5.0f, 1.0f);

            g.setColour(juce::Colours::black.withAlpha(0.90f));
            g.drawRoundedRectangle(rf, 5.0f, 2.0f);
        };

    juce::Rectangle<int> oscPanel(contentX, 130, 360, 240);
    juce::Rectangle<int> filterPanel(oscPanel.getRight() + gap, 130, 320, 240);
    juce::Rectangle<int> envPanel(filterPanel.getRight() + gap, 130, 410, 240);

    juce::Rectangle<int> lfoPanel(contentX, 388, 360, 220);
    juce::Rectangle<int> fxPanel(lfoPanel.getRight() + gap, 388, 320, 220);
    juce::Rectangle<int> infoPanel(fxPanel.getRight() + gap, 388, 410, 220);

    drawPanel(oscPanel);
    drawPanel(filterPanel);
    drawPanel(envPanel);
    drawPanel(lfoPanel);
    drawPanel(fxPanel);
    drawPanel(infoPanel);

    // Panel titles and labels
    g.setFont(juce::FontOptions(21.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);

    const int titleWidth = 120;

    g.drawFittedText(
        "SYNTH",
        getWidth() / 2 - titleWidth / 2,
        32,      // was 24/26
        titleWidth,
        42,
        juce::Justification::centred,
        1);

    auto drawTitle = [&](juce::String title, juce::Rectangle<int> r)
        {
            g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
            g.setColour(juce::Colours::white);
            g.drawText(title.toUpperCase(), r.getX(), r.getY() + 22,
                r.getWidth(), 26, juce::Justification::centred);
        };

    drawTitle("Oscillator", oscPanel);
    drawTitle("Filter", filterPanel);
    drawTitle("Amp Envelope", envPanel);
    drawTitle("LFO", lfoPanel);
    drawTitle("Effects", fxPanel);
    drawTitle("Signal Flow", infoPanel);

    g.setFont(juce::FontOptions(14.0f));
    g.setColour(juce::Colours::white);

    auto label = [&](juce::String text, juce::Component& c)
        {
            g.drawText(text.toUpperCase(), c.getX(), c.getY() - 24,
                c.getWidth(), 22, juce::Justification::centred);
        };

    label("Detune", detuneSlider);
    label("Noise", noiseSlider);
    label("Cutoff", filterSlider);
    label("Resonance", resonanceSlider);
    label("Attack", attackSlider);
    label("Decay", decaySlider);
    label("Sustain", sustainSlider);
    label("Release", releaseSlider);
    label("Rate", lfoRateSlider);
    label("Depth", lfoDepthSlider);
    label("Reverb", reverbSlider);

    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.setColour(juce::Colours::lightgrey);

    g.drawText(
        "OSC  >  VCF  >  AMP  >  FX",
        infoPanel.getX(),
        infoPanel.getY() + 82,
        infoPanel.getWidth(),
        26,
        juce::Justification::centred);

    g.setColour(juce::Colours::cyan.withAlpha(0.8f));
    g.drawLine((float)infoPanel.getX() + 60.0f,
        (float)infoPanel.getY() + 118.0f,
        (float)infoPanel.getRight() - 60.0f,
        (float)infoPanel.getY() + 118.0f,
        2.0f);

    g.setColour(juce::Colours::lightgrey.withAlpha(0.85f));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("Traditional subtractive synthesis layout",
        infoPanel.getX(), infoPanel.getY() + 145,
        infoPanel.getWidth(), 22, juce::Justification::centred);

    // Oscilloscope rendering
    auto scopeArea = juce::Rectangle<int>(52, 625, getWidth() - 104, 90);
    auto sf = scopeArea.toFloat();

    g.setColour(juce::Colours::black.withAlpha(0.75f));
    g.fillRoundedRectangle(sf.translated(2.0f, 3.0f), 4.0f);

    g.setColour(juce::Colour::fromRGB(3, 8, 10));
    g.fillRoundedRectangle(sf, 4.0f);

    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawRoundedRectangle(sf, 4.0f, 1.0f);

    g.setColour(juce::Colours::cyan.withAlpha(0.12f));

    for (int x = scopeArea.getX(); x < scopeArea.getRight(); x += 32)
        g.drawVerticalLine(x, (float)scopeArea.getY(), (float)scopeArea.getBottom());

    for (int y = scopeArea.getY(); y < scopeArea.getBottom(); y += 22)
        g.drawHorizontalLine(y, (float)scopeArea.getX(), (float)scopeArea.getRight());

    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.setColour(juce::Colours::cyan);
    g.drawText("OSCILLOSCOPE", scopeArea.getX() + 38, scopeArea.getY() + 8,
        200, 20, juce::Justification::left);

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

    g.setColour(juce::Colours::cyan.withAlpha(0.18f));
    g.strokePath(waveformPath, juce::PathStrokeType(6.0f));

    g.setColour(juce::Colours::cyan.withAlpha(0.45f));
    g.strokePath(waveformPath, juce::PathStrokeType(3.0f));

    g.setColour(juce::Colours::cyan);
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
}

void MainComponent::resized()
{
    const int contentX = 52;
    const int gap = 18;

    const int topY = 40;
    const int buttonW = 160;
    const int comboW = 280;
    const int controlH = 42;
    const int sideMargin = 60;
    const int gapTop = 28;

    // Top bar controls
    powerButton.setBounds(sideMargin, topY, buttonW, controlH);

    presetSelector.setBounds(
        powerButton.getRight() + gapTop,
        topY,
        comboW,
        controlH);

    midiInputList.setBounds(
        getWidth() - sideMargin - comboW - buttonW - gapTop,
        topY,
        comboW,
        controlH);

    initButton.setBounds(
        midiInputList.getRight() + gapTop,
        topY,
        buttonW,
        controlH);

    // Task controls
    taskSelector.setBounds(
        getWidth() - 360,
        90,
        220,
        28);

    taskFinishedButton.setBounds(
        getWidth() - 130,
        90,
        100,
        28);

    // Synthesiser control layout
    juce::Rectangle<int> oscPanel(contentX, 130, 360, 240);
    juce::Rectangle<int> filterPanel(oscPanel.getRight() + gap, 130, 320, 240);
    juce::Rectangle<int> envPanel(filterPanel.getRight() + gap, 130, 410, 240);

    juce::Rectangle<int> lfoPanel(contentX, 388, 360, 220);
    juce::Rectangle<int> fxPanel(lfoPanel.getRight() + gap, 388, 320, 220);

    detuneSlider.setBounds(oscPanel.getX() + 65, oscPanel.getY() + 105, 120, 120);
    noiseSlider.setBounds(oscPanel.getX() + 205, oscPanel.getY() + 105, 120, 120);

    filterSlider.setBounds(filterPanel.getX() + 45, filterPanel.getY() + 95, 125, 125);
    resonanceSlider.setBounds(filterPanel.getX() + 185, filterPanel.getY() + 105, 110, 110);

    attackSlider.setBounds(envPanel.getX() + 50, envPanel.getY() + 95, 72, 140);
    decaySlider.setBounds(envPanel.getX() + 140, envPanel.getY() + 95, 72, 140);
    sustainSlider.setBounds(envPanel.getX() + 230, envPanel.getY() + 95, 72, 140);
    releaseSlider.setBounds(envPanel.getX() + 320, envPanel.getY() + 95, 72, 140);

    lfoRateSlider.setBounds(lfoPanel.getX() + 65, lfoPanel.getY() + 90, 120, 120);
    lfoDepthSlider.setBounds(lfoPanel.getX() + 205, lfoPanel.getY() + 90, 120, 120);

    reverbSlider.setBounds(fxPanel.getCentreX() - 62, fxPanel.getY() + 85, 125, 125);

    // MIDI keyboard
    keyboardComponent.setAvailableRange(24, 108);
    keyboardComponent.setKeyWidth(21.7f);
    keyboardComponent.setBounds(52, 730, getWidth() - 104, 100);
}

//==============================================================================
void MainComponent::logAction(const juce::String& action)
{
    if (logFile.existsAsFile() || logFile.create())
    {
        auto time = juce::Time::getCurrentTime().formatted("%H:%M:%S");

        logFile.appendText(
            time + " | " + action + "\n");
    }
}

//==============================================================================
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