#include "MainComponent.h"

/// source: https://stackoverflow.com/a/57809972
bool getline_async(std::istream& is, std::string& str, char delim = '\n') {

    static std::string lineSoFar;
    char inChar;
    int charsRead = 0;
    bool lineRead = false;
    str = "";

    do {
        charsRead = is.readsome(&inChar, 1);
        if (charsRead == 1) {
            // if the delimiter is read then return the string so far
            if (inChar == delim) {
                str = lineSoFar;
                lineSoFar = "";
                lineRead = true;
            } else {  // otherwise add it to the string so far
                lineSoFar.append(1, inChar);
            }
        }
    } while (charsRead != 0 && !lineRead);

    return lineRead;
}

class stdin_watcher_thread : public juce::Thread {
    std::atomic<bool> stopped = false;

public:
    stdin_watcher_thread() : juce::Thread("stdin watcher thread") {}
    void run() override {
        for(;;) {
            if(stopped) {
                return;
            }

            std::cout << "enter a window's GUID to toggle its visibility\n";
            std::string line;

            std::getline(std::cin, line);

            try {
                unsigned guid = std::stoi(line);
                auto* component = MainComponent::guid_to_component_.at(guid);
                {
                    juce::MessageManager::getInstance()->callAsync([&](){
                        component->setVisible(!component->isShowing());
                    });

                }
                std::cout << "toggling visibility\n";
            }
            catch (std::exception& e) {
                std::cerr << "invalid input: " << e.what() << '\n';
            }
        }
    }

    ~stdin_watcher_thread() override {
        stopped = true;

        stopThread(-1);
    }
};

//==============================================================================
class GuiAppApplication final : public juce::JUCEApplication
{
public:
    //==============================================================================
    GuiAppApplication() {
        stdin_thread_.startThread();

    }

    // We inject these as compile definitions from the CMakeLists.txt
    // If you've enabled the juce header with `juce_generate_juce_header(<thisTarget>)`
    // you could `#include <JuceHeader.h>` and use `ProjectInfo::projectName` etc. instead.
    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused (commandLine);

        mainWindow.reset (new MainComponent("main-window", true));
        mainWindow->setVisible(true);
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused (commandLine);
    }


private:
    std::unique_ptr<MainComponent> mainWindow;
    stdin_watcher_thread stdin_thread_;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (GuiAppApplication)
