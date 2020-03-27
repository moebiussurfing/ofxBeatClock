# ofxBeatClock

openFrameworks addon to run a DAW styled BPM-Beat clock with tap tempo and external MIDI sync.


## Screencast
![Alt text](/ofxBeatClock.gif?raw=true "ofxBeatClock.gif")


## Screenshots

Internal clock mode:

![Alt text](/screenshot0.JPG?raw=true "screenshot0")
![Alt text](/screenshot1.JPG?raw=true "screenshot1")

External clock MIDI Sync mode:

![Alt text](/screenshot2.JPG?raw=true "screenshot2")


## Usage

Create your project with OF ProjectGenerator as usual.


```c++
ofApp.h:

#include "ofxBeatClock.h"

ofxBeatClock beatClock;

//--

ofApp.cpp:

//setup()
beatClock.setup();

//update()
beatClock.update();

//draw()
beatClock.draw();
```


## Features

- internal clock based in a threaded timer from ofxDawMetro from https://github.com/castovoid
- external clock source as input MIDI clock using ofxMidi 
- easy to sync to Ableton Live or any sequencer with midi clock 
- cute tap tempo engine
- save/load mode settings
- cute customizable GUI by editing JSON file theme. ("theme_bleurgh.json". Based on @Transat theme)
- customizable GUI positions by code
- nice metronome sound ticks


## Tested system

- OF 0.11
- Visual Studio 2017
- macOS / HighSierra


## About

An addon by MoebiusSurfing.


## Requeriments

https://github.com/danomatika/ofxMidi


https://github.com/castovoid/ofxDawMetro

https://github.com/moebiussurfing/ofxGuiExtended2 (my fork)

/data/ofxBeatClock/  
xml settings, gui font file, and json theme. (crashes if not present)


## TODO:

- add different timer approach using the audio buffer to avoid out-of-sync problems of current normal and threaded timers.
- on-the-fly re-sync to bar beat start.



PLEASE FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS
