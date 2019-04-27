
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
    
    //-
    
    // MIDI IN CLOCK
    
    ofxMidiIn midiIn_CLOCK;
    ofxMidiMessage midiCLOCK_Message;
    ofxMidiClock MIDI_clock; //< clock message parser

    void newMidiMessage(ofxMidiMessage& eventArgs);
    
    bool clockRunning; //< is the clock sync running?
    unsigned int MIDI_beats; //< song pos in beats
    double MIDI_seconds; //< song pos in seconds, computed from beats
    double MIDI_CLOCK_bpm; //< song tempo in bpm, computed from clock length
    
    //float bpm_CLOCK; //< song tempo in bpm, computed from clock length
    //void bpm_CLOCK_Changed(double & bpm_CLOCK);//listener for tempo changes
    //void bpm_CLOCK_Changed(float & bpm_CLOCK);//listener for tempo changes
    
    // a MIDI beat is a 16th note, so do a little math to convert to a time signature:
    // 4/4 -> 4 notes per bar & quarter note = 1 beat, add 1 to count from 1 instead of 0
    int MIDI_quarters; // convert total # beats to # quarters
    int MIDI_bars; // compute # of bars
    
    //-
    
    ofParameter<int> MIDI_beatsInBar; // compute remainder as # notes within the current bar
    void Changed_MIDI_beatsInBar(int & beatsInBar);
    int beatsInBar_PRE;//not required
    
    bool MIDI_Bang_Beat_Monitor;
    
    //-
    
    // EXTERNAL CLOCK
    
    void setup_MIDI_CLOCK();

    //-

    // MONITOR

    void draw_MONITOR(int x, int y);
    int posMon_x, posMon_Y;
    void CLOCK_Tick_MONITOR(int beat);
    bool TRIG_Ball_draw = false;

    //-

    // REFRESH FEQUENCY
    
    unsigned long BPM_LAST_Tick_Time_LAST;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED_PRE;//test
    long ELLAPSED_diff;//test
    
    unsigned long bpm_CheckUpdated_lastTime;
    
    //-

    // GUI
    
    void setup_Gui();
    ofxGui gui_CLOCKER;
    ofxGuiContainer* container_controls;
    ofxGuiContainer* container_clocker;
    
    //-
    
    // PARAMS
    
    ofParameterGroup params_control;
    ofParameter<bool> PLAYER_state;// player state
    ofParameter<bool> ENABLE_CLOCKS;// enable clock
    ofParameter<bool> ENABLE_INTERNAL_CLOCK;// enable internal clock
    ofParameter<bool> ENABLE_EXTERNAL_CLOCK;// enable midi clock sync
    
    ofParameterGroup params_clocker;
    ofParameter<float> BPM_Global;//tempo bpm global
    ofParameter<int> BPM_TimeBar;//ms time of 1 bar = 4 beats
    ofParameter<bool> BPM_Tap_Tempo_TRIG;//trig measurements of tap tempo
    
    void Changed_Params(ofAbstractParameter& e);
    
    //-
    
    // TAP TEMPO
    
    void barFunc(int &count);
    void minimFunc(int &count);
    void crochetFunc(int &count);
    void draw_Tapper();

    //-
    
    // DAW METRO
    
    // overide ofxDawMetro::MetroListener's method if necessary
    void onBarEvent(int & bar) override;
    void onBeatEvent(int & beat) override;
    void onSixteenthEvent(int & sixteenth) override;
    ofxDawMetro metro;
    
    ofParameterGroup params_daw;
    ofParameter<float> DAW_bpm;
    ofParameter<bool> DAW_active;
    void Changed_DAW_bpm(float & value);
    void Changed_DAW_active(bool & value);

    ofxGuiContainer* container_daw;

    //-
    
    // XML SETTINGS
    
    void saveSettings(string path);
    void loadSettings(string path);
    string pathSettings;

    //-

    // DRAW STUFF:
    
    // FONT
    
    string TTF_message;
    ofTrueTypeFont TTF_small;
    ofTrueTypeFont TTF_medium;
    ofTrueTypeFont TTF_big;

    //-

    // BALL
    
    ofPoint metronome_ball_pos;
    int metronome_ball_radius;
    
    //-
    
    // SOUND
    
    ofParameter<bool> ENABLE_sound;//enable sound ticks
    ofSoundPlayer tic;
    ofSoundPlayer tac;
    
    //-

    // STRINGS FOR MONITOR DRAWING
    
    string BPM_input_str;
    string BPM_name_str;
    string BPM_bar_str;
    string BPM_beat_str;
    string BPM_sixteen_str;

    int BPM_beat_current;

    //-
    
    // API

    void draw_BigClockTime(int x, int y);

    void PLAYER_START();
    void PLAYER_STOP();
    void PLAYER_TOGGLE();
    bool isPlaying;

    //-

};

