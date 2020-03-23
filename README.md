# ofxBeatClock

openFrameworks addon to run a typical beat clock based in bpm / DAW style.


## Screenshots

Internal clock mode:
![Alt text](/screenshot0.JPG?raw=true "screenshot0")
![Alt text](/screenshot1.JPG?raw=true "screenshot1")

External clock MIDI Sync:
![Alt text](/screenshot2.JPG?raw=true "screenshot2")


## Usage

```c++
ofApp.h:

#include "ofxBeatClock.h"

ofxBeatClock CLOCKER;

//--

ofApp.cpp:

//setup()
CLOCKER.setup();

//update()
CLOCKER.update();

//draw()
CLOCKER.draw();
```


## Features

- internal clock based in a threaded timer from ofxDawMetro
- external clock source as midi input mtc clock using ofxMidi 
- easy to sync to Ableton Live or any sequencer
- tap tempo
- save/load settings
- cute customizable gui by editing json file theme
- nice metronome sound ticks


## Tested system

- OF 0.11
- Visual Studio 2017
- macOS / HighSierra


## About

Addon by MoebiusSurfing.


## Requeriments

ofxGuiExtended2 (my fork)
ofxMidi
ofxDawMetro

data/ofxBeatClock: xml settings, gui font file, and json theme. (crashes if not present)


## TODO:

- improve example to get beat ticks to our ofApp.
- add different timer approach using the audio buffer to avoid out-of-sync problems of current normal and threaded timers.
- on-the-fly re-sync to bar beat start


PLEASE FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS
