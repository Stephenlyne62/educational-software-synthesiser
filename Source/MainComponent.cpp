#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(600, 400);

    frequencySlider.setRange(50.0, 2000.0, 1.0);
    frequencySlider.setValue(440.0);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);

    frequencySlider.onValueChange = [this]
        {
            auto frequency = frequencySlider.getValue();

            if (currentSampleRate > 0.0)
                angleDelta = frequency * 2.0 * juce::MathConstants<double>::pi / currentSampleRate;
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

    // 0 input channels, 2 output channels
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;

    auto frequency = frequencySlider.getValue();
    angleDelta = frequency * 2.0 * juce::MathConstants<double>::pi / currentSampleRate;
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);

    auto* rightBuffer = bufferToFill.buffer->getNumChannels() > 1
        ? bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample)
        : nullptr;

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        auto currentSample = (float)std::sin(currentAngle) * (float)volumeSlider.getValue();

        currentAngle += angleDelta;

        leftBuffer[sample] = currentSample;

        if (rightBuffer != nullptr)
            rightBuffer[sample] = currentSample;
    }
}

void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(juce::FontOptions(16.0f));
    g.setColour(juce::Colours::white);
    g.drawText("First JUCE Synth - Frequency + Volume Control",
        getLocalBounds(),
        juce::Justification::centredTop,
        true);
}

void MainComponent::resized()
{
    frequencySlider.setBounds(150, 150, 350, 40);
    volumeSlider.setBounds(150, 210, 350, 40);
}