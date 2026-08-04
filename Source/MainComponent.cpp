#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(1050, 820);

    voices.resize(8);
    waveformBuffer.resize(512, 0.0f);

    //==========================================================================
    // Oscillator / Waveform

    frequencySlider.setRange(50.0, 2000.0, 1.0);
    frequencySlider.setValue(440.0);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(frequencySlider);
    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.attachToComponent(&frequencySlider, true);
    addAndMakeVisible(frequencyLabel);

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.3);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(volumeSlider);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent(&volumeSlider, true);
    addAndMakeVisible(volumeLabel);

    detuneSlider.setRange(0.0, 25.0, 0.1);
    detuneSlider.setValue(5.0);
    detuneSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(detuneSlider);
    detuneLabel.setText("Detune", juce::dontSendNotification);
    detuneLabel.attachToComponent(&detuneSlider, true);
    addAndMakeVisible(detuneLabel);

    waveformSelector.addItem("Sine", 1);
    waveformSelector.addItem("Square", 2);
    waveformSelector.addItem("Saw", 3);
    waveformSelector.addItem("Triangle", 4);
    waveformSelector.setSelectedId(1);
    addAndMakeVisible(waveformSelector);
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.attachToComponent(&waveformSelector, true);
    addAndMakeVisible(waveformLabel);

    noiseSlider.setRange(0.0, 1.0, 0.01);
    noiseSlider.setValue(0.0);
    noiseSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(noiseSlider);
    noiseLabel.setText("Noise", juce::dontSendNotification);
    noiseLabel.attachToComponent(&noiseSlider, true);
    addAndMakeVisible(noiseLabel);

    //==========================================================================
    // Filter

    // Range is now in Hz — works directly with Filter::setCutoffHz()
    filterSlider.setRange(200.0, 18000.0, 1.0);
    filterSlider.setValue(8000.0);
    filterSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(filterSlider);
    filterLabel.setText("Cutoff (Hz)", juce::dontSendNotification);
    filterLabel.attachToComponent(&filterSlider, true);
    addAndMakeVisible(filterLabel);

    resonanceSlider.setRange(0.0, 0.95, 0.01);
    resonanceSlider.setValue(0.0);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(resonanceSlider);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.attachToComponent(&resonanceSlider, true);
    addAndMakeVisible(resonanceLabel);

    //==========================================================================
    // Amplitude Envelope

    attackSlider.setRange(0.001, 2.0, 0.001);
    attackSlider.setValue(0.05);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(attackSlider);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&attackSlider, true);
    addAndMakeVisible(attackLabel);

    decaySlider.setRange(0.001, 2.0, 0.001);
    decaySlider.setValue(0.2);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(decaySlider);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.attachToComponent(&decaySlider, true);
    addAndMakeVisible(decayLabel);

    sustainSlider.setRange(0.0, 1.0, 0.01);
    sustainSlider.setValue(0.7);
    sustainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(sustainSlider);
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.attachToComponent(&sustainSlider, true);
    addAndMakeVisible(sustainLabel);

    releaseSlider.setRange(0.001, 2.0, 0.001);
    releaseSlider.setValue(0.4);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(releaseSlider);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.attachToComponent(&releaseSlider, true);
    addAndMakeVisible(releaseLabel);

    //==========================================================================
    // Filter Envelope

    // Amount: how many Hz the filter envelope adds at its peak.
    // Positive = filter opens on attack. Negative = filter closes on attack.
    filterEnvAmountSlider.setRange(-8000.0, 8000.0, 10.0);
    filterEnvAmountSlider.setValue(0.0);
    filterEnvAmountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(filterEnvAmountSlider);
    filterEnvAmountLabel.setText("F.Env Amt", juce::dontSendNotification);
    filterEnvAmountLabel.attachToComponent(&filterEnvAmountSlider, true);
    addAndMakeVisible(filterEnvAmountLabel);

    filterAttackSlider.setRange(0.001, 2.0, 0.001);
    filterAttackSlider.setValue(0.01);
    filterAttackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(filterAttackSlider);
    filterAttackLabel.setText("F.Attack", juce::dontSendNotification);
    filterAttackLabel.attachToComponent(&filterAttackSlider, true);
    addAndMakeVisible(filterAttackLabel);

    filterDecaySlider.setRange(0.001, 2.0, 0.001);
    filterDecaySlider.setValue(0.3);
    filterDecaySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(filterDecaySlider);
    filterDecayLabel.setText("F.Decay", juce::dontSendNotification);
    filterDecayLabel.attachToComponent(&filterDecaySlider, true);
    addAndMakeVisible(filterDecayLabel);

    filterSustainSlider.setRange(0.0, 1.0, 0.01);
    filterSustainSlider.setValue(0.0);
    filterSustainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(filterSustainSlider);
    filterSustainLabel.setText("F.Sustain", juce::dontSendNotification);
    filterSustainLabel.attachToComponent(&filterSustainSlider, true);
    addAndMakeVisible(filterSustainLabel);

    filterReleaseSlider.setRange(0.001, 2.0, 0.001);
    filterReleaseSlider.setValue(0.3);
    filterReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(filterReleaseSlider);
    filterReleaseLabel.setText("F.Release", juce::dontSendNotification);
    filterReleaseLabel.attachToComponent(&filterReleaseSlider, true);
    addAndMakeVisible(filterReleaseLabel);

    //==========================================================================
    // LFO

    lfoRateSlider.setRange(0.1, 20.0, 0.1);
    lfoRateSlider.setValue(5.0);
    lfoRateSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(lfoRateSlider);
    lfoRateLabel.setText("LFO Rate", juce::dontSendNotification);
    lfoRateLabel.attachToComponent(&lfoRateSlider, true);
    addAndMakeVisible(lfoRateLabel);

    lfoDepthSlider.setRange(0.0, 1.0, 0.01);
    lfoDepthSlider.setValue(0.0);
    lfoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(lfoDepthSlider);
    lfoDepthLabel.setText("LFO Depth", juce::dontSendNotification);
    lfoDepthLabel.attachToComponent(&lfoDepthSlider, true);
    addAndMakeVisible(lfoDepthLabel);

    lfoTargetSelector.addItem("Volume", 1);
    lfoTargetSelector.addItem("Pitch", 2);
    lfoTargetSelector.addItem("Filter", 3);
    lfoTargetSelector.setSelectedId(1);
    addAndMakeVisible(lfoTargetSelector);
    lfoTargetLabel.setText("LFO Target", juce::dontSendNotification);
    lfoTargetLabel.attachToComponent(&lfoTargetSelector, true);
    addAndMakeVisible(lfoTargetLabel);

    //==========================================================================
    // Reverb

    reverbSlider.setRange(0.0, 1.0, 0.01);
    reverbSlider.setValue(0.0);
    reverbSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(reverbSlider);
    reverbLabel.setText("Reverb", juce::dontSendNotification);
    reverbLabel.attachToComponent(&reverbSlider, true);
    addAndMakeVisible(reverbLabel);

    //==========================================================================
    // Buttons

    powerButton.setButtonText("Sound On");
    powerButton.setToggleState(false, juce::dontSendNotification);
    powerButton.onClick = [this]
        {
            noteOn = powerButton.getToggleState();

            if (noteOn)
            {
                voices[0].setEnvelopeParameters(
                    (float)attackSlider.getValue(),
                    (float)decaySlider.getValue(),
                    (float)sustainSlider.getValue(),
                    (float)releaseSlider.getValue()
                );
                voices[0].setFilterEnvelopeParameters(
                    (float)filterAttackSlider.getValue(),
                    (float)filterDecaySlider.getValue(),
                    (float)filterSustainSlider.getValue(),
                    (float)filterReleaseSlider.getValue()
                );
                voices[0].setFilterEnvAmount((float)filterEnvAmountSlider.getValue());
                voices[0].setFilterCutoff((float)filterSlider.getValue());
                voices[0].setDetuneCents((float)detuneSlider.getValue());
                voices[0].startNote(69, frequencySlider.getValue());
            }
            else
            {
                for (auto& voice : voices)
                    voice.stopNote();
            }
        };
    addAndMakeVisible(powerButton);

    bassPresetButton.setButtonText("Bass");
    bassPresetButton.onClick = [this] { applyPreset(1); };
    addAndMakeVisible(bassPresetButton);

    leadPresetButton.setButtonText("Lead");
    leadPresetButton.onClick = [this] { applyPreset(2); };
    addAndMakeVisible(leadPresetButton);

    padPresetButton.setButtonText("Pad");
    padPresetButton.onClick = [this] { applyPreset(3); };
    addAndMakeVisible(padPresetButton);

    pluckPresetButton.setButtonText("Pluck");
    pluckPresetButton.onClick = [this] { applyPreset(4); };
    addAndMakeVisible(pluckPresetButton);

    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this] { savePreset(); };
    addAndMakeVisible(savePresetButton);

    loadPresetButton.setButtonText("Load");
    loadPresetButton.onClick = [this] { loadPreset(); };
    addAndMakeVisible(loadPresetButton);

    //==========================================================================
    // MIDI

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

    addAndMakeVisible(midiInputList);
    midiInputLabel.setText("MIDI Input", juce::dontSendNotification);
    midiInputLabel.attachToComponent(&midiInputList, true);
    addAndMakeVisible(midiInputLabel);

    keyboardState.addListener(this);
    keyboardComponent.setAvailableRange(36, 84);
    addAndMakeVisible(keyboardComponent);

    setAudioChannels(0, 2);
    startTimerHz(60);
}

