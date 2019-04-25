
#pragma once

#include "ofMain.h"

#include "ofxMidi.h"
#include "ofxMidiClock.h"
#include "ofxMidiTimecode.h"
#include "ofxGui.h"
#include "ofxUI.h"

#define MODE_ENABLE_BPM_ENGINE
#ifdef MODE_ENABLE_BPM_ENGINE
#include "MSABPMTapper.h"
#include "ofxBpm.h"
#endif

#define BPM_INIT 120
#define BPM_MIDI_CLOCK_REFRESH_RATE 200//refresh received MTC by clock. disabled/commented to "realtime" by frame update


//class ofApp : public ofBaseApp, public ofxMidiListener {
class ofxBeatClock : public ofxMidiListener

//class ofxBeatClock

{
    
public:

    ofxBeatClock () {}
    ~ofxBeatClock ();

    
    //-
    
    void setup();
    void update();
    void draw();
    void exit();

    
    
    //------------------------------
    
    // BPM CLOCK
    
    void setup_MIDI_CLOCK();
    void draw_MIDI_IN_CLOCK();
    
    //--
    
    /// ofxMidiListener callback
    //void newMidiMessage_CLOCK(ofxMidiMessage& eventArgs);
    void newMidiMessage(ofxMidiMessage& eventArgs);
    
    ofxMidiIn midiIn_CLOCK;
    ofxMidiMessage midiCLOCK_Message;
    
    // MIDI CLOCK
    
    ofxMidiClock clock; //< clock message parser
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
    ofParameter<int> beatsInBar; // compute remainder as # notes within the current bar
    
    unsigned long bpm_CheckUpdated_lastTime;
    
    //-
    
#ifdef MODE_ENABLE_BPM_ENGINE
    // BPM ENGINE
    
    ofSoundPlayer  mySound1;
    ofSoundPlayer  mySound2;
    
    msa::BPMTapper  bpmTapper;
    int tappet_division_SELECTED;
    //bool trig_curve_byNoise ;
    
    ofxBpm bpm;
    void onBeatEvent();
    bool BPM_gotBeat;
    
    //test bpm sync
    unsigned long BPM_LAST_Tick_Time_LAST;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED_PRE;//test
    long ELLAPSED_diff;//test
    
    //-
    
    ofParameter<float> BPM_of_PLAYER;//tempo bpm global
    void BPM_of_PLAYER_Changed(float & BPM_of_PLAYER);//listener for tempo changes
    
    ofParameter<bool> PLAYER_state;//true: playing | false: stopped
    void PLAYER_state_Changed(bool & PLAYER_state);
    
    ofParameter<bool> PLAYER_start_TRIG;//true: starting play
    ofParameter<bool> PLAYER_stop_TRIG;//true: stoping trig
    
    void PLAYER_START();//run master clock player
    void PLAYER_STOP();//stop master clock player
    
    // gui
    
    bool BPM_Metronome;//enable sound ticks
    ofParameter<bool> BPM_Tap_Tempo_TRIG;//trig measurements of tap tempo
    
    //-
    
    void beatsInBar_Changed(int & beatsInBar);
    
    //unsigned long bpm_beat_TICKER_lastTime;
    bool bpm_beat_TICKER;
    
#endif
    
    void gui_set_BPM();
    
    ofxUISuperCanvas *gui_BPM;
    
    ofParameter<bool> BPM_MASTER_CLOCK;//sync master tempo enabler
    ofParameter<int> BPM_TimeBar;//tiempo de 1 bar = 4 beats
    int beatsInBar_PRE;
    
    void gui_Event(ofxUIEventArgs &e);
    
    void keyPressed(int key);
    
    string TTF_message;
    ofTrueTypeFont TTF_small;
    ofTrueTypeFont TTF_medium;
    ofTrueTypeFont TTF_big;
    
    ofPoint metronome_ball_pos;
    int metronome_ball_radius;

};

