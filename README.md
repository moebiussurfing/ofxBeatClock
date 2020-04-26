# ofxBeatClock

openFrameworks addon to run a DAW styled BPM-Beat clock with tap tempo and external MIDI sync (slave) and Ableton Link (master/slave).


## Screencast
![Alt text](/ofxBeatClock.gif?raw=true "ofxBeatClock.gif")


## Screenshots

Internal clock mode:

![Alt text](/screenshot1.JPG?raw=true "screenshot1")

External clock MIDI Sync mode:

![Alt text](/screenshot2.JPG?raw=true "screenshot2")


## Usage

Create the example project or your own with OF ProjectGenerator as usual. Take care of required addons and the included /data files.


```c++
ofApp.h:

#include "ofxBeatClock.h"

...

ofxBeatClock beatClock;

//callback to receive beat-clock beat-ticks
ofEventListener beatListener;
void callback_BeatTick();
```

```
ofApp.cpp:

//setup()
beatClock.setup();

//callback to receive BeatTicks
beatListener = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->callback_BeatTick(); });

//-

//update()
beatClock.update();

//-

//draw()
beatClock.draw();

void ofApp::callback_BeatTick()
{
	if (beatClock.BeatTick_TRIG)
		ofLogWarning("ofApp") << "BeatTick ! " << beatClock.Beat_current;
}
```


## Features

- **New feature**: **Ableton LINK** sync engine (master/slave). __[WIP but almost working]__
  Link engine is (maybe) disabled by default. To enable Link:
  1. add ofxAbletonLink addon to your app project. 
  2. uncomment `#define USE_ofxAbletonLink` on `ofxBeatClock.h`. 
- Internal clock based in a threaded timer from ofxDawMetro from @castovoid.
  You can uncomment `#define USE_AUDIO_BUFFER_TIMER_MODE` on `ofxBeatClock.h` to enable *BETA* alternative timer.
- External clock source as input MIDI clock (slave) using ofxMidi from @danomatika.
  Easy to sync to Ableton Live or any sequencer with midi clock.
- Cute tap tempo engine.
- Save/load of all settings.
- Cute customizable GUI by editing the JSON file theme. ("theme_bleurgh.json". Based on a @transat theme)
- Customizable GUI positions by code. Check other methods too.
- Nice metronome sound ticks.



## Tested systems

- OF 0.11
- Windows10 / Visual Studio 2017
- macOS / HighSierra



## Requeriments

https://github.com/danomatika/ofxMidi  
https://github.com/castovoid/ofxDawMetro  
https://github.com/moebiussurfing/ofxGuiExtended2 (my fork)
https://github.com/2bbb/ofxAbletonLink (optional)

Take care of data folder:
`/data/ofxBeatClock/`  
xml settings, gui font file, and json theme. (app may crash if not present)



## About

An addon by **MoebiusSurfing**, 2020.  
__Thanks to developers of the included add-ons! @danomatika, @2bb, @castovoid & @frauzufall.__



## TODO:

- BUG: Repair problems when sometimes beat 1 tick it's displaced to beat 2...
- BUG: Some log errors must be repaired on ofxAbletonLink that seems drop down fps/performance...
~~- Add alternative and better timer approach using the audio-buffer to avoid out-of-sync problems of current timers (https://forum.openframeworks.cc/t/audio-programming-basics/34392/10). Problems happen when minimizing or moving the app window.. Any help is welcome!~~
- On-the-fly re-sync to bar beat start.
- A better link between play button/params in all internal/external clock source modes with one unique play button.  
- Add filter to smooth/stabilize BPM number when using external midi clock mode.

<br/>


**_PLEASE FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS OR ISSUES!_**