MainComponent::~MainComponent()
{
    stopTimer();
    keyboardState.removeListener(this);
    midiInput.reset();
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;

    lfo.setSampleRate(sampleRate);
    lfo.setRate(lfoRateSlider.getValue());

    reverb.reset();

    for (auto& voice : voices)
    {
        voice.setSampleRate(sampleRate);
        voice.setEnvelopeParameters(
            (float)attackSlider.getValue(),
            (float)decaySlider.getValue(),
            (float)sustainSlider.getValue(),
            (float)releaseSlider.getValue()
        );
        voice.setFilterEnvelopeParameters(
            (float)filterAttackSlider.getValue(),
            (float)filterDecaySlider.getValue(),
            (float)filterSustainSlider.getValue(),
            (float)filterReleaseSlider.getValue()
        );
        voice.setFilterEnvAmount((float)filterEnvAmountSlider.getValue());
        voice.setDetuneCents((float)detuneSlider.getValue());
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getNumChannels() > 1
        ? bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample)
        : nullptr;

    // Update reverb parameters once per block, not per sample
    {
        auto reverbAmount = (float)reverbSlider.getValue();
        reverbParameters.roomSize = reverbAmount;
        reverbParameters.wetLevel = reverbAmount * 0.35f;
        reverbParameters.dryLevel = 1.0f - (reverbAmount * 0.25f);
        reverbParameters.width = 1.0f;
        reverbParameters.damping = 0.4f;
        reverb.setParameters(reverbParameters);
    }

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

        if (selectedTarget == 1)
        {
            // Volume tremolo
            tremoloGain = 1.0f - (lfoDepth * lfoNormalised);
        }
        else if (selectedTarget == 2)
        {
            // Pitch vibrato (±50 cents at full depth)
            pitchModulationCents = lfoValue * lfoDepth * 50.0f;
        }
        else if (selectedTarget == 3)
        {
            // Filter cutoff modulation (±4000 Hz at full depth)
            filterCutoff = juce::jlimit(200.0f, 18000.0f,
                filterCutoff + (lfoValue * lfoDepth * 4000.0f));
        }

        for (auto& voice : voices)
        {
            voice.setWaveform(waveformSelector.getSelectedId());
            voice.setEnvelopeParameters(
                (float)attackSlider.getValue(),
                (float)decaySlider.getValue(),
                (float)sustainSlider.getValue(),
                (float)releaseSlider.getValue()
            );
            voice.setFilterEnvelopeParameters(
                (float)filterAttackSlider.getValue(),
                (float)filterDecaySlider.getValue(),
                (float)filterSustainSlider.getValue(),
                (float)filterReleaseSlider.getValue()
            );
            voice.setFilterEnvAmount((float)filterEnvAmountSlider.getValue());
            voice.setFilterCutoff(filterCutoff);
            voice.setFilterResonance((float)resonanceSlider.getValue());
            voice.setNoiseAmount((float)noiseSlider.getValue());
            voice.setDetuneCents((float)detuneSlider.getValue());
            voice.setPitchModulationCents(pitchModulationCents);

            oscillatorValue += voice.getNextSample();
        }

        // Scale by number of active voices to prevent clipping with polyphony
        int activeVoiceCount = 0;
        for (auto& v : voices)
            if (v.isActive()) ++activeVoiceCount;

        if (activeVoiceCount > 0)
            oscillatorValue /= (float)activeVoiceCount;

        oscillatorValue *= 0.5f; // Additional headroom

        auto rawSample = oscillatorValue
            * (float)volumeSlider.getValue()
            * tremoloGain;

        float leftSample = rawSample;
        float rightSample = rawSample;

        reverb.processStereo(&leftSample, &rightSample, 1);

        leftBuffer[sample] = leftSample;

        if (rightBuffer != nullptr)
            rightBuffer[sample] = rightSample;

        waveformBuffer[(size_t)waveformBufferIndex] = rawSample;
        waveformBufferIndex = (waveformBufferIndex + 1) % (int)waveformBuffer.size();
    }
}

