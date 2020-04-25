#pragma once

/// TODO:
///
/// + On-the-fly bang re-sync to bar beat start. (kind of manual syncer)
/// + Add filter to smooth / stabilize BPM number when using external midi clock mode.
/// + Add audio output selector to metronome sounds. maybe share audioBuffer with better timer mode
///	 	on USE_AUDIO_BUFFER_TIMER_MODE. still disabled by default yet

#include "ofMain.h"

#include "ofxMidiClock.h"//used for external midi clock sync (1)
#include "ofxMidi.h"
#include "ofxMidiTimecode.h"
#include "ofxDawMetro.h"//used for internal (using threaded timer) clock (2)
#include "ofxGuiExtended2.h"

//----

#define USE_ofxAbletonLink
#ifdef USE_ofxAbletonLink
#include "ofxAbletonLink.h"//used for external Ableton Live Link engine (3)
#endif
///What is Ableton Link?
///"This is the codebase for Ableton Link, a technology that synchronizes musical beat,
///tempo, and phase across multiple applications running on one or more devices.
///Applications on devices connected to a local network discover each other automatically
///and form a musical session in which each participant can perform independently: 
///anyone can start or stop while still staying in time.Anyone can change the tempo, 
///the others will follow.Anyone can join or leave without disrupting the session."
///https://github.com/Ableton/link
///https://www.ableton.com/en/link/

//----

///TODO:
//#define USE_AUDIO_BUFFER_TIMER_MODE
//used as audioBuffer timer as an alternative for the internal clock (4)
///when it's enabled ofxDawMetro is not used and could be not loaded.
///WIP: alternative and better timer approach using the audio-buffer to avoid out-of-sync problems of current timers
///(https://forum.openframeworks.cc/t/audio-programming-basics/34392/10). 
///Problems happen when minimizing or moving the app window.. Any help is welcome!
///(code is at the bottom)
///un-comment to enable this NOT WORKING yet alternative mode
///THE PROBLEM: clock drift very often.. maybe in wasapi sound api?
///help on improve this is welcome!
///NOTE: if the audio output/driver is not opened properly, fps performance seems to fall...
///TODO: should make easier to select sound output

//----

///TODO:
///smooth global bpm clock
///could be only visual refreshing the midi clock slower or using a real filter.
///#define BPM_MIDI_CLOCK_REFRESH_RATE 1000
/////refresh received MTC by clock. disabled/commented to "realtime" by every-frame-update

//----

//only to long song mode on external midi sync vs simpler 4 bars / 16 beats
#define ENABLE_PATTERN_LIMITING//comment to disable
#define PATTERN_STEP_BAR_LIMIT 4
#define PATTERN_STEP_BEAT_LIMIT 16

#define BPM_INIT 120
#define BPM_INIT_MIN 30
#define BPM_INIT_MAX 300

#define USE_VISUAL_FEEDBACK_FADE//comment to try improve performance... Could add more optimizations maybe

//-

class ofxBeatClock : public ofxMidiListener, public ofxDawMetro::MetroListener {

	//-

#pragma mark - OF_MAIN

public:
	void setup();
	void update();
	void draw();
	void exit();

	//-

#pragma mark - MIDI_IN_CLOCK

private:
	ofxMidiIn midiIn;
	ofxMidiMessage midiIn_Clock_Message;
	ofxMidiClock midiIn_Clock; //< clock message parser

	void newMidiMessage(ofxMidiMessage& eventArgs);

	double midiIn_Clock_Bpm; //< song tempo in bpm, computed from clock length
	bool bMidiInClockRunning; //< is the clock sync running?
	unsigned int MIDI_beats; //< song pos in beats
	double MIDI_seconds; //< song pos in seconds, computed from beats
	int MIDI_quarters; //convert total # beats to # quarters
	int MIDI_bars; //compute # of bars
	//TEST
	//int MIDI_ticks;//16th ticks are not implemented on the used ofxMidi example

	//-

