#pragma once

///TODO:
///+ On-the-fly re-sync to bar beat start.
///+ A better link between play button / params in both internal / external modes with one unique play button.
///+ Add filter to smooth / stabilize BPM number when using external midi clock mode.
///NOTE: Sorry, I am not sure why I am using more than one BPM vars...
///Maybe one of them is from midi clock, other from local daw timer, and other as the global and this is the finaly used, also the one to be smoothed..

#include "ofMain.h"

#include "ofxGuiExtended2.h"
#include "ofxMidi.h"
#include "ofxMidiClock.h"
#include "ofxMidiTimecode.h"
#include "ofxDawMetro.h"

#define USE_ofxAbletonLink
#ifdef USE_ofxAbletonLink
#include "ofxAbletonLink.h"
#endif

///+ Add alternative and better timer approach using the audio - buffer to avoid out - of - sync problems of current timers
///(https://forum.openframeworks.cc/t/audio-programming-basics/34392/10). Problems happen when minimizing or moving the app window.. Any help is welcome!
///audioBuffer alternative timer mode
///(code is at the bottom)
///un-comment to enable this NOT WORKING yet alternative mode
///THE PROBLEM: clock drift very often.. maybe in wasapi sound api?
///help on improve this is welcome!
//#define USE_AUDIO_BUFFER_TIMER_MODE

#define BPM_INIT 120

//only to long song mode on external midi sync
#define ENABLE_PATTERN_LIMITING//comment to disable
#define PATTERN_STEP_BAR_LIMIT 4
#define PATTERN_STEP_BEAT_LIMIT 16

//TODO:
//smooth global bpm clock
//#define BPM_MIDI_CLOCK_REFRESH_RATE 1000
////refresh received MTC by clock. disabled/commented to "realtime" by every-frame-update

//-

class ofxBeatClock : public ofxMidiListener, public ofxDawMetro::MetroListener {

public:

#ifdef USE_ofxAbletonLink
	ofxAbletonLink link;

	ofParameter<bool> ENABLE_LINK_SYNC;

	//TODO:
	//beat callback to LINK?
	//link.beat
	//use Changed_MIDI_beatsInBar(int &beatsInBar)

	void LINK_bpmChanged(double &bpm) {
		ofLogNotice("ofxBeatClock") << "LINK_bpmChanged" << bpm;

		BPM_Global = (float)bpm;
		DAW_bpm = (float)bpm;
		
	}

	void LINK_numPeersChanged(std::size_t &peers) {
		ofLogNotice("ofxBeatClock") << "LINK_numPeersChanged" << peers;
	}

	void LINK_playStateChanged(bool &state) {
		ofLogNotice("ofxBeatClock") << "LINK_playStateChanged" << (state ? "play" : "stop");
	}

	void LINK_setup() {
		link.setup();

		//ofAddListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
		//ofAddListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
		//ofAddListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);
	}

	void LINK_update();

	void LINK_draw() {
		ofPushStyle();

		float x = ofGetWidth() * link.getPhase() / link.getQuantum();
		ofSetColor(255, 0, 0);
		ofDrawLine(x, 0, x, ofGetHeight());

		std::stringstream ss("");
		ss
			<< "bpm:   " << link.getBPM() << std::endl
			<< "beat:  " << link.getBeat() << std::endl
			<< "phase: " << link.getPhase() << std::endl
			<< "peers: " << link.getNumPeers() << std::endl
			<< "play?: " << (link.isPlaying() ? "play" : "stop");

		ofSetColor(255);
		if (fontMedium.isLoaded())
		{
			fontMedium.drawString(ss.str(), 20, 20);
		}
		else
		{
			ofDrawBitmapString(ss.str(), 20, 20);
		}

		ofPopStyle();
	}

	void LINK_keyPressed(int key) {
		if (key == OF_KEY_LEFT) {
			if (20 < link.getBPM()) link.setBPM(link.getBPM() - 1.0);
		}
		else if (key == OF_KEY_RIGHT) {
			link.setBPM(link.getBPM() + 1.0);
		}
		else if (key == 'b') {
			link.setBeat(0.0);
		}
		else if (key == 'B') {
			link.setBeatForce(0.0);
		}
		else if (key == ' ') {
			link.setIsPlaying(!link.isPlaying());
		}
	}
#endif

	//-

public:
	void setup();
	void update();
	void draw();
	void exit();

	//-

#pragma mark - MIDI_IN_CLOCK

private:
	ofxMidiIn midiIn_CLOCK;
	ofxMidiMessage midiCLOCK_Message;
	ofxMidiClock MIDI_clock; //< clock message parser

	void newMidiMessage(ofxMidiMessage& eventArgs);

	bool bMidiClockRunning; //< is the clock sync running?
	unsigned int MIDI_beats; //< song pos in beats
	double MIDI_seconds; //< song pos in seconds, computed from beats
	double MIDI_CLOCK_bpm; //< song tempo in bpm, computed from clock length
	int MIDI_quarters; //convert total # beats to # quarters
	int MIDI_bars; //compute # of bars

