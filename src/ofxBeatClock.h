
//---------------------------------
//
//	OPTIONAL DEFINES

//TODO: must review all protocol, master/slave etc
#define OFXBEATCLOCK_USE_ofxAbletonLink // -> Can be commented to not include the Ableton Link feature/add-on.

//TODO:
//#define OFXBEATCLOCK_USE_AUDIO_BUFFER_TIMER_MODE // -> [WIP] A better alternative clock engine based on audio buffer.

//
//---------------------------------

/*
	BUG:

	+ add circle widget to show external beat happens!

	+ OFXBEATCLOCK_USE_ofxAbletonLink broken:
		fix callbacks by hard coded name
		disable parts to fix error


	TODO:

	+ clean/remove native gui stuff
	+	BPM slider on internal clock does applies only when releasing the slider.
	+ 	On-the-fly bang re-sync to bar beat start. (kind of manual syncer)
	+ 	Add fast filter to smooth / stabilize BPM number when using external midi clock mode.
	+ 	Add audio output selector to metronome sounds.
			maybe share audioBuffer with better timer mode
			on OFXBEATCLOCK_USE_AUDIO_BUFFER_TIMER_MODE. still disabled by default yet
	+	Fix workflow of labels internal, external, etc...


	NOTE:

	More info about SoundStream buffer timer. For better precision and/or performance.
	https://forum.openframeworks.cc/t/pass-this-pointer-from-parent-to-child-object-scheduler-oftimer-system/22088/6?u=moebiussurfing


	NOTE:

	From @dimitre:
	Audio buffer + some alternative modes like bet beat 1 on sinus or to add phase.
	https://forum.openframeworks.cc/t/hola/33161/10
	https://github.com/dimitre/ofxMicroUI/
	#ifdef USEAUDIOOUT
	ofSoundStream soundStream;
	ofSoundStreamSettings settings;
	void audioOut(ofSoundBuffer & buffer) {
		seconds = buffer.getTickCount() * buffer.getNumFrames() / double(buffer.getSampleRate());
		bpm.setSeconds(seconds);
		bpm2.setSeconds(seconds);
		bpm3.setSeconds(seconds);
		tapper.setSeconds(seconds);
	}
	#endif

*/

//-

/*

	TODO:

	- BUG: Repair problems when sometimes beat 1 tick it's displaced to beat 2...
	- BUG: Some log errors must be repaired on ofxAbletonLink that seems drop down fps/performance...
	- Add the correct workflow for LINK. Must add some mode toggle.
	- On-the-fly re-sync to bar beat start.
	- A better link between play button/params in all internal/external clock source modes, with one unique play button for all clock sources.
	- Add alternative and better timer approach using the audio-buffer to avoid out-of-sync problems of current timers
	(https://forum.openframeworks.cc/t/audio-programming-basics/34392/10).
	Problems happen when minimizing or moving the app window.. Any help is welcome!
	- Add kind of plug-in to add audio input to get BeatTick on the fly from incoming audio signal. (Using ofxBTrack)

*/

//-

/*

	BUG:

	Sometimes metronome ticks goes to beat 2 instead 1.
	Works better with 0 and 4 detectors, but why?
	SOLUTION:
	We must check better all the beat%limit bc should be the problem!!
	Maybe we can add another beat_current variable, independent of the received beat from source clocks
	Then to eliminate the all the limiters.
	Must check each source clock type what's the starting beat: 0 or 1!!

*/

//----

#pragma once

#include "ofMain.h"

#include "BpmTapTempo.h"
#include "CircleBeat.h"
#include "ofxDawMetro.h" // used for internal (using threaded timer) clock (2)
#include "ofxInteractiveRect.h" // engine to move the gui. TODO: add resize by mouse too.
#include "ofxMidi.h"
#include "ofxMidiClock.h" // used for external midi clock sync (1)
#include "ofxMidiTimecode.h"
#include "ofxSurfingHelpers.h"

// NOTE: Can't be disabled for the moment. but is not difficult if you need it.
// Required ImGui, can't go straightforward with ofxGui!
#define OFXBEATCLOCK_USE_OFX_SURFING_IM_GUI
#ifdef OFXBEATCLOCK_USE_OFX_SURFING_IM_GUI
#include "ofxSurfingImGui.h"
#endif

//----

// * OPTIONAL : Ableton Link feature *

#ifdef OFXBEATCLOCK_USE_ofxAbletonLink
#include "ofxAbletonLink.h" // used for external Ableton Live Link engine (3)
#endif

/// * Is the only external mode where OF app works as clock master.
/// (besides external midi sync slave)
/// * It can control the Ableton bpm, play/stop etc from OF.
/// What is Ableton Link?
/// "This is the codebase for Ableton Link, a technology that synchronizes musical beat,
/// tempo, and phase across multiple applications running on one or more devices.
/// Applications on devices connected to a local network discover each other automatically
/// and form a musical session in which each participant can perform independently:
/// anyone can start or stop while still staying in time.Anyone can change the tempo,
/// the others will follow.Anyone can join or leave without disrupting the session."
/// https://github.com/Ableton/link -> original libs
/// https://www.ableton.com/en/link/ -> videos and tutorials
/// NOTES:(?)
/// I don't understand yet what does when we make link.setBeat()...
/// bc beat position of Ableton will not be updated changing this..
/// So, it seems this is not the philosophy behind LINK:
/// The idea of LINK seems to link the "gloabl" /phase/bar/beat to play live simultaneously
/// many musicians or devices/apps, not to sync two long musics projects playing together.

//----

//* OPTIONAL : maybe better alternative internal clock *

///TODO:
/// used as audioBuffer timer as an alternative for the internal clock (4)
/// when it's enabled ofxDawMetro is not used and could be not loaded.
/// WIP: alternative and better timer approach using the audio-buffer
/// to avoid out-of-sync problems of current timers
/// (https://forum.openframeworks.cc/t/audio-programming-basics/34392/10).
/// Problems happen when minimizing or moving the app window.. Any help is welcome!
/// (code is at the bottom)
/// un-comment to enable this NOT WORKING yet alternative mode
/// THE PROBLEM: clock drift very often.. maybe bc wasapi sound api? ASIO seems a better choice.
/// help on improve this is welcome!
/// NOTE: if the audio output/driver is not opened properly, fps performance seems to fall...
/// TODO: should make easier to select sound output

//----

/// TODO:
/// WIP
/// smooth global bpm clock that is received from external fluctuating clocks
/// could be done with only visual refreshing the midi clock slower,
/// or using a real filter to the bpm variable.
/// #define BPM_MIDI_CLOCK_REFRESH_RATE 1000
/// refresh received MTC by clock. disabled/commented to "realtime" by every-frame-update

//----

// Only to long song mode on external midi sync vs simpler 4 bars / 16 beats
#define PATTERN_LIMITING_ENABLE //comment to disable
#define PATTERN_STEP_BAR_LIMIT 4
#define PATTERN_STEP_BEAT_LIMIT 16 //TODO: this are 16th ticks not beat!
#define BPM_INIT 120
#define BPM_INIT_MIN 40
#define BPM_INIT_MAX 400
#define OFXBEATCLOCK_USE_VISUAL_FEEDBACK_FADE //comment to try improve performance... Could add more optimizations maybe
#define OFXBEATCLOCK_BPM_INCREMENTS 0.001f;

//-

class ofxBeatClock : public ofBaseApp, public ofxMidiListener, public ofxDawMetro::MetroListener {
public:
	ofxBeatClock();
	~ofxBeatClock();

	//----

	// API

	//----

	void setup();
	void draw();

	ofParameter<bool> BeatTick; // Get bang beat!!
	// Also this trigs to draw a flashing circle for a frame only
	// This variable is used to subscribe an external (in ofApp) callback / listeners to get the beat bangs!

	// Main transport control for master mode
	// (not in external midi sync that OF app is slave)
	void start(); // only used on internal or link clock source mode
	void stop(); // only used on internal or link clock source mode
	void setTogglePlay(); // only used on internal or link clock source mode

	// A super simple callback
	//--------------------------------------------------------------
	int getBeat() {
		if (BeatTick) {
			//ofLogNotice("ofxBeatClock")<<(__FUNCTION__) << "BeatTick ! #" << Beat_current;
			return Beat_current;
		}
		else
			return -1;
	}

	// Current_bpm_clock_values
	float getBpm() const; //returns BPM_Global
	int getTimeBar() const; //returns duration of global bar in ms

	void setBpm_ClockInternal(float bpm); // to set bpm from outside

	// Gui visible toggles
	ofParameter<bool> bGui_ClockMonitor;
	ofParameter<bool> bGui_Sources;
	ofParameter<bool> bGui_ClockBpm;
	//ofParameter<bool> bGui_PreviewClockNative; // beat boxes, text info and beat ball (all except gui panels)

	//--------------------------------------------------------------
	bool isPlaying() const {
		bool _isPlaying = false;
#ifdef OFXBEATCLOCK_USE_ofxAbletonLink
		_isPlaying = bPlaying_Internal_State || bPlaying_ExternalSync_State || bPlaying_LinkState;
#else
		_isPlaying = bPlaying_Internal_State || bPlaying_ExternalSync_State;
#endif
		return _isPlaying;
	}

	// Tap_engine
	void doTapTrig();

	// Customize path for settings
	//--------------------------------------------------------------
	void setPathglobal(std::string _path) {
		path_Global = _path;
		ofxSurfingHelpers::CheckFolder(path_Global);
	}

private:
	//TODO: could be nice to add some callback system..
	ofParameter<int> Bar_current;
	ofParameter<int> Beat_current;
	ofParameter<int> Tick_16th_current; // used only with audioBuffer timer mode

	//--

private:
	void update(ofEventArgs& args);
	void exit();
	//void windowResized(int w, int h);
	void keyPressed(ofKeyEventArgs& eventArgs);

	void drawGui();

	//-

#ifdef OFXBEATCLOCK_USE_OFX_SURFING_IM_GUI
	ofxSurfingGui ui;
#endif

	//-

private:
	int window_W;
	int window_H;

	//-

private:
	void startup();

	CircleBeat circleBeat;
	BpmTapTempo bpmTapTempo;

	//-

private:
	ofParameter<bool> bKeys;

	//-

	// Midi_out_clock
	void setup_MidiOut_Clock();
	float bpmOut;
	ofxMidiOut midiOut;
	float pulseDuration; // microseconds per pulse
	uint64_t lastPulseTime;
	bool clockRunning;
	int channelOut;

	//-

// Midi_in_clock

private:
	ofxMidiIn midiIn;
	ofxMidiClock midiIn_Clock; //< clock message parser

	ofxMidiMessage midiIn_Clock_Message;
	void newMidiMessage(ofxMidiMessage& eventArgs);

	double midiIn_Clock_Bpm; //< song tempo in bpm, computed from clock length
	bool bMidiInClockRunning; //< is the clock sync running?
	unsigned int MIDI_beats; //< song pos in beats
	int MIDI_quarters; //convert total # beats to # quarters
	int MIDI_bars; //compute # of bars
	double MIDI_seconds; //< song pos in seconds, computed from beats

	//TEST
	//int MIDI_ticks;//16th ticks are not implemented on the used ofxMidi example

	//-

	ofParameter<int> midiIn_BeatsInBar; //compute remainder as # TARGET_NOTES_params within the current bar
	void Changed_Midi_In_BeatsInBar(int& beatsInBar); //only used in midiIn clock sync
	int beatsInBar_PRE; //not required

	//-

	// External Midi Clock

	void setup_MidiIn_Clock();

public:
	//--------------------------------------------------------------
	void toggleVisibleGui() {
		bGui = !bGui;
	}

	//--

private:
	void draw_ImGui_CircleBeatWidget();

	bool bb[4];
	ofColor cb[4];
	std::string infoClockTimer;

	//--

	// Debug helpers

	// Red anchor circle to debug mark
	bool bDebugLayout = false;
	//--------------------------------------------------------------
	void draw_Anchor(int x, int y) {
		if (bDebugLayout) {
			ofPushStyle();
			ofFill();
			ofSetColor(ofColor(0, 200));

			//circle
			ofDrawCircle(x, y, 3);

			//text
			int pad;
			if (y < 15)
				pad = 10;
			else
				pad = -10;
			ofDrawBitmapStringHighlight(ofToString(x) + "," + ofToString(y), x, y + pad);
			ofPopStyle();
		}
	}

	//public:

	//--------------------------------------------------------------
	void setDebug_Clock(bool b) {
		ui.bDebug = b;
	}
	//--------------------------------------------------------------
	void toggleDebug_Clock() {
		ui.bDebug = !ui.bDebug;
	}
	//--------------------------------------------------------------
	void setDebug_Layout(bool b) {
		bDebugLayout = b;
	}
	//--------------------------------------------------------------
	void toggleDebug_Layout() {
		bDebugLayout = !bDebugLayout;
	}

	//--

private:
	// Main text color
	//ofColor colorText;

	//-

	// Monitor_visual_feedback

private:
	// Beat Ball
	ofPoint circlePos;

	float dt;

	//public:

	//--------------------------------------------------------------
	void setFrameRate(float _fps) {
		dt = 1.0f / _fps;
	}

private:
	// Main receiver
	// Trigs sound and gui drawing ball visual feedback
	void doBeatTickMonitor(int beat); //trigs ball drawing and sound ticker

	int lastBeatFlash = -1;
	//void draw_BeatBalFlash(int _onBeat){}

	//-

	////TODO:
	////WIP
	//private:
	//smooth clock for midi input clock sync
	////  REFRESH_FEQUENCY
	////used only when BPM_MIDI_CLOCK_REFRESH_RATE is defined
	//unsigned long BPM_LAST_Tick_Time_LAST;//test
	//unsigned long BPM_LAST_Tick_Time_ELLAPSED;//test
	//unsigned long BPM_LAST_Tick_Time_ELLAPSED_PRE;//test
	//long ELLAPSED_diff;//test
	//unsigned long bpm_CheckUpdated_lastTime;

	//-

	//public:
private:
	void setupGui();
	void buildGuiStyles();
	void refresh_Gui();
	void refresh_GuiWidgets();

#ifdef OFXBEATCLOCK_USE_OFX_SURFING_IM_GUI
	void draw_ImGui_Windows();
	void draw_ImGui_Sources();
	void draw_ImGui_ClockMonitor();
	void draw_ImGui_ClockBpm();
#endif

public:
	void draw_ImGui_GameMode(); // final user selected / most important controls!

	//-

private:
	ofParameterGroup params_INTERNAL;
	ofParameterGroup params_EXTERNAL_MIDI;
	ofParameterGroup params_BPM_Clock;

	ofJson confg_Button_C, confg_Button_L, confg_ButtonSmall, confg_Sliders; //json theme

	//-

	// Params

public:
	ofParameter<bool> bPlay; // Play global for all different source clock modes
	ofParameter<bool> bGui; // Main GUI

private:
	ofParameterGroup params_CONTROL;

	ofParameter<bool> bPlaying_Internal_State; // Player state only for internal clock
	ofParameter<bool> bPlaying_ExternalSync_State; // Player state only for external clock

	ofParameter<bool> bEnableClock; // Enable clock (affects all clock types)
	ofParameter<bool> bMode_Internal_Clock; // Enable internal clock
	ofParameter<bool> bMode_External_MIDI_Clock; // Enable external midi clock sync
	ofParameter<int> midiIn_Port_SELECT;
	int midiIn_numPorts = 0;

	//----

public:
	// This is the main and final target bpm, is the destination of all other clocks (internal, external midi sync or ableton link)

	ofParameter<float> BPM_Global; // Global tempo bpm.
	ofParameter<int> BPM_Global_TimeBar; // ms time of 1 bar = 4 beats

	//----

private:
	ofParameter<bool> BPM_Tap_Tempo_TRIG; // Trig the measurements of tap tempo

	// Helpers to modify current bpm
	ofParameter<bool> bReset_BPM_Global;
	ofParameter<bool> bHalf_BPM; // Divide bpm by 2
	ofParameter<bool> bDouble_BPM; // Multiply bpm by 2

	//--

	//public:
private:
	//--

	// These methods could be useful only to visualfeedback on integrating to other bigger guis on ofApp..
	//--------------------------------------------------------------
	bool getInternalClockModeState() {
		return bMode_Internal_Clock;
	}
	//--------------------------------------------------------------
	bool getExternalClockModeState() {
		return bMode_External_MIDI_Clock;
	}

#ifdef OFXBEATCLOCK_USE_ofxAbletonLink
	//--------------------------------------------------------------
	bool getLinkClockModeState() {
		return bMODE_AbletonLinkSync;
	}
#endif

	//--

private:
	// Main callback handler
	void Changed_Params(ofAbstractParameter& e);

	//-

	// Internal clock

	// Based on threaded timer using ofxDawMetro
	// Internal_clock

	ofxDawMetro clockInternal;

	// Callbacks defined inside the addon class. can't be renamed here
	// Overide ofxDawMetro::MetroListener's method if necessary
	void onBarEvent(int& bar) override;
	void onBeatEvent(int& beat) override;
	void onSixteenthEvent(int& sixteenth) override;

	ofParameter<float> BPM_ClockInternal;

	//NOTE:
	// For the moment this BPM variables is being used as
	// main tempo (sometimes) for other clock sources too.
	// TODO:
	// Maybe we should improve this using global bpm variable (BPM_Global) as main.

	ofParameterGroup params_ClockInternal;
	ofParameter<bool> clockInternal_Active;
	void Changed_ClockInternal_Bpm(float& value);
	void Changed_ClockInternal_Active(bool& value);

	//-

	////TODO:
	//void reSync();
	//ofParameter<bool> bSync_Trig;

	//--

	// Settings

private:
	std::string path_Global;

private:
	void saveSettings(std::string path);
	void loadSettings(std::string path);

	std::string file_BeatClock = "BeatClock_Settings.xml";
	std::string file_Midi = "Midi_Settings.xml";
	std::string file_App = "App_Settings.xml";

	ofParameterGroup params_AppSettings;

	//-

	// fonts
	std::string infoClockBpmLabel;
	std::string strTapTempo;
	std::string strExtMidiClock;
	std::string infoClock1;
	std::string strLink;
	std::string strMessageInfo;
	std::string infoDebug;
	std::string strMessageInfoFull;
	ofTrueTypeFont fontSmall;
	ofTrueTypeFont fontMedium;
	ofTrueTypeFont fontBig;

	//--

	// Beat ball
	ofPoint metronome_ball_pos;
	int metronome_ball_radius;

	//--

private:
	// Sound metronome
	ofParameter<bool> bSoundTickEnable; // enable sound ticks
	ofParameter<float> soundVolume; // sound ticks volume

	//-

private:
	void reset_ClockValues(); // set gui display text clock to 0:0:0

private:
	// Strings for monitor drawing
	// 1:1:1
	std::string Bar_string;
	std::string Beat_string;
	std::string Tick_16th_string;

	std::string clockActive_Type; // internal/external/link clock types name
	std::string infoClock2; // midi in port, and extra info for any clock source

	//----

	// Tap_engine

private:
	void tap_Update();
	float tap_BPM;
	bool SOUND_wasDisabled = false; // sound disabler to better user workflow

	//----

	// Change_midi_in_port

	void setup_MidiIn_Port(int p);
	int midiIn_Clock_Port_OPENED;
	int midiIn_Port_PRE = -1;
	ofParameter<std::string> midiIn_PortName{ "", "" };
	vector<string> names_MidiInPorts;

	//----

	// Step limiting

	// we don't need to use long song patterns
	// and we will limit bars to 4 like a simple step sequencer.
	bool ENABLE_pattern_limits;
	int pattern_BEAT_limit;
	int pattern_BAR_limit;

private:
	int Beat_current_PRE; // used to detect changes only on link mode
	float Beat_float_current; // link beat received  with decimals (float) and starting from 0 not 1
	//TODO:
	// Link
	std::string Beat_float_string;

	//----

	//TODO:
	// audioBuffer alternative timer mode to get a a more accurate clock!
	// based on:
	// https://forum.openframeworks.cc/t/audio-programming-basics/34392/10?u=moebiussurfing
	// by davidspry:
	// "the way I’m generating the clock is naive and simple.I’m simply counting the number of samples written to the buffer and sending a notification each time the number of samples is equal to one subdivided “beat”, as in BPM.
	// Presumably there’s some inconsistency because the rate of writing samples to the buffer is faster than the sample rate, but it seems fairly steady with a small buffer size."

	// NOTE: we will want maybe to use the same soundStream used for the sound tick also for the audiBuffer timer..
	// maybe will be a better solution to put this timer into ofxDawMetro class!!

#ifdef OFXBEATCLOCK_USE_AUDIO_BUFFER_TIMER_MODE
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
	void audioOut(ofSoundBuffer& buffer);
#endif

	//----

#ifdef OFXBEATCLOCK_USE_ofxAbletonLink
private:

	ofxAbletonLink link;

	ofParameter<bool> bMODE_AbletonLinkSync;

	ofParameterGroup params_LINK;

	ofParameter<bool> LINK_Enable; //enable link
	ofParameter<float> LINK_BPM; //link bpm
	ofParameter<bool> bPlaying_LinkState; //control and get Ableton Live playing too, mirrored like Link does
	ofParameter<float> LINK_Phase; //phase on the bar. cycled from 0.0f to 4.0f
	ofParameter<bool> LINK_ResyncBeat; //set beat 0
	ofParameter<bool> LINK_ResetBeats; //reset "unlimited-growing" beat counter
	ofParameter<std::string> LINK_Beat_string; //monitor beat counter
	//amount of beats are not limited nor in sync / mirrored with Ableton Live.
	ofParameter<std::string> LINK_Peers_string; //number of synced devices/apps on your network
	//ofParameter<int> LINK_Beat_Selector;//TODO: TEST
	//int LINK_Beat_Selector_PRE = -1;

	void LINK_setup();
	void LINK_update();
	void LINK_draw();
	void LINK_exit();
	void LINK_bpmChanged(double& bpm);
	void LINK_numPeersChanged(std::size_t& peers);
	void LINK_playStateChanged(bool& state);
	void Changed_LINK_Params(ofAbstractParameter& e);
#endif

};