	ofParameter<int> midiIn_BeatsInBar;//compute remainder as # TARGET_NOTES_params within the current bar
	void Changed_midiIn_BeatsInBar(int & beatsInBar);//only used in midiIn clock sync 
	int beatsInBar_PRE;//not required

	//-

#pragma mark - EXTERNAL_MIDI_CLOCK

	void setup_MidiIn_Clock();

	//-

#pragma mark - LAYOUT

private:
	bool DEBUG_moreInfo = false;//more debug

	glm::vec2 pos_Global;//main anchor to reference all the other above gui elements
	glm::vec2 pos_ClockInfo;
	glm::vec2 pos_BpmInfo;
	int pos_BeatBoxes_x, pos_BeatBoxes_y, pos_BeatBoxes_width;
	int pos_BeatBall_x, pos_BeatBall_y, pos_BeatBall_radius;

	//TODO:
	//ofParameter<glm::vec2> position_BeatBoxes;
	//ofParameter<glm::vec2> position_BeatBall;
	//ofParameter<glm::vec2> position_ClockInfo;
	//ofParameter<glm::vec2> position_BpmInfo;

public:
	//api setters

	void setPosition_GuiGlobal(int x, int y);//main global position setter for gui panel and extra elements

	void setPosition_GuiExtra(int x, int y);//extra elements position setter with default layout of the other elements
	void setPosition_BeatBoxes(int x, int y, int w);//position x, y and w = width of all 4 squares
	void setPosition_BeatBall(int x, int y, int w);//position x, y and w = width of ball
	void setPosition_ClockInfo(int _x, int _y);//all clock source info
	void setPosition_BpmInfo(int _x, int _y);//current bpm

	//beat boxes
	void draw_BeatBoxes(int x, int y, int w);

	//beat tick ball
	void draw_BeatBall(int x, int y, int radius);

	//beat tick ball
	void draw_ClockInfo(int x, int y);

	//beat tick ball
	void draw_BpmInfo(int x, int y);

	//big clock
	void draw_BigClockTime(int x, int y);

	//--

	//debug helpers

	//red anchor circle to debug mark
	bool DEBUG_Layout = false;
	void draw_Anchor(int x, int y)
	{
		if (DEBUG_Layout)
		{
			ofPushStyle();
			ofFill();
			ofSetColor(ofColor::red);
			ofDrawCircle(x, y, 3);
			int pad;
			if (y < 15) pad = 10;
			else pad = -10;
			ofDrawBitmapStringHighlight(ofToString(x) + "," + ofToString(y), x, y + pad);
			ofPopStyle();
		}
	}
	void setDebug_Clock(bool b)
	{
		DEBUG_moreInfo = b;
	}
	void toggleDebug_Clock()
	{
		DEBUG_moreInfo = !DEBUG_moreInfo;
	}
	void setDebug_Layout(bool b)
	{
		DEBUG_Layout = b;
	}
	void toggleDebug_Layout()
	{
		DEBUG_Layout = !DEBUG_Layout;
	}

	//--

private:
	//main text color
	ofColor colorText;

	//-

#pragma mark - MONITOR_VISUAL_FEEDBACK

private:
	//beat ball
	ofPoint circlePos;
	float fadeOut_animTime, fadeOut_animCounter;
	bool fadeOut_animRunning;
	float dt = 1.0f / 60.f;

	//main receiver
	//trigs sound and gui drawing ball visual feedback
	void beatTick_MONITOR(int beat);

public:
	ofParameter<bool> BeatTick_TRIG;//this trigs to draw a flashing circle for a frame only

	//-

////TODO:
//private:
//smooth clock for midi input clock sync
//#pragma mark - REFRESH_FEQUENCY
////used only when BPM_MIDI_CLOCK_REFRESH_RATE is defined
//unsigned long BPM_LAST_Tick_Time_LAST;//test
//unsigned long BPM_LAST_Tick_Time_ELLAPSED;//test
//unsigned long BPM_LAST_Tick_Time_ELLAPSED_PRE;//test
//long ELLAPSED_diff;//test
//unsigned long bpm_CheckUpdated_lastTime;

