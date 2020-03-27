# ofxBeatClock

openFrameworks addon to run a DAW styled BPM-Beat clock with tap tempo and external MIDI sync.


## Screencast
![Alt text](/ofxBeatClock.gif?raw=true "ofxBeatClock.gif")


## Screenshots

Internal clock mode:

![Alt text](/screenshot1.JPG?raw=true "screenshot1")

External clock MIDI Sync mode:

![Alt text](/screenshot2.JPG?raw=true "screenshot2")


## Usage

Create the example project or your own with OF ProjectGenerator as usual. Take care of included /data files.


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

- Internal clock based in a threaded timer from ofxDawMetro from https://github.com/castovoid
- External clock source as input MIDI clock using ofxMidi 
- Easy to sync to Ableton Live or any sequencer with midi clock 
- Cute tap tempo engine
- Save/load mode settings
- Cute customizable GUI by editing the JSON file theme. ("theme_bleurgh.json". Based on a @transat theme)
- Customizable GUI positions by code
- Nice metronome sound ticks



## Tested systems

- OF 0.11
- Visual Studio 2017
- macOS / HighSierra



## Requeriments

https://github.com/danomatika/ofxMidi  
https://github.com/castovoid/ofxDawMetro  
https://github.com/moebiussurfing/ofxGuiExtended2 (my fork)

>/data/ofxBeatClock/  

xml settings, gui font file, and json theme. (app may crash if not present)



## About

An addon by MoebiusSurfing, 2020.  
Thanks to developers of the included add-ons!  
@danomatika & @castovoid.



## TODO:

- Add different and better timer approach using the audio-buffer to avoid out-of-sync problems of current normal and threaded timers. Problems happen when minimizing or moving the app window.. Any help is welcome!  
- On-the-fly re-sync to bar beat start.
- A better link between play button/params int both internal/external modes with one unique button.  
- Add filter to smooth/stabilize BPM number when using external mode.

<br/>


**PLEASE FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS OR ISSUES!**
