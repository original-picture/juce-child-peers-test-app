# Child peers test application

This is a simple program for testing [my fork of JUCE](https://github.com/original-picture/JUCE) that adds support for floating child windows  

All the interesting code is in MainComponent.h, specifically in the constructor of `MainComponent`.  
The GUI should be pretty self-explanatory.
There's also a simple command-line interface that lets you change the state of windows and print info about them and whatnot  

The application might appear to freeze when you close the main window on macOS. 
This only happens because of the way I wrote the test application, and isn't an issue with my JUCE fork.
That wouldn't happen in a properly written application lol  
The code is very nasty and stinky but