#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>

struct buttons_component : public juce::Component {
    juce::TextButton   create_window_button     = juce::TextButton("create child window");
    juce::ToggleButton set_always_on_top_button = juce::ToggleButton("make this window always on top");

    buttons_component() {
        addAndMakeVisible(create_window_button);
        addAndMakeVisible(set_always_on_top_button);
    }

    void paint (juce::Graphics& g) override {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        g.setFont (juce::FontOptions (16.0f));
        g.setColour (juce::Colours::white);

    }

    void resized() override {
        create_window_button.setTopLeftPosition(0,0);
        create_window_button.setSize(getWidth()/2, getHeight());

        set_always_on_top_button.setTopLeftPosition(getWidth()/2, 0);
        set_always_on_top_button.setSize(getWidth()/2, getHeight());
    }

};


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent final : public juce::DocumentWindow
{
public:
    //==============================================================================
    MainComponent(juce::String name = "window0", MainComponent* parent = nullptr) : parent(parent), juce::DocumentWindow(name, juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 1.f), juce::DocumentWindow::minimiseButton|juce::DocumentWindow::closeButton) {
        setSize (600, 400);
        setUsingNativeTitleBar(true);
        setResizable(true, false);

        setContentNonOwned(&buttons, true);

        buttons.setSize(600,400);

        resized();

        buttons.create_window_button.onClick = [&]() {
            auto new_child = std::make_unique<MainComponent>(getName()+".window"+juce::String(children_created), this);

            this->getPeer()->addTopLevelChildPeer(*new_child->getPeer());
            new_child->setVisible(true);

            global_windows_list.emplace(new_child.get(), std::move(new_child));

            ++children_created;

            std::cerr << this->getName() << " created\n";
        };

        buttons.set_always_on_top_button.onClick = [&, this]() {
            this->getPeer()->setAlwaysOnTop(!this->getPeer()->isInherentlyAlwaysOnTop());

            if(this->getPeer()->isInherentlyAlwaysOnTop()) {
                std::cerr << this->getName() << " made always on top\n";
            }
            else {
                std::cerr << this->getName() << " made NOT always on top\n";
            }
        };
    }

    MainComponent (juce::JUCEApplication* application) : MainComponent() {
        application_ = application;
    }

    //==============================================================================
    void paint (juce::Graphics& g) override {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        g.setFont (juce::FontOptions (16.0f));
        g.setColour (juce::Colours::white);

    }
    void resized() override {
        buttons.resized();
    }

    void closeButtonPressed() override {

        if(global_windows_list.empty()) {
            juce::JUCEApplication::quit();
        }

        global_windows_list.erase(this);

        std::cerr << "window deleted\n";
    }

private:
    juce::JUCEApplication* application_ = nullptr;

    inline static std::unordered_map<MainComponent*, std::unique_ptr<MainComponent>> global_windows_list; // keep track of all children to prevent juce from complaining about memory leaks

    unsigned children_created = 0;
    MainComponent* parent = nullptr;

    buttons_component buttons;
    //std::unordered_set<MainComponent*> children;
    //==============================================================================
    // Your private member variables go here...

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};