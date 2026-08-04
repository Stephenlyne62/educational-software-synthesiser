/*
  ==============================================================================

    Application entry point and main window setup for the JUCE synthesiser.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent_B.h"

//==============================================================================
class TestJuiceApp2Application : public juce::JUCEApplication
{
public:
    TestJuiceApp2Application() = default;

    const juce::String getApplicationName() override
    {
        return ProjectInfo::projectName;
    }

    const juce::String getApplicationVersion() override
    {
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    //==============================================================================
    void initialise(const juce::String&) override
    {
        // Create the main application window.
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        // Release the main window and its owned content.
        mainWindow = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // Allow the application to close when requested by the operating system.
        quit();
    }

    void anotherInstanceStarted(const juce::String&) override
    {
        // No additional action is required when another instance is launched.
    }

    //==============================================================================
    // Desktop window containing the main synthesiser component.
    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(juce::String name)
            : DocumentWindow(
                name,
                juce::Desktop::getInstance()
                .getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            // Forward the close request to the JUCE application.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// Generate the main() entry point and launch the JUCE application.
START_JUCE_APPLICATION(TestJuiceApp2Application)