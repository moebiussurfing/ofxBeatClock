#include "ofxBeatClock.h"

//--------------------------------------------------------------
ofxBeatClock::ofxBeatClock() {
	ofSetLogLevel("ofxBeatClock", OF_LOG_NOTICE);
	path_Global = "ofxBeatClock/";

	// subscribed to auto run update and draw without required 'manual calls'
	ofAddListener(ofEvents().update, this, &ofxBeatClock::update);
	ofAddListener(ofEvents().keyPressed, this, &ofxBeatClock::keyPressed);
}

//--------------------------------------------------------------
ofxBeatClock::~ofxBeatClock() {
	exit();

	ofRemoveListener(ofEvents().update, this, &ofxBeatClock::update);
	ofRemoveListener(ofEvents().keyPressed, this, &ofxBeatClock::keyPressed);
}

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
	reset_ClockValues(); // set gui display text clock to 0:0:0

	//--

	setup_MidiIn_Clock();
	// should be defined before rest of GUI to list midi ports being included on gui

	//--

	// Define all parameters

	// Gui layout
	window_W = ofGetWidth();
	window_H = ofGetHeight();

	dt = 1.0f / 60.f; // default speed/fps is 60 fps

	//---

	bKeys.set("Keys", true);

	// 1.1 Controls

	bReset_BPM_Global.set("Reset", false);

	params_CONTROL.setName("USER CONTROL");
	params_CONTROL.add(bEnableClock.set("ENABLE", true));
	params_CONTROL.add(bPlay.set("PLAY", false));//TEST
	params_CONTROL.add(BPM_ClockInternal.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_CONTROL.add(bReset_BPM_Global);

	params_CONTROL.add(bMode_Internal_Clock.set("INTERNAL", false));
	params_CONTROL.add(bMode_External_MIDI_Clock.set("EXTERNAL", true));

#ifdef USE_ofxAbletonLink
	params_CONTROL.add(bMODE_AbletonLinkSync.set("ABLETON LINK", false));
#endif

	params_CONTROL.add(bGui_ClockMonitor.set("CLOCK MONITOR", true));
	params_CONTROL.add(bGui_ClockBpm.set("CLOCK BPM", true));
	params_CONTROL.add(bGui_Sources.set("CLOCK SOURCES", true));

	//params_CONTROL.add(bGui_PreviewClockNative.set("Clock Native", false));

	bGui.set("BEAT CLOCK", true);

	//--

	// 1.2 Internal Clock

	// Bar / beat / sixteenth listeners
	clockInternal.addBeatListener(this);
	clockInternal.addBarListener(this);
	clockInternal.addSixteenthListener(this);

	BPM_ClockInternal.addListener(this, &ofxBeatClock::Changed_ClockInternal_Bpm);
	BPM_ClockInternal.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX);

	clockInternal_Active.addListener(this, &ofxBeatClock::Changed_ClockInternal_Active);
	clockInternal_Active.set("Active", false);

	clockInternal.setBpm(BPM_ClockInternal);

	//-

	params_INTERNAL.setName("INTERNAL CLOCK");

	// Global play
	params_INTERNAL.add(bPlaying_Internal_State.set("PLAY INTERNAL", false));

	params_INTERNAL.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
	params_INTERNAL.add(bKeys);

	//TODO: should better gui-behavior-feel being a button not toggle

	//-

	//TODO:
	/// Trig re sync to closest beat? (not so simple as to go to bar start)
	//params_INTERNAL.add(bSync_Trig.set("SYNC", false));

	//-

	// 1.3 External Midi
	params_EXTERNAL_MIDI.setName("EXTERNAL MIDI CLOCK");
	params_EXTERNAL_MIDI.add(bPlaying_External_State.set("PLAY SYNC", false));
	params_EXTERNAL_MIDI.add(midiIn_Port_SELECT.set("INPUT", 0, 0, midiIn_numPorts - 1));
	params_EXTERNAL_MIDI.add(midiIn_PortName);

	//-

	// 1.4 Ableton Link

	//-

#ifdef USE_ofxAbletonLink
	LINK_setup();
#endif

	//-