	//-

	ofParameter<int> MIDI_beatsInBar;//compute remainder as # TARGET_NOTES_params within the current bar
	void Changed_MIDI_beatsInBar(int & beatsInBar);//only used in midiIn clock sync 
	int beatsInBar_PRE;//not required

	//-

#pragma mark - EXTERNAL_CLOCK

	void setup_MIDI_CLOCK();

	//-

#pragma mark - MONITOR

private:
	int pos_BeatBoxes_x, pos_BeatBoxes_y, pos_BeatBoxes_w;
	int pos_BeatBall_x, pos_BeatBall_y, pos_BeatBall_w;
	glm::vec2 pos_TextBpm;

public:
	void setPosition_BeatBoxes(int x, int y, int w);
	void setPosition_BeatBall(int x, int y, int w);
	void setPosition_Gui_ALL(int _x, int _y, int _w);
	void setPosition_TextBpm(int _x, int _y);//TODO:

	//beat boxes
	void drawBeatBoxes(int x, int y, int w);

	//beat tick ball
	void draw_BeatBall(int x, int y, int w);

private:
	//beat ball
	ofPoint circlePos;
	float animTime, animCounter;
	bool animRunning;
	float dt = 1.0f / 60.f;

	//main receiver
	void beatTick_MONITOR(int beat);///trigs sound and gui drawing ball visual feedback

public:
	ofParameter<bool> TRIG_TICK;

	//-

//	//TODO:
//private:
//smooth clock
//#pragma mark - REFRESH_FEQUENCY
//	//used only when BPM_MIDI_CLOCK_REFRESH_RATE is defined
//	unsigned long BPM_LAST_Tick_Time_LAST;//test
//	unsigned long BPM_LAST_Tick_Time_ELLAPSED;//test
//	unsigned long BPM_LAST_Tick_Time_ELLAPSED_PRE;//test
//	long ELLAPSED_diff;//test
//
//	unsigned long bpm_CheckUpdated_lastTime;

	//-

#pragma mark - GUI

public:
	void setup_Gui();
	void refresh_Gui();
	ofxGui gui;

private:
	ofxGuiGroup2* group_BEAT_CLOCK;//nested folder
	ofxGuiGroup2* group_Controls;
	ofxGuiGroup2* group_BpmTarget;
	ofxGuiGroup2* group_INTERNAL;
	ofxGuiGroup2* group_EXTERNAL;
	ofParameterGroup params_INTERNAL;
	ofParameterGroup params_EXTERNAL;
	//json theme
	ofJson confg_Button, confg_Sliders;

	//-

#pragma mark - PARAMS
private:
	ofParameterGroup params_CONTROL;
	ofParameter<bool> PLAYER_state;//player state
	ofParameter<bool> ENABLE_CLOCKS;//enable clock
	ofParameter<bool> ENABLE_INTERNAL_CLOCK;//enable internal clock
	ofParameter<bool> ENABLE_EXTERNAL_CLOCK;//enable midi clock sync
	ofParameter<int> MIDI_Port_SELECT;
	int num_MIDI_Ports = 0;

	ofParameterGroup params_BpmTarget;

public:
	ofParameter<float> BPM_Global;//global tempo bpm.
	//this is the final target bpm, is the destinations of all other clocks (internal, external midi sync or ableton link)
	ofParameter<int> BPM_GLOBAL_TimeBar;//ms time of 1 bar = 4 beats

private:
	ofParameter<bool> BPM_Tap_Tempo_TRIG;//trig the measurements of tap tempo

	//helpers
	ofParameter<bool> RESET_BPM_Global;
	ofParameter<bool> BPM_half_TRIG;//divide bpm by 2
	ofParameter<bool> BPM_double_TRIG;//multiply bpm by 2

	//-

	//API

public:
	bool getInternalClockModeState()
	{
		return ENABLE_INTERNAL_CLOCK;
	}
	bool getExternalClockModeState()
	{
		return ENABLE_EXTERNAL_CLOCK;
	}
	float get_BPM();
	int get_TimeBar();

private:
	bool bBallAutoPos = true;
public:
	void setPosition_BeatBall_Auto(bool b)
	{
		bBallAutoPos = b;
	}

	//-

private:
	void Changed_Params(ofAbstractParameter& e);

	//-

	//internal clock

	//based on threaded timer using ofxDawMetro
#pragma mark - DAW METRO
	void reSync();
	ofParameter<bool> bSync_Trig;

	ofxDawMetro dawMetro;

	//callbacks
	//overide ofxDawMetro::MetroListener's method if necessary
	void onBarEvent(int & bar) override;
	void onBeatEvent(int & beat) override;
	void onSixteenthEvent(int & sixteenth) override;

	ofParameterGroup params_daw;
	ofParameter<float> DAW_bpm;
	ofParameter<bool> DAW_active;
	void Changed_DAW_bpm(float & value);
	void Changed_DAW_active(bool & value);
	//ofxGuiContainer* container_daw;

public:
	void set_DAW_bpm(float bpm);//to set bpm from outside

