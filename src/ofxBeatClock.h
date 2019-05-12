
#pragma once

// TODO:
// + fade blink ball when tapping tempo

#include "ofMain.h"

#include "ofxGuiExtended.h"
#include "ofxMidi.h"
#include "ofxMidiClock.h"
#include "ofxMidiTimecode.h"
#include "ofxDawMetro.h"

#define BPM_INIT 120
#define ENABLE_PATTERN_LIMITING //comment to disable: to long song mode
#define PATTERN_STEP_BAR_LIMIT 4
#define PATTERN_STEP_BEAT_LIMIT 16

#define BPM_MIDI_CLOCK_REFRESH_RATE 200
//refresh received MTC by clock. disabled/commented to "realtime" by frame update

class ofxBeatClock : public ofxMidiListener, public ofxDawMetro::MetroListener {

public:
    
    //-
    
    void setup();
    void update();
    void draw();
    void exit();
    
    //-

#pragma mark - MIDI IN CLOCK
    
    ofxMidiIn midiIn_CLOCK;
    ofxMidiMessage midiCLOCK_Message;
    ofxMidiClock MIDI_clock; //< clock message parser

    void newMidiMessage(ofxMidiMessage& eventArgs);
    
    bool clockRunning; //< is the clock sync running?
    unsigned int MIDI_beats; //< song pos in beats
    double MIDI_seconds; //< song pos in seconds, computed from beats
    double MIDI_CLOCK_bpm; //< song tempo in bpm, computed from clock length
    int MIDI_quarters; // convert total # beats to # quarters
    int MIDI_bars; // compute # of bars
    
    //-
    
    ofParameter<int> MIDI_beatsInBar; // compute remainder as # notes within the current bar
    void Changed_MIDI_beatsInBar(int & beatsInBar);
    int beatsInBar_PRE;//not required

    
    //-
    
#pragma mark - EXTERNAL CLOCK
    
    void setup_MIDI_CLOCK();

    //-

#pragma mark - MONITOR

    void setPosition_Squares(int x, int y, int w);
    void setPosition_Ball(int x, int y, int w);

    //squares
    void draw_SQUARES(int x, int y, int w);
    int pos_Squares_x, pos_Squares_y, pos_Squares_w;

    //tick ball
    void draw_BALL(int x, int y, int w);
    int pos_Ball_x, pos_Ball_y, pos_Ball_w;

    void CLOCK_Tick_MONITOR(int beat);
    bool TRIG_Ball_draw ;

    void setPosition_Gui_ALL(int _x, int _y, int _w);
    
    //-

#pragma mark - REFRESH FEQUENCY
    
    unsigned long BPM_LAST_Tick_Time_LAST;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED;//test
    unsigned long BPM_LAST_Tick_Time_ELLAPSED_PRE;//test
    long ELLAPSED_diff;//test
    
    unsigned long bpm_CheckUpdated_lastTime;
    
    //-

#pragma mark - GUI
    
    void setup_Gui();

    // TODO: convert to panel to enable mouse drag
    // TODO: also disabler for header
    
    ofxGui gui;
    
    //ofxGuiGroup* group_transport;//nested folder
    ofxGuiPanel* group_transport;// main nested folder

    ofJson conf_Cont, confg_Sliders, confg_Button;//json theme
    ofxGuiContainer* container_controls;
    //TODO: switch to groups to minimize..
    ofxGuiContainer* container_clocker;

    ofxGuiGroup* group_INTERNAL;
    ofxGuiGroup* group_EXTERNAL;

    ofParameterGroup params_INTERNAL;
    ofParameterGroup params_EXTERNAL;

    //-
    
#pragma mark - PARAMS
    
    ofParameterGroup params_control;
    ofParameter<bool> PLAYER_state;// player state
    ofParameter<bool> ENABLE_CLOCKS;// enable clock
    ofParameter<bool> ENABLE_INTERNAL_CLOCK;// enable internal clock
    ofParameter<bool> ENABLE_EXTERNAL_CLOCK;// enable midi clock sync
    ofParameter<int> MIDI_Port_SELECT;
    int num_MIDI_Ports = 0;

    ofParameterGroup params_clocker;
    ofParameter<float> BPM_Global;//tempo bpm global
    ofParameter<int> BPM_GLOBAL_TimeBar;//ms time of 1 bar = 4 beats

    ofParameter<bool> BPM_Tap_Tempo_TRIG;//trig measurements of tap tempo
    //ofParameter<void> BPM_Tap_Tempo_button;//trig measurements of tap tempo

    //-
    
    // API

    float get_BPM();
    int get_TimeBar();

    //-

    void Changed_Params(ofAbstractParameter& e);

    //-
    
#pragma mark - DAW METRO
    
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

    void set_DAW_bpm(float bpm);//to set bpm from outside

    //-
    
#pragma mark - XML SETTINGS
    
    void saveSettings(string path);
    void loadSettings(string path);
    string pathSettings;

    //-

#pragma mark - DRAW STUFF:
    
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
    
#pragma mark - SOUND
    
    ofParameter<bool> ENABLE_sound;//enable sound ticks
    ofSoundPlayer tic;
    ofSoundPlayer tac;
    
    //-

#pragma mark - CURRENT BPM CLOCK VALUES


    void RESET_clockValues();

    // TODO: could be nice to add listener system..

    ofParameter<int> BPM_bar_current;
    ofParameter<int> BPM_beat_current;
    ofParameter<int> BPM_16th_current;

    // STRINGS FOR MONITOR DRAWING

    string BPM_bar_str;
    string BPM_beat_str;
    string BPM_16th_str;

    string BPM_input_str;//internal/external
    string BPM_name_str;//midi in port

    //-

#pragma mark - API

    void draw_BigClockTime(int x, int y);

    void PLAYER_START();
    void PLAYER_STOP();
    void PLAYER_TOGGLE();
    bool isPlaying;
    void setPosition_Gui(int x, int y, int w);
    ofPoint getPosition_Gui();
    void set_Gui_visible( bool b);
    
    // gui screen settings
    int gui_Panel_W, gui_Panel_posX, gui_Panel_posY, gui_Panel_padW;

    //-

#pragma mark - TAP BPM

    void Tap_Trig();
    void Tap_update();
    int tapCount, lastTime, avgBarMillis;
    float Tap_BPM;
    vector<int> intervals;
    bool Tap_running;
    bool SOUND_wasDisabled = false;// sound disbler to better flow

    //-

#pragma mark - CHANGE MIDI PORT

    int midiIn_CLOCK_port_OPENED;
    void setup_MIDI_PORT(int p);
    int MIDI_Port_PRE = -1;

    //-

#pragma mark - STEP LIMITING

    // we dont need to use long song patterns
    bool ENABLE_pattern_limits;
    int pattern_BEAT_limit;
    int pattern_BAR_limit;

    //--

    string myTTF;// gui font path
    int sizeTTF;

    //-

//    void toggleGroupHeader(bool val);

};