void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source,
    const juce::MidiMessage& message)
{
    keyboardState.processNextMidiEvent(message);
}

void MainComponent::handleNoteOn(juce::MidiKeyboardState* source,
    int midiChannel,
    int midiNoteNumber,
    float velocity)
{
    auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    frequencySlider.setValue(frequency, juce::dontSendNotification);

    auto* voice = findFreeVoice();

    if (voice != nullptr)
    {
        voice->setEnvelopeParameters(
            (float)attackSlider.getValue(),
            (float)decaySlider.getValue(),
            (float)sustainSlider.getValue(),
            (float)releaseSlider.getValue()
        );
        voice->setFilterEnvelopeParameters(
            (float)filterAttackSlider.getValue(),
            (float)filterDecaySlider.getValue(),
            (float)filterSustainSlider.getValue(),
            (float)filterReleaseSlider.getValue()
        );
        voice->setFilterEnvAmount((float)filterEnvAmountSlider.getValue());
        voice->setFilterCutoff((float)filterSlider.getValue());
        voice->setFilterResonance((float)resonanceSlider.getValue());
        voice->setNoiseAmount((float)noiseSlider.getValue());
        voice->setDetuneCents((float)detuneSlider.getValue());
        voice->startNote(midiNoteNumber, frequency);
    }

    noteOn = true;
    powerButton.setToggleState(true, juce::dontSendNotification);
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState* source,
    int midiChannel,
    int midiNoteNumber,
    float velocity)
{
    for (auto& voice : voices)
    {
        if (voice.getMidiNoteNumber() == midiNoteNumber)
            voice.stopNote();
    }

    bool anyVoiceActive = false;

    for (auto& voice : voices)
    {
        if (voice.isActive())
            anyVoiceActive = true;
    }

    noteOn = anyVoiceActive;
    powerButton.setToggleState(anyVoiceActive, juce::dontSendNotification);
}