	//-

	//settings
#pragma mark - XML SETTINGS
private:
	void saveSettings(string path);
	void loadSettings(string path);
	string pathSettings;
	string filenameControl = "BeatClock_Settings.xml";
	string filenameMidiPort = "MidiInputPort_Settings.xml";

	//-

#pragma mark - DRAW_STUFF:

	//FONT

	string messageInfo;
	ofTrueTypeFont fontSmall;
	ofTrueTypeFont fontMedium;
	ofTrueTypeFont fontBig;

	//-

	//BEAT BALL

	ofPoint metronome_ball_pos;
	int metronome_ball_radius;

	//-

	ofParameter<bool> SHOW_Extra;//beat boxes and beat ball

	//-

#pragma mark - SOUND

	ofParameter<bool> ENABLE_sound;//enable sound ticks
	ofParameter<float> volumeSound;//sound ticks volume
	ofSoundPlayer tic;
	ofSoundPlayer tac;
	ofSoundPlayer tapBell;

	//-

#pragma mark - CURRENT_BPM_CLOCK_VALUES
public:
	void Reset_clockValuesAndStop();

	//TODO: could be nice to add some listener system..

	ofParameter<int> Bar_current;
	ofParameter<int> Beat_current;
	ofParameter<int> Tick_16th_current;

	//TODO: LINK
#ifdef USE_ofxAbletonLink
	int Beat_current_PRE;
	float Beat_float_current;
	string Beat_float_string;
#endif

private:
	//STRINGS FOR MONITOR DRAWING

	//1:1:1
	string Bar_string;
	string Beat_string;
	string Tick_16th_string;

	string clockActive_Type;//internal/external
	string clockActive_Info;//midi in port

	//-

#pragma mark - API

public:
	void draw_BigClockTime(int x, int y);

	void PLAYER_START();
	void PLAYER_STOP();
	void PLAYER_TOGGLE();

	//layout
	void setPosition_Gui(int x, int y, int w);
	ofPoint getPosition_Gui();
	void set_Gui_visible(bool b);
	void set_BeatBall_visible(bool b);

	bool get_Gui_visible()
	{
		return gui.getVisible();
	}
	void toggle_Gui_visible()
	{
		bool b = get_Gui_visible();
		set_Gui_visible(!b);
	}

	bool isPlaying()
	{
		return bIsPlaying;
	}

	void loadTheme(string s)
	{
		group_BEAT_CLOCK->loadTheme(s);
		group_Controls->loadTheme(s);
		group_BpmTarget->loadTheme(s);
		group_INTERNAL->loadTheme(s);
		group_EXTERNAL->loadTheme(s);
	}
private:
	int gui_Panel_W, gui_Panel_posX, gui_Panel_posY, gui_Panel_padW;

	bool bIsPlaying;

	//-

#pragma mark - TAP_ENGINE

public:
	void Tap_Trig();
	void Tap_update();

private:
	int tapCount, lastTime, avgBarMillis;
	float Tap_BPM;
	vector<int> tapIntervals;
	bool bTap_running;
	bool SOUND_wasDisabled = false;//sound disbler to better flow

	//-

#pragma mark - CHANGE MIDI PORT

	int midiIn_CLOCK_port_OPENED;
	void setup_MIDI_PORT(int p);
	int MIDI_Port_PRE = -1;

	ofParameter <string> midiPortName{ "","" };

	//-

#pragma mark - STEP LIMITING

	//we don't need to use long song patterns
	//and we will limit bars to 4 like a simple step sequencer.
	bool ENABLE_pattern_limits;
	int pattern_BEAT_limit;
	int pattern_BAR_limit;

	//--

	string myTTF;//gui font path
	int sizeTTF;

	//-

	//TODO:
	//audioBuffer alternative timer mode
	//based on:
	//https://forum.openframeworks.cc/t/audio-programming-basics/34392/10?u=moebiussurfing
	//by davidspry
	//"the way I’m generating the clock is naive and simple.I’m simply counting the number of samples written to the buffer and sending a notification each time the number of samples is equal to one subdivided “beat”, as in BPM.
	//Presumably there’s some inconsistency because the rate of writing samples to the buffer is faster than the sample rate, but it seems fairly steady with a small buffer size."

	//we can maybe use the same soundStream used for the sound tick also for the audiBuffer timer..
	//maybe will be a better solution to put this timer into ofxDawMetro class!!

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
private:
	void setupAudioBuffer(int _device);
	void closeAudioBuffer();
	ofParameter<bool> MODE_AudioBufferTimer;
	ofSoundStream soundStream;
	int deviceOut;
	int samples = 0;
	int ticksPerBeat = 4;
	//default is 4. 
	//is this a kind of resolution if we set bigger like 16?
	int samplesPerTick;
	int sampleRate;
	int bufferSize;
	int DEBUG_ticks = 0;
	bool DEBUG_bufferAudio = false;
public:
	void audioOut(ofSoundBuffer &buffer);
#endif

	//-
};

