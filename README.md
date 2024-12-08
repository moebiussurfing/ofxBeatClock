# ofxBeatClock

**openFrameworks** add-on to run a *DAW-Styled BPM Beat-Clock*.  
**Internal Clock** with **Tap Tempo**, **External MIDI Sync** and **Ableton Link** modes.  
Receives a callback notification when each beat happens.  

## Screencast

![](/readme_images/ofxBeatClock.gif)  

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
void ofApp::draw()
{
  beatClock.draw();
}
void ofApp::Changed_Tick() // -> Callback to receive BeatTicks
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
* [ofxImGuiSurfing](https://github.com/moebiussurfing/ofxSurfingImGui)
* [ofxImGui](https://github.com/Daandelange/ofxImGui/). / _Fork from_ @**Daandelange**.  
* [ofxSurfingHelpers](https://github.com/moebiussurfing/ofxSurfingHelpers)
* [ofxAbletonLink](https://github.com/2bbb/ofxAbletonLink). / _Optional. It can be disabled._  
* [ofxMidiOutClock](https://github.com/moebiussurfing/ofxMidiOutClock). / _Optional for MIDI out clock. It can be disabled._  
* [ofxWindowApp](https://github.com/moebiussurfing/ofxWindowApp). / _Not required. For examples only._  

Already included into ```OF_ADD-ON/libs```. No need to add manually:  
* [ofxDawMetro](https://github.com/castovoid/ofxDawMetro)  

## Tested Systems
- **Windows 10** / **VS 2022** / **OF 0.12.0** / **OF 0.12+ Master branch could break**

## Author
An add-on by **@moebiusSurfing**  
( _ManuMolina_ ). _2020-2022_.

**_Thanks to the developers of the included core add-ons! @danomatika, @2bb and @castovoid._**

## License
*MIT License.*

## TODO
* Improve on-the-fly pushing sync/tweaking BPM smoothly.
* Test/improve all Ableton Link features like Ableton in slave mode or multiple peers. [?]
* Finish the improved Audio Buffer-based clock to allow more precision.  
This seems important when moving/hiding the Window. Sometimes the sync is lost. [?]

**FEEL FREE TO ADD MODIFICATIONS OR FEATURES AND TO SEND ME PULL REQUESTS OR ISSUES!**