#ifdef USE_ofxAbletonLink
	params_LINK.setName("ABLETON LINK");
	params_LINK.add(bPlaying_LinkState.set("PLAY LINK", false));
	params_LINK.add(LINK_Enable.set("LINK", true));
	params_LINK.add(LINK_Peers_string.set("PEERS", " "));
	params_LINK.add(LINK_Beat_string.set("BEAT", ""));
	params_LINK.add(LINK_Phase.set("PHASE", 0.f, 0.f, 4.f));
	params_LINK.add(LINK_BPM.set("BPM LINK", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_LINK.add(LINK_ResyncBeat.set("RESYNC", false));
	params_LINK.add(LINK_ResetBeats.set("FORCE RESET", false));
	//params_LINK.add(LINK_Beat_Selector.set("GO BEAT", 0, 0, 100));//TODO: TEST

	// Not required..
	LINK_Enable.setSerializable(false);
	LINK_BPM.setSerializable(false);
	bPlaying_LinkState.setSerializable(false);
	LINK_Phase.setSerializable(false);
	LINK_Beat_string.setSerializable(false);
	LINK_ResyncBeat.setSerializable(false);
	LINK_ResetBeats.setSerializable(false);
	LINK_Peers_string.setSerializable(false);
	//LINK_Beat_Selector.setSerializable(false);
#endif

	//--

	// 1.5 Extra and advanced settings

	// This smoothed (or maybe slower refreshed than fps) clock will be sended to target sequencer outside the class. see BPM_MIDI_CLOCK_REFRESH_RATE.
	params_BPM_Clock.setName("Clock BPM");
	params_BPM_Clock.add(BPM_Global.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_BPM_Clock.add(BPM_Global_TimeBar.set("BAR ms", (int)60000 / BPM_Global, 100, 2000));
	params_BPM_Clock.add(bReset_BPM_Global);

	// Added to group in other method (?)
	bHalf_BPM.set("Half", false);
	bDouble_BPM.set("Double", false);
	params_BPM_Clock.add(bHalf_BPM);
	params_BPM_Clock.add(bDouble_BPM);

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	MODE_AudioBufferTimer.set("MODE AUDIO BUFFER", false);
	params_BPM_Clock.add(MODE_AudioBufferTimer);
#endif

	params_BPM_Clock.add(bSoundTickEnable.set("Tick Sound", false));
	params_BPM_Clock.add(soundVolume.set("Volume", 0.5f, 0.f, 1.f));

	//-

	// Exclude
	midiIn_PortName.setSerializable(false);
	bPlaying_External_State.setSerializable(false);
	bReset_BPM_Global.setSerializable(false);

	//--

	// Display text
	std::string strFont;

	int sz1, sz2, sz3;

	strFont = "assets/fonts/overpass-mono-bold.otf";
	sz1 = 7;
	sz2 = 12;
	sz3 = 17;

	bool bOk = false;
	bOk |= fontSmall.load(strFont, sz1);
	bOk |= fontMedium.load(strFont, sz2);
	bOk |= fontBig.load(strFont, sz3);
	if (!bOk) // substitute font if not present on data/asssets/font with the OF native
	{
		fontSmall.load(OF_TTF_MONO, sz1);
		fontMedium.load(OF_TTF_MONO, sz2);
		fontBig.load(OF_TTF_MONO, sz3);
	}
	if (!fontSmall.isLoaded())
	{
		ofLogError(__FUNCTION__) << "ERROR LOADING FONT " << strFont;
		ofLogError(__FUNCTION__) << "Check your font file into /data/assets/fonts/";
	}

	params_CONTROL.add(guiManager.bMinimize);

	//-

	// Callbacks
	ofAddListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_INTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_EXTERNAL_MIDI.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_BPM_Clock.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//--

	// Add already initiated params from transport and all controls to the gui panel
	setupGui();

	//--

	// Other settings only to storing
	params_AppSettings.setName("AppSettings");

	params_AppSettings.add(bGui);
	params_AppSettings.add(bGui_ClockMonitor);
	params_AppSettings.add(bGui_ClockBpm);
	params_AppSettings.add(bGui_Sources);

	params_AppSettings.add(bMode_Internal_Clock);
	params_AppSettings.add(bMode_External_MIDI_Clock);

#ifdef USE_ofxAbletonLink
	params_AppSettings.add(bMODE_AbletonLinkSync);
#endif

	params_AppSettings.add(bPlay);
	params_AppSettings.add(bSoundTickEnable);
	params_AppSettings.add(soundVolume);
	params_AppSettings.add(guiManager.bDebug);

	params_AppSettings.add(bKeys);

	//---

	// main text color white
	//colorText = ofColor(255, 255);

	//---

	// draw beat ball trigger for sound and visual feedback monitor
	// this trigs to draw a flashing circle for a frame only
	BeatTick = false;

	// swap to a class..
	bpmTapTempo.setPathSounds("assets/sounds/");
	//bpmTapTempo.setPathSounds(path_Global + "sounds/");
	bpmTapTempo.setup();

	//-

	// text time to display
	Bar_string = "0";
	Beat_string = "0";
	Tick_16th_string = "0";

	//--

	// pattern limiting. (vs long song mode)
	// this only makes sense when syncing to external midi clock (playing a long song)
	// maybe not a poper solution yet..
#ifdef ENABLE_PATTERN_LIMITING
	ENABLE_pattern_limits = true;
#else
	pattern_limits = false;
#endif
	if (ENABLE_pattern_limits)
	{
		pattern_BEAT_limit = PATTERN_STEP_BEAT_LIMIT;
		pattern_BAR_limit = PATTERN_STEP_BAR_LIMIT;
	}

	//--

	startup();
}

//--------------------------------------------------------------
void ofxBeatClock::startup()
{
	// Load settings

	// Folder to both (control and midi input port) settings files
	string s = path_Global + "settings/";
	loadSettings(s);

	//-

	//TODO:
	// workaround
	// to ensure that gui workflow is updated after settings are loaded...
	// because sometimes initial gui state fails...
	// it seems that changed_params callback is not called after loading settings?
	//refresh_Gui();

	//--

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	setupAudioBuffer(0);
#endif

	bGui = true;
	bGui_ClockMonitor = true;
}

//--------------------------------------------------------------
void ofxBeatClock::buildGuiStyles()
{
	ofLogNotice(__FUNCTION__);

	guiManager.clearStyles();

	//if (0)
	{
		guiManager.AddStyle(bReset_BPM_Global, OFX_IM_BUTTON_SMALL);
		guiManager.AddStyle(bEnableClock, OFX_IM_BUTTON_BIG);

		guiManager.AddStyle(bMode_Internal_Clock, OFX_IM_BUTTON_BIG);
		guiManager.AddStyle(bMode_External_MIDI_Clock, OFX_IM_BUTTON_BIG);

#ifdef USE_ofxAbletonLink
		guiManager.AddStyle(bMODE_AbletonLinkSync, OFX_IM_BUTTON_BIG);
#endif
		guiManager.AddStyle(bPlaying_Internal_State, OFX_IM_TOGGLE_BIG);
		guiManager.AddStyle(BPM_Tap_Tempo_TRIG, OFX_IM_BUTTON_SMALL);
		//guiManager.AddStyle(bGui_PreviewClockNative, OFX_IM_TOGGLE_SMALL);

		if (guiManager.bMinimize.get())
		{
			guiManager.AddStyle(bHalf_BPM, OFX_IM_HIDDEN, 2, true);
			guiManager.AddStyle(bDouble_BPM, OFX_IM_HIDDEN, 2);
			guiManager.AddStyle(bSoundTickEnable, OFX_IM_HIDDEN);
			guiManager.AddStyle(soundVolume, OFX_IM_HIDDEN);
		}
		else
		{
			guiManager.AddStyle(bHalf_BPM, OFX_IM_BUTTON_SMALL, 2, true);
			guiManager.AddStyle(bDouble_BPM, OFX_IM_BUTTON_SMALL, 2);
			guiManager.AddStyle(bSoundTickEnable, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
			guiManager.AddStyle(soundVolume, OFX_IM_SLIDER);
		}
	}

	//-

	// Link

#ifdef USE_ofxAbletonLink
	//if (0)
	{
		guiManager.AddStyle(bPlaying_LinkState, OFX_IM_TOGGLE_BIG);
		guiManager.AddStyle(LINK_Enable, OFX_IM_TOGGLE_SMALL);
		guiManager.AddStyle(LINK_Peers_string, OFX_IM_TEXT_DISPLAY);
		guiManager.AddStyle(LINK_Beat_string, OFX_IM_TEXT_DISPLAY);
		guiManager.AddStyle(LINK_Phase, OFX_IM_INACTIVE);
		guiManager.AddStyle(LINK_BPM, OFX_IM_INACTIVE);
		guiManager.AddStyle(LINK_ResyncBeat, OFX_IM_BUTTON_SMALL);
		guiManager.AddStyle(LINK_ResetBeats, OFX_IM_BUTTON_SMALL);
	}
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::setupGui()
{
#ifdef USE_OFX_SURFING_IM_GUI

	guiManager.setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	guiManager.setup();

	guiManager.addWindowSpecial(bGui_ClockMonitor);
	guiManager.addWindowSpecial(bGui_Sources);
	guiManager.addWindowSpecial(bGui_ClockBpm);

	guiManager.startup();

	//-

	// Fonts

	// Add an extra bigger font
	// will be the index 4th, bc 3 font styles are added by default on ofxSurfingImGui / guiManger

	string _fontName = "JetBrainsMono-Bold.ttf"; // WARNING: will crash if font not present!
	std::string _path = "assets/fonts/"; // assets folder
	guiManager.pushFont(_path + _fontName, 60);

	//--

	// Customize widgets
	buildGuiStyles();

#endif
}

//--------------------------------------------------------------
void ofxBeatClock::refresh_Gui()
{
	// workflow

	//--

	if (bMode_Internal_Clock)
	{
		bMode_External_MIDI_Clock.setWithoutEventNotifications(false);
#ifdef USE_ofxAbletonLink
		bMODE_AbletonLinkSync = false;
#endif
		//--

		// Display Text
		clockActive_Type = bMode_Internal_Clock.getName();

		infoClock1 = clockActive_Type;
		infoClock2 = "\n";
	}

	//--

	else if (bMode_External_MIDI_Clock)
	{
		bMode_Internal_Clock.setWithoutEventNotifications(false);
#ifdef USE_ofxAbletonLink
		bMODE_AbletonLinkSync = false;
#endif
		//--

		//// workflow
		//// auto stop
		//if (bPlaying_Internal_State)
		//	bPlaying_Internal_State = false;
		////disable internal
		//if (clockInternal_Active)
		//	clockInternal_Active = false;

		//--

		clockInternal.stop();

		//TODO: 
		// trying to avoid log exceptions 
		//FF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
		//	The thread 0xc9d0 has exited with code 0 (0x0).
		//	Exception thrown at 0x00007FFF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
		//	The thread 0x17fa0 has exited with code 0 (0x0).
		//bPlaying_Internal_State = false;
		//clockInternal_Active = false;

		//--

		midiIn_PortName = midiIn.getName();

		// Display Text
		clockActive_Type = bMode_External_MIDI_Clock.getName();

		infoClock1 = clockActive_Type;
		infoClock2 = midiIn.getName();
	}

	//--

#ifdef USE_ofxAbletonLink

	else if (bMODE_AbletonLinkSync)
	{
		bMode_Internal_Clock.setWithoutEventNotifications(false);
		bMode_External_MIDI_Clock.setWithoutEventNotifications(false);

		//--

		if (bPlaying_Internal_State) bPlaying_Internal_State = false;
		if (clockInternal_Active) clockInternal_Active = false;

		ofLogNotice(__FUNCTION__) << "Add link listeners";
		ofAddListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
		ofAddListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
		ofAddListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

		bMode_Internal_Clock = false;
		bMode_External_MIDI_Clock = false;

		//-

		////TODO:
		//// this could be deleted/simplified
		//// workflow
		//// auto stop
		//if (bPlaying_Internal_State)
		//	bPlaying_Internal_State = false;
		//// disable internal
		//if (clockInternal_Active)
		//	clockInternal_Active = false;

		clockInternal.stop();//TODO: trying to avoid log exceptions 
		//FF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
		//	The thread 0xc9d0 has exited with code 0 (0x0).
		//	Exception thrown at 0x00007FFF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
		//	The thread 0x17fa0 has exited with code 0 (0x0).
		//bPlaying_Internal_State = false;
		//clockInternal_Active = false;

		//-

		// Display Text
		clockActive_Type = bMODE_AbletonLinkSync.getName();
		//clockActive_Type = "3 ABLETON LINK";

		infoClock1 = clockActive_Type;
		infoClock2 = "";

		if (bPlay != bPlaying_LinkState) bPlay = bPlaying_LinkState;
	}

#endif

	//--

	//  remove display labels if all disabled
#ifdef USE_ofxAbletonLink
	if (!bMode_Internal_Clock && !bMode_External_MIDI_Clock && !bMODE_AbletonLinkSync)
#else
	if (!bMode_Internal_Clock && !bMode_External_MIDI_Clock)
#endif
	{
		infoClock1 = "\n";
		infoClock2 = "\n";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::refresh_GuiWidgets()
{
	//if (!bGui_PreviewClockNative) circleBeat.update();
	circleBeat.update();

	//-

	// Text labels

	//-

	if (bpmTapTempo.isRunning())
	{
		strTapTempo = "     TAP " + ofToString(ofMap(bpmTapTempo.getCountTap(), 1, 4, 3, 0));
	}
	else
	{
		strTapTempo = "\n";
	}

	//-

	// A. External midi clock
	// midi in port. id number and name
	if (bMode_External_MIDI_Clock)
	{
		strExtMidiClock = ofToString(infoClock2);
	}

	//-

#ifdef USE_ofxAbletonLink
	// C. Ableton Link
	else if (bMODE_AbletonLinkSync)
	{
		strLink = ofToString(infoClock2);
	}
#endif	

	//--

	// 1.4 More debug info
	if (guiManager.bDebug)
	{
		infoDebug = "";
		infoDebug += ("BPM: " + ofToString(BPM_Global, 2));
		infoDebug += "\n";
		infoDebug += ("BAR: " + Bar_string);
		infoDebug += "\n";
		infoDebug += ("BEAT: " + Beat_string);
		infoDebug += "\n";
		infoDebug += ("Tick 16th: " + Tick_16th_string);
		infoDebug += "\n";
	}
	else { infoDebug = ""; }

	//-

	// Time pos
	infoClockTimer = ofToString(Bar_string, 3, ' ') + " : " + ofToString(Beat_string);

	// Only internal clock mode gets 16th ticks (beat / 16 duration)
	// Midi sync and link modes do not gets the 16th ticks, so we hide the (last) number to avoid confusion..

#ifndef USE_ofxAbletonLink
	if (!bMode_External_MIDI_Clock)
#else
	if (!bMode_External_MIDI_Clock && !bMODE_AbletonLinkSync)
#endif
	{
		infoClockTimer += " : " + ofToString(Tick_16th_string);
	}

	//--

	infoClockBpmLabel = ofToString(BPM_Global.get(), 2);

	//------

	// Colors
	int colorBoxes = 64; // grey
	int colorGreyDark = 32;
	int alphaBoxes = 164; // alpha of several above draw fills

	//--

	// 2. Beats squares [1] [2] [3] [4] 

	for (int i = 0; i < 4; i++)
	{
		// Define squares colors:
#ifdef USE_ofxAbletonLink
		if (bEnableClock && (bMode_External_MIDI_Clock || bMode_Internal_Clock || bMODE_AbletonLinkSync))
#else
		if (bEnableClock && (bMode_External_MIDI_Clock || bMode_Internal_Clock))
#endif
		{
			// Ligther color (colorBoxes) for beat squares that "has been passed [left]"
			
			if (i <= (Beat_current - 1))
			{
#ifdef USE_ofxAbletonLink
				//TEST:
				if (bMODE_AbletonLinkSync && LINK_Enable)
				{
					cb[i] = (bPlaying_LinkState) ? ofColor(colorBoxes, alphaBoxes) : ofColor(colorGreyDark, alphaBoxes);
				}
				else if (bMode_Internal_Clock)
				{
					cb[i] = (bPlaying_Internal_State) ? ofColor(colorBoxes, alphaBoxes) : ofColor(colorGreyDark, alphaBoxes);
				}

#else
				if (bMode_Internal_Clock)
				{
					cb[i] = (bPlaying_Internal_State) ? ofColor(colorBoxes, alphaBoxes) : ofColor(colorGreyDark, alphaBoxes);
				}
#endif
				else if (bMode_External_MIDI_Clock)
				{
					cb[i] = (bMidiInClockRunning) ? ofColor(colorBoxes, alphaBoxes) : ofColor(colorGreyDark, alphaBoxes);
				}
			}

			// dark color if beat square "is comming [right]"..
			else
			{
				cb[i] = ofColor(colorGreyDark, alphaBoxes);
			}
		}

		//-

		else // disabled all the 3 source clock modes
		{
			cb[i] = ofColor(colorGreyDark, alphaBoxes);
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MidiIn_Clock()
{
	// external midi clock

	ofLogNotice(__FUNCTION__);

	ofLogNotice(__FUNCTION__) << "LIST PORTS:";
	midiIn.listInPorts();

	midiIn_numPorts = midiIn.getNumInPorts();
	ofLogNotice(__FUNCTION__) << "NUM MIDI-IN PORTS:" << midiIn_numPorts;

	midiIn_Clock_Port_OPENED = 0;
	midiIn.openPort(midiIn_Clock_Port_OPENED);

	ofLogNotice(__FUNCTION__) << "connected to MIDI CLOCK IN port: " << midiIn.getPort();

	midiIn.ignoreTypes(
		true,	// sysex  <-- ignore timecode messages!
		false,	// timing <-- don't ignore clock messages!
		true);	// sensing

	////TEST:
	//// this can be used to add support to timecode sync too, if desired!
	//midiIn.ignoreTypes(
	//	false, // sysex  <-- don't ignore timecode messages!
	//	false, // timing <-- don't ignore clock messages!
	//	true); // sensing

	midiIn.addListener(this);

	bMidiInClockRunning = false; // < is the clock sync running?
	MIDI_beats = 0; // < song pos in beats
	MIDI_seconds = 0; // < song pos in seconds, computed from beats
	midiIn_Clock_Bpm = (double)BPM_INIT; //< song tempo in bpm, computed from clock length

	//-

	midiIn_BeatsInBar.addListener(this, &ofxBeatClock::Changed_Midi_In_BeatsInBar);
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MidiIn_Port(int p)
{
	ofLogNotice(__FUNCTION__) << "setup_MidiIn_Port: " << p;

	midiIn.closePort();
	midiIn_Clock_Port_OPENED = p;
	midiIn.openPort(midiIn_Clock_Port_OPENED);

	ofLogNotice(__FUNCTION__) << "PORT NAME: " << midiIn.getInPortName(p);

	ofLogNotice(__FUNCTION__) << "Connected to MIDI IN CLOCK Port: " << midiIn.getPort();


	// refresh
	bMode_External_MIDI_Clock = bMode_External_MIDI_Clock;
}

//--------------------------------------------------------------
void ofxBeatClock::update(ofEventArgs& args)
{
	refresh_GuiWidgets();

	//--

	// tap engine
	tap_Update();

	//--

#ifdef USE_ofxAbletonLink
	if (bMODE_AbletonLinkSync) LINK_update();
#endif
}

#ifdef USE_OFX_SURFING_IM_GUI
//--------------------------------------------------------------
void ofxBeatClock::draw_ImGui_Sources()
{
	IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;
	if (bGui_Sources)
		if (guiManager.beginWindowSpecial(bGui_Sources))
		{
			guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
			guiManager.AddSpacingSeparated();

			guiManager.Add(bGui_ClockMonitor, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
			guiManager.Add(bGui_ClockBpm, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
			guiManager.AddSpacingSeparated();

			guiManager.Add(bEnableClock, OFX_IM_TOGGLE_SMALL);

			// Hide all

			if (!bEnableClock)
			{
				guiManager.endWindowSpecial();

				return;
			}

			//----

			// Play

			if (!bGui)
			{
				const int hUnits = 4;

				float val = ofMap(circleBeat.getValue(), 0, 1, 0.25, 1.f, true);
				ofxImGuiSurfing::AddBigToggleNamed(bPlay, -1, -1, "PLAYING", "PLAY", true, val);

				//guiManager.Add(bPlay, OFX_IM_TOGGLE_BIG_XXL);

				//TODO:
				////guiManager.Add(bPlaying_Internal_State, OFX_IM_TOGGLE_BIG);
				//if (bMODE_AbletonLinkSync) AddBigToggleNamed(bPlaying_Internal_State, _w1, hUnits * _h, "PLAYING", "PLAY", true, link.getPhase());
				//else AddBigToggleNamed(bPlaying_Internal_State, _w1, hUnits * _h, "PLAYING", "PLAY", true, 1.f);
			}

			//ImGui::Spacing();

			//--

			// Sources

			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();

				static bool bOpen = false;
				ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt |= ImGuiTreeNodeFlags_Framed;

				if (ImGui::TreeNodeEx("SOURCES", _flagt))
				{
					guiManager.refreshLayout();

					guiManager.Add(bMode_Internal_Clock, OFX_IM_TOGGLE_BIG);
					guiManager.Add(bMode_External_MIDI_Clock, OFX_IM_TOGGLE_BIG);
#ifdef USE_ofxAbletonLink
					guiManager.Add(bMODE_AbletonLinkSync, OFX_IM_TOGGLE_BIG);
#endif
					ImGui::TreePop();
				}
			}

			//--

			// modes
			if (!guiManager.bMinimize)
			{
				//ImGui::Indent();
				if (bMode_Internal_Clock)
				{
					guiManager.AddSpacingSeparated();

					static bool bOpen = false;
					ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
					_flagt |= ImGuiTreeNodeFlags_Framed;

					if (ImGui::TreeNodeEx("INTERNAL", _flagt))
					{
						guiManager.refreshLayout();

						guiManager.Add(bPlaying_Internal_State, OFX_IM_TOGGLE_BIG);
						guiManager.Add(BPM_Tap_Tempo_TRIG, OFX_IM_BUTTON_BIG);
						//guiManager.AddGroup(params_INTERNAL, flags, SurfingImGuiTypesGroups::OFX_IM_GROUP_COLLAPSED);

						ImGui::TreePop();
					}
					//ImGui::Spacing();
				}

				//-

				if (bMode_External_MIDI_Clock)
				{
					guiManager.AddSpacingSeparated();

					static bool bOpen = false;
					ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
					_flagt |= ImGuiTreeNodeFlags_Framed;

					if (ImGui::TreeNodeEx("EXTERNAL", _flagt))
					{
						guiManager.refreshLayout();
						{
							guiManager.Add(bPlaying_External_State, OFX_IM_TOGGLE_BIG);
							guiManager.Add(midiIn_Port_SELECT, OFX_IM_SLIDER);
							//guiManager.Add(midiIn_Port_SELECT, OFX_IM_STEPPER);
							guiManager.Add(midiIn_PortName, OFX_IM_TEXT_DISPLAY);
							//guiManager.AddGroup(params_EXTERNAL_MIDI, flags, SurfingImGuiTypesGroups::OFX_IM_GROUP_COLLAPSED);
						}
						ImGui::TreePop();
					}
					guiManager.refreshLayout();

					//ImGui::Spacing();
				}

				//-

				// Link

#ifdef USE_ofxAbletonLink
				if (bMODE_AbletonLinkSync)
				{
					guiManager.AddSpacingSeparated();

					guiManager.AddGroup(params_LINK);

					//--

					//TODO: 
					// Circled progress preview... 
					if (0)
					{
						const char* label = "label";
						float control = ofMap(LINK_Phase.get(), LINK_Phase.getMin(), LINK_Phase.getMax(), 0.0f, 1.0f);
						static float radius = 20;
						static int thickness = 9;
						//static float thickness = 1.0f;

						//ImGui::SliderFloat("control", &control, 0, 1);
						//ImGui::SliderFloat("radius", &radius, 10, 200);
						//ImGui::SliderInt("thickness", &thickness, 0, 40);
						////ImGui::SliderFloat("thickness", &thickness, 0, 1.0f);

						//ImGui::ColorConvertFloat4ToU32(_color));
						ofColor colorTick = ofColor(128, 200);
						ImVec4 _color = colorTick;
						//ImVec4 _color = ImVec4(1, 1, 1, 1);
						static ImU32 color = ImGui::ColorConvertFloat4ToU32(_color);

						////circleCycled(label, &control);
						circleCycled(label, &control, radius, false, thickness, color, 20);
					}

					//ImGui::Spacing();
				}
#endif
				//ImGui::Unindent();
			}

			//--

			if (!guiManager.bMinimize)
			{
				static bool bExtra = false;
				//ImGui::Dummy(ImVec2(0, 5));

				guiManager.AddSpacingSeparated();

				ofxImGuiSurfing::ToggleRoundedButton("Extra", &bExtra);//TODO: remove all+
				if (bExtra)
				{
					ImGui::Indent();

					guiManager.Add(guiManager.bDebug, OFX_IM_TOGGLE_ROUNDED);

					//ofxImGuiSurfing::AddToggleRoundedButton(bGui_PreviewClockNative);//TODO: remove all+
					//if (bGui_PreviewClockNative) {
					//	ImGui::Indent();
					//	ofxImGuiSurfing::AddToggleRoundedButton(bShow_PreviewBoxEditor);
					//	ofxImGuiSurfing::AddToggleRoundedButton(bEdit_PreviewBoxEditor);
					//	ImGui::Unindent();
					//}

					ofxImGuiSurfing::AddToggleRoundedButton(bKeys);
					ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bAutoResize);

					ImGui::Unindent();
				}
			}

			guiManager.endWindowSpecial();
		}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGui_ClockBpm()
{
	IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

	if (bGui_ClockBpm)
		if (guiManager.beginWindowSpecial(bGui_ClockBpm))
		{
			guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
			guiManager.AddSpacingSeparated();

			float _w1 = ofxImGuiSurfing::getWidgetsWidth(1);
			ImGui::PushItemWidth(_w1 * 0.5);
			guiManager.AddGroup(params_BPM_Clock, ImGuiTreeNodeFlags_DefaultOpen, OFX_IM_GROUP_HIDDEN_HEADER);
			ImGui::PopItemWidth();

			guiManager.endWindowSpecial();
		}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGui_CircleBeatWidget()
{
	//TODO:
	// Convert to a new ImGui widget
	// Circle widget

	{
		// Big circle segments outperforms..
		//const int nsegm = 4;
		const int nsegm = 24;

		ofColor colorBeat = ofColor(ofColor::red, 200);
		ofColor colorTick = ofColor(128, 200);
		ofColor colorBallTap = ofColor(16, 200);
		ofColor colorBallTap2 = ofColor(96);

		//---

		float pad = 10;
		float __w100 = ImGui::GetContentRegionAvail().x - 2 * pad;

		float radius = 30;
		//float radius = __w100 / 2; // *circleBeat.getRadius();

		const char* label = " ";

		float radius_inner = radius * circleBeat.getValue();
		//float radius_inner = radius * ofxSurfingHelpers::getFadeBlink();
		float radius_outer = radius;
		//float spcx = radius * 0.1;

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		ImVec2 pos = ImGui::GetCursorScreenPos(); // get top left of current widget

		//float line_height = ImGui::GetTextLineHeight();
		//float space_height = radius * 0.1; // to add between top, text, knob, value and bottom
		//float space_width = radius * 0.1; // to add on left and right to diameter of knob

		float xx = pos.x + pad;
		float yy = pos.y + pad / 2;

		//ImVec4 widgetRec = ImVec4(
		//	pos.x,
		//	pos.y,
		//	radius * 2.0f + space_width * 2.0f,
		//	space_height * 4.0f + radius * 2.0f + line_height * 2.0f);

		const int spcUnits = 3;

		ImVec4 widgetRec = ImVec4(
			xx,
			yy,
			radius * 2.0f,
			radius * 2.0f + spcUnits * pad);

		//ImVec2 labelLength = ImGui::CalcTextSize(label);

		//ImVec2 center = ImVec2(
		//	pos.x + space_width + radius,
		//	pos.y + space_height * 2 + line_height + radius);

		//ImVec2 center = ImVec2(
		//	xx + radius,
		//	yy + radius);

		ImVec2 center = ImVec2(
			xx + __w100 / 2,
			yy + radius + pad);

		//yy + __w100 / 2);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImGui::InvisibleButton(label, ImVec2(widgetRec.z, widgetRec.w));

		//bool value_changed = false;
		//bool is_active = ImGui::IsItemActive();
		//bool is_hovered = ImGui::IsItemActive();
		//if (is_active && io.MouseDelta.x != 0.0f)
		//{
		//	value_changed = true;
		//}

		//-

		//// Draw label

		//float texPos = pos.x + ((widgetRec.z - labelLength.x) * 0.5f);
		//draw_list->AddText(ImVec2(texPos, pos.y + space_height), ImGui::GetColorU32(ImGuiCol_Text), label);

		//-

		ofColor cbg;

		// Background black ball
		if (!bpmTapTempo.isRunning())
		{
			// ball background when not tapping
			cbg = colorBallTap;
		}
		else
		{
			// white alpha fade when measuring tapping
			float t = (ofGetElapsedTimeMillis() % 1000);
			float fade = sin(ofMap(t, 0, 1000, 0, 2 * PI));
			ofLogVerbose(__FUNCTION__) << "fade: " << fade << endl;
			int alpha = (int)ofMap(fade, -1.0f, 1.0f, 0, 50) + 205;
			cbg = ofColor(colorBallTap2.r, colorBallTap2.g, colorBallTap2.b, alpha);
		}

		//-

		// Outer Circle

		draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImVec4(cbg)), nsegm);
		//draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : ImGuiCol_FrameBg), nsegm);
		//draw_list->AddCircleFilled(center, radius_outer * 0.8, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : ImGuiCol_FrameBg), nsegm);

		//-

		// Inner Circle

		// highlight 1st beat
		ofColor c;
		if (Beat_current == 1) c = colorBeat;
		else c = colorTick;

		draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(ImVec4(c)), nsegm);

		//draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_ButtonActive : is_hovered ? ImGuiCol_ButtonHovered : ImGuiCol_SliderGrab), nsegm);
		//draw_list->AddCircleFilled(center, radius_inner * 0.8, ImGui::GetColorU32(is_active ? ImGuiCol_ButtonActive : is_hovered ? ImGuiCol_ButtonHovered : ImGuiCol_SliderGrab), nsegm);

		//// draw value
		//char temp_buf[64];
		//sprintf(temp_buf, "%.2f", *p_value);
		//labelLength = ImGui::CalcTextSize(temp_buf);
		//texPos = pos.x + ((widgetRec.z - labelLength.x) * 0.5f);
		//draw_list->AddText(ImVec2(texPos, pos.y + space_height * 3 + line_height + radius * 2), ImGui::GetColorU32(ImGuiCol_Text), temp_buf);

		//-

		// Border Arc Progress

		float control = 0;

		if (bMode_Internal_Clock) control = ofMap(Beat_current, 0, 4, 0, 1);
		else if (bMode_External_MIDI_Clock) control = ofMap(Beat_current, 0, 4, 0, 1);
#ifdef USE_ofxAbletonLink
		else if (bMODE_AbletonLinkSync) control = ofMap(LINK_Phase.get(), LINK_Phase.getMin(), LINK_Phase.getMax(), 0.0f, 1.0f);
#endif
		const int padc = 0;
		static float _radius = radius_outer + padc;
		static int num_segments = 35;
		static float startf = 0.0f;
		static float endf = IM_PI * 2.0f;
		static float offsetf = -IM_PI / 2.0f;
		static float _thickness = 3.0f;
		ofColor cb = ofColor(c.r, c.g, c.b, c.a * 0.6f);
		draw_list->PathArcTo(center, _radius, startf + offsetf, control * endf + offsetf, num_segments);
		draw_list->PathStroke(ImGui::ColorConvertFloat4ToU32(cb), ImDrawFlags_None, _thickness);
	}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGui_ClockMonitor()
{
	IMGUI_SUGAR__WINDOWS_CONSTRAINTSW;

	if (bGui_ClockMonitor)
		if (guiManager.beginWindowSpecial(bGui_ClockMonitor))
		{
			float __w100 = ImGui::GetContentRegionAvail().x;
			float _w4 = __w100 / 4;
			float _h = _w4;

			// Minimize

			guiManager.Add(guiManager.bMinimize, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
			guiManager.AddSpacingSeparated();

			//--

			// 1. BPM + counters

			// BPM number

			guiManager.pushStyleFont(3);
			const auto sz1 = ImGui::CalcTextSize(infoClockBpmLabel.c_str());
			float sw1 = GetContentRegionAvail().x;
			ImGui::Text("");
			ImGui::SameLine(sw1 - sz1.x);
			ImGui::Text(infoClockBpmLabel.c_str());
			guiManager.popStyleFont();

			//--

			// BPM label

			string infoClockBpmValue = "BPM";
			guiManager.pushStyleFont(3);
			float sw2 = GetContentRegionAvail().x;
			ImGui::Text("");
			const auto sz2 = ImGui::CalcTextSize(infoClockBpmValue.c_str());
			ImGui::SameLine(sw2 - sz2.x);
			ImGui::Text(infoClockBpmValue.c_str());
			guiManager.popStyleFont();

			//guiManager.AddLabelHuge(infoClockBpmValue.c_str(), false, true);

			//-

			if (!guiManager.bMinimize)
			{
				// 1 : 1

				guiManager.pushStyleFont(2);
				{
					const auto sz = ImGui::CalcTextSize(infoClockTimer.c_str());
					float sw = GetContentRegionAvail().x;
					ImGui::Text("");
					ImGui::SameLine(sw - sz.x);
					ImGui::Text(infoClockTimer.c_str());
				}
				guiManager.popStyleFont();

				ImGui::Spacing();

				//--

				// Source Input Info

				//if (infoClock1 != "" && infoClock2 != "") 
				{
					guiManager.pushStyleFont(1);
					{
						// 1
						if (infoClock1 != "")
						{
							//// right
							//const auto sz = ImGui::CalcTextSize(infoClock1.c_str());
							//float sw = GetContentRegionAvail().x;
							//ImGui::Text("");
							//ImGui::SameLine(sw - sz.x);

							ImGui::TextWrapped(infoClock1.c_str());

							ImGui::Spacing();
						}

						// 2
						if (infoClock2 != "")
						{
							ImGui::TextWrapped(infoClock2.c_str());
						}
					}
					guiManager.popStyleFont();

					guiManager.AddSpacing();
				}
			}

			//ImGui::TextWrapped(strExtMidiClock.c_str());
			//ImGui::TextWrapped(strLink.c_str());

			if (!guiManager.bMinimize)
			{
				// 3. Debug
				if (infoDebug != "")
				{
					guiManager.pushStyleFont(0);
					{
						ImGui::TextWrapped(infoDebug.c_str());
					}
					guiManager.popStyleFont();
				}
			}

			//ImGui::TextWrapped(strMessageInfoFull.c_str());

			//--

			// 1. Beat Boxes 1-2-3-4

			if (!guiManager.bMinimize)
			{
				guiManager.pushStyleFont(1);
				{
					ImGui::PushStyleColor(ImGuiCol_Button, cb[0]);
					ImGui::Button("1", ImVec2(_w4, _h));
					ImGui::PopStyleColor();
					ImGui::SameLine(0, 0);

					ImGui::PushStyleColor(ImGuiCol_Button, cb[1]);
					ImGui::Button("2", ImVec2(_w4, _h));
					ImGui::PopStyleColor();
					ImGui::SameLine(0, 0);

					ImGui::PushStyleColor(ImGuiCol_Button, cb[2]);
					ImGui::Button("3", ImVec2(_w4, _h));
					ImGui::PopStyleColor();
					ImGui::SameLine(0, 0);

					ImGui::PushStyleColor(ImGuiCol_Button, cb[3]);
					ImGui::Button("4", ImVec2(_w4, _h));
					ImGui::PopStyleColor();

					ImGui::Spacing();
				}
				guiManager.popStyleFont();
			}

			//--

			// 2. Circle Beat
			draw_ImGui_CircleBeatWidget();

			//--

			// 4. Tap
			if (strTapTempo != "")
			{
				guiManager.pushStyleFont(2);
				{
					int speed = 10;//blink speed when measuring taps
					bool b = (ofGetFrameNum() % (2 * speed) < speed);
					ImVec4 c = ImVec4(1.f, 1.f, 1.f, b ? 0.5f : 1.f);
					ImGui::PushStyleColor(ImGuiCol_Text, c);
					ImGui::TextWrapped(strTapTempo.c_str());
					ImGui::PopStyleColor();
				}
				guiManager.popStyleFont();
			}

			//--

			// 5. Play 

			float val = ofMap(circleBeat.getValue(), 0, 1, 0.25, 1.f, true);
			ofxImGuiSurfing::AddBigToggleNamed(bPlay, -1, -1, "PLAYING", "PLAY", true, val);

			//--

			// 6. Extra
			// BPM, reset 
			//if (guiManager.bMinimize)
			{
				if (bMode_Internal_Clock)
				{
					if (!bGui_Sources || guiManager.bMinimize)
						guiManager.Add(BPM_Tap_Tempo_TRIG, OFX_IM_BUTTON_BIG);

					if (!bGui_ClockBpm || guiManager.bMinimize) {
						guiManager.AddSpacing();
						guiManager.Add(BPM_Global);
						guiManager.Add(bReset_BPM_Global, OFX_IM_BUTTON);
					}
				}
			}

			//--

			// 7. Panels
			if (!guiManager.bMinimize)
			{
				guiManager.AddSpacingSeparated();

				guiManager.Add(bGui_Sources, OFX_IM_TOGGLE_ROUNDED);
				guiManager.Add(bGui_ClockBpm, OFX_IM_TOGGLE_ROUNDED);
			}

			guiManager.endWindowSpecial();
		}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGui_Widgets()
{
	if (!bGui) return;

	guiManager.begin();
	{
		if (bGui_ClockMonitor) draw_ImGui_ClockMonitor();
		if (bGui_Sources) draw_ImGui_Sources();
		if (bGui_ClockBpm) draw_ImGui_ClockBpm();
	}
	guiManager.end();
}
#endif

//--------------------------------------------------------------
void ofxBeatClock::drawGui()
{
#ifdef USE_OFX_SURFING_IM_GUI
	if (bGui) draw_ImGui_Widgets();
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::draw()
{
	//#ifdef USE_ofxAbletonLink
	//		if (bMODE_AbletonLinkSync && guiManager.bDebug)
	//		{
	//			LINK_draw();
	//		}
	//#endif
	//}

	//--

	drawGui();
}

//--------------------------------------------------------------
void ofxBeatClock::exit()
{
	ofLogNotice(__FUNCTION__);

	//--

	saveSettings(path_Global + "settings/");

	//--

	// Internal clock

	// bar/beat/sixteenth listeners
	clockInternal.removeBeatListener(this);
	clockInternal.removeBarListener(this);
	clockInternal.removeSixteenthListener(this);

	BPM_ClockInternal.removeListener(this, &ofxBeatClock::Changed_ClockInternal_Bpm);
	clockInternal_Active.removeListener(this, &ofxBeatClock::Changed_ClockInternal_Active);

	//--

	// External midi

	midiIn.closePort();
	midiIn.removeListener(this);
	midiIn_BeatsInBar.removeListener(this, &ofxBeatClock::Changed_Midi_In_BeatsInBar);

	//--

	// Link
#ifdef USE_ofxAbletonLink
	LINK_exit();
#endif

	//--

	// Remove other listeners

	ofRemoveListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofRemoveListener(params_BPM_Clock.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofRemoveListener(params_INTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofRemoveListener(params_EXTERNAL_MIDI.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//--
}

//--------------------------------------------------------
void ofxBeatClock::start()//only used in internal and link modes
{
	ofLogNotice(__FUNCTION__);

	if (!bEnableClock) return;

	if (bMode_Internal_Clock)
	{
		ofLogNotice(__FUNCTION__) << "START (internal clock)";

		if (!bPlaying_Internal_State) bPlaying_Internal_State = true;
		if (!clockInternal_Active) clockInternal_Active = true;

		//-

		////TODO:
		//if (MODE_AudioBufferTimer)
		//{
		//	samples = 0;
		//}
		//else
		//{
		//	bIsPlaying = true;
		//}
	}

#ifdef USE_ofxAbletonLink
	else if (bMODE_AbletonLinkSync)
	{
		ofLogNotice(__FUNCTION__) << "bPlaying_LinkState";
		bPlaying_LinkState = true;
	}
#endif

	else
	{
		ofLogNotice(__FUNCTION__) << "skip start";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::stop() // only used in internal mode
{
	ofLogNotice(__FUNCTION__);

	if (bMode_Internal_Clock && bEnableClock)
	{
		ofLogNotice(__FUNCTION__) << "STOP";

		if (bPlaying_Internal_State) bPlaying_Internal_State = false;
		if (clockInternal_Active) clockInternal_Active = false;

		reset_ClockValues(); // set gui display text clock to 0:0:0
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (bMODE_AbletonLinkSync && bEnableClock)
	{
		bPlaying_LinkState = false;
		ofLogNotice(__FUNCTION__) << "bPlaying_LinkState: " << bPlaying_LinkState;

		reset_ClockValues(); // set gui display text clock to 0:0:0
	}
#endif

	//-

	else
	{
		ofLogNotice(__FUNCTION__) << "skip stop";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::setTogglePlay() // only used in internal mode
{
	ofLogNotice(__FUNCTION__);

	if (!bEnableClock) return;

	//-

	// workflow
#ifdef USE_ofxAbletonLink
	if (!bMode_External_MIDI_Clock && !bMODE_AbletonLinkSync && !bMode_Internal_Clock) bMode_Internal_Clock = true;
#else	
	if (!bMode_External_MIDI_Clock && !bMode_Internal_Clock) bMode_Internal_Clock = true;
#endif

	//-

	if (bMode_Internal_Clock)
	{
		bPlaying_Internal_State = !bPlaying_Internal_State;
		ofLogNotice(__FUNCTION__) << "PLAY internal: " << bPlaying_Internal_State;
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (bMODE_AbletonLinkSync)
	{
		bPlaying_LinkState = !bPlaying_LinkState;
		ofLogNotice(__FUNCTION__) << "bPlaying_LinkState: " << bPlaying_LinkState;
	}
#endif

	//-

	else
	{
		ofLogNotice(__FUNCTION__) << "Skip setTogglePlay";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::doBeatTickMonitor(int _beat)
{
	// Trigs ball drawing and sound ticker
	ofLogVerbose(__FUNCTION__) << "> BeatTick : " << Beat_string;

#ifdef USE_ofxAbletonLink
	if (bEnableClock && (bMode_Internal_Clock || bMode_External_MIDI_Clock || bMODE_AbletonLinkSync))
#else
	if (bEnableClock && (bMode_Internal_Clock || bMode_External_MIDI_Clock))
#endif
	{
		BeatTick = true;

		// gui engine preview
		circleBeat.bang();

		//-

		// play metronome sound
		if (bSoundTickEnable)
		{
			if (Beat_current == 1)
			{
				bpmTapTempo.trigSound(0);
			}
			else
			{
				bpmTapTempo.trigSound(1);
			}
		}

		////TODO: 
		////BUG: 
		////why sometimes sound 1st tick its trigged at second beat??

		////play tic on the first beat of a bar
		////set metronome silent when measuring tap tempo engine
		//if (bSoundTickEnable && !bTap_Running)
		//{
		//	//BUG:
		//	//sometimes metronome ticks goes on beat 2 instead 1.
		//	//works better with 0 and 4 detectors, but why?
		//	//!!
		//	//we must check better all the beat%limit bc should be the problem!!
		//	//must check each source clock type what's the starting beat: 0 or 1!!
		//	//if (_beat == 0 || _beat == 4 )
		//	if (Beat_current == 1)
		//	{
		//		tic.play();
		//	}
		//	else
		//	{
		//		tac.play();
		//	}
		//}
	}
}

//--------------------------------------------------------------
float ofxBeatClock::getBpm() const
{
	return BPM_Global.get();
}

//--------------------------------------------------------------
int ofxBeatClock::getTimeBar() const
{
	return BPM_Global_TimeBar.get();
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_Params(ofAbstractParameter& e)
{
	string name = e.getName();

	if (name != BPM_Global.getName() &&
		name != BPM_Global_TimeBar.getName() &&
		name != bPlaying_External_State.getName()
		)

		ofLogNotice(__FUNCTION__) << name << " : " << e;

	//--

	if (0) {}

	//--

	// Play toggles

	// Global

	else if (name == bPlay.getName())
	{
		ofLogNotice(__FUNCTION__) << name << (bPlay ? "TRUE" : "FALSE");
		{
			//-

			// lock stopped if all modes disabled
			if (bPlay)
			{
#ifdef USE_ofxAbletonLink
				if (!bMode_External_MIDI_Clock && !bMODE_AbletonLinkSync && !bMode_Internal_Clock) {
#else
				if (!bMode_External_MIDI_Clock && !bMode_Internal_Clock) {
#endif
					bPlay = false;
					return;
				}
			}

			//// worklow
			//// put one mode active. default is internal
			//#ifdef USE_ofxAbletonLink
			//			if (!bMode_External_MIDI_Clock && !bMODE_AbletonLinkSync && !bMode_Internal_Clock) bMode_Internal_Clock = true;
			//#else
			//			if (!bMode_External_MIDI_Clock && !bMode_Internal_Clock) bMode_Internal_Clock = true;
			//#endif

			if (bMode_Internal_Clock) {
				if (bPlaying_Internal_State != bPlay) bPlaying_Internal_State = bPlay;
			}
			else if (bMode_External_MIDI_Clock) {
				if (bPlaying_External_State != bPlay) bPlaying_External_State = bPlay;
			}
#ifdef USE_ofxAbletonLink
			else if (bMODE_AbletonLinkSync) {
				if (bPlaying_LinkState != bPlay) bPlaying_LinkState = bPlay;
			}
#endif
		}
	}

	//--

	// Minimize

	if (name == guiManager.bMinimize.getName())
	{
		buildGuiStyles();
	}

	//--

	// External

	else if (name == bPlaying_External_State.getName())//button for internal only
	{
		if (bMode_External_MIDI_Clock)
			if (bPlay != bPlaying_External_State)
				bPlay = bPlaying_External_State;
	}

	//--

	// Internal

	else if (name == bPlaying_Internal_State.getName())//button for internal only
	{
		ofLogNotice(__FUNCTION__) << "bPlaying_Internal_State: " << (bPlaying_Internal_State ? "TRUE" : "FALSE");
		if (!bEnableClock) return;

		//-

		// workflow
		// put one mode active. default is internal
#ifdef USE_ofxAbletonLink
		if (!bMode_External_MIDI_Clock && !bMODE_AbletonLinkSync && !bMode_Internal_Clock) bMode_Internal_Clock = true;
#else
		if (!bMode_External_MIDI_Clock && !bMode_Internal_Clock) bMode_Internal_Clock = true;
#endif
		//--

		//if (bMODE_AbletonLinkSync)
		//{
		//	// workflow
		//	//if (bMODE_AbletonLinkSync) {
		//	//	if (!LINK_Enable) LINK_Enable = true;
		//	//	bPlaying_LinkState = bPlaying_Internal_State;
		//	//	// play
		//	//	if (bPlaying_Internal_State)
		//	//	{
		//	//		start();
		//	//	}
		//	//	// stop
		//	//	else
		//	//	{
		//	//		stop();
		//	//	}
		//	//}
		//}

		//// clocks are disabled or using external midi clock. dont need playing, just to be enabled
		//else if ((bMode_External_MIDI_Clock || !bEnableClock || !bMode_Internal_Clock) && bPlaying_Internal_State)
		//{
		//	bPlaying_Internal_State = false; // skip and restore to false the play button
		//}

		else if (bMode_Internal_Clock)
		{
			// Play
			if (bPlaying_Internal_State)
			{
				start();
			}

			// Stop
			else
			{
				stop();
			}

			//-

			if (bPlay != bPlaying_Internal_State)
				bPlay = bPlaying_Internal_State;
		}
	}

	//--

	// Link

#ifdef USE_ofxAbletonLink
	else if (name == bPlaying_LinkState.getName())
	{
		if (!bMODE_AbletonLinkSync) bMODE_AbletonLinkSync = true;
		if (bMODE_AbletonLinkSync)
		{
			if (!LINK_Enable) LINK_Enable = true;

			//-

			if (bPlay != bPlaying_LinkState)
				bPlay = bPlaying_LinkState;
		}
	}
#endif

	//----

	// BPM's

	else if (name == BPM_Global.getName())
	{
		ofLogVerbose(__FUNCTION__) << "GLOBAL BPM   : " << BPM_Global;

		//calculate bar duration to the new tempo
		BPM_Global_TimeBar = 60000.0f / (float)BPM_Global;//60,000 / BPM = one beat in milliseconds

		//---

		//NOTE: this update maybe can cause a kind of loop,
		//because BPM_ClockInternal, BPM_Global and LINK_BPM
		//are updated each others too...
		//TODO: we need some flag to avoid multiple updates when they are not required?

		//TODO:
		BPM_ClockInternal = BPM_Global;

		//-

		//TODO:
#ifdef USE_ofxAbletonLink
		if (bMODE_AbletonLinkSync) LINK_BPM = BPM_Global;
#endif

		//---

		//TODO:
#ifdef USE_AUDIO_BUFFER_TIMER_MODE
		samplesPerTick = (sampleRate * 60.0f) / BPM_Global / ticksPerBeat;
		ofLogNotice(__FUNCTION__) << "samplesPerTick: " << samplesPerTick;
#endif
		//-

		//ofLogVerbose(__FUNCTION__) << "TIME BEAT : " << BPM_TimeBar << "ms";
		//ofLogVerbose(__FUNCTION__) << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";
	}

	//--

	else if (name == BPM_Global_TimeBar.getName())
	{
		ofLogVerbose(__FUNCTION__) << "BAR ms: " << BPM_Global_TimeBar;

		BPM_Global = 60000.0f / (float)BPM_Global_TimeBar;

		//NOTE: this update maybe can cause a kind of loop,
		//because BPM_ClockInternal, BPM_Global and LINK_BPM
		//are updated each others too...
		//TODO: we need some flag to avoid multiple updates when they are not required?
	}

	//--

#ifdef USE_ofxAbletonLink
	else if (name == LINK_BPM.getName())
	{
		ofLogVerbose(__FUNCTION__) << "LINK BPM   : " << BPM_Global;
		if (bEnableClock && bMODE_AbletonLinkSync) {
			BPM_Global = LINK_BPM.get();
		}
	}
#endif

	//--

	// Source clock type

	//--

	// Internal

	else if (name == bMode_Internal_Clock.getName())
	{
		if (bMode_Internal_Clock)
		{
			bMode_External_MIDI_Clock.setWithoutEventNotifications(false);
#ifdef USE_ofxAbletonLink
			bMODE_AbletonLinkSync.setWithoutEventNotifications(false);
#endif
			if (bPlay != bPlaying_Internal_State) bPlay = bPlaying_Internal_State;
		}

		refresh_Gui();
	}

	//--

	// External

	else if (name == bMode_External_MIDI_Clock.getName())
	{
		if (bMode_External_MIDI_Clock)
		{
			bMode_Internal_Clock.setWithoutEventNotifications(false);
#ifdef USE_ofxAbletonLink
			bMODE_AbletonLinkSync.setWithoutEventNotifications(false);
#endif
			if (bPlay != bPlaying_External_State) bPlay = bPlaying_External_State;
		}

		refresh_Gui();
	}

	//--

	// Ableton Link

#ifdef USE_ofxAbletonLink
	else if (name == bMODE_AbletonLinkSync.getName())
	{
		if (bMODE_AbletonLinkSync)
		{
			bMode_External_MIDI_Clock.setWithoutEventNotifications(false);
			bMode_Internal_Clock.setWithoutEventNotifications(false);

			if (bPlay != bPlaying_LinkState) bPlay = bPlaying_LinkState;
		}
		else
		{
			ofLogNotice(__FUNCTION__) << "Remove link listeners";
			ofRemoveListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
			ofRemoveListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
			ofRemoveListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);
		}

		refresh_Gui();
}
#endif

	//--

	//TODO:
	// Should check this better

	else if (name == bEnableClock.getName())
	{
		ofLogNotice(__FUNCTION__) << "ENABLE: " << bEnableClock;

		// workflow
		if (!bEnableClock)
		{
			// internal clock
			if (bMode_Internal_Clock)
			{
				// auto stop
				if (bPlaying_Internal_State) bPlaying_Internal_State = false;

				// disable internal
				if (clockInternal_Active) clockInternal_Active = false;
			}
		}

		//--

		// workflow
		//#ifdef USE_ofxAbletonLink
		//		if (bMODE_AbletonLinkSync)
		//		{
		//			link.setLinkEnable(bEnableClock);
		//		}
		//#endif

		//--

		// workflow
		// if none clock mode is selected, we select the internal clock by default
		if (bEnableClock)
		{
#ifndef USE_ofxAbletonLink
			if (!bMode_Internal_Clock && !bMode_External_MIDI_Clock)
#else
			if (!bMode_Internal_Clock && !bMode_External_MIDI_Clock && !bMODE_AbletonLinkSync)
#endif
			{
				bMode_Internal_Clock = true; // force default state / clock selected
			}
		}
	}

	//---

	// External Midi Clock

	else if (name == midiIn_Port_SELECT.getName())
	{
		ofLogNotice(__FUNCTION__) << "midiIn_Port_SELECT: " << midiIn_Port_SELECT;

		if (midiIn_Port_PRE != midiIn_Port_SELECT)
		{
			midiIn_Port_PRE = midiIn_Port_SELECT;

			ofLogNotice(__FUNCTION__) << "LIST PORTS:";
			midiIn.listInPorts();
			midiIn_Port_SELECT.setMax(midiIn.getNumInPorts() - 1);

			ofLogNotice(__FUNCTION__) << "OPENING PORT: " << midiIn_Port_SELECT;
			setup_MidiIn_Port(midiIn_Port_SELECT);

			midiIn_PortName = midiIn.getName();
			ofLogNotice(__FUNCTION__) << "PORT NAME: " << midiIn_PortName;

			midiIn_Port_SELECT = ofClamp(midiIn_Port_SELECT, midiIn_Port_SELECT.getMin(), midiIn_Port_SELECT.getMax());
		}
	}

	//--

#ifdef USE_AUDIO_BUFFER_TIMER_MODE

	else if (name == "MODE AUDIO BUFFER")
	{
		ofLogNotice(__FUNCTION__) << "AUDIO BUFFER: " << MODE_AudioBufferTimer;

		// workflow
		//stop to avoid freeze bug
		bPlaying_Internal_State = false;

		//-

		//NOTE: both modes are ready to work and setted up.
		//this is required only if we want to use only one internal mode at the same time.
		//but we need to enable/disable listeners
		//(clockInternal and audioBuffer) 
		//but it seems that causes problems when connecting/disconnectings ports..
		//if (MODE_AudioBufferTimer)
		//{
		//	setupAudioBuffer(3);

		//	//if (clockInternal.is)//?
		//	//disable daw metro
		//	clockInternal.removeBeatListener(this);
		//	clockInternal.removeBarListener(this);
		//	clockInternal.removeSixteenthListener(this);
		//}
		//else
		//{
		//	closeAudioBuffer();

		//	//enable daw metro
		//	clockInternal.addBeatListener(this);
		//	clockInternal.addBarListener(this);
		//	clockInternal.addSixteenthListener(this);
		//}
	}
#endif

	//--

	// Helpers

	else if (name == bReset_BPM_Global.getName())
	{
		ofLogNotice(__FUNCTION__) << "BPM RESET";

		if (bReset_BPM_Global)
		{
			bReset_BPM_Global = false;//TODO: could be a button not toggle to better gui workflow

			BPM_ClockInternal = 120.00f;

			// not required bc
			//BPM_Global will be updated on the BPM_ClockInternal callback..etc

			//clockInternal.setBpm(BPM_ClockInternal);
			//BPM_Global = 120.00f;

			ofLogNotice(__FUNCTION__) << "BPM_Global: " << BPM_Global;
		}
	}

	else if (name == bDouble_BPM.getName())
	{
		if (bDouble_BPM)
		{
			ofLogNotice(__FUNCTION__) << "DOUBLE BPM";
			bDouble_BPM = false;

			BPM_ClockInternal = BPM_ClockInternal * 2.0f;
		}
	}

	else if (name == bHalf_BPM.getName())
	{
		if (bHalf_BPM)
		{
			ofLogNotice(__FUNCTION__) << "HALF BPM";
			bHalf_BPM = false;

			BPM_ClockInternal = BPM_ClockInternal / 2.0f;
		}
	}

	else if (name == bGui_ClockBpm.getName())
	{
		ofLogNotice(__FUNCTION__) << bGui_ClockBpm;
	}

	//--

	// Metronome ticks volume

	else if (name == soundVolume.getName())
	{
		ofLogNotice(__FUNCTION__) << "VOLUME: " << ofToString(soundVolume, 1);

		bpmTapTempo.setVolume(soundVolume);
	}
	else if (name == bSoundTickEnable.getName())
	{
		ofLogNotice(__FUNCTION__) << "TICK SOUND: " << (bSoundTickEnable ? "ON" : "OFF");
		bpmTapTempo.setEnableSound(bSoundTickEnable);
	}

	//--

	// Tap

	else if (name == BPM_Tap_Tempo_TRIG.getName())
	{
		ofLogNotice(__FUNCTION__) << "TAP BUTTON";

		if ((BPM_Tap_Tempo_TRIG == true) && (bMode_Internal_Clock))
		{
			BPM_Tap_Tempo_TRIG = false;

			ofLogNotice(__FUNCTION__) << "BPM_Tap_Tempo_TRIG: " << BPM_Tap_Tempo_TRIG;

			doTapTrig();
		}
	}

	//--

	//TODO:
	//else if (name == "SYNC")
	//{
	//	ofLogNotice(__FUNCTION__) << "bSync_Trig: " << bSync_Trig;
	//	if (bSync_Trig)
	//	{
	//		bSync_Trig = false;
	//		reSync();
	//	}
	//}
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_Midi_In_BeatsInBar(int& beatsInBar)
{
	ofLogVerbose(__FUNCTION__) << beatsInBar;

	// Only used in midiIn clock sync 
	// This function trigs when any midi tick (beatsInBar) updating, so we need to filter if (we want beat or 16th..) has changed.
	// Problem is maybe that beat param refresh when do not changes too, jus because it's accesed..

	if (ENABLE_pattern_limits)
	{
		midiIn_BeatsInBar = beatsInBar % pattern_BEAT_limit;
	}

	//--

	if (midiIn_BeatsInBar != beatsInBar_PRE)
	{
		ofLogVerbose(__FUNCTION__) << "MIDI-IN CLOCK BEAT TICK! " << beatsInBar;
		beatsInBar_PRE = midiIn_BeatsInBar;

		//-

		if (ENABLE_pattern_limits)
		{
			Beat_current = midiIn_BeatsInBar;
			Beat_string = ofToString(Beat_current);

			Bar_current = MIDI_bars % pattern_BAR_limit;
			Bar_string = ofToString(Bar_current);

			//TODO: should compute too?.. better precision maybe? but we are ignoring it now.
			Tick_16th_current = 0;
			Tick_16th_string = ofToString(Tick_16th_current);
		}
		else
		{
			Beat_current = midiIn_BeatsInBar;
			Bar_current = MIDI_bars;

			Beat_string = ofToString(Beat_current);
			Bar_string = ofToString(Bar_current);

			//TODO: not using ticks. should compute too?.. better precision maybe? but we are ignoring now
			Tick_16th_current = 0;
			Tick_16th_string = ofToString(Tick_16th_current);
		}

		//-

		// Trig ball drawing and sound ticker

		doBeatTickMonitor(Beat_current);

		//if (pattern_limits)
		//{
		//    beatsInBar_PRE = beatsInBar;
		//}
		//else
		//{
		//beatsInBar_PRE = midiIn_BeatsInBar;
		//}
	}
}

////TODO:
////--------------------------------------------------------------
//void ofxBeatClock::reSync()
//{
//	ofLogVerbose(__FUNCTION__) << "reSync";
//	//clockInternal.resetTimer();
//	//clockInternal.stop();
//	//clockInternal.start();
//	//bPlaying_Internal_State = true;
//}

//--------------------------------------------------------------
void ofxBeatClock::Changed_ClockInternal_Bpm(float& value)
{
	clockInternal.setBpm(value);

	//TODO:
	clockInternal.resetTimer(); //(?) is this required (?)

	//-

	BPM_Global = value;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_ClockInternal_Active(bool& value)
{
	ofLogVerbose(__FUNCTION__) << value;

	if (value)
	{
		clockInternal.start();
	}
	else
	{
		clockInternal.stop();
	}
}

//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage & message)
{
	if (bMode_External_MIDI_Clock && bEnableClock)
	{
		//1. MIDI CLOCK:

		if ((message.status == MIDI_TIME_CLOCK) ||
			(message.status == MIDI_SONG_POS_POINTER) ||
			(message.status == MIDI_START) ||
			(message.status == MIDI_CONTINUE) ||
			(message.status == MIDI_STOP))
		{

			//midiIn_Clock_Message = message;

			//1. MIDI CLOCK

			//update the clock length and song pos in beats
			if (midiIn_Clock.update(message.bytes))
			{
				//we got a new song pos
				MIDI_beats = midiIn_Clock.getBeats();
				MIDI_seconds = midiIn_Clock.getSeconds();
				////TEST:
				//MIDI_ticks = midiIn_Clock.();

				//-

				MIDI_quarters = MIDI_beats / 4; //convert total # beats to # quarters
				MIDI_bars = (MIDI_quarters / 4) + 1; //compute # of bars
				midiIn_BeatsInBar = (MIDI_quarters % 4) + 1; //compute remainder as # TARGET_NOTES_params within the current bar

				//-
			}

			//compute the seconds and bpm

			switch (message.status)
			{

			case MIDI_TIME_CLOCK:
				MIDI_seconds = midiIn_Clock.getSeconds();
				midiIn_Clock_Bpm += (midiIn_Clock.getBpm() - midiIn_Clock_Bpm) / 5.0;

				//average the last 5 bpm values
				//no break here so the next case statement is checked,
				//this way we can set bMidiInClockRunning if we've missed a MIDI_START
				//ie. master was running before we started this example

				//-

				BPM_Global = (float)midiIn_Clock_Bpm;

				//-

				//BPM_ClockInternal = (float)midiIn_Clock_Bpm;

				//-

				//transport control

			case MIDI_START:
			case MIDI_CONTINUE:
				if (!bMidiInClockRunning)
				{
					bMidiInClockRunning = true;
					ofLogVerbose(__FUNCTION__) << "external midi clock started";
				}
				break;

			case MIDI_STOP:
				if (bMidiInClockRunning)
				{
					bMidiInClockRunning = false;
					ofLogVerbose(__FUNCTION__) << "external midi clock stopped";

					reset_ClockValues();
				}
				break;

			default:
				break;
			}
		}

		//-

		bPlaying_External_State = bMidiInClockRunning;
		//TODO:
		//BUG:
		//sometimes, when external sequencer is stopped, 
		//bMidiInClockRunning do not updates to false!

		//bPlaying_Internal_State = bPlaying_External_State;
	}
}

//--------------------------------------------------------------
void ofxBeatClock::saveSettings(std::string path)
{
	ofxSurfingHelpers::CheckFolder(path);

	//save settings
	ofLogNotice(__FUNCTION__) << path;

	ofXml settings1;
	ofSerialize(settings1, params_CONTROL);
	settings1.save(path + file_BeatClock);

	ofXml settings2;
	ofSerialize(settings2, params_EXTERNAL_MIDI);
	settings2.save(path + file_Midi);

	ofXml settings3;
	ofSerialize(settings3, params_AppSettings);
	settings3.save(path + file_App);
}

//--------------------------------------------------------------
void ofxBeatClock::loadSettings(std::string path)
{
	//load settings
	ofLogNotice(__FUNCTION__) << path;
	ofxSurfingHelpers::CheckFolder(path);

	ofXml settings1;
	settings1.load(path + file_BeatClock);
	ofLogNotice(__FUNCTION__) << path + file_BeatClock << " : " << settings1.toString();
	ofDeserialize(settings1, params_CONTROL);

	ofXml settings2;
	settings2.load(path + file_Midi);
	ofLogNotice(__FUNCTION__) << path + file_Midi << " : " << settings2.toString();
	ofDeserialize(settings2, params_EXTERNAL_MIDI);

	ofXml settings3;
	settings3.load(path + file_App);
	ofLogNotice(__FUNCTION__) << path + file_App << " : " << settings3.toString();
	ofDeserialize(settings3, params_AppSettings);
}

//--------------------------------------------------------------

// Not used and callbacks are disabled on audioBuffer timer mode(?)

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int& bar)
{
	ofLogVerbose(__FUNCTION__) << "Internal Clock - BAR: " << bar;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (bEnableClock && bMode_Internal_Clock)
		{
			if (ENABLE_pattern_limits)
			{
				Bar_current = bar % pattern_BAR_limit;
			}
			else
			{
				Bar_current = bar;
			}

			Bar_string = ofToString(Bar_current);
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::onBeatEvent(int& beat)
{
	ofLogVerbose(__FUNCTION__) << "Internal Clock - BEAT: " << beat;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (bEnableClock && bMode_Internal_Clock)
		{
			if (ENABLE_pattern_limits)
			{
				Beat_current = beat % pattern_BEAT_limit;//limited to 16 beats
				//TODO: this are 16th ticks not beat!
			}
			else
			{
				Beat_current = beat;
			}
			Beat_string = ofToString(Beat_current);

			//-

			doBeatTickMonitor(Beat_current);

			//-
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::onSixteenthEvent(int& sixteenth)
{
	//ofLogVerbose(__FUNCTION__) << "Internal Clock - 16th: " << sixteenth;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (bEnableClock && bMode_Internal_Clock)
		{
			Tick_16th_current = sixteenth;
			Tick_16th_string = ofToString(Tick_16th_current);
		}
	}
}

//-

//--------------------------------------------------------------
void ofxBeatClock::reset_ClockValues()//set gui display text clock to 0:0:0
{
	ofLogVerbose(__FUNCTION__);

	//if (bMode_Internal_Clock)
	//{
	//	clockInternal.stop();
	//	clockInternal.resetTimer();
	//}

	Beat_current = 0;//0 is for stopped. goes from 1-2-3-4 when running
	Beat_string = ofToString(Beat_current);

	Bar_current = 0;
	Bar_string = ofToString(Bar_current);

	Tick_16th_current = 0;
	Tick_16th_string = ofToString(Tick_16th_current);
}

//--------------------------------------------------------------

//todo: implement bpm divider / multiplier x2 x4 /2 /4
//--------------------------------------------------------------
void ofxBeatClock::doTapTrig()
{
	if (bMode_Internal_Clock)//extra verified, not mandatory
		bpmTapTempo.bang();
}

//--------------------------------------------------------------
void ofxBeatClock::tap_Update()
{
	bpmTapTempo.update();
	if (bpmTapTempo.isUpdatedBpm()) {

		BPM_Global = bpmTapTempo.getBpm();
	}
}

//--------------------------------------------------------------
void ofxBeatClock::setBpm_ClockInternal(float bpm)
{
	BPM_ClockInternal = bpm;
}

//----

//TODO:
#ifdef USE_AUDIO_BUFFER_TIMER_MODE
//clock by audio buffer. not normal threaded clockInternal timer

//--------------------------------------------------------------
void ofxBeatClock::closeAudioBuffer()
{
	ofLogNotice(__FUNCTION__) << "closeAudioBuffer()";
	soundStream.stop();
	soundStream.close();
}

//--------------------------------------------------------------
void ofxBeatClock::setupAudioBuffer(int _device)
{
	ofLogNotice(__FUNCTION__) << "setupAudioBuffer()";

	//--

	//TODO:
	//start the sound stream with a sample rate of 44100 Hz, and a buffer
	//size of 512 samples per audioOut() call
	sampleRate = 44100;
	bufferSize = 512;
	//sampleRate = 48000;
	//bufferSize = 256;

	ofLogNotice(__FUNCTION__) << "OUTPUT devices";
	soundStream.printDeviceList();
	std::vector<ofSoundDevice> devicesOut;

	ofSoundStreamSettings settings;

	//-

	//different audio apis (wasapi/ds/asio)

	deviceOut = _device;

	//deviceOut = 3;//wasapi (line-1 out)//it seems that clock drift bad...
	//devicesOut = soundStream.getDeviceList(ofSoundDevice::Api::MS_WASAPI);
	//settings.setApi(ofSoundDevice::Api::MS_WASAPI);

	//deviceOut = 0;//ds
	//devicesOut = soundStream.getDeviceList(ofSoundDevice::Api::MS_DS);
	//settings.setApi(ofSoundDevice::Api::MS_DS);

	//deviceOut = 0;//asio//it seems that clock works more accurate!!
	devicesOut = soundStream.getDeviceList(ofSoundDevice::Api::MS_ASIO);
	settings.setApi(ofSoundDevice::Api::MS_ASIO);

	//-

	ofLogNotice(__FUNCTION__) << "connecting to device: " << deviceOut;

	settings.numOutputChannels = 2;
	settings.sampleRate = sampleRate;
	settings.bufferSize = bufferSize;
	settings.numBuffers = 1;
	//settings.setOutListener(ofGetAppPtr());//?
	settings.setOutListener(this);
	settings.setOutDevice(devicesOut[deviceOut]);

	soundStream.setup(settings);

	//-

	samplesPerTick = (sampleRate * 60.0f) / BPM_Global / ticksPerBeat;
	ofLogNotice(__FUNCTION__) << "samplesPerTick: " << samplesPerTick;

	//soundStream.name ? debug port name...
}

//--------------------------------------------------------------
void ofxBeatClock::audioOut(ofSoundBuffer & buffer)
{
	if (bPlaying_Internal_State && MODE_AudioBufferTimer)
	{
		for (size_t i = 0; i < buffer.getNumFrames(); ++i)
		{
			if (++samples == samplesPerTick)
			{
				//tick!
				samples = 0;

				if (DEBUG_bufferAudio)
				{
					DEBUG_ticks++;
					ofLogNotice(__FUNCTION__) << "[audioBuffer] Samples-TICK [" << DEBUG_ticks << "]";
				}

				//-

				if (bEnableClock && bMode_Internal_Clock)
				{
					//16th
					Tick_16th_current++;
					Tick_16th_string = ofToString(Tick_16th_current);

					//accumulate ticks until ticksPerBeat (default 4), then a 16th is filled
					//a beat tick happens
					if (Tick_16th_current == ticksPerBeat)
					{
						if (DEBUG_bufferAudio)
						{
							DEBUG_ticks = 0;
						}

						Tick_16th_current = 0;
						Tick_16th_string = ofToString(Tick_16th_current);

						//-

						//beat
						Beat_current++;
						Beat_string = ofToString(Beat_current);

						//-

						if (Beat_current == 5)//?
						{
							Beat_current = 1;
							Beat_string = ofToString(Beat_current);

							Bar_current++;
							Bar_string = ofToString(Bar_current);
						}

						if (Bar_current == 5)
						{
							Bar_current = 1;
							Bar_string = ofToString(Bar_current);
						}

						//-

						//trig beatTick for sound and gui feedback
						doBeatTickMonitor(Beat_current);

						//ofLogNotice(__FUNCTION__) << "[audioBuffer] BEAT: " << Beat_string;
					}
				}
			}
		}
	}
}
#endif

////--------------------------------------------------------------
//void ofxBeatClock::windowResized(int _w, int _h)
//{
//
//}

//--------------------------------------------------------------
void ofxBeatClock::keyPressed(ofKeyEventArgs & eventArgs)
{
	if (!bKeys) return;

	const int key = eventArgs.key;

	// Modifiers
	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

	ofLogNotice(__FUNCTION__) << " : " << key;

	switch (key)
	{
		// Toggle PLAY / STOP
	case ' ':
		setTogglePlay();
		break;

		// Trig tap-tempo 4 times 
		// when using internal clock 
		// only to set the bpm on the fly
	case 't':
		doTapTrig();
		break;

		// Get some beatClock info. look api methods into ofxBeatClock.h
	case OF_KEY_RETURN:
		ofLogWarning(__FUNCTION__) << "BPM     : " << getBpm() << " beats per minute";
		ofLogWarning(__FUNCTION__) << "BAR TIME: " << getTimeBar() << " ms";
		break;

		// Debug
	case 'd':
		toggleDebug_Clock(); // clock debug
		toggleDebug_Layout(); // layout debug
		break;

		// Show gui controls
	case 'g':
		toggleVisibleGui();
		break;

		//	// Show gui previews
		//case 'G':
		//	setToggleVisible_GuiPreview();
		//	break;

		//	// Edit gui preview
		//case 'e':
		//	bEdit_PreviewBoxEditor = !bEdit_PreviewBoxEditor;
		//	break;

	case '-':
		BPM_Global -= 1.0;
		break;

	case '+':
		BPM_Global += 1.0;
		break;
	}
}