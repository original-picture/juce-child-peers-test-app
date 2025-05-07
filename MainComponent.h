#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>


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

        setContentNonOwned(&create_window_button, true);
        create_window_button.setVisible(true);

        resized();

        create_window_button.onClick = [&]() {
            std::cerr << "window created\n";

            auto& window = *children.insert(new MainComponent(getName()+".window"+juce::String(children_created), this)).first;

            ++children_created;
            window->setTransientFor(this);
            window->setVisible(true);
        };
    }

    //==============================================================================
    void paint (juce::Graphics& g) override {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        g.setFont (juce::FontOptions (16.0f));
        g.setColour (juce::Colours::white);

    }
    void resized() override {
        create_window_button.setTopLeftPosition(0,0);
        create_window_button.setSize(getWidth(), getHeight());
    }

    void closeButtonPressed() override {
        for(auto& e : children) {
            e->parent = nullptr;
        }

        if(this->parent) {
            this->parent->children.erase(this);
            delete this;
        }
        else {
            std::exit(0);
        }

        std::cerr << "window deleted\n";
    }

private:
    unsigned children_created = 0;
    MainComponent* parent = nullptr;
    juce::TextButton create_window_button = juce::TextButton("create child window");

    std::unordered_set<MainComponent*> children;
    //==============================================================================
    // Your private member variables go here...

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};