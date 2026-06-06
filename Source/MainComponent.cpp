#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(900, 820);

    voices.resize(8);
    waveformBuffer.resize(512, 0.0f);

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

    waveformSelector.addItem("Sine", 1);
    waveformSelector.addItem("Square", 2);
    waveformSelector.addItem("Saw", 3);
    waveformSelector.addItem("Triangle", 4);
    waveformSelector.setSelectedId(1);
    addAndMakeVisible(waveformSelector);

    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.attachToComponent(&waveformSelector, true);
    addAndMakeVisible(waveformLabel);

    filterSlider.setRange(0.01, 1.0, 0.01);
    filterSlider.setValue(1.0);
    filterSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(filterSlider);

    filterLabel.setText("Filter", juce::dontSendNotification);
    filterLabel.attachToComponent(&filterSlider, true);
    addAndMakeVisible(filterLabel);

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

    for (auto& voice : voices)
    {
        voice.setSampleRate(sampleRate);
        voice.setEnvelopeParameters(
            (float)attackSlider.getValue(),
            (float)decaySlider.getValue(),
            (float)sustainSlider.getValue(),
            (float)releaseSlider.getValue()
        );
    }

    filterStateLeft = 0.0f;
    filterStateRight = 0.0f;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);

    auto* rightBuffer = bufferToFill.buffer->getNumChannels() > 1
        ? bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample)
        : nullptr;

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        float oscillatorValue = 0.0f;

        for (auto& voice : voices)
        {
            voice.setWaveform(waveformSelector.getSelectedId());
            voice.setEnvelopeParameters(
                (float)attackSlider.getValue(),
                (float)decaySlider.getValue(),
                (float)sustainSlider.getValue(),
                (float)releaseSlider.getValue()
            );

            voice.setFilterCutoff((float)filterSlider.getValue());

            oscillatorValue += voice.getNextSample();
        }

        oscillatorValue *= 0.5f;

        auto rawSample = oscillatorValue * (float)volumeSlider.getValue();

        leftBuffer[sample] = rawSample;

        if (rightBuffer != nullptr)
            rightBuffer[sample] = rawSample;

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

        voice->setFilterCutoff((float)filterSlider.getValue());
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

    return &voices[0];
}

void MainComponent::applyPreset(int presetNumber)
{
    switch (presetNumber)
    {
    case 1: // Bass
        waveformSelector.setSelectedId(2);
        volumeSlider.setValue(0.45);
        filterSlider.setValue(0.18);
        attackSlider.setValue(0.005);
        decaySlider.setValue(0.15);
        sustainSlider.setValue(0.65);
        releaseSlider.setValue(0.12);
        break;

    case 2: // Lead
        waveformSelector.setSelectedId(3);
        volumeSlider.setValue(0.35);
        filterSlider.setValue(0.75);
        attackSlider.setValue(0.01);
        decaySlider.setValue(0.2);
        sustainSlider.setValue(0.85);
        releaseSlider.setValue(0.25);
        break;

    case 3: // Pad
        waveformSelector.setSelectedId(4);
        volumeSlider.setValue(0.3);
        filterSlider.setValue(0.35);
        attackSlider.setValue(1.2);
        decaySlider.setValue(0.8);
        sustainSlider.setValue(0.8);
        releaseSlider.setValue(1.5);
        break;

    case 4: // Pluck
        waveformSelector.setSelectedId(1);
        volumeSlider.setValue(0.4);
        filterSlider.setValue(0.6);
        attackSlider.setValue(0.001);
        decaySlider.setValue(0.1);
        sustainSlider.setValue(0.25);
        releaseSlider.setValue(0.08);
        break;

    default:
        break;
    }
}

void MainComponent::savePreset()
{
    auto preset = std::make_unique<juce::DynamicObject>();

    preset->setProperty("waveform", waveformSelector.getSelectedId());
    preset->setProperty("volume", volumeSlider.getValue());
    preset->setProperty("filter", filterSlider.getValue());
    preset->setProperty("attack", attackSlider.getValue());
    preset->setProperty("decay", decaySlider.getValue());
    preset->setProperty("sustain", sustainSlider.getValue());
    preset->setProperty("release", releaseSlider.getValue());

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
        attackSlider.setValue((double)object->getProperty("attack"));
        decaySlider.setValue((double)object->getProperty("decay"));
        sustainSlider.setValue((double)object->getProperty("sustain"));
        releaseSlider.setValue((double)object->getProperty("release"));
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
    g.drawText("First JUCE Synth - Save / Load Preset Prototype",
        getLocalBounds().removeFromTop(45),
        juce::Justification::centred,
        true);

    auto scopeArea = juce::Rectangle<int>(80, 590, 760, 90);

    g.setColour(juce::Colours::black.withAlpha(0.35f));
    g.fillRect(scopeArea);

    g.setColour(juce::Colours::grey);
    g.drawRect(scopeArea);

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
    frequencySlider.setBounds(170, 65, 420, 35);
    volumeSlider.setBounds(170, 110, 420, 35);
    waveformSelector.setBounds(170, 160, 420, 30);
    filterSlider.setBounds(170, 205, 420, 35);

    attackSlider.setBounds(170, 255, 420, 35);
    decaySlider.setBounds(170, 300, 420, 35);
    sustainSlider.setBounds(170, 345, 420, 35);
    releaseSlider.setBounds(170, 390, 420, 35);

    powerButton.setBounds(170, 435, 120, 30);

    bassPresetButton.setBounds(310, 435, 90, 30);
    leadPresetButton.setBounds(410, 435, 90, 30);
    padPresetButton.setBounds(510, 435, 90, 30);
    pluckPresetButton.setBounds(610, 435, 90, 30);

    savePresetButton.setBounds(710, 435, 70, 30);
    loadPresetButton.setBounds(790, 435, 70, 30);

    midiInputList.setBounds(170, 485, 420, 30);

    keyboardComponent.setBounds(80, 705, 760, 75);
}