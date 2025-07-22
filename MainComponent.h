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

            this->getPeer()->addFloatingChildPeer(*new_child->getPeer());
            new_child->setVisible(true);
            std::cerr << new_child->getName() << " created\n";

            global_windows_list.emplace(new_child.get(), std::move(new_child));

            ++children_created;

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

    int getDesktopWindowStyleFlags() const override {
        auto styleFlags = juce::DocumentWindow::getDesktopWindowStyleFlags();

        if(!is_main_window_)
        {
            styleFlags |= juce::ComponentPeer::windowUsesNormalTitlebarWhenSkippingTaskbar;
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
    //std::unordered_set<MainComponent*> children;
    //==============================================================================
    // Your private member variables go here...

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};