	//-

#pragma mark - GUI

public:
	void setup_GuiPanel();
	void refresh_Gui();
	ofxGui gui;

private:
	ofxGuiGroup2* group_BeatClock;//nested folder
	ofxGuiGroup2* group_Controls;
	ofxGuiGroup2* group_Advanced;
	ofxGuiGroup2* group_INTERNAL;
	ofxGuiGroup2* group_EXTERNAL_MIDI;
	ofParameterGroup params_INTERNAL;
	ofParameterGroup params_EXTERNAL_MIDI;
	ofJson confg_Button, confg_ButtonSmall, confg_Sliders;//json theme

	//-

#pragma mark - PARAMS
	public:
		ofParameter<bool> PLAYING_Global_State;//for all different source clock modes

private:
	ofParameterGroup params_CONTROL;
	ofParameter<bool> PLAYING_State;//player state
	ofParameter<bool> ENABLE_CLOCKS;//enable clock
	ofParameter<bool> ENABLE_INTERNAL_CLOCK;//enable internal clock
	ofParameter<bool> ENABLE_EXTERNAL_MIDI_CLOCK;//enable midi clock sync
	ofParameter<int> midiIn_Port_SELECT;
	int midiIn_numPorts = 0;

	ofParameterGroup params_Advanced;

	//----

public:
	//this is the final target bpm, is the destinations of all other clocks (internal, external midi sync or ableton link)
	ofParameter<float> BPM_Global;//global tempo bpm.
	ofParameter<int> BPM_Global_TimeBar;//ms time of 1 bar = 4 beats

	//----

private:
	ofParameter<bool> BPM_Tap_Tempo_TRIG;//trig the measurements of tap tempo

	//helpers to modify current bpm
	ofParameter<bool> RESET_BPM_Global;
	ofParameter<bool> BPM_half_TRIG;//divide bpm by 2
	ofParameter<bool> BPM_double_TRIG;//multiply bpm by 2

	//-

	//API

public:
	float getBPM();
	int getTimeBar();

	bool getInternalClockModeState()
	{
		return ENABLE_INTERNAL_CLOCK;
	}
	bool getExternalClockModeState()
	{
		return ENABLE_EXTERNAL_MIDI_CLOCK;
	}
#ifdef USE_ofxAbletonLink
	bool getLinkClockModeState()
	{
		return ENABLE_LINK_SYNC;
	}
#endif

	//-

private:
	//main callback handler
	void Changed_Params(ofAbstractParameter& e);

	//-

	//internal clock

	//based on threaded timer using ofxDawMetro
#pragma mark - INTERNAL_CLOCK

	ofxDawMetro clockInternal;

	//callbacks defined inside the addon class. can't be renamed here
	//overide ofxDawMetro::MetroListener's method if necessary
	void onBarEvent(int & bar) override;
	void onBeatEvent(int & beat) override;
	void onSixteenthEvent(int & sixteenth) override;

	ofParameterGroup params_ClockInternal;
	ofParameter<float> clockInternal_Bpm;
	ofParameter<bool> clockInternal_Active;
	void Changed_ClockInternal_Bpm(float & value);
	void Changed_ClockInternal_Active(bool & value);

	//TODO:
	void reSync();
	ofParameter<bool> bSync_Trig;

public:
	void setBpm_ClockInternal(float bpm);//to set bpm from outside

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

	//font
	string messageInfo;
	ofTrueTypeFont fontSmall;
	ofTrueTypeFont fontMedium;
	ofTrueTypeFont fontBig;

	//-

	//beat ball
	ofPoint metronome_ball_pos;
	int metronome_ball_radius;

	//-

	ofParameter<bool> SHOW_Extra;//beat boxes, text info and beat ball
	ofParameter<bool> SHOW_Advanced;
	//-

#pragma mark - SOUND
	//sound metronome
	ofParameter<bool> ENABLE_sound;//enable sound ticks
	ofParameter<float> volumeSound;//sound ticks volume
	ofSoundPlayer tic;
	ofSoundPlayer tac;
	ofSoundPlayer tapBell;

