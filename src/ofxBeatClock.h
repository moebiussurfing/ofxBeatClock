
#pragma once

#include "ofMain.h"

#include "ofxGuiExtended.h"
#include "ofxMidi.h"
#include "ofxMidiClock.h"
#include "ofxMidiTimecode.h"
#include "ofxTapMachine.h"
#include "ofxDawMetro.h"

#define BPM_INIT 120
#define BPM_MIDI_CLOCK_REFRESH_RATE 200
//refresh received MTC by clock. disabled/commented to "realtime" by frame update

class ofxBeatClock : public ofxMidiListener, public ofxDawMetro::MetroListener {
    
private:
    ofPtr<ofxTapMachine> tapMachine;
    
public:
    
    //-
    
    void setup();
    void update();
    void draw();
    void exit();
    
    //--
    
    // DRAW STUFF
    
    // font
    
    string TTF_message;
    ofTrueTypeFont TTF_small;
    ofTrueTypeFont TTF_medium;
    ofTrueTypeFont TTF_big;
    
    // ball
    
    ofPoint metronome_ball_pos;
    int metronome_ball_radius;

    //------------------------------
    

    
    //--
    
    // MIDI IN CLOCK
    
    ofxMidiIn midiIn_CLOCK;
    ofxMidiMessage midiCLOCK_Message;
    ofxMidiClock clock; //< clock message parser

    void newMidiMessage(ofxMidiMessage& eventArgs);
    
    bool clockRunning; //< is the clock sync running?
    unsigned int beats; //< song pos in beats
    double seconds; //< song pos in seconds, computed from beats
    double bpm_CLOCK; //< song tempo in bpm, computed from clock length
    
    //float bpm_CLOCK; //< song tempo in bpm, computed from clock length
    //void bpm_CLOCK_Changed(double & bpm_CLOCK);//listener for tempo changes
    //void bpm_CLOCK_Changed(float & bpm_CLOCK);//listener for tempo changes
    
    // a MIDI beat is a 16th note, so do a little math to convert to a time signature:
    // 4/4 -> 4 notes per bar & quarter note = 1 beat, add 1 to count from 1 instead of 0
    int quarters; // convert total # beats to # quarters
    int bars; // compute # of bars
    
    //-
    
    ofParameter<int> beatsInBar; // compute remainder as # notes within the current bar
    void beatsInBar_Changed(int & beatsInBar);
    int beatsInBar_PRE;
    
    bool bpm_beat_TICKER;
    
    //--
    
    unsigned long bpm_CheckUpdated_lastTime;

    //--
    
    // SOUND
    
    bool ENABLE_sound;//enable sound ticks
    
    ofSoundPlayer  tic;
    ofSoundPlayer  tac;
    
    //-
    
    // INTERNAL CLOCK
    
    void setup_MIDI_CLOCK();
    void draw_BPM_CLOCK();
    
    //-

    // REFRESH FEQUENCY
    
    unsigned long BPM_LAST_Tick_Time_LAST;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED_PRE;//test
    long ELLAPSED_diff;//test
    
    //-
    
    // PLAYER MANAGER
    
    void PLAYER_START();
    void PLAYER_STOP();
    

    
    //---

    // GUI PARAMS
    
    void setup_Gui_CLOCKER();
    ofxGui gui_CLOCKER;
    ofxGuiContainer* container_controls;
    ofxGuiContainer* container_clocker;
    
    ofParameterGroup params_control;
    ofParameter<bool> enable_CLOCK;// enable clock
    ofParameter<bool> PLAYER_state;// player state
    ofParameter<bool> ENABLE_INTERNAL_CLOCK;// enable internal clock
    ofParameter<bool> ENABLE_MIDI_CLOCK;// enable midi clock sync
    
    ofParameterGroup params_clocker;
    ofParameter<float> BPM_value;//tempo bpm global
    ofParameter<int> BPM_TimeBar;//ms time of 1 bar = 4 beats
    ofParameter<bool> BPM_Tap_Tempo_TRIG;//trig measurements of tap tempo
    
    void Changed_gui_CLOCKER(ofAbstractParameter& e);
    
    //-
    
     // TAP TEMPO
    
    void barFunc(int &count);
    void minimFunc(int &count);
    void crochetFunc(int &count);
    void draw_Tapper();
    
    //-
    
    void saveSettings(string path);
    void loadSettings(string path);
    string pathSettings;
    
    int beatSquare = -1;
    bool beat_changed = false;
    
    //--
    
    // DAW METRO
    
    void draw_DAW();
    
    // overide ofxDawMetro::MetroListener's method if necessary
    void onBarEvent(int & bar) override;
    void onBeatEvent(int & beat) override;
    void onSixteenthEvent(int & sixteenth) override;

    
    ofxDawMetro metro;
    
    ofxGuiContainer* container_daw;
    ofParameterGroup params_daw;
    ofParameter<float> DAW_bpm;
    ofParameter<bool> DAW_active;
    
    void Changed_bpm_DAW(float & value);
    void Changed_DAW_active(bool & value);

    
};

