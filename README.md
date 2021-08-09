# ofxBeatClock

**openFrameworks** add-on to run a *DAW-Styled BPM Beat-Clock*.  
**Internal Clock** with **Tap Tempo**, **External MIDI Sync** and **Ableton Link**.  

## Screencast

![GIF](/readme_images/ofxBeatClock.gif?raw=true "gif")  

## Usage

### ofApp.h
```cpp
#include "ofxBeatClock.h"

ofxBeatClock beatClock;

ofEventListener listener;
void Changed_Tick();
```

### ofApp.cpp
```cpp
void ofApp::setup()
{
  beatClock.setup();
  listener = beatClock.BeatTick.newListener([&](bool&) {this->Changed_Tick(); });
}
void ofApp::Changed_Tick() // callback to receive BeatTicks
{
  ofLogNotice() << "Beat! #" << beatClock.getBeat();
}
```

## Features

* **NEW FEATURE**:  
  **Ableton LINK** sync engine (Master/Slave). [*WIP: Maybe some protocol feature could be missing.. but working*]  
* **Internal Clock** based in a threaded timer from **ofxDawMetro** from **@castovoid**.  
You can uncomment ```#define USE_AUDIO_BUFFER_TIMER_MODE``` on ```ofxBeatClock.h``` to enable *BETA* alternative timer. [**WIP**]
* **Tap Tempo Engine**.
* **External Source** as **Input MIDI Clock** (*Slave*) using **ofxMidi** from **@danomatika**. Easy to Sync to **Ableton Live** or any sequencer app with **Midi Clock**.
* **Metronome Sound Ticks**.
* **ImGui** based **GUI**.  
* **Auto Save/Load** of all settings.

## Requeriments

* [ofxMidi](https://github.com/danomatika/ofxMidi)  
* [ofxAbletonLink](https://github.com/2bbb/ofxAbletonLink). Optional. Can be disabled.  
* [ofxImGuiSurfing](https://github.com/moebiussurfing/ofxSurfingImGui)
* [ofxImGui](https://github.com/Daandelange/ofxImGui/). Fork from @**Daandelange**.  
* [ofxScaleDragRect](https://github.com/moebiussurfing/ofxScaleDragRect)  
* [ofxWindowApp](https://github.com/moebiussurfing/ofxWindowApp). Optional. Can be disabled. 

Already included into ```OF_ADD-ON/libs```. No need to add manually:  
* [ofxDawMetro](https://github.com/castovoid/ofxDawMetro)  

## Tested Systems
- **Windows10** / **VS2017** / **OF ~0.11**

## Author
An add-on by **@moebiusSurfing**  
*(ManuMolina). 2020-2021.*

**_Thanks to the developers of the included core add-ons! @danomatika, @2bb and @castovoid._**

## License
*MIT License.*

## TODO
* Improve on-the-fly pushing sync/tweaking BPM smoothly.
* Test/improve all Ableton Link features like Ableton in slave mode or multiple peers. [?]
* Finish the improved Audio Buffer-based clock to allow more precision. This seems important when moving/hiding the Window. Sometimes the sync is lost. [?]

**PLEASE FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS OR ISSUES!**
