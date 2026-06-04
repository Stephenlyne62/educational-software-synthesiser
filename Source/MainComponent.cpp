#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(600, 400);

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

    auto frequency = 440.0; // A4 note
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
        auto currentSample = (float)std::sin(currentAngle) * 0.1f;

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
    g.drawText("First JUCE Synth Tone - 440 Hz", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
}