	//-

#pragma mark - CURRENT_BPM_CLOCK_VALUES
public:
	void reset_clockValuesAndStop();

	//TODO: could be nice to add some listener system..
	ofParameter<int> Bar_current;
	ofParameter<int> Beat_current;
	ofParameter<int> Tick_16th_current;

	//TODO: 
	//link
#ifdef USE_ofxAbletonLink
	int Beat_current_PRE;
	float Beat_float_current;
	string Beat_float_string;
#endif

private:

	//strings for monitor drawing
	//1:1:1
	string Bar_string;
	string Beat_string;
	string Tick_16th_string;

	string clockActive_Type;//internal/external/link clock types name
	string clockActive_Info;//midi in port, and extra info for any clock source

	//----

#pragma mark - API

	//----

public:

	//transport control
	void start();
	void stop();
	void togglePlay();

	//----

	//layout
	void setPosition_GuiPanel(int x, int y, int w);//gui panel
	ofPoint getPosition_GuiPanel();
	void setVisible_GuiPanel(bool b);
	void setVisible_BeatBall(bool b);

	bool getVisible_GuiPanel()
	{
		return gui.getVisible();
	}
	void toggleVisible_GuiPanel()
	{
		bool b = getVisible_GuiPanel();
		setVisible_GuiPanel(!b);
	}

	bool isPlaying()
	{
		return bIsPlaying;
	}

	//NOTE: take care with the path font defined on the config json 
	//because ofxGuiExtended crashes if fonts are not located on /data
	void loadTheme(string s)
	{
		group_BeatClock->loadTheme(s);
		group_Controls->loadTheme(s);
		group_Advanced->loadTheme(s);
		group_INTERNAL->loadTheme(s);
		group_EXTERNAL_MIDI->loadTheme(s);
	}

private:
	int gui_Panel_Width, gui_Panel_posX, gui_Panel_posY;

	bool bIsPlaying;

	//-

#pragma mark - TAP_ENGINE

public:
	void tap_Trig();
	void tap_Update();

private:
	float tap_BPM;
	int tap_Count, tap_LastTime, tap_AvgBarMillis;
	vector<int> tap_Intervals;
	bool bTap_Running;
	bool SOUND_wasDisabled = false;//sound disbler to better user workflow

	//-

#pragma mark - CHANGE_MIDI_IN_PORT

	void setup_MidiIn_Port(int p);
	int midiIn_Clock_Port_OPENED;
	int midiIn_Port_PRE = -1;
	ofParameter<string> midiIn_PortName{ "","" };

	//-

#pragma mark - STEP LIMITING

	//we don't need to use long song patterns
	//and we will limit bars to 4 like a simple step sequencer.
	bool ENABLE_pattern_limits;
	int pattern_BEAT_limit;
	int pattern_BAR_limit;

	//----

	//TODO:
	//audioBuffer alternative timer mode to get a a more accurate clock!
	//based on:
	//https://forum.openframeworks.cc/t/audio-programming-basics/34392/10?u=moebiussurfing
	//by davidspry:
	//"the way I’m generating the clock is naive and simple.I’m simply counting the number of samples written to the buffer and sending a notification each time the number of samples is equal to one subdivided “beat”, as in BPM.
	//Presumably there’s some inconsistency because the rate of writing samples to the buffer is faster than the sample rate, but it seems fairly steady with a small buffer size."

	//NOTE: we will want maybe to use the same soundStream used for the sound tick also for the audiBuffer timer..
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

	//----

#ifdef USE_ofxAbletonLink
private:

	ofxAbletonLink link;

	ofParameter<bool> ENABLE_LINK_SYNC;

	ofxGuiGroup2* group_LINK;
	ofParameterGroup params_LINK;

	ofParameter<bool> LINK_Enable;
	ofParameter<float> LINK_Bpm;//link bpm
	ofParameter<bool> LINK_Play;//control and get Ableton Live playing too, mirrored like Link does
	ofParameter<float> LINK_Phase;//phase on the bar. cycled from 0.0f to 4.0f
	ofParameter<bool> LINK_RestartBeat;//set beat 0
	ofParameter<bool> LINK_ResetBeats;//reset "unlimited-growing" beat counter
	ofParameter<string> LINK_Beat_string;//monitor beat counter
	//amount of beats are not limited nor in sync / mirrored with Ableton Live.
	ofParameter<string> LINK_Peers_string;//number of synced devices/apps on your network
	//ofParameter<int> LINK_Beat_Selector;//TODO: TEST
	//int LINK_Beat_Selector_PRE = -1;

	void LINK_setup()
	{
		link.setup();

		ofAddListener(params_LINK.parameterChangedE(), this, &ofxBeatClock::Changed_LINK_Params);
	}

	void LINK_update()
	{
		if (ENABLE_LINK_SYNC)//not required but prophylactic
		{
			LINK_Phase = link.getPhase();//bar phase
			LINK_Beat_string = ofToString(link.getBeat(), 0);
			//amount of beats are not limited nor in sync / mirrored with Ableton Live.

			//display text
			clockActive_Type = "ABLETON LINK";

			clockActive_Info = "BEAT: " + ofToString(link.getBeat(), 1);
			clockActive_Info += "\n";
			clockActive_Info += "PHASE:  " + ofToString(link.getPhase(), 1);
			clockActive_Info += "\n";
			clockActive_Info += "PEERS:  " + ofToString(link.getNumPeers());

			Beat_float_current = (float)link.getBeat() + 1.0f;
			Beat_float_string = ofToString(Beat_float_current, 2);

			//---

			//if (ENABLE_pattern_limits)
			//{
			//	Beat_current = 1 + ((int)Beat_float_current) % pattern_BEAT_limit;//limited to 16 beats
			//}
			//else
			//{
			//	Beat_current = 1 + ((int)Beat_float_current);
			//}
			//Beat_string = ofToString(Beat_current);

			//---

			if (ENABLE_pattern_limits)
			{
				Beat_current = 1 + (((int)Beat_float_current) % 4);//limited to 4 beats
			}
			else
			{
				Beat_current = 1 + (((int)Beat_float_current));
			}
			Beat_string = ofToString(Beat_current);

			//---
			
			if (Beat_current != Beat_current_PRE)
			{
				ofLogNotice("ofxBeatClock") << "LINK beat changed:" << Beat_current;
				Beat_current_PRE = Beat_current;

				//-

				beatTick_MONITOR(Beat_current);

				//-
			}
		}
	}

	void LINK_draw()
	{
		ofPushStyle();

		//text
		int xpos = 400;
		int ypos = 750;

		//line
		float x = ofGetWidth() * link.getPhase() / link.getQuantum();

		//red vertical line
		ofSetColor(255, 0, 0);
		ofDrawLine(x, 0, x, ofGetHeight());

		std::stringstream ss("");
		ss
			<< "bpm:   " << ofToString(link.getBPM(), 2) << std::endl
			<< "beat:  " << ofToString(link.getBeat(), 2) << std::endl
			<< "phase: " << ofToString(link.getPhase(), 2) << std::endl
			<< "peers: " << link.getNumPeers() << std::endl
			<< "play?: " << (link.isPlaying() ? "play" : "stop");

		ofSetColor(255);
		if (fontMedium.isLoaded())
		{
			fontMedium.drawString(ss.str(), xpos, ypos);
		}
		else
		{
			ofDrawBitmapString(ss.str(), xpos, ypos);
		}

		ofPopStyle();
	}

	void LINK_exit()
	{
		ofLogNotice("ofxBeatClock") << "LINK_exit()";
		ofLogNotice("ofxBeatClock") << "Remove LINK listeners";
		ofRemoveListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
		ofRemoveListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
		ofRemoveListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

		ofRemoveListener(params_LINK.parameterChangedE(), this, &ofxBeatClock::Changed_LINK_Params);
	}

	//-

	//callbacks

	void Changed_LINK_Params(ofAbstractParameter &e)
	{
		string name = e.getName();
		ofLogVerbose("ofxBeatClock") << "Changed_LINK_Params '" << name << "': " << e;

		//-

		if (name == "PLAY")
		{
			ofLogNotice("ofxBeatClock") << "LINK PLAY: " << LINK_Play;
			
			//TODO:
			//BUG:
			//play engine do not works fine

			//TEST:
			//link.setIsPlaying(LINK_Play);

			//TEST:
			if (LINK_Play)
			{
				link.play();
			}
			else
			{
				link.stop();
			}
		}

		else if (name == "ENABLE")
		{
			ofLogNotice("ofxBeatClock") << "ENABLE: " << LINK_Enable;

			//TEST:
			//if (LINK_Enable)
			//{
			//	link.enablePlayStateSync();
			//}
			//else
			//{
			//	link.disablePlayStateSync();
			//}

			//TEST:
			if (LINK_Enable)
			{
				link.enableLink();
			}
			else
			{
				link.disableLink();
			}
		}

		else if (name == "BPM")
		{
			ofLogNotice("ofxBeatClock") << "LINK BPM";

			link.setBPM(LINK_Bpm);
			if (ENABLE_LINK_SYNC)
			{
				//TODO: 
				//it's required if ofxDawMetro is not being used?
				clockInternal_Bpm = LINK_Bpm;

				//will be autoupdate on clockInternal callback
				//BPM_Global = LINK_Bpm;
			}
		}

		else if (name == "RESTART" && LINK_RestartBeat)
		{
			ofLogNotice("ofxBeatClock") << "LINK RESTART";
			LINK_RestartBeat = false;

			link.setBeat(0.0);

			if (ENABLE_LINK_SYNC)
			{
				Tick_16th_current = 0;
				Tick_16th_string = ofToString(Tick_16th_current);

				Beat_current = 0;
				Beat_string = ofToString(Beat_current);

				Bar_current = 0;
				Bar_string = ofToString(Bar_current);
			}
		}

		else if (name == "RESET" && LINK_ResetBeats)
		{
			ofLogNotice("ofxBeatClock") << "LINK RESET";
			LINK_ResetBeats = false;

			link.setBeatForce(0.0);
		}

		//TODO:
		//don't understand yet what setBeat does...
		//else if (name == "GO BEAT")
		//{
		//	if (LINK_Beat_Selector != LINK_Beat_Selector_PRE)//changed
		//	{
		//		ofLogNotice("ofxBeatClock") << "LINK GO BEAT: " << LINK_Beat_Selector;
		//		LINK_Beat_Selector_PRE = LINK_Beat_Selector;

		//		link.setBeat(LINK_Beat_Selector);

		//		if (ENABLE_LINK_SYNC)
		//		{
		//			//Tick_16th_current = 0;
		//			//Tick_16th_string = ofToString(Tick_16th_current);

		//			Beat_current = 0;
		//			Beat_string = ofToString(Beat_current);
		//		}
		//	}
		//}
	}

	void LINK_bpmChanged(double &bpm)
	{
		ofLogNotice("ofxBeatClock") << "LINK_bpmChanged" << bpm;

		LINK_Bpm = bpm;

		//BPM_Global will be update on the LINK_Bpm callback
		//clockInternal_Bpm will be updated too
	}

	void LINK_numPeersChanged(std::size_t &peers)
	{
		ofLogNotice("ofxBeatClock") << "LINK_numPeersChanged" << peers;
		LINK_Peers_string = ofToString(peers);
	}

	void LINK_playStateChanged(bool &state)
	{
		ofLogNotice("ofxBeatClock") << "LINK_playStateChanged" << (state ? "play" : "stop");

		LINK_Play = false;
	}
#endif


};

