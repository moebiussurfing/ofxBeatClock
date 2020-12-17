# ofxBeatClock

**openFrameworks** addon to run a *DAW-Styled* BPM Beat-Clock with Internal Clock with Tap Tempo and External MIDI Sync and Ableton Link.

## Screencast

![gif](/readme_images/ofxBeatClock.gif?raw=true "gif")

## Screenshots

1 - Internal Clock mode:  
![image](/readme_images/Capture1.PNG?raw=true "image")

2 - External MidiClock mode:  
![image](/readme_images/Capture2.PNG?raw=true "image")

3 - Ableton Link mode:  
![image](/readme_images/Capture3.PNG?raw=true "image")


## Usage

Create the example project or your own with ```OF Project Generator``` as usual.  
Take care of required addons and the included ```bin/data``` files.  

### ofApp.h
```cpp
#include "ofxBeatClock.h"

ofxBeatClock beatClock;

ofEventListener listenerBeat;
void Changed_BeatTick();
```

### ofApp.cpp
```cpp
//ofApp::setup()
beatClock.setup();

//callback to receive BeatTicks
listenerBeat = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->Changed_BeatTick(); });

//-

//callback
void ofApp::Changed_BeatTick()
{
	if (beatClock.BeatTick_TRIG) ofLogWarning("ofApp") << "BeatTick ! Number: " << beatClock.Beat_current;
}
```

## Features

* **New feature**:  
**Ableton LINK** sync engine (master/slave).  
  **_[WIP but almost working]_**  
  Link engine is (maybe) disabled by default. To enable Link:
  * Add ```ofxAbletonLink``` addon to your app project. 
  * Uncomment ```#define USE_ofxAbletonLink``` on ```ofxBeatClock.h```. 
* **Internal Clock** based in a threaded timer from **ofxDawMetro** from **@castovoid**.  
You can uncomment `#define USE_AUDIO_BUFFER_TIMER_MODE` on `ofxBeatClock.h` to enable *BETA* alternative timer. (*WIP*)
* **External Clock Source** as **input MIDI Clock** (*Slave*) using **ofxMidi** from **@danomatika**.  
Easy to Sync to **Ableton Live** or any sequencer app with **Midi Clock**.
* **Tap Tempo Engine**.
* **Auto Save/Load** of all settings.
* Cute **customizable GUI** by editing the **JSON file theme**.
* Customizable GUI positions by code. Check other *API* methods too.
* Nice **Metronome Sound Ticks**.
* Draggable box to edit to layout: change preview info and GUI panel positions.

## Tested Systems
- **Windows10** / **VS2017** / **OF ~0.11**
- **macOS. High Sierra** / **Xcode9** & **Xcode10** / **OF ~0.11**

## Requeriments

* ofxMidi  
https://github.com/danomatika/ofxMidi  

* ofxGuiExtended2  
https://github.com/moebiussurfing/ofxGuiExtended2 [fork]  

* ofxAbletonLink  
https://github.com/2bbb/ofxAbletonLink [optional]  

* ofxDawMetro  
(allready included into ```OF_ADDON/libs```. No need to add manually!)  
https://github.com/castovoid/ofxDawMetro

* ofxSurfingHelpers
https://github.com/moebiussurfing/ofxSurfingHelpers

Take care of data folder files:
```/data/ofxBeatClock/```  
Xml settings, gui font file, and JSON theme. (app may crash if not present)

## Author
Addon by **@moebiusSurfing**  
*(ManuMolina). 2020.*

**_Thanks to developers of the included add-ons! @danomatika, @2bb, @castovoid & @frauzufall._**

## License
*MIT License.*

**_PLEASE FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS OR ISSUES!_**