void MainComponent::setFrequencyFromMidiNote(int midiNoteNumber)
{
    auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    frequencySlider.setValue(frequency, juce::dontSendNotification);
}

Voice* MainComponent::findFreeVoice()
{
    for (auto& voice : voices)
    {
        if (!voice.isActive())
            return &voice;
    }

    return &voices[0]; // Steal oldest voice if all are active
}

//==============================================================================
void MainComponent::applyPreset(int presetNumber)
{
    switch (presetNumber)
    {
    case 1: // Bass
        waveformSelector.setSelectedId(2);
        volumeSlider.setValue(0.45);
        filterSlider.setValue(800.0);       // Hz
        resonanceSlider.setValue(0.55);
        noiseSlider.setValue(0.02);
        attackSlider.setValue(0.005);
        decaySlider.setValue(0.15);
        sustainSlider.setValue(0.65);
        releaseSlider.setValue(0.12);
        filterEnvAmountSlider.setValue(2000.0);
        filterAttackSlider.setValue(0.005);
        filterDecaySlider.setValue(0.2);
        filterSustainSlider.setValue(0.0);
        filterReleaseSlider.setValue(0.15);
        lfoRateSlider.setValue(4.0);
        lfoDepthSlider.setValue(0.15);
        lfoTargetSelector.setSelectedId(3);
        detuneSlider.setValue(3.0);
        break;

    case 2: // Lead
        waveformSelector.setSelectedId(3);
        volumeSlider.setValue(0.35);
        filterSlider.setValue(6000.0);      // Hz
        resonanceSlider.setValue(0.35);
        noiseSlider.setValue(0.00);
        attackSlider.setValue(0.01);
        decaySlider.setValue(0.2);
        sustainSlider.setValue(0.85);
        releaseSlider.setValue(0.25);
        filterEnvAmountSlider.setValue(3000.0);
        filterAttackSlider.setValue(0.01);
        filterDecaySlider.setValue(0.3);
        filterSustainSlider.setValue(0.5);
        filterReleaseSlider.setValue(0.25);
        lfoRateSlider.setValue(5.5);
        lfoDepthSlider.setValue(0.2);
        lfoTargetSelector.setSelectedId(2);
        detuneSlider.setValue(8.0);
        break;

    case 3: // Pad
        waveformSelector.setSelectedId(4);
        volumeSlider.setValue(0.3);
        filterSlider.setValue(2000.0);      // Hz
        resonanceSlider.setValue(0.15);
        noiseSlider.setValue(0.08);
        attackSlider.setValue(1.2);
        decaySlider.setValue(0.8);
        sustainSlider.setValue(0.8);
        releaseSlider.setValue(1.5);
        filterEnvAmountSlider.setValue(1500.0);
        filterAttackSlider.setValue(1.0);
        filterDecaySlider.setValue(0.5);
        filterSustainSlider.setValue(0.6);
        filterReleaseSlider.setValue(1.5);
        lfoRateSlider.setValue(0.8);
        lfoDepthSlider.setValue(0.25);
        lfoTargetSelector.setSelectedId(3);
        detuneSlider.setValue(14.0);
        break;

    case 4: // Pluck
        waveformSelector.setSelectedId(1);
        volumeSlider.setValue(0.4);
        filterSlider.setValue(5000.0);      // Hz
        resonanceSlider.setValue(0.45);
        noiseSlider.setValue(0.04);
        attackSlider.setValue(0.001);
        decaySlider.setValue(0.1);
        sustainSlider.setValue(0.25);
        releaseSlider.setValue(0.08);
        filterEnvAmountSlider.setValue(5000.0);
        filterAttackSlider.setValue(0.001);
        filterDecaySlider.setValue(0.15);
        filterSustainSlider.setValue(0.0);
        filterReleaseSlider.setValue(0.1);
        lfoRateSlider.setValue(8.0);
        lfoDepthSlider.setValue(0.1);
        lfoTargetSelector.setSelectedId(1);
        detuneSlider.setValue(1.0);
        break;

    default:
        break;
    }
}

