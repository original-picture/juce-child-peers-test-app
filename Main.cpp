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
        std::cout << "enter one of the following commands:\n"
                     "tv (toggle visibility) <window-id>\n"
                     "tf (to front) <window-id>\n"
                     "tff (to front and take focus) <window-id>\n"
                     "tb  (to behind) <window-to-move> <window-to-put-behind>\n"
                     "lsc (list children) <window-id>\n"
                     "gf (grab focus) <widow-id>\n"
                     "sm (set minimised) <widow-id>";

        for(;;) {
            if(stopped) {
                return;
            }

            std::string line;

            std::getline(std::cin, line);

            auto tokens = split(line);


            if(tokens.size()) {
                try {
                    if(tokens[0] == "tv") {
                        if(tokens.size() > 1) {
                            unsigned guid = std::stoi(tokens[1]);
                            auto* component = MainComponent::guid_to_component_.at(guid);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    component->getPeer()->setVisible(!component->getPeer()->isShowing());
                                                                });

                            }

                            std::cout << "toggling visibility...\n";
                        }
                        else {
                            std::cerr << "tv needs an argument <window-id>\n";
                        }
                    }
                    else if(tokens[0] == "tf" || tokens[0] == "tff") {
                        if(tokens.size() > 1) {
                            unsigned guid = std::stoi(tokens[1]);
                            auto* component = MainComponent::guid_to_component_.at(guid);
                            juce::MessageManager::callAsync([&]()
                                                            {
                                                                component->getPeer()->toFront(tokens[0] == "tff");
                                                            });


                            std::cout << "calling toFront...\n";
                        }
                        else {
                            std::cerr << "tf needs an argument <window-id>\n";
                        }
                    }
                    else if(tokens[0] == "tb") {
                        if(tokens.size() > 2) {
                            unsigned guid0 = std::stoi(tokens[1]),
                                     guid1 = std::stoi(tokens[2]);
                            auto* component = MainComponent::guid_to_component_.at(guid0);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    component->getPeer()->toBehind(MainComponent::guid_to_component_.at(guid1)->getPeer());
                                                                });

                            }

                            std::cout << "calling toBehind...\n";
                        }
                        else {
                            std::cerr << "tb needs two arguments <window-id> <window-to-put-behind-id>\n";
                        }
                    }
                    else if(tokens[0] == "lsc") {
                        if(tokens.size() > 1) {
                            unsigned guid = std::stoi(tokens[1]);
                            auto* component = MainComponent::guid_to_component_.at(guid);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    std::cout << "child components:\n";
                                                                    for(auto& e : component->getPeer()->getFloatingChildren()) {
                                                                        auto& child_component = e->getComponent();
                                                                        auto* component_dynamic_type = dynamic_cast<MainComponent*>(&child_component);
                                                                        std::cout << component_dynamic_type->guid() << '\n';
                                                                    }
                                                                });

                            }

                        }
                        else {
                            std::cerr << "lsc needs an argument <window-id>\n";
                        }
                    }
                    else if(tokens[0] == "gf") {
                        if(tokens.size() > 1) {
                            unsigned guid = std::stoi(tokens[1]);
                            auto* component = MainComponent::guid_to_component_.at(guid);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    std::cerr << "calling grabFocus...\n";
                                                                    component->getPeer()->grabFocus();
                                                                });

                            }

                        }
                        else {
                            std::cerr << "gf needs an argument <window-id>\n";
                        }
                    }
                    else if(tokens[0] == "sm") {
                        if(tokens.size() > 2) {
                            unsigned guid = std::stoi(tokens[1]);
                            bool shouldBeMinimised = std::stoi(tokens[2]);
                            auto* component = MainComponent::guid_to_component_.at(guid);
                            {
                                juce::MessageManager::callAsync([&]()
                                                                {
                                                                    std::cerr << "calling setMinimised...\n";
                                                                    component->getPeer()->setMinimised (shouldBeMinimised);
                                                                });

                            }

                        }
                        else {
                            std::cerr << "sm needs two arguments <window-id> <should-be-minimised:int>\n";
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
