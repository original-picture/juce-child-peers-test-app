#pragma once

// I'm lazy so I just include this file directly in Main.cpp, so there is no accompanying MainComponent.cpp

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




class MainComponent final : public juce::DocumentWindow
{
public:
    //==============================================================================
    MainComponent(juce::String name = "window0", bool is_main_window = false) : name_without_guid_(name), is_main_window_(is_main_window), juce::DocumentWindow(name+", GUID="+juce::String(guid_counter_), juce::Colour::fromFloatRGBA(1.f, 1.f, 1.f, 1.f), juce::DocumentWindow::minimiseButton|juce::DocumentWindow::closeButton, false) {

        addToDesktop();

        setSize (600, 400);
        setUsingNativeTitleBar(true);
        setResizable(true, false);

        setContentNonOwned(&buttons, true);

        buttons.setSize(600,400);

        resized();

        guid_ = guid_counter_;
        guid_to_component_.emplace(guid_, this);
        ++guid_counter_;

        buttons.create_window_button.onClick = [&]() {
            auto new_child = std::make_unique<MainComponent>(name_without_guid_+".window"+juce::String(children_created));

#ifndef NO_CHILD_PEERS_SUPPORT
            this->getPeer()->addFloatingChildPeer(*new_child->getPeer()); /////////////////////////////////////////////// THIS IS THE IMPORTANT PART /////////////////////////////////////////
#endif
            new_child->setTopLeftPosition(this->getPosition()+juce::Point(50,50));

            new_child->setVisible(true);
            std::cerr << new_child->getName() << " created\n";

            global_windows_list.emplace(new_child.get(), std::move(new_child));

            ++children_created;

        };

        buttons.set_always_on_top_button.onClick = [&, this]() {
#ifndef NO_CHILD_PEERS_SUPPORT

            this->getPeer()->setAlwaysOnTop(!this->getPeer()->isInherentlyAlwaysOnTop());

            if(this->getPeer()->isInherentlyAlwaysOnTop()) { ////////////////////////// THIS IS NEW IN MY FORK /////////////////////////////
                std::cerr << this->getName() << " made always on top\n";
            }
            else {
                std::cerr << this->getName() << " made NOT always on top\n";
            }
#endif
        };
    }

    int getDesktopWindowStyleFlags() const override {
        auto styleFlags = juce::DocumentWindow::getDesktopWindowStyleFlags();

        if(!is_main_window_)
        {
#ifndef NO_CHILD_PEERS_SUPPORT
            styleFlags &= ~juce::ComponentPeer::windowAppearsOnTaskbar;
            styleFlags &= ~juce::ComponentPeer::windowHasMinimiseButton;

            styleFlags |= juce::ComponentPeer::windowUsesNormalTitlebarWhenSkippingTaskbar; ////////////////////////// THIS IS NEW IN MY FORK /////////////////////////////
#endif
        }

        return styleFlags;
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

        if(is_main_window_) {
            global_windows_list.clear(); // erase all windows
            guid_to_component_.clear();

            juce::JUCEApplication::quit();

            std::cerr << "main window closed. Press enter key to quit\n";
        }
        else {
            global_windows_list.erase(this);
            guid_to_component_.erase(guid_);

            std::cerr << "window closed\n";
        }
    }

    inline static std::unordered_map<unsigned, MainComponent*> guid_to_component_;

    unsigned guid() const {
        return guid_;
    }

private:
    inline static std::unordered_map<MainComponent*, std::unique_ptr<MainComponent>> global_windows_list; // keep track of all children to prevent juce from complaining about memory leaks
    inline static unsigned guid_counter_ = 0;

    juce::String name_without_guid_;

    unsigned guid_;
    unsigned children_created = 0;

    bool is_main_window_;

    buttons_component buttons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};