//==============================================================================
void MainComponent::savePreset()
{
    auto preset = std::make_unique<juce::DynamicObject>();

    preset->setProperty("waveform", waveformSelector.getSelectedId());
    preset->setProperty("volume", volumeSlider.getValue());
    preset->setProperty("filter", filterSlider.getValue());
    preset->setProperty("resonance", resonanceSlider.getValue());
    preset->setProperty("noise", noiseSlider.getValue());
    preset->setProperty("attack", attackSlider.getValue());
    preset->setProperty("decay", decaySlider.getValue());
    preset->setProperty("sustain", sustainSlider.getValue());
    preset->setProperty("release", releaseSlider.getValue());
    preset->setProperty("filterEnvAmount", filterEnvAmountSlider.getValue());
    preset->setProperty("filterAttack", filterAttackSlider.getValue());
    preset->setProperty("filterDecay", filterDecaySlider.getValue());
    preset->setProperty("filterSustain", filterSustainSlider.getValue());
    preset->setProperty("filterRelease", filterReleaseSlider.getValue());
    preset->setProperty("lfoRate", lfoRateSlider.getValue());
    preset->setProperty("lfoDepth", lfoDepthSlider.getValue());
    preset->setProperty("lfoTarget", lfoTargetSelector.getSelectedId());
    preset->setProperty("detune", detuneSlider.getValue());

    juce::var presetVar(preset.release());

    auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("MySynthPreset.json");

    file.replaceWithText(juce::JSON::toString(presetVar));
}

void MainComponent::loadPreset()
{
    auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("MySynthPreset.json");

    if (!file.existsAsFile())
        return;

    auto json = juce::JSON::parse(file);

    if (auto* object = json.getDynamicObject())
    {
        waveformSelector.setSelectedId((int)object->getProperty("waveform"));
        volumeSlider.setValue((double)object->getProperty("volume"));
        filterSlider.setValue((double)object->getProperty("filter"));
        resonanceSlider.setValue((double)object->getProperty("resonance"));
        noiseSlider.setValue((double)object->getProperty("noise"));
        attackSlider.setValue((double)object->getProperty("attack"));
        decaySlider.setValue((double)object->getProperty("decay"));
        sustainSlider.setValue((double)object->getProperty("sustain"));
        releaseSlider.setValue((double)object->getProperty("release"));
        filterEnvAmountSlider.setValue((double)object->getProperty("filterEnvAmount"));
        filterAttackSlider.setValue((double)object->getProperty("filterAttack"));
        filterDecaySlider.setValue((double)object->getProperty("filterDecay"));
        filterSustainSlider.setValue((double)object->getProperty("filterSustain"));
        filterReleaseSlider.setValue((double)object->getProperty("filterRelease"));
        lfoRateSlider.setValue((double)object->getProperty("lfoRate"));
        lfoDepthSlider.setValue((double)object->getProperty("lfoDepth"));
        lfoTargetSelector.setSelectedId((int)object->getProperty("lfoTarget"));
        detuneSlider.setValue((double)object->getProperty("detune"));
    }
}

