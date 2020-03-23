# ofxBeatClock

openFrameworks addon to run a typical beat clock based in bpm / DAW style.



## Screenshots

![Alt text](/screenshot1.JPG?raw=true "screenshot1")
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

- internal clock base in timer
- external clock source as midi input mtc clock. (from Ableton Live or any sequencer)
- tap tempo
- save/load settings
- cute customizable gui


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

data/ofxBeatClock : xml settings, gui font file, and json theme. (crashes if not present)


## TODO:

- add different timer approach using the audio buffer to avoid out-of--sync problems of normal and threaded timers.
- on-the-fly re-sync to bar beat start


PLEASE FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS
