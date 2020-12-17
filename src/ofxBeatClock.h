
//---------------------------------
//
//	DEFINES
//
//
//#define USE_ofxAbletonLink
//
//
//---------------------------------


/// TODO:
///
/// + 	On-the-fly bang re-sync to bar beat start. (kind of manual syncer)
/// + 	Add filter to smooth / stabilize BPM number when using external midi clock mode.
/// + 	Add audio output selector to metronome sounds. maybe share audioBuffer with better timer mode
///	 		on USE_AUDIO_BUFFER_TIMER_MODE. still disabled by default yet
/// + 	NOTE: more info about soundStream timer
///			https://forum.openframeworks.cc/t/pass-this-pointer-from-parent-to-child-object-scheduler-oftimer-system/22088/6?u=moebiussurfing

/// BUG: [1]
///sometimes metronome ticks goes on beat 2 instead 1.
///works better with 0 and 4 detectors, but why?
///SOLUTION:
///we must check better all the beat%limit bc should be the problem!!
///maybe we can add another beat_current varialbe, independent of the received beat from source clocks
///then to eliminate the all the limiters.
///must check each source clock type what's the starting beat: 0 or 1!!

///BUG: [2]
///VS2017. Release/x64: 
///I am getting several log errors maybe related to LINK addon ofxAbletonLink(?)
///Maybe are affecting the app performance, reducing the FPS but I am not sure...
///Exception thrown at 0x00007FFF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
///The thread 0x22120 has exited with code 0 (0x0).
///Exception thrown at 0x00007FFF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
///The thread 0x224dc has exited with code 0 (0x0).
///The thread 0x213cc has exited with code 0 (0x0).
///The thread 0x645c has exited with code 0 (0x0).


//----

#pragma once
#include "ofMain.h"

#include "ofxMidiClock.h"//used for external midi clock sync (1)
#include "ofxMidi.h"
#include "ofxMidiTimecode.h"
#include "ofxDawMetro.h"//used for internal (using threaded timer) clock (2)
#include "ofxGuiExtended2.h"
#include "ofxSurfingHelpers.h"

//----

//* OPTIONAL : Ableton Link feature *

#ifdef USE_ofxAbletonLink
#include "ofxAbletonLink.h"//used for external Ableton Live Link engine (3)
#endif
///* Is the only external mode where OF app works as clock master. 
///(besides external midi sync slave)
///* It can control the Ableton bpm, play/stop etc from OF.
///What is Ableton Link?
///"This is the codebase for Ableton Link, a technology that synchronizes musical beat,
///tempo, and phase across multiple applications running on one or more devices.
///Applications on devices connected to a local network discover each other automatically
///and form a musical session in which each participant can perform independently: 
///anyone can start or stop while still staying in time.Anyone can change the tempo, 
///the others will follow.Anyone can join or leave without disrupting the session."
///https://github.com/Ableton/link -> original libs
///https://www.ableton.com/en/link/ -> videos and tutorials
///NOTES:(?)
///I don't understand yet what does when we make link.setBeat()...
///bc beat position of Ableton will not be updated changing this..
///So, it seems this is not the philosophy behind LINK:
///The idea of LINK seems to link the "gloabl" /phase/bar/beat to play live simultaneously
///many musicians or devices/apps, not to sync two long musics projects playing together.

//----

//* OPTIONAL : maybe better alternative internal clock *

///TODO:
//#define USE_AUDIO_BUFFER_TIMER_MODE
//used as audioBuffer timer as an alternative for the internal clock (4)
///when it's enabled ofxDawMetro is not used and could be not loaded.
///WIP: alternative and better timer approach using the audio-buffer to avoid out-of-sync problems of current timers
///(https://forum.openframeworks.cc/t/audio-programming-basics/34392/10). 
///Problems happen when minimizing or moving the app window.. Any help is welcome!
///(code is at the bottom)
///un-comment to enable this NOT WORKING yet alternative mode
///THE PROBLEM: clock drift very often.. maybe bc wasapi sound api? ASIO seems a better choice.
///help on improve this is welcome!
///NOTE: if the audio output/driver is not opened properly, fps performance seems to fall...
///TODO: should make easier to select sound output

//----

///TODO:
///WIP
///smooth global bpm clock that is received from external fluctuating clocks
///could be done with only visual refreshing the midi clock slower,
///or using a real filter to the bpm variable.
///#define BPM_MIDI_CLOCK_REFRESH_RATE 1000
///refresh received MTC by clock. disabled/commented to "realtime" by every-frame-update

//----

//only to long song mode on external midi sync vs simpler 4 bars / 16 beats
#define ENABLE_PATTERN_LIMITING//comment to disable
#define PATTERN_STEP_BAR_LIMIT 4
#define PATTERN_STEP_BEAT_LIMIT 16//TODO: this are 16th ticks not beat!

#define BPM_INIT 120
#define BPM_INIT_MIN 40
#define BPM_INIT_MAX 400

#define USE_VISUAL_FEEDBACK_FADE//comment to try improve performance... Could add more optimizations maybe

//-

class ofxBeatClock : public ofxMidiListener, public ofxDawMetro::MetroListener {

#pragma mark - OF_MAIN

public:
	ofxBeatClock();
	~ofxBeatClock();

	void setup();
	void update();
	void draw();
	void exit();

	//-

private:
	void startup();

#pragma mark - MIDI_IN_CLOCK

private:
	ofxMidiIn midiIn;
	ofxMidiClock midiIn_Clock;//< clock message parser

	ofxMidiMessage midiIn_Clock_Message;
	void newMidiMessage(ofxMidiMessage& eventArgs);

	double midiIn_Clock_Bpm;//< song tempo in bpm, computed from clock length
	bool bMidiInClockRunning;//< is the clock sync running?
	unsigned int MIDI_beats;//< song pos in beats
	int MIDI_quarters;//convert total # beats to # quarters
	int MIDI_bars;//compute # of bars
	double MIDI_seconds;//< song pos in seconds, computed from beats
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

	//NOTE: all the layout system is a little messy yet. sometimes using glm, or x y points ...etc

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

	//----

	//gui panels theme
	//NOTE: take care with the path font defined on the config json 
	//because ofxGuiExtended crashes if fonts are not located on /data
	//--------------------------------------------------------------
	void loadTheme(std::string s)
	{
		path_Theme = s;

		ofLogNotice(__FUNCTION__) << "Loadinoad JSON ofxGuiExtended2 Theme : " << path_Theme;

		group_BeatClock->loadTheme(s);
		group_Controls->loadTheme(s);
		group_Advanced->loadTheme(s);
		group_INTERNAL->loadTheme(s);
		group_EXTERNAL_MIDI->loadTheme(s);
	}
	std::string path_Theme;

	int gui_Panel_Width, gui_Panel_posX, gui_Panel_posY;

public:
	//api setters

	void setPosition_GuiPanel(int x, int y, int w);//gui panel
	ofPoint getPosition_GuiPanel();
	void setVisible_GuiPanel(bool b);
	void setVisible_BeatBall(bool b);

	//--------------------------------------------------------------
	bool getVisible_GuiPanel()
	{
		return gui.getVisible();
	}
	//--------------------------------------------------------------
	void toggleVisible_GuiPanel()
	{
		bool b = getVisible_GuiPanel();
		setVisible_GuiPanel(!b);
	}

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
	//--------------------------------------------------------------
	void draw_Anchor(int x, int y)
	{
		if (DEBUG_Layout)
		{
			ofPushStyle();
			ofFill();
			ofSetColor(ofColor::red);

			//circle
			ofDrawCircle(x, y, 3);

			//text
			int pad;
			if (y < 15) pad = 10;
			else pad = -10;
			ofDrawBitmapStringHighlight(ofToString(x) + "," + ofToString(y), x, y + pad);
			ofPopStyle();
		}
	}
	//--------------------------------------------------------------
	void setDebug_Clock(bool b)
	{
		DEBUG_moreInfo = b;
	}
	//--------------------------------------------------------------
	void toggleDebug_Clock()
	{
		DEBUG_moreInfo = !DEBUG_moreInfo;
	}
	//--------------------------------------------------------------
	void setDebug_Layout(bool b)
	{
		DEBUG_Layout = b;
	}
	//--------------------------------------------------------------
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
	void beatTick_MONITOR(int beat);//trigs ball drawing and sound ticker

	int lastBeatFlash = -1;
	//void draw_BeatBalFlash(int _onBeat){}

public:
	ofParameter<bool> BeatTick_TRIG;//get bang beat!!
	//also this trigs to draw a flashing circle for a frame only
	//this variable is used to subscribe external (in ofApp) listeners to get the beat bangs!

	//-

////TODO:
////WIP
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

#pragma mark - PARAMS_FOR_GUI_PANEL_AND_CLOCK_ENGINE

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
	ofParameterGroup params_Advanced;
	ofJson confg_Button, confg_ButtonSmall, confg_Sliders;//json theme

	//-

#pragma mark - PARAMS
public:
	ofParameter<bool> PLAYING_Global_State;//for all different source clock modes

private:
	ofParameterGroup params_CONTROL;

	ofParameter<bool> PLAYING_State;//player state only for internal clock
	ofParameter<bool> PLAYING_External_State;//player state only for external clock

	ofParameter<bool> ENABLE_CLOCKS;//enable clock (affects all clock types)
	ofParameter<bool> ENABLE_INTERNAL_CLOCK;//enable internal clock
	ofParameter<bool> ENABLE_EXTERNAL_MIDI_CLOCK;//enable midi clock sync
	ofParameter<int> midiIn_Port_SELECT;
	int midiIn_numPorts = 0;

	//----

public:
	//this is the main and final target bpm, is the destination of all other clocks (internal, external midi sync or ableton link)
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
	float getBPM();//returns BPM_Global
	int getTimeBar();//returns duration of global bar in ms

	//--

	//this methods could be useful only to visualfeedback on integrating to other bigger guis on ofApp..
	//--------------------------------------------------------------
	bool getInternalClockModeState()
	{
		return ENABLE_INTERNAL_CLOCK;
	}
	//--------------------------------------------------------------
	bool getExternalClockModeState()
	{
		return ENABLE_EXTERNAL_MIDI_CLOCK;
	}
#ifdef USE_ofxAbletonLink
	//--------------------------------------------------------------
	bool getLinkClockModeState()
	{
		return ENABLE_LINK_SYNC;
	}
#endif

	//--

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

	ofParameter<float> clockInternal_Bpm;
	//NOTE: 
	//for the momment this bpm variables is being used as
	//main tempo (sometimes) for other clock sources too.
	//TODO:
	//Maybe we should improve this using global bpm variable (BPM_Global) as main.

	ofParameterGroup params_ClockInternal;
	ofParameter<bool> clockInternal_Active;
	void Changed_ClockInternal_Bpm(float & value);
	void Changed_ClockInternal_Active(bool & value);

	//-

	////TODO:
	//void reSync();
	//ofParameter<bool> bSync_Trig;

	//-

public:
	void setBpm_ClockInternal(float bpm);//to set bpm from outside

	//-

	//settings
#pragma mark - XML SETTINGS

private:
	std::string path_Global;

public:
	//--------------------------------------------------------------
	void setPathglobal(std::string _path) {
		path_Global = _path;

		ofxSurfingHelpers::CheckFolder(path_Global);
	}

private:
	void saveSettings(std::string path);
	void loadSettings(std::string path);

	std::string filenameControl = "BeatClock_Settings.xml";
	std::string filenameMidiPort = "Midi_Settings.xml";
	std::string filenameApp = "App_Settings.xml";
	
	ofParameterGroup params_App;

	//-

#pragma mark - DRAW_STUFF:

	//font
	std::string messageInfo;
	ofTrueTypeFont fontSmall;
	ofTrueTypeFont fontMedium;
	ofTrueTypeFont fontBig;

	//-

	//beat ball
	ofPoint metronome_ball_pos;
	int metronome_ball_radius;

	//-

	ofParameter<bool> SHOW_Extra;//beat boxes, text info and beat ball (all except gui panels)
	ofParameter<bool> SHOW_Advanced;//some helpers other secondary settings/controls 

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
	void reset_ClockValues();//set gui display text clock to 0:0:0

	//TODO: could be nice to add some listener system..
	ofParameter<int> Bar_current;
	ofParameter<int> Beat_current;
	ofParameter<int> Tick_16th_current;//used only with audioBuffer timer mode

	//TODO: 
	//link
#ifdef USE_ofxAbletonLink
	int Beat_current_PRE;//used to detect changes only on link mode
	float Beat_float_current;//link beat received  with decimals (float) and starting from 0 not 1 
	std::string Beat_float_string;
#endif

private:

	//strings for monitor drawing
	//1:1:1
	std::string Bar_string;
	std::string Beat_string;
	std::string Tick_16th_string;

	std::string clockActive_Type;//internal/external/link clock types name
	std::string clockActive_Info;//midi in port, and extra info for any clock source

	//----

#pragma mark - API

	//----

public:

	//main transport control for master mode (not in external midi sync that OF app is slave)
	void start();//only used on internal or link clock source mode
	void stop();//only used on internal or link clock source mode
	void setTogglePlay();//only used on internal or link clock source mode

	//bool isPlaying()
	//{
	//	return bIsPlaying;
	//}

	//----

private:

	//bool bIsPlaying;//used only for internal clock mode.. should be usefull for all clock types

	//----

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

	//----

#pragma mark - CHANGE_MIDI_IN_PORT

	void setup_MidiIn_Port(int p);
	int midiIn_Clock_Port_OPENED;
	int midiIn_Port_PRE = -1;
	ofParameter<std::string> midiIn_PortName{ "","" };

	//----

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

	ofParameter<bool> LINK_Enable;//enable link
	ofParameter<float> LINK_Bpm;//link bpm
	ofParameter<bool> LINK_Play;//control and get Ableton Live playing too, mirrored like Link does
	ofParameter<float> LINK_Phase;//phase on the bar. cycled from 0.0f to 4.0f
	ofParameter<bool> LINK_RestartBeat;//set beat 0
	ofParameter<bool> LINK_ResetBeats;//reset "unlimited-growing" beat counter
	ofParameter<std::string> LINK_Beat_string;//monitor beat counter
	//amount of beats are not limited nor in sync / mirrored with Ableton Live.
	ofParameter<std::string> LINK_Peers_string;//number of synced devices/apps on your network
	//ofParameter<int> LINK_Beat_Selector;//TODO: TEST
	//int LINK_Beat_Selector_PRE = -1;

	//--------------------------------------------------------------
	void LINK_setup()
	{
		link.setup();

		ofAddListener(params_LINK.parameterChangedE(), this, &ofxBeatClock::Changed_LINK_Params);
	}

	//--------------------------------------------------------------
	void LINK_update()
	{
		if (ENABLE_LINK_SYNC)//not required but prophylactic
		{
			//display text
			clockActive_Type = "ABLETON LINK";

			clockActive_Info = "BEAT: " + ofToString(link.getBeat(), 1);
			clockActive_Info += "\n";
			clockActive_Info += "PHASE: " + ofToString(link.getPhase(), 1);
			clockActive_Info += "\n";
			clockActive_Info += "PEERS: " + ofToString(link.getNumPeers());

			//-

			//assign link states to our variables

			if (LINK_Enable && (link.getNumPeers() != 0))
			{
				LINK_Phase = link.getPhase();//link bar phase

				Beat_float_current = (float)link.getBeat();
				Beat_float_string = ofToString(Beat_float_current, 2);

				LINK_Beat_string = ofToString(link.getBeat(), 0);//link beat with decimal
				//amount of beats are not limited nor in sync / mirrored with Ableton Live.
			}

			//---

			//update mean clock counters and update gui
			if (LINK_Enable && LINK_Play && (link.getNumPeers() != 0))
			{
				int _beats = (int)Beat_float_current;//starts in beat 0 not 1

				//-

				//beat
				if (ENABLE_pattern_limits)
				{
					Beat_current = 1 + (_beats % 4);//limited to 4 beats. starts in 1
				}
				else
				{
					Beat_current = 1 + (_beats);
				}
				Beat_string = ofToString(Beat_current);

				//-
				
				//bar
				int _bars = _beats / 4;
				if (ENABLE_pattern_limits)
				{
					Bar_current = 1 + _bars % pattern_BAR_limit;
				}
				else
				{
					Bar_current = 1 + _bars;
				}
				Bar_string = ofToString(Bar_current);

				//---

				if (Beat_current != Beat_current_PRE)
				{
					ofLogVerbose(__FUNCTION__) << "LINK beat changed:" << Beat_current;
					Beat_current_PRE = Beat_current;

					//-

					beatTick_MONITOR(Beat_current);

					//-
				}
			}
		}

		//-

		//blink gui label if link is not connected! (no peers)
		//to alert user to re click LINK buttons(in OF app and Ableton too)
		if (link.getNumPeers() == 0)
		{
			if ((ofGetFrameNum() % 60) < 30)
			{
				LINK_Peers_string = "0";
			}
			else
			{
				LINK_Peers_string = " ";
			}
		}
	}

	//--------------------------------------------------------------
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

	//--------------------------------------------------------------
	void LINK_exit()
	{
		ofLogNotice(__FUNCTION__) << "LINK_exit()";
		ofLogNotice(__FUNCTION__) << "Remove LINK listeners";
		ofRemoveListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
		ofRemoveListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
		ofRemoveListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

		ofRemoveListener(params_LINK.parameterChangedE(), this, &ofxBeatClock::Changed_LINK_Params);
	}

	//-

	//callbacks

	//--------------------------------------------------------------
	void Changed_LINK_Params(ofAbstractParameter &e)
	{
		std::string name = e.getName();
		
		if (name != "PEERS")//exclude log
		{
			ofLogVerbose(__FUNCTION__) << "Changed_LINK_Params '" << name << "': " << e;
		}

		//-

		if (name == "PLAY")
		{
			ofLogNotice(__FUNCTION__) << "LINK PLAY: " << LINK_Play;

			if (LINK_Enable && (link.getNumPeers() != 0))
			{
				//TODO:
				//BUG:
				//play engine do not works fine

				//TEST:
				if (link.isPlaying() != LINK_Play)//don't need update if it's already "mirrored"
				{
					link.setIsPlaying(LINK_Play);
				}

				////TEST:
				//if (LINK_Play)
				//{
				//	link.play();
				//}
				//else
				//{
				//	link.stop();
				//}

				//workflow
				//set gui display text clock to 0:0:0
				if (!LINK_Play)
				{
					reset_ClockValues();
				}
			}
			//workflow
			else if (LINK_Play)
			{
				LINK_Play = false;//if not enable block to play disabled
			}
		}

		else if (name == "LINK")
		{
			ofLogNotice(__FUNCTION__) << "LINK: " << LINK_Enable;

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

				//workflow
				if (LINK_Play)
				{
					LINK_Play = false;//if not enable block to play disabled
				}
			}
		}

		else if (name == "BPM" && LINK_Enable)
		{
			ofLogNotice(__FUNCTION__) << "LINK BPM";

			if (link.getBPM() != LINK_Bpm)
			{
				link.setBPM(LINK_Bpm);
			}

			if (ENABLE_LINK_SYNC)
			{
				//TODO: 
				//it's required if ofxDawMetro is not being used?
				clockInternal_Bpm = LINK_Bpm;

				//will be autoupdate on clockInternal callback
				//BPM_Global = LINK_Bpm;
			}
		}

		else if (name == "RESYNC" && LINK_RestartBeat && LINK_Enable)
		{
			ofLogNotice(__FUNCTION__) << "LINK RESTART";
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

		else if (name == "FORCE RESET" && LINK_ResetBeats && LINK_Enable)
		{
			ofLogNotice(__FUNCTION__) << "LINK RESET";
			LINK_ResetBeats = false;

			link.setBeatForce(0.0);
		}

		//TODO:
		//I don't understand yet what setBeat does...
		//bc beat position of Ableton will not be updated changing this..
		//So, it seems this is not the philosophy behind LINK:
		//The idea of LINK seems to link the bar/beat to play live simultaneously
		//many musicians or devices/apps

		//else if (name == "GO BEAT" && LINK_Enable)
		//{
		//	if (LINK_Beat_Selector != LINK_Beat_Selector_PRE)//changed
		//	{
		//		ofLogNotice(__FUNCTION__) << "LINK GO BEAT: " << LINK_Beat_Selector;
		//		LINK_Beat_Selector_PRE = LINK_Beat_Selector;
		//
		//		link.setBeat(LINK_Beat_Selector);
		//
		//		if (ENABLE_LINK_SYNC)
		//		{
		//			//Tick_16th_current = 0;
		//			//Tick_16th_string = ofToString(Tick_16th_current);
		//
		//			Beat_current = 0;
		//			Beat_string = ofToString(Beat_current);
		//		}
		//	}
		//}
	}

	//receive from master device (i.e Ableton Live)
	//--------------------------------------------------------------
	void LINK_bpmChanged(double &bpm)
	{
		ofLogNotice(__FUNCTION__) << "LINK_bpmChanged" << bpm;

		LINK_Bpm = bpm;

		//BPM_Global will be update on the LINK_Bpm callback
		//clockInternal_Bpm will be updated too
	}

	//--------------------------------------------------------------
	void LINK_numPeersChanged(std::size_t &peers)
	{
		ofLogNotice(__FUNCTION__) << "LINK_numPeersChanged" << peers;
		LINK_Peers_string = ofToString(peers);

		if (peers == 0)
		{
			LINK_Phase = 0.f;
		}
	}

	//--------------------------------------------------------------
	void LINK_playStateChanged(bool &state)
	{
		ofLogNotice(__FUNCTION__) << "LINK_playStateChanged" << (state ? "play" : "stop");

		if (state != LINK_Play && ENABLE_LINK_SYNC && LINK_Enable)//don't need update if it's already "mirrored"
		{
			LINK_Play = state;
		}
	}
#endif

};