//==============================================================================
void MainComponent::timerCallback()
{
    repaint();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(juce::FontOptions(18.0f));
    g.setColour(juce::Colours::white);
    g.drawText("Subtractive Synthesiser",
        getLocalBounds().removeFromTop(45),
        juce::Justification::centred,
        true);

    // Section labels
    g.setFont(juce::FontOptions(12.0f));
    g.setColour(juce::Colours::lightgrey);
    g.drawText("OSCILLATOR", 10, 50, 150, 20, juce::Justification::left);
    g.drawText("FILTER", 10, 230, 150, 20, juce::Justification::left);
    g.drawText("AMP ENV", 370, 50, 150, 20, juce::Justification::left);
    g.drawText("FILTER ENV", 600, 50, 150, 20, juce::Justification::left);
    g.drawText("LFO", 370, 265, 150, 20, juce::Justification::left);
    g.drawText("REVERB", 370, 430, 150, 20, juce::Justification::left);

    // Waveform scope background
    auto scopeArea = juce::Rectangle<int>(20, 580, 1010, 100);
    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRect(scopeArea);
    g.setColour(juce::Colours::grey);
    g.drawRect(scopeArea);

    // Draw waveform
    g.setColour(juce::Colours::deepskyblue);
    juce::Path waveformPath;

    auto centreY = (float)scopeArea.getCentreY();
    auto width = (float)scopeArea.getWidth();
    auto height = (float)scopeArea.getHeight();

    for (size_t i = 0; i < waveformBuffer.size(); ++i)
    {
        auto index = (waveformBufferIndex + (int)i) % (int)waveformBuffer.size();
        auto sampleValue = waveformBuffer[(size_t)index];

        auto x = scopeArea.getX() + ((float)i / (float)waveformBuffer.size()) * width;
        auto y = centreY - sampleValue * height * 2.0f;

        if (i == 0)
            waveformPath.startNewSubPath(x, y);
        else
            waveformPath.lineTo(x, y);
    }

    g.strokePath(waveformPath, juce::PathStrokeType(2.0f));
}

void MainComponent::resized()
{
    // ---- Left column: Oscillator + Filter (x=170, width=180) ----
    frequencySlider.setBounds(170, 65, 180, 30);
    volumeSlider.setBounds(170, 100, 180, 30);
    detuneSlider.setBounds(170, 135, 180, 30);
    waveformSelector.setBounds(170, 170, 180, 28);
    noiseSlider.setBounds(170, 205, 180, 30);

    filterSlider.setBounds(170, 250, 180, 30);
    resonanceSlider.setBounds(170, 285, 180, 30);

    // ---- Middle-left column: Amp Envelope (x=460, width=130) ----
    attackSlider.setBounds(460, 65, 130, 30);
    decaySlider.setBounds(460, 100, 130, 30);
    sustainSlider.setBounds(460, 135, 130, 30);
    releaseSlider.setBounds(460, 170, 130, 30);

    // ---- Middle-left column continued: LFO + Reverb ----
    lfoRateSlider.setBounds(460, 280, 130, 30);
    lfoDepthSlider.setBounds(460, 315, 130, 30);
    lfoTargetSelector.setBounds(460, 350, 130, 28);

    reverbSlider.setBounds(460, 445, 130, 30);

    // ---- Right column: Filter Envelope (x=720, width=130) ----
    filterEnvAmountSlider.setBounds(720, 65, 130, 30);
    filterAttackSlider.setBounds(720, 100, 130, 30);
    filterDecaySlider.setBounds(720, 135, 130, 30);
    filterSustainSlider.setBounds(720, 170, 130, 30);
    filterReleaseSlider.setBounds(720, 205, 130, 30);

    // ---- Buttons ----
    powerButton.setBounds(20, 490, 110, 30);
    bassPresetButton.setBounds(145, 490, 80, 30);
    leadPresetButton.setBounds(235, 490, 80, 30);
    padPresetButton.setBounds(325, 490, 80, 30);
    pluckPresetButton.setBounds(415, 490, 80, 30);
    savePresetButton.setBounds(520, 490, 70, 30);
    loadPresetButton.setBounds(600, 490, 70, 30);

    // ---- MIDI ----
    midiInputList.setBounds(20, 535, 420, 28);

    // ---- Keyboard ----
    keyboardComponent.setBounds(20, 700, 1010, 75);
}