#include "MainComponent.h"

#include <charconv>


std::vector<std::string>
split(const std::string& str) {

    std::vector<std::string> ret;
    std::string current_str;

    for(unsigned i = 0; i < str.size(); ++i) {
        char c = str[i];

        if(!std::isspace(c)) {
            current_str += c;
        }
        else {
            if(current_str.size()) {
                ret.push_back(std::move(current_str));
            }
        }
    }

    if(current_str.size()) {
        ret.push_back(std::move(current_str));
    }

    return ret;
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

            std::cout << "enter one of the following commands:\n"
                         "toggle-visibility <window-id>\n"
                         "tofront <window-id>\n"
                         "tobehind <window-to-move> <window-to->\n";

            std::string line;

            std::getline(std::cin, line);

            auto tokens = split(line);


            if(tokens.size()) {
                try {
                    if(tokens[0] == "toggle-visibility") {
                        if(tokens.size() > 1) {
                            unsigned guid = std::stoi(tokens[1]);
                            auto* component = MainComponent::guid_to_component_.at(guid);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    component->setVisible(!component->isShowing());
                                                                });

                            }

                            std::cout << "toggling visibility...\n";
                        }
                        else {
                            std::cerr << "toggle-visibility needs an argument <window-id>\n";
                        }
                    }
                    else if(tokens[0] == "tofront") {
                        if(tokens.size() > 1) {
                            unsigned guid = std::stoi(tokens[1]);
                            auto* component = MainComponent::guid_to_component_.at(guid);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    component->toFront(true);
                                                                });

                            }

                            std::cout << "calling toFront...\n";
                        }
                        else {
                            std::cerr << "tofront needs an argument <window-id>\n";
                        }
                    }
                    else if(tokens[0] == "tobehind") {
                        if(tokens.size() > 2) {
                            unsigned guid0 = std::stoi(tokens[1]),
                                     guid1 = std::stoi(tokens[2]);
                            auto* component = MainComponent::guid_to_component_.at(guid0);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    component->toBehind(MainComponent::guid_to_component_.at(guid1));
                                                                });

                            }

                            std::cout << "calling toBehind...\n";
                        }
                        else {
                            std::cerr << "tofront needs two arguments <window-id> <window-to-put-behind-id>\n";
                        }
                    }
                    else {
                        std::cerr << "command not recognized\n";
                    }
                }
                catch (std::exception& e) {
                    std::cerr << "invalid input: " << e.what() << '\n';
                }
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
