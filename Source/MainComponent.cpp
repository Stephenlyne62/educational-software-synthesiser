#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(900, 780);

    waveformBuffer.resize(512, 0.0f);

    frequencySlider.setRange(50.0, 2000.0, 1.0);
    frequencySlider.setValue(440.0);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    frequencySlider.onValueChange = [this]
        {
            oscillator.setFrequency(frequencySlider.getValue());
        };
    addAndMakeVisible(frequencySlider);

    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.attachToComponent(&frequencySlider, true);
    addAndMakeVisible(frequencyLabel);

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.1);
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
    waveformSelector.onChange = [this]
        {
            oscillator.setWaveform(waveformSelector.getSelectedId());
        };
    addAndMakeVisible(waveformSelector);

    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.attachToComponent(&waveformSelector, true);
    addAndMakeVisible(waveformLabel);

    filterSlider.setRange(0.01, 1.0, 0.01);
    filterSlider.setValue(0.2);
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
                oscillator.setFrequency(frequencySlider.getValue());
                oscillator.setWaveform(waveformSelector.getSelectedId());
            }
        };
    addAndMakeVisible(powerButton);

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

    oscillator.setSampleRate(sampleRate);
    oscillator.setFrequency(frequencySlider.getValue());
    oscillator.setWaveform(waveformSelector.getSelectedId());

    filterStateLeft = 0.0f;
    filterStateRight = 0.0f;
    envelopeLevel = 0.0f;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);

    auto* rightBuffer = bufferToFill.buffer->getNumChannels() > 1
        ? bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample)
        : nullptr;

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        auto oscillatorValue = oscillator.getNextSample();

        auto attackTime = (float)attackSlider.getValue();
        auto releaseTime = (float)releaseSlider.getValue();

        if (noteOn)
        {
            envelopeLevel += 1.0f / ((float)currentSampleRate * attackTime);

            if (envelopeLevel > 1.0f)
                envelopeLevel = 1.0f;
        }
        else
        {
            envelopeLevel -= 1.0f / ((float)currentSampleRate * releaseTime);

            if (envelopeLevel < 0.0f)
                envelopeLevel = 0.0f;
        }

        auto rawSample = oscillatorValue
            * (float)volumeSlider.getValue()
            * envelopeLevel;

        auto filterAmount = (float)filterSlider.getValue();

        filterStateLeft = filterStateLeft + filterAmount * (rawSample - filterStateLeft);
        filterStateRight = filterStateRight + filterAmount * (rawSample - filterStateRight);

        leftBuffer[sample] = filterStateLeft;

        if (rightBuffer != nullptr)
            rightBuffer[sample] = filterStateRight;

        waveformBuffer[(size_t)waveformBufferIndex] = filterStateLeft;
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
    setFrequencyFromMidiNote(midiNoteNumber);

    noteOn = true;
    powerButton.setToggleState(true, juce::dontSendNotification);
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState* source,
    int midiChannel,
    int midiNoteNumber,
    float velocity)
{
    noteOn = false;
    powerButton.setToggleState(false, juce::dontSendNotification);
}

void MainComponent::setFrequencyFromMidiNote(int midiNoteNumber)
{
    auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

    frequencySlider.setValue(frequency, juce::dontSendNotification);
    oscillator.setFrequency(frequency);
    oscillator.setWaveform(waveformSelector.getSelectedId());
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
    g.drawText("First JUCE Synth - Object-Oriented Oscillator Prototype",
        getLocalBounds().removeFromTop(45),
        juce::Justification::centred,
        true);

    auto scopeArea = juce::Rectangle<int>(80, 560, 760, 90);

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
    frequencySlider.setBounds(170, 70, 420, 35);
    volumeSlider.setBounds(170, 115, 420, 35);
    waveformSelector.setBounds(170, 165, 420, 30);
    filterSlider.setBounds(170, 210, 420, 35);

    attackSlider.setBounds(170, 270, 420, 35);
    decaySlider.setBounds(170, 315, 420, 35);
    sustainSlider.setBounds(170, 360, 420, 35);
    releaseSlider.setBounds(170, 405, 420, 35);

    powerButton.setBounds(170, 455, 120, 30);
    midiInputList.setBounds(170, 500, 420, 30);

    keyboardComponent.setBounds(80, 675, 760, 75);
}