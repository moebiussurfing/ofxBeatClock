#include "ofxBeatClock.h"

//--------------------------------------------------------------
ofxBeatClock::ofxBeatClock() {
	ofSetLogLevel("ofxBeatClock", OF_LOG_NOTICE);
	path_Global = "ofxBeatClock/";

	// subscribed to auto run update and draw without required 'manual calls'
	ofAddListener(ofEvents().update, this, &ofxBeatClock::update);
	ofAddListener(ofEvents().draw, this, &ofxBeatClock::draw);
	ofAddListener(ofEvents().keyPressed, this, &ofxBeatClock::keyPressed);
}

//--------------------------------------------------------------
ofxBeatClock::~ofxBeatClock() {
	exit();

	ofRemoveListener(ofEvents().update, this, &ofxBeatClock::update);
	ofRemoveListener(ofEvents().draw, this, &ofxBeatClock::draw);
	ofRemoveListener(ofEvents().keyPressed, this, &ofxBeatClock::keyPressed);
}

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
	reset_ClockValues();//set gui display text clock to 0:0:0

	//--

	setup_MidiIn_Clock();
	//should be defined before rest of gui to list midi ports being included on gui

	//--

	//define all parameters

	//gui layout
	window_W = ofGetWidth();
	window_H = ofGetHeight();

	dt = 1.0f / 60.f;//default speed/fps is 60 fps

	pos_Global.set("GUI POSITION GLOBAL",
		glm::vec2(window_W * 0.5, window_H * 0.5),
		glm::vec2(10, 10),
		glm::vec2(window_W, window_H)
	);

	pos_ClockInfo.set("GUI POSITION CLOCK INFO",
		glm::vec2(window_W * 0.5, window_H * 0.5),
		glm::vec2(210, 10),
		glm::vec2(window_W, window_H)
	);

	pos_BpmInfo.set("GUI POSITION BPM INFO",
		glm::vec2(window_W * 0.5, window_H * 0.5),
		glm::vec2(420, 10),
		glm::vec2(window_W, window_H)
	);

	pos_Gui.set("GUI POSITION PANEL",
		glm::vec2(window_W * 0.5, window_H * 0.5),
		glm::vec2(420, 10),
		glm::vec2(window_W, window_H)
	);

	//---

	bKeys.set("Keys", true);

	//1.1 controls

	RESET_BPM_Global.set("BPM Reset", false);

	params_CONTROL.setName("USER CONTROL");
	params_CONTROL.add(ENABLE_CLOCKS.set("ENABLE", true));
	//params_CONTROL.add(PLAYING_Global_State.set("PLAY", false));//TEST
	params_CONTROL.add(clockInternal_Bpm.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_CONTROL.add(RESET_BPM_Global);

	params_CONTROL.add(MODE_INTERNAL_CLOCK.set("INTERNAL", false));
	params_CONTROL.add(MODE_EXTERNAL_MIDI_CLOCK.set("EXTERNAL MIDI", true));
#ifdef USE_ofxAbletonLink
	params_CONTROL.add(MODE_ABLETON_LINK_SYNC.set("ABLETON LINK", false));
#endif
	params_CONTROL.add(bGui_BpmClock.set("Clock BPM", true));
	params_CONTROL.add(bGui_Transport.set("Clock Monitor", true));
	params_CONTROL.add(bGui_PreviewClockNative.set("Clock Native", false));

	bGui.set("BEAT CLOCK", true);

	//--

	//1.2 internal clock

	//bar/beat/sixteenth listeners
	clockInternal.addBeatListener(this);
	clockInternal.addBarListener(this);
	clockInternal.addSixteenthListener(this);

	clockInternal_Bpm.addListener(this, &ofxBeatClock::Changed_ClockInternal_Bpm);
	clockInternal_Bpm.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX);

	clockInternal_Active.addListener(this, &ofxBeatClock::Changed_ClockInternal_Active);
	clockInternal_Active.set("Active", false);

	clockInternal.setBpm(clockInternal_Bpm);

	//-

	params_INTERNAL.setName("INTERNAL CLOCK");

	// global play
	params_INTERNAL.add(PLAYING_State.set("PLAY", false));

	params_INTERNAL.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
	params_INTERNAL.add(bKeys);

	//TODO: should better gui-behavior-feel being a button not toggle

	//-

	//TODO:
	///trig resync to closest beat? (not so simple as to go to bar start)
	//params_INTERNAL.add(bSync_Trig.set("SYNC", false));

	//-

	//1.3 external midi
	params_EXTERNAL_MIDI.setName("EXTERNAL MIDI CLOCK");
	params_EXTERNAL_MIDI.add(PLAYING_External_State.set("PLAY SYNC", false));
	params_EXTERNAL_MIDI.add(midiIn_Port_SELECT.set("INPUT", 0, 0, midiIn_numPorts - 1));
	params_EXTERNAL_MIDI.add(midiIn_PortName);

	//-

	//1.4 ableton link

	//-

#ifdef USE_ofxAbletonLink
	LINK_setup();
#endif

	//-

#ifdef USE_ofxAbletonLink
	params_LINK.setName("ABLETON LINK");
	params_LINK.add(LINK_Enable.set("LINK", true));
	params_LINK.add(LINK_Peers_string.set("PEERS", " "));
	params_LINK.add(LINK_Beat_string.set("BEAT", ""));
	params_LINK.add(LINK_Play.set("PLAY LINK", false));
	params_LINK.add(LINK_Phase.set("PHASE", 0.f, 0.f, 4.f));
	params_LINK.add(LINK_Bpm.set("BPM LINK", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_LINK.add(LINK_ResyncBeat.set("RESYNC", false));
	params_LINK.add(LINK_ResetBeats.set("FORCE RESET", false));
	//params_LINK.add(LINK_Beat_Selector.set("GO BEAT", 0, 0, 100));//TODO: TEST

	//not required..
	LINK_Enable.setSerializable(false);
	LINK_Bpm.setSerializable(false);
	LINK_Play.setSerializable(false);
	LINK_Phase.setSerializable(false);
	LINK_Beat_string.setSerializable(false);
	LINK_ResyncBeat.setSerializable(false);
	LINK_ResetBeats.setSerializable(false);
	LINK_Peers_string.setSerializable(false);
	//LINK_Beat_Selector.setSerializable(false);

	//LINK_Play.makeReferenceTo(PLAYING_State);
#endif

	//--

	// 1.5 extra and advanced settings

	//this smoothed (or maybe slower refreshed than fps) clock will be sended to target sequencer outside the class. see BPM_MIDI_CLOCK_REFRESH_RATE.
	params_BPM_Clock.setName("Clock BPM");
	params_BPM_Clock.add(BPM_Global.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_BPM_Clock.add(BPM_Global_TimeBar.set("BAR ms", (int)60000 / BPM_Global, 100, 2000));
	params_BPM_Clock.add(RESET_BPM_Global);

	//added to group in other method (?)
	BPM_half_TRIG.set("BPM Half", false);
	BPM_double_TRIG.set("BPM Double", false);
	params_BPM_Clock.add(BPM_half_TRIG);
	params_BPM_Clock.add(BPM_double_TRIG);

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	MODE_AudioBufferTimer.set("MODE AUDIO BUFFER", false);
	params_BPM_Clock.add(MODE_AudioBufferTimer);
#endif

	params_BPM_Clock.add(ENABLE_sound.set("Tick Sound", false));
	params_BPM_Clock.add(volumeSound.set("Tick Volume", 0.5f, 0.f, 1.f));

	SHOW_Editor.set("Draw Box", true);
	MODE_Editor.set("Edit", false);

	//-

	//params_BPM_Clock.add(SHOW_Editor);
	//params_BPM_Clock.add(MODE_Editor);
	params_CONTROL.add(SHOW_Editor);
	params_CONTROL.add(MODE_Editor);

	//-

	//exclude
	midiIn_PortName.setSerializable(false);
	//PLAYING_External_State.setSerializable(false);
	RESET_BPM_Global.setSerializable(false);
	MODE_Editor.setSerializable(false);

	//----

	// default config. to be setted after with .setPosition_GuiPanel

	gui_Panel_Width = 200;
	gui_Panel_posX = 5;
	gui_Panel_posY = 5;

	//-

	//display text
	std::string strFont;

	//strFont = "assets/fonts/telegrama_render.otf";
	//fontSmall.load(strFont, 9);
	//fontMedium.load(strFont, 11);
	//fontBig.load(strFont, 15);

	int sz1, sz2, sz3;

	//strFont = "assets/fonts/PrgmtB.ttf";
	//sz1 = 10;
	//sz2 = 13;
	//sz3 = 17;

	//strFont = "assets/fonts/telegrama_render.otf";
	//sz1 = 7;
	//sz2 = 12;
	//sz3 = 17;

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

	//--

	//add already initiated params from transport and all controls to the gui panel
	//#ifdef USE_OFX_GUI_EXTENDED2
	setup_GuiPanel();
	//#endif

	//--

	// other settings only to storing
	params_App.setName("AppSettings");
	params_App.add(ENABLE_sound);

	params_App.add(MODE_INTERNAL_CLOCK);
	params_App.add(MODE_EXTERNAL_MIDI_CLOCK);
#ifdef USE_ofxAbletonLink
	params_App.add(MODE_ABLETON_LINK_SYNC);
#endif
	params_App.add(PLAYING_State);
	//params_App.add(PLAYING_External_State);
	//params_App.add(PLAYING_Global_State);

	params_App.add(volumeSound);
	params_App.add(pos_Gui);
	params_App.add(MODE_Editor);
	params_App.add(SHOW_Editor);
	params_App.add(bGui);
	params_App.add(bKeys);

	//TODO:
	//params_App.add(pos_Global);
	//params_App.add(pos_ClockInfo);
	//params_App.add(pos_BpmInfo);

	//--

	// GUI

#ifdef USE_OFX_SURFING_IM_GUI

	//guiManager.setAutoResize(true);
	guiManager.setAutoSaveSettings(true);
	guiManager.setImGuiAutodraw(true);
	guiManager.setup();

	//-

	// fonts
	std::string _fontName;
	float _fontSizeParam;
	_fontName = "telegrama_render.otf"; // WARNING: will crash if font not present!
	_fontSizeParam = 11;
	std::string _path = "assets/fonts/"; // assets folder

	guiManager.pushFont(_path + _fontName, 30); // is index 1. 0 is for default font
	guiManager.pushFont(_path + _fontName, 20);
	guiManager.pushFont(_path + _fontName, 14); // 3

	//--

	// customize widgets
	//if (0)
	{
		//guiManager.AddStyle(bMode3, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, true, 2);
		guiManager.AddStyle(clockInternal_Bpm, SurfingImGuiTypes::OFX_IM_STEPPER);
		guiManager.AddStyle(RESET_BPM_Global, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL);

		guiManager.AddStyle(ENABLE_CLOCKS, SurfingImGuiTypes::OFX_IM_BUTTON_BIG, false, 1, 0);
		guiManager.AddStyle(MODE_INTERNAL_CLOCK, SurfingImGuiTypes::OFX_IM_BUTTON_BIG, false, 1, 0);
		guiManager.AddStyle(MODE_EXTERNAL_MIDI_CLOCK, SurfingImGuiTypes::OFX_IM_BUTTON_BIG, false, 1, 0);
#ifdef USE_ofxAbletonLink
		guiManager.AddStyle(MODE_ABLETON_LINK_SYNC, SurfingImGuiTypes::OFX_IM_BUTTON_BIG, false, 1, 0);
#endif

		guiManager.AddStyle(PLAYING_State, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG, false, 1, 0);
		guiManager.AddStyle(BPM_Tap_Tempo_TRIG, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL, false, 1, 0);

		guiManager.AddStyle(bGui_PreviewClockNative, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1, 0);
		//guiManager.AddStyle(bGui_BpmClock, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 1, 0);

		guiManager.AddStyle(BPM_half_TRIG, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL, true, 2, 0);
		guiManager.AddStyle(BPM_double_TRIG, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL, false, 2, 0);

		//TODO:
		guiManager.AddStyle(SHOW_Editor, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, true, 2, 0);
		guiManager.AddStyle(MODE_Editor, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 2, 0);
		//guiManager.AddStyle(SHOW_Editor, SurfingImGuiTypes::OFX_IM_HIDDEN, true, 2, 0);
		//guiManager.AddStyle(MODE_Editor, SurfingImGuiTypes::OFX_IM_HIDDEN, false, 2, 0);
	}

	//-

#ifdef USE_ofxAbletonLink
	//if (0)
	{
		guiManager.AddStyle(LINK_Play, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
		guiManager.AddStyle(LINK_Enable, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
		guiManager.AddStyle(LINK_Peers_string, SurfingImGuiTypes::OFX_IM_TEXT_DISPLAY);
		guiManager.AddStyle(LINK_Beat_string, SurfingImGuiTypes::OFX_IM_TEXT_DISPLAY);
		guiManager.AddStyle(LINK_Phase, SurfingImGuiTypes::OFX_IM_INACTIVE);
		guiManager.AddStyle(LINK_Bpm, SurfingImGuiTypes::OFX_IM_INACTIVE);
		guiManager.AddStyle(LINK_ResyncBeat, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL);
		guiManager.AddStyle(LINK_ResetBeats, SurfingImGuiTypes::OFX_IM_BUTTON_SMALL);
	}
#endif

	//-

	// external midi
	//guiManager.AddStyle(PLAYING_External_State, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);

	//--

#endif

	//---

	// default gui panel position
	setPosition_GuiPanel(gui_Panel_posX, gui_Panel_posY, gui_Panel_Width);

	//--

	//default preview position
	setPosition_GuiPreviewWidget(5, 770);

	//-

	////TODO:
	////improve layout system with glm::vec2..? to allow another external addon,
	////to handle better layout management?
	//int sw, sh;
	//sw = 1920;
	//sh = 1080;
	//position_BeatBoxes.set("position_BeatBoxes", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));
	//position_BeatBall.set("position_BeatBall", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));
	//position_ClockInfo.set("position_ClockInfo", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));
	//position_BpmInfo.set("position_BpmInfo", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));

	//-

	//main text color white
	colorText = ofColor(255, 255);

	//---

	//draw beat ball trigger for sound and visual feedback monitor
	//this trigs to draw a flashing circle for a frame only
	BeatTick = false;

	//swap to a class..
	bpmTapTempo.setPathSounds("assets/sounds/");
	//bpmTapTempo.setPathSounds(path_Global + "sounds/");
	bpmTapTempo.setup();

	//-

	//text time to display
	Bar_string = "0";
	Beat_string = "0";
	Tick_16th_string = "0";

	//--

	//pattern limiting. (vs long song mode)
	//this only makes sense when syncing to external midi clock (playing a long song)
	//maybe not a poper solution yet..
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
	//load settings

	//folder to both (control and midi input port) settings files
	loadSettings(path_Global + "settings/");

	//-

	// edit layout

	////A. hardcoded init. comment to use settings file
	//rPreview.enableEdit();
	rPreview.setLockResize(true);
	rPreview.disableEdit();
	rPreview.setRect(225, 325, 35, 70);

	//B. load settings
	//rPreview.loadSettings(name_r1, name_r2, false);
	//rPreview.loadSettings();
	rPreview.loadSettings("", path_Global + "settings/", false);

	//setPosition_GuiPreviewWidget(rPreview.x + padx, rPreview.y + pady);// ?

	//-

	//TODO:
	//workaround
	//to ensure that gui workflow is updated after settings are loaded...
	//because sometimes initial gui state fails...
	//it seems that changed_params callback is not called after loading settings?
	refresh_Gui();

	//--

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	setupAudioBuffer(0);
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::setup_GuiPanel()
{
#ifdef USE_OFX_GUI_EXTENDED2
	//to customize some widgets
	confg_Sliders =
	{
		{"height", 40}
	};
	confg_Button_C =
	{
		{"type", "fullsize"},
		{"text-align", "center"},
		{"height", 40}
	};
	confg_Button_L =
	{
		{"type", "fullsize"},
		{"text-align", "left"},
		{"height", 40}
	};
	confg_ButtonSmall =
	{
		{"type", "fullsize"},
		{"text-align", "center"},
		{"height", 23}
	};

	//--

	//1. main panel
	panel_BeatClock = gui.addPanel("BEAT CLOCK");//main container
	//panel_BeatClock = gui.addGroup("BEAT CLOCK");//main container
	panel_BeatClock->setShowHeader(true);

	//1.1 control
	group_Controls = panel_BeatClock->addGroup(params_CONTROL);

	//1.2 internal
	group_INTERNAL = panel_BeatClock->addGroup("INTERNAL CLOCK");
	group_INTERNAL->add(params_INTERNAL);

	//1.3 external midi
	group_EXTERNAL_MIDI = panel_BeatClock->addGroup("EXTERNAL MIDI CLOCK");
	group_EXTERNAL_MIDI->add(params_EXTERNAL_MIDI);

	//1.4 link
#ifdef USE_ofxAbletonLink
	group_LINK = panel_BeatClock->addGroup("ABLETON LINK");
	group_LINK->add(params_LINK);
#endif

	//1.5 extra and advanced settings
	group_Advanced = panel_BeatClock->addGroup(params_BPM_Clock);

	//--

	// theme customize

	//1.1
	group_Controls->getControl("ENABLE")->setConfig(confg_Button_C);
	//group_Controls->getToggle("PLAY")->setConfig(confg_Button_C);//global play//TEST
	group_Controls->getControl("1 INTERNAL CLOCK")->setConfig(confg_Button_L);
	group_Controls->getControl("2 EXTERNAL MIDI CLOCK")->setConfig(confg_Button_L);
#ifdef USE_ofxAbletonLink
	group_Controls->getControl("3 ABLETON LINK")->setConfig(confg_Button_L);
#endif
	group_Controls->getFloatSlider("BPM")->setConfig(confg_Sliders);
	group_Controls->getFloatSlider("BPM")->setPrecision(2);

	//1.2
	group_INTERNAL->getToggle("PLAY")->setConfig(confg_Button_C);
	group_INTERNAL->getToggle("TAP")->setConfig(confg_Button_C);

	//1.3
	group_EXTERNAL_MIDI->getToggle("PLAY SYNC")->setConfig(confg_Button_C);
	group_EXTERNAL_MIDI->getControl("PLAY SYNC")->unregisterMouseEvents();

	//1.4
#ifdef USE_ofxAbletonLink
	group_LINK->getToggle("LINK")->setConfig(confg_ButtonSmall);
	group_LINK->getToggle("PLAY")->setConfig(confg_Button_C);
	group_LINK->getToggle("RESYNC")->setConfig(confg_ButtonSmall);
	group_LINK->getToggle("FORCE RESET")->setConfig(confg_ButtonSmall);
	group_LINK->getFloatSlider("PHASE")->setConfig(ofJson{
		{"precision", 1},
		{"height", 40},
		{"fill-color", "rgb(128,128,0)"}//yellow
		});
#endif

	//1.5
	group_Advanced->getFloatSlider("GLOBAL BPM")->setPrecision(2);
#endif

	//----

	// listeners
	ofAddListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_INTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_EXTERNAL_MIDI.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_BPM_Clock.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//--

#ifdef USE_OFX_GUI_EXTENDED2
	//theme
	path_Theme = "assets/theme/";
	path_Theme += "theme_ofxGuiExtended2_01.json";
	loadTheme(path_Theme);

	//--

	// expand/collapse pannels
	panel_BeatClock->maximize();
	group_Controls->maximize();
	group_Advanced->minimize();

	if (MODE_INTERNAL_CLOCK)
	{
		group_INTERNAL->maximize();
	}
	else
	{
		group_INTERNAL->minimize();
	}

	if (MODE_EXTERNAL_MIDI_CLOCK)
	{
		group_EXTERNAL_MIDI->maximize();
	}
	else
	{
		group_EXTERNAL_MIDI->minimize();
	}

#ifdef USE_ofxAbletonLink
	if (MODE_ABLETON_LINK_SYNC)
	{
		group_LINK->maximize();
	}
	else
	{
		group_LINK->minimize();
	}
#endif
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::refresh_Gui()
{
	//workflow

	if (MODE_INTERNAL_CLOCK)
	{
		MODE_EXTERNAL_MIDI_CLOCK = false;
#ifdef USE_ofxAbletonLink
		MODE_ABLETON_LINK_SYNC = false;
#endif
		//display text
		clockActive_Type = MODE_INTERNAL_CLOCK.getName();
		clockActive_Type += "\n";
		//clockActive_Type = "1 INTERNAL CLOCK";
		clockActive_Info = "";
	}

	//-

	else if (MODE_EXTERNAL_MIDI_CLOCK)
	{
		MODE_INTERNAL_CLOCK = false;

#ifdef USE_ofxAbletonLink
		MODE_ABLETON_LINK_SYNC = false;
#endif

		if (PLAYING_State) PLAYING_State = false;
		if (clockInternal_Active) clockInternal_Active = false;

		//display text
		clockActive_Type = MODE_EXTERNAL_MIDI_CLOCK.getName();
		//clockActive_Type = "2 EXTERNAL MIDI CLOCK";
		clockActive_Type += "\n";
		clockActive_Info = "PORT: ";
		clockActive_Info += "'" + midiIn.getName() + "'";
		//clockActive_Info += ofToString(midiIn.getPort());
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (MODE_ABLETON_LINK_SYNC)
	{
		MODE_INTERNAL_CLOCK = false;
		MODE_EXTERNAL_MIDI_CLOCK = false;

		if (PLAYING_State) PLAYING_State = false;
		if (clockInternal_Active) clockInternal_Active = false;

		//display text
		clockActive_Type = MODE_ABLETON_LINK_SYNC.getName();
		//clockActive_Type = "3 ABLETON LINK";
		clockActive_Type += "\n";
		clockActive_Info = "";
	}
#endif

	//-

#ifdef USE_OFX_GUI_EXTENDED2
	if (MODE_INTERNAL_CLOCK)
	{
		group_INTERNAL->maximize();
	}
	else
	{
		group_INTERNAL->minimize();
	}

	//-

	if (MODE_EXTERNAL_MIDI_CLOCK)
	{
		group_EXTERNAL_MIDI->maximize();
	}
	else
	{
		group_EXTERNAL_MIDI->minimize();
	}
#endif

	//-

#ifdef USE_ofxAbletonLink
#ifdef USE_OFX_GUI_EXTENDED2
	if (MODE_ABLETON_LINK_SYNC)
	{
		group_LINK->maximize();
	}
	else
	{
		group_LINK->minimize();
	}
#endif
#endif

	//--
}

//--------------------------------------------------------------
void ofxBeatClock::refresh_GuiWidgets()
{
	if (!bGui_PreviewClockNative) circleBeat.update();

	//-

	// text labels

	//strMessageInfoFull = "";

	//-

	strTapTempo = "";
	if (bpmTapTempo.isRunning())
	{
		strTapTempo = "     TAP " + ofToString(ofMap(bpmTapTempo.getCountTap(), 1, 4, 3, 0));
		//strMessageInfoFull += strTapTempo + "\n";
	}
	else
	{
		strTapTempo = "";
		//strMessageInfoFull += strTapTempo + "\n";
	}

	//-

	// A. external midi clock
	// midi in port. id number and name
	if (MODE_EXTERNAL_MIDI_CLOCK)
	{
		strExtMidiClock = ofToString(clockActive_Info);
		//strMessageInfoFull += strExtMidiClock + "\n";
	}

	//-

#ifdef USE_ofxAbletonLink
	// C. Ableton Link
	else if (MODE_ABLETON_LINK_SYNC)
	{
		strLink = ofToString(clockActive_Info);
		//strMessageInfoFull += strLink + "\n";
	}
#endif	

	//--

	// 1.4 more debug info
	if (DEBUG_moreInfo)
	{
		strDebugInfo = "";
		strDebugInfo += ("BPM: " + ofToString(BPM_Global, 2));
		strDebugInfo += "\n";
		strDebugInfo += ("BAR: " + Bar_string);
		strDebugInfo += "\n";
		strDebugInfo += ("BEAT: " + Beat_string);
		strDebugInfo += "\n";
		strDebugInfo += ("Tick 16th: " + Tick_16th_string);
		strDebugInfo += "\n";
		//strMessageInfoFull = strDebugInfo + "\n";
	}
	else { strDebugInfo = ""; }

	//-

	// show clock source (internal, external or link)
	//strClock = clockActive_Type;
	//strClock = "CLOCK  " + clockActive_Type;
	//strMessageInfoFull += strClock + "\n";

	//-

	// time pos
	strTimeBeatPos = ofToString(Bar_string, 3, ' ') + " : " + ofToString(Beat_string);

	//only internal clock mode gets 16th ticks (beat / 16 duration)
	//midi sync and link modes do not gets the 16th ticks, so we hide the (last) number to avoid confusion..

#ifndef USE_ofxAbletonLink
	if (!MODE_EXTERNAL_MIDI_CLOCK)
#else
	if (!MODE_EXTERNAL_MIDI_CLOCK && !MODE_ABLETON_LINK_SYNC)
#endif
	{
		strTimeBeatPos += " : " + ofToString(Tick_16th_string);
	}

	//-

	strBpmInfo = "BPM: " + ofToString(BPM_Global.get(), 2);

	//------

	// colors
	int colorBoxes = 64; // grey
	int colorGreyDark = 32;
	//int colorBoxes = 192; // grey
	int alphaBoxes = 164; // alpha of several above draw fills

	//--

	// 2. BEATS [4] SQUARES

	for (int i = 0; i < 4; i++)
	{
		// define squares colors:
#ifdef USE_ofxAbletonLink
		if (ENABLE_CLOCKS && (MODE_EXTERNAL_MIDI_CLOCK || MODE_INTERNAL_CLOCK || MODE_ABLETON_LINK_SYNC))
#else
		if (ENABLE_CLOCKS && (MODE_EXTERNAL_MIDI_CLOCK || MODE_INTERNAL_CLOCK))
#endif
		{
			// ligther color (colorBoxes) for beat squares that "has been passed [left]"
			if (i <= (Beat_current - 1))
			{
#ifdef USE_ofxAbletonLink
				//TEST:
				if (MODE_ABLETON_LINK_SYNC && LINK_Enable)
				{
					cb[i] = (LINK_Play) ? ofColor(colorBoxes, alphaBoxes) : ofColor(colorGreyDark, alphaBoxes);
				}
				else if (MODE_INTERNAL_CLOCK)
				{
					cb[i] = (PLAYING_State) ? ofColor(colorBoxes, alphaBoxes) : ofColor(colorGreyDark, alphaBoxes);
				}

#else
				if (MODE_INTERNAL_CLOCK)
				{
					cb[i] = (PLAYING_State) ? ofColor(colorBoxes, alphaBoxes) : ofColor(colorGreyDark, alphaBoxes);
				}
#endif
				else if (MODE_EXTERNAL_MIDI_CLOCK)
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
	//external midi clock

	ofLogNotice(__FUNCTION__);

	ofLogNotice(__FUNCTION__) << "LIST PORTS:";
	midiIn.listInPorts();

	midiIn_numPorts = midiIn.getNumInPorts();
	ofLogNotice(__FUNCTION__) << "NUM MIDI-IN PORTS:" << midiIn_numPorts;

	midiIn_Clock_Port_OPENED = 0;
	midiIn.openPort(midiIn_Clock_Port_OPENED);

	ofLogNotice(__FUNCTION__) << "connected to MIDI CLOCK IN port: " << midiIn.getPort();

	midiIn.ignoreTypes(
		true, //sysex  <-- ignore timecode messages!
		false, //timing <-- don't ignore clock messages!
		true); //sensing

	////TEST:
	////this can be used to add support to timecode sync too, if desired!
	//midiIn.ignoreTypes(
	//	false, // sysex  <-- don't ignore timecode messages!
	//	false, // timing <-- don't ignore clock messages!
	//	true); // sensing

	midiIn.addListener(this);

	bMidiInClockRunning = false; //< is the clock sync running?
	MIDI_beats = 0; //< song pos in beats
	MIDI_seconds = 0; //< song pos in seconds, computed from beats
	midiIn_Clock_Bpm = (double)BPM_INIT; //< song tempo in bpm, computed from clock length

	//-

	midiIn_BeatsInBar.addListener(this, &ofxBeatClock::Changed_midiIn_BeatsInBar);

	//-
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MidiIn_Port(int p)
{
	ofLogNotice(__FUNCTION__) << "setup_MidiIn_Port: " << p;

	midiIn.closePort();
	midiIn_Clock_Port_OPENED = p;
	midiIn.openPort(midiIn_Clock_Port_OPENED);
	ofLogNotice(__FUNCTION__) << "PORT NAME: " << midiIn.getInPortName(p);

	//display text
	clockActive_Type = MODE_EXTERNAL_MIDI_CLOCK.getName();
	clockActive_Info = "PORT: ";
	clockActive_Info += "'" + midiIn.getName() + "'";
	//clockActive_Info += ofToString(midiIn.getPort());

	strClock = clockActive_Type;

	ofLogNotice(__FUNCTION__) << "Connected to MIDI IN CLOCK Port: " << midiIn.getPort();
}

//--------------------------------------------------------------
void ofxBeatClock::update(ofEventArgs & args)
{
	refresh_GuiWidgets();

	//--

	// tap engine
	tap_Update();

	//--

#ifdef USE_ofxAbletonLink
	if (MODE_ABLETON_LINK_SYNC) LINK_update();
#endif

	//--
}

//--------------------------------------------------------------
void ofxBeatClock::draw_PreviewWidget()
{
	ofPushMatrix();

	// rectangle editor
	if (SHOW_Editor)
	{
		ofPushStyle();
		ofFill();
		ofSetColor(ofColor(0, 220));
		rPreview.draw();
		ofDrawRectRounded(rPreview, 5);
		ofPopStyle();
	}

	rPreview.setWidth(shapePreview.x);
	rPreview.setHeight(shapePreview.y);

	// get clicker position from being edited rectangle
	//if (MODE_Editor.get())
	{
		setPosition_GuiPreviewWidget(rPreview.x + padx, rPreview.y + pady);
		// sets all below variables...
	}

	//-

	draw_PreviewWidgetItems();

	ofPopMatrix();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_PreviewWidgetItems()
{
	draw_BpmInfo(pos_BpmInfo.get().x, pos_BpmInfo.get().y);
	draw_ClockInfo(pos_ClockInfo.get().x, pos_ClockInfo.get().y);
	draw_BeatBoxes(pos_BeatBoxes_x, pos_BeatBoxes_y, pos_BeatBoxes_width);
	draw_BeatBall(pos_BeatBall_x, pos_BeatBall_y, pos_BeatBall_radius);
}

#ifdef USE_OFX_SURFING_IM_GUI
//--------------------------------------------------------------
void ofxBeatClock::draw_ImGuiControl()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
	if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

	guiManager.beginWindow(bGui.getName().c_str(), (bool*)&bGui.get(), window_flags);
	{
		// Precalculate common widgets sizes to fit current window, "to be responsive".
		float _w1 = ofxImGuiSurfing::getWidgetsWidth(1); // 1 widget full width
		float _w2 = ofxImGuiSurfing::getWidgetsWidth(2); // 2 widgets half width
		float _w3 = ofxImGuiSurfing::getWidgetsWidth(3); // 3 widgets third width
		float _w4 = ofxImGuiSurfing::getWidgetsWidth(4); // 4 widgets quarter width
		float _h = ofxImGuiSurfing::getWidgetsHeightRelative(); // one unit height relative to ImGui theme
		guiManager.refresh();

		//ofxImGuiSurfing::AddToggleRoundedButton(bGui);
		ofxImGuiSurfing::AddToggleRoundedButton(bGui_Transport);
		//ofxImGuiSurfing::AddToggleRoundedButton(bGui_PreviewClockNative);//TODO: remove all
		ofxImGuiSurfing::AddToggleRoundedButton(bGui_BpmClock);

		guiManager.Add(ENABLE_CLOCKS, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);

		//TODO:
		//hide all
		if (!ENABLE_CLOCKS)
		{
			guiManager.endWindow();
			return;
		}

		//----

		// play 
		const int hUnits = 4;
		//guiManager.Add(PLAYING_State, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
		if (MODE_ABLETON_LINK_SYNC) AddBigToggleNamed(PLAYING_State, _w1, hUnits * _h, "PLAYING", "PLAY", true, link.getPhase());
		else AddBigToggleNamed(PLAYING_State, _w1, hUnits * _h, "PLAYING", "PLAY", true, 1.f);

		ImGui::Dummy(ImVec2(0, 2));

		//--

		// sources

		{
			static bool bOpen = false;
			ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
			_flagt |= ImGuiTreeNodeFlags_Framed;
			if (ImGui::TreeNodeEx("SOURCES", _flagt))
			{
				guiManager.refresh();

				guiManager.Add(MODE_INTERNAL_CLOCK, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
				guiManager.Add(MODE_EXTERNAL_MIDI_CLOCK, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
#ifdef USE_ofxAbletonLink
				guiManager.Add(MODE_ABLETON_LINK_SYNC, SurfingImGuiTypes::OFX_IM_TOGGLE_BIG);
#endif
				ImGui::TreePop();
			}
			guiManager.refresh();
		}

		ImGui::Dummy(ImVec2(0, 2));

		//--

		// modes
		ImGui::Indent();
		{
			if (MODE_INTERNAL_CLOCK)
			{
				//ImGuiTreeNodeFlags flags;
				//flags = ImGuiTreeNodeFlags_None;
				//flags |= ImGuiTreeNodeFlags_Framed; // uncomment to draw dark tittle bar
				////flags |= ImGuiTreeNodeFlags_DefaultOpen; // comment to start closed

				static bool bOpen = true;
				ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt |= ImGuiTreeNodeFlags_Framed;
				if (ImGui::TreeNodeEx("INTERNAL", _flagt))
				{
					guiManager.refresh();

					guiManager.Add(BPM_Tap_Tempo_TRIG, SurfingImGuiTypes::OFX_IM_BUTTON_BIG);
					//ofxImGuiSurfing::AddBigButton(BPM_Tap_Tempo_TRIG);
					//guiManager.AddGroup(params_INTERNAL, flags, SurfingImGuiTypesGroups::OFX_IM_GROUP_COLLAPSED);

					ImGui::TreePop();
				}
				guiManager.refresh();

				ImGui::Dummy(ImVec2(0, 2));
			}

			//-

			if (MODE_EXTERNAL_MIDI_CLOCK)
			{
				static bool bOpen = true;
				ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				_flagt |= ImGuiTreeNodeFlags_Framed;

				if (ImGui::TreeNodeEx("EXTERNAL", _flagt))
				{
					guiManager.refresh();
					{
						guiManager.AddStyle(PLAYING_External_State, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL);
						guiManager.Add(midiIn_Port_SELECT, SurfingImGuiTypes::OFX_IM_STEPPER);
						guiManager.Add(midiIn_PortName, SurfingImGuiTypes::OFX_IM_TEXT_DISPLAY);

						//guiManager.AddGroup(params_EXTERNAL_MIDI, flags, SurfingImGuiTypesGroups::OFX_IM_GROUP_COLLAPSED);
					}
					ImGui::TreePop();
				}
				guiManager.refresh();

				ImGui::Dummy(ImVec2(0, 2));
			}

			//-

			// link

#ifdef USE_ofxAbletonLink
			if (MODE_ABLETON_LINK_SYNC)
			{
				guiManager.AddGroup(params_LINK);

				//static bool bOpen = true;
				//ImGuiTreeNodeFlags _flagt = (bOpen ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None);
				//_flagt |= ImGuiTreeNodeFlags_Framed;
				//if (ImGui::TreeNodeEx("ABLETON LINK", _flagt))
				//{
				//	guiManager.refresh();
				//	guiManager.AddGroup(params_LINK);
				//	ImGui::TreePop();
				//}
				//guiManager.refresh();

				ImGui::Dummy(ImVec2(0, 2));
			}
#endif
		}
		ImGui::Unindent();

		//--

		static bool bExtra = false;
		ImGui::Dummy(ImVec2(0, 5));
		ofxImGuiSurfing::ToggleRoundedButton("Extra", &bExtra);//TODO: remove all+
		if (bExtra) {
			ImGui::Indent();
			ofxImGuiSurfing::AddToggleRoundedButton(bGui_PreviewClockNative);//TODO: remove all+
			if (bGui_PreviewClockNative) {
				ImGui::Indent();
				ofxImGuiSurfing::AddToggleRoundedButton(SHOW_Editor);
				ofxImGuiSurfing::AddToggleRoundedButton(MODE_Editor);
				ImGui::Unindent();
			}
			ofxImGuiSurfing::AddToggleRoundedButton(bKeys);
			ofxImGuiSurfing::AddToggleRoundedButton(guiManager.bAutoResize);
			ImGui::Unindent();
		}
	}
	guiManager.endWindow();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGuiBpmClock()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
	if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

	guiManager.beginWindow(bGui_BpmClock.getName().c_str(), (bool*)&bGui_BpmClock.get(), window_flags);
	{
		// TODO: '-' fails
		//ImGui::PushID("##Stepper");
		//guiManager.Add(BPM_Global, SurfingImGuiTypes::OFX_IM_STEPPER);
		//ImGui::PopID();

		guiManager.AddGroup(params_BPM_Clock, ImGuiTreeNodeFlags_DefaultOpen, OFX_IM_GROUP_HIDDEN_HEADER);

		//ImGuiTreeNodeFlags flags;
		//flags = ImGuiTreeNodeFlags_None;
		//guiManager.AddGroup(params_BPM_Clock, flags, SurfingImGuiTypesGroups::OFX_IM_GROUP_COLLAPSED);

		//ImGui::Dummy(ImVec2(0, 2));
	}
	guiManager.endWindow();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGuiCircleBeatWidget()
{
	// circle widget
	{
		ofColor colorBeat = ofColor(ofColor::red, 200);
		ofColor colorTick = ofColor(128, 200);
		ofColor colorBallTap = ofColor(16, 200);
		ofColor colorBallTap2 = ofColor(96);

		//---

		float pad = 10;
		float __w100 = ImGui::GetContentRegionAvail().x - 2 * pad;

		float radius = 30;
		//float radius = __w100 / 2; // *circleBeat.getRadius();

		//ImGui::Text("BEAT");
		const char* label = " ";
		//const char* label = "BEAT";

		float radius_inner = radius * circleBeat.getValue();
		//float radius_inner = radius * ofxSurfingHelpers::getFadeBlink();
		float radius_outer = radius;
		//float spcx = radius * 0.1;

		//const int nsegm = 4;
		const int nsegm = 24; // big resgments outperforms..

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

		//// draw label

		//float texPos = pos.x + ((widgetRec.z - labelLength.x) * 0.5f);
		//draw_list->AddText(ImVec2(texPos, pos.y + space_height), ImGui::GetColorU32(ImGuiCol_Text), label);

		//-

		ofColor cbg;

		// background black ball
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

		// outer circle

		draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImVec4(cbg)), nsegm);
		//draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : ImGuiCol_FrameBg), nsegm);
		//draw_list->AddCircleFilled(center, radius_outer * 0.8, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : ImGuiCol_FrameBg), nsegm);

		//-

		// inner circle

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
	}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGuiClockMonitor()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
	if (1) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	//if (guiManager.bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::SetNextWindowPos(ImVec2(250, 10), ImGuiCond_FirstUseEver);

	guiManager.beginWindow(bGui_Transport.getName().c_str(), (bool*)&bGui_Transport.get(), window_flags);
	{
		float __w100 = ImGui::GetContentRegionAvail().x;
		float _w4 = __w100 / 4;
		float _h = _w4;

		// 1. bpm + counters
		guiManager.pushStyleFont(1);
		ImGui::TextWrapped(strBpmInfo.c_str());
		ImGui::TextWrapped(strTimeBeatPos.c_str());
		guiManager.popStyleFont();

		ImGui::Dummy(ImVec2(0, 5));

		// 2. input info
		guiManager.pushStyleFont(3);
		ImGui::TextWrapped(strClock.c_str());
		ImGui::Dummy(ImVec2(0, 2));
		ImGui::TextWrapped(clockActive_Info.c_str());
		guiManager.popStyleFont();

		//ImGui::TextWrapped(strExtMidiClock.c_str());
		//ImGui::TextWrapped(strLink.c_str());

		ImGui::Dummy(ImVec2(0, 5));

		// 3. debug
		if (strDebugInfo != "") {
			guiManager.pushStyleFont(2);
			ImGui::TextWrapped(strDebugInfo.c_str());
			guiManager.popStyleFont();
		}

		// 4. Tap
		int speed = 10;//blink speed when measuring taps
		bool b = (ofGetFrameNum() % (2 * speed) < speed);
		ImVec4 c = ImVec4(1.f, 1.f, 1.f, b ? 0.5f : 1.f);
		guiManager.pushStyleFont(2);
		ImGui::PushStyleColor(ImGuiCol_Text, c);
		ImGui::TextWrapped(strTapTempo.c_str());
		ImGui::PopStyleColor();
		guiManager.popStyleFont();

		//ImGui::TextWrapped(strMessageInfoFull.c_str());

		ImGui::Dummy(ImVec2(0, 5));

		//--

		//guiManager.pushStyleFont(0);

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

		ImGui::Dummy(ImVec2(0, 2));

		// circle beat
		draw_ImGuiCircleBeatWidget();

		//guiManager.popStyleFont();

		//--

		guiManager.setDefaultFont();
		//guiManager.setDefaultFontIndex(0);
	}
	guiManager.endWindow();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ImGuiWidgets()
{
	guiManager.begin();
	{
		//guiManager.setDefaultFont();

		if (bGui) draw_ImGuiControl();
		if (bGui_Transport) draw_ImGuiClockMonitor();
		if (bGui_BpmClock) draw_ImGuiBpmClock();
	}
	guiManager.end();
}
#endif

//--------------------------------------------------------------
void ofxBeatClock::draw(ofEventArgs & args)
{
	//TODO: maybe could improve performance with fbo drawings for all BeatBoxes/text/ball?

	if (!MODE_INTERNAL_CLOCK && !MODE_EXTERNAL_MIDI_CLOCK && !MODE_ABLETON_LINK_SYNC)
	{
		clockActive_Info = "";
		strClock = "";
		clockActive_Type = "";
	}

	if (bGui_PreviewClockNative)
	{
		draw_PreviewWidget();

		//-

#ifdef USE_ofxAbletonLink
		if (MODE_ABLETON_LINK_SYNC && DEBUG_moreInfo)
		{
			LINK_draw();
		}
#endif
	}

	//-

#ifdef USE_OFX_SURFING_IM_GUI
	if (bGui) draw_ImGuiWidgets();
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BeatBoxes(int px, int py, int w)///draws text info and boxes
{
	ofPushStyle();

	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	//sizes and paddings
	int squaresW;//squares beats size
	squaresW = w / 4;//width of each square box
	//int xPad = 5;//x margin
	int interline = 15; //text line heigh
	int i = 0;//vertical spacer to accumulate text lines

	int alphaBoxes = 200; // alpha of several above draw fills

	//----

	// 2. BEATS [4] SQUARES

	for (int i = 0; i < 4; i++)
	{
		ofPushStyle();

		ofSetColor(cb[i]);

		//--

		// draw each i square

		//filled rectangle
		////optional:
		////adds alpha faded 16th blink, hearth-pulse mode for visual feedback only
		//if (i == Beat_current - 1 && Tick_16th_current % 2 == 0)
		//{
		//	ofSetColor(colorBoxes, alphaBoxes - 64);
		//}
		ofFill();
		ofDrawRectRounded(px + i * squaresW, py, squaresW, squaresW, 3.0f);

		// rectangle with border only
		ofNoFill();
		ofSetColor(8, alphaBoxes);
		ofSetLineWidth(2.0f);
		ofDrawRectRounded(px + i * squaresW, py, squaresW, squaresW, 3.0f);

		//--

		ofPopStyle();

		if (i == 3)
		{
			shapePreview.x = (4 * squaresW) + 2 * padx;
		}
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ClockInfo(int px, int py)
{
	ofPushStyle();

	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	// this method draws: tap debug, clock source, clock info, bar-beat-tick16th clock 
	int xPad = 5;
	int interline = 15; //text line heigh
	int i = 0;//vertical spacer to accumulate text lines
	int h = fontBig.getSize();//font spacer to respect px, py anchor

	//--

	// clock info:

	ofSetColor(colorText);

	//-

	ofPushMatrix();
	{
		ofTranslate(px, py);

		// 1.2. tap mode

		int speed = 10;//blink speed when measuring taps
		bool b = (ofGetFrameNum() % (2 * speed) < speed);
		ofSetColor(255, b ? 128 : 255);

		fontBig.drawString(strTapTempo, xPad, h + interline * i++);

		i++; // one extra line vertical spacer
		i++;

		//int speed = 10;//blink speed when measuring taps
		//bool b = (ofGetFrameNum() % (2 * speed) < speed);
		//ofSetColor(255, b ? 128 : 255);
		//if (bTap_Running)
		//{
		//	strMessageInfo = "     TAP " + ofToString(ofMap(tap_Count, 1, 4, 3, 0));
		//}
		//else
		//{
		//	strMessageInfo = "";
		//}
		//fontBig.drawString(strMessageInfo, xPad, h + interline * i++);

		//i++;//one extra line vertical spacer
		//i++;

		//-

		ofSetColor(colorText);

		// 1.3. clock

		fontSmall.drawString(strClock, xPad, interline * i++);

		//-

		// A. external midi clock
		// midi in port. id number and name
		if (MODE_EXTERNAL_MIDI_CLOCK)
		{
			fontSmall.drawString(strExtMidiClock, xPad, interline * i++);
		}

		//-

		// B. internal clock
		else if (MODE_INTERNAL_CLOCK)
		{
			i++;//empty line
		}

		//-

#ifdef USE_ofxAbletonLink
		// C. Ableton Link
		else if (MODE_ABLETON_LINK_SYNC)
		{
			fontSmall.drawString(strLink, xPad, interline * i++);
		}
#endif
		//-

		else // space in case no clock source is selected to mantain layout "static" below
		{
			i++;
		}

		//-

		// 1.5 main big clock: [bar:beat:tick16th]
		//TODO: could split this and allow to draw independently
		ofSetColor(colorText);

		int yPad = 30;

		i = i + 2;//spacer

		draw_BigClockTime(xPad, yPad + interline * i);

	}
	ofPopMatrix();

	//--

	// 1.4 more debug info
	if (DEBUG_moreInfo)
	{
		int xpos = 240;
		int ypos = 750;
		fontMedium.drawString(strDebugInfo, xpos, ypos);
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BpmInfo(int px, int py)
{
	ofPushStyle();

	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	int xPad = 5;
	int h = fontBig.getSize();

	ofSetColor(colorText);

	fontBig.drawString(strBpmInfo, px + xPad, py + h);

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BeatBall(int px, int py, int _radius)
{
	ofPushStyle();

	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	//-

	// tick ball
	metronome_ball_radius = _radius;
	py += metronome_ball_radius;
	metronome_ball_pos.x = px + metronome_ball_radius;
	metronome_ball_pos.y = py;

	// beat circle
	circleBeat.setPosition(glm::vec2(metronome_ball_pos.x, metronome_ball_pos.y));
	circleBeat.setRadius(metronome_ball_radius);

	// highlight 1st beat
	ofColor c;
	if (Beat_current == 1) c = (ofColor::red);
	else c = (ofColor::white);
	circleBeat.setColor(c);

	//-

	// background black ball
	if (!bpmTapTempo.isRunning())
	{
		// ball background when not tapping
		circleBeat.setColorBackground(ofColor(16, 200));
	}
	else
	{
		// white alpha fade when measuring tapping
		float t = (ofGetElapsedTimeMillis() % 1000);
		float fade = sin(ofMap(t, 0, 1000, 0, 2 * PI));
		ofLogVerbose(__FUNCTION__) << "fade: " << fade << endl;
		int alpha = (int)ofMap(fade, -1.0f, 1.0f, 0, 50) + 205;
		circleBeat.setColorBackground(ofColor(96, alpha));
	}

	//-

	// beat circle
	circleBeat.update();
	circleBeat.draw();

	//-

	//#else//TODO:
//	//not doing the candy-fading-out, but it seems that do not improve fps much neither...
//	if (!bTap_Running)
//	{
//		ofPushStyle();
//
//		circlePos.set(metronome_ball_pos.x, metronome_ball_pos.y);
//
//		ofFill();
//		ofSetColor(c, alphaMax);
//		ofDrawCircle(circlePos, radius);
//
//		//border circle
//		ofNoFill();
//		ofSetLineWidth(2.0f);
//		ofSetColor(255, alphaMax * 0.5f);
//		ofDrawCircle(circlePos, radius);
//		ofPopStyle();
//}
//#endif

	//-

//	//first beat circle of bar is drawed red. other ones are white
//	//this circle flahses will not be drawed when measuring tap tempo engine
//
//	if (ENABLE_CLOCKS && !bTap_Running &&
//#ifndef USE_ofxAbletonLink
//	(MODE_EXTERNAL_MIDI_CLOCK || MODE_INTERNAL_CLOCK))
//#else
//		(MODE_EXTERNAL_MIDI_CLOCK || MODE_INTERNAL_CLOCK || MODE_ABLETON_LINK_SYNC))
//#endif
//	{
//		//this trigs to draw a flashing circle for a frame only
//		if (BeatTick)
//		{
//			BeatTick = false;
//
//			//TODO: this could be the problem of confusing when beat 1/2 are
//			//highlight 1st beat
//			//if (Beat_current == 1)
//			if (lastBeatFlash == 1) ofSetColor(ofColor::red);
//			else ofSetColor(ofColor::white);
//
//			ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
//		}
//	}

	//-

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BigClockTime(int x, int y)
{
	ofPushStyle();

	ofSetColor(colorText);

	int xpad = 12;

	fontBig.drawString(strTimeBeatPos, x + xpad, y);

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_GuiGlobal(int x, int y)
{
	setPosition_GuiPanel(x, y, 200);
	setPosition_GuiPreviewWidget(x, y + 750);
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_GuiPanel(int _x, int _y, int _w)
{
#ifdef USE_OFX_GUI_EXTENDED2
	gui_Panel_posX = _x;
	gui_Panel_posY = _y;
	gui_Panel_Width = _w;

	panel_BeatClock->setPosition(ofPoint(gui_Panel_posX, gui_Panel_posY));

	panel_BeatClock->setWidth(gui_Panel_Width);
	group_INTERNAL->setWidth(gui_Panel_Width);
	group_Controls->setWidth(gui_Panel_Width);
	group_Advanced->setWidth(gui_Panel_Width);
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_GuiPreviewWidget(int x, int y)
{
	// pos global is the main anchor
	// to reference all other gui elements

	pos_Global = glm::vec2(x, y);

	//-

	// bpm big text info
	pos_BpmInfo = glm::vec2(pos_Global.get().x, pos_Global.get().y);

	// clock info
	int _pad1 = 25; // spacing
	pos_ClockInfo = glm::vec2(pos_Global.get().x, pos_BpmInfo.get().y + _pad1);

	// beat boxes
	pos_BeatBoxes_width = 190;
	pos_BeatBoxes_x = pos_Global.get().x;
	int _pad2 = 155;
	pos_BeatBoxes_y = pos_ClockInfo.get().y + _pad2;

	// beat ball
	pos_BeatBall_radius = 30;
	int _pad3 = 62;
	pos_BeatBall_x = pos_Global.get().x + pos_BeatBoxes_width * 0.5f - pos_BeatBall_radius;
	pos_BeatBall_y = pos_BeatBoxes_y + _pad3;

	//-

	shapePreview.y = pos_BeatBall_y + 2 * circleBeat.getRadius() - y + 2.0 * pady;
}

//--------------------------------------------------------------
ofPoint ofxBeatClock::getPosition_GuiPanel()
{
	ofPoint p;
	//p = panel_BeatClock->getShape().getTopLeft();
	p = ofPoint(gui_Panel_posX, gui_Panel_posY);
	return p;
}

////--------------------------------------------------------------
//glm::vec2 ofxBeatClock::getPosition_GuiPanel()
//{
//	glm::vec2 p = glm::vec2(gui_Panel_posX, gui_Panel_posY);
//	return p;
//}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_BeatBoxes(int x, int y, int w)
{
	pos_BeatBoxes_x = x;
	pos_BeatBoxes_y = y;
	pos_BeatBoxes_width = w;
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_BeatBall(int x, int y, int w)
{
	pos_BeatBall_x = x;
	pos_BeatBall_y = y;
	pos_BeatBall_radius = w;
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_ClockInfo(int x, int y)
{
	pos_ClockInfo = glm::vec2(x, y);
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_BpmInfo(int x, int y)
{
	pos_BpmInfo = glm::vec2(x, y);
}

//--------------------------------------------------------------
void ofxBeatClock::setVisible_GuiPreview(bool b)
{
	bGui_PreviewClockNative = b;
}

//--------------------------------------------------------------
void ofxBeatClock::setVisible_GuiPanel(bool b)
{
#ifdef USE_OFX_GUI_EXTENDED2
	gui.getVisible().set(b);
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::exit()
{
	ofLogNotice(__FUNCTION__);

	//gui layout
	rPreview.disableEdit();
	//rPreview.saveSettings(name_r1, path_Global + name_r2, false);
	//rPreview.saveSettings("", "", false);
	rPreview.saveSettings("", path_Global + "settings/", false);

	//-

	// workflow: force
	////default desired settings when it will opened
	//PLAYING_State = false;

	//#ifdef USE_ofxAbletonLink
	//	if (!MODE_INTERNAL_CLOCK && !MODE_EXTERNAL_MIDI_CLOCK && !MODE_ABLETON_LINK_SYNC)
	//#else
	//	if (!MODE_INTERNAL_CLOCK && !MODE_EXTERNAL_MIDI_CLOCK)
	//#endif
	//		MODE_INTERNAL_CLOCK = true;//force to enable one mode, this mode by default

	//-

	saveSettings(path_Global + "settings/");

	//--

	//internal clock

	//bar/beat/sixteenth listeners
	clockInternal.removeBeatListener(this);
	clockInternal.removeBarListener(this);
	clockInternal.removeSixteenthListener(this);

	clockInternal_Bpm.removeListener(this, &ofxBeatClock::Changed_ClockInternal_Bpm);
	clockInternal_Active.removeListener(this, &ofxBeatClock::Changed_ClockInternal_Active);

	//--

	//external midi

	midiIn.closePort();
	midiIn.removeListener(this);
	midiIn_BeatsInBar.removeListener(this, &ofxBeatClock::Changed_midiIn_BeatsInBar);

	//--

	//link
#ifdef USE_ofxAbletonLink
	LINK_exit();
#endif

	//--

	//remove other listeners

	ofRemoveListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofRemoveListener(params_BPM_Clock.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//--
}

//--------------------------------------------------------
void ofxBeatClock::start()//only used in internal and link modes
{
	ofLogNotice(__FUNCTION__);

	if (MODE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice(__FUNCTION__) << "START (internal clock)";

		if (!PLAYING_State) PLAYING_State = true;
		if (!clockInternal_Active) clockInternal_Active = true;

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
	else if (MODE_ABLETON_LINK_SYNC && ENABLE_CLOCKS)
	{
		ofLogNotice(__FUNCTION__) << "LINK_Play";
		LINK_Play = true;
	}
#endif
	else
	{
		ofLogNotice(__FUNCTION__) << "skip start";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::stop()//only used in internal mode
{
	ofLogNotice(__FUNCTION__);

	if (MODE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice(__FUNCTION__) << "STOP";

		if (PLAYING_State) PLAYING_State = false;
		if (clockInternal_Active) clockInternal_Active = false;

		//bIsPlaying = false;

		reset_ClockValues();//set gui display text clock to 0:0:0
	}
#ifdef USE_ofxAbletonLink
	else if (MODE_ABLETON_LINK_SYNC && ENABLE_CLOCKS)
	{
		LINK_Play = false;
		ofLogNotice(__FUNCTION__) << "LINK_Play: " << LINK_Play;

		reset_ClockValues();//set gui display text clock to 0:0:0
	}
#endif
	else
	{
		ofLogNotice(__FUNCTION__) << "skip stop";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::setTogglePlay()//only used in internal mode
{
	ofLogNotice(__FUNCTION__);

	//worklow
#ifdef USE_ofxAbletonLink
	if (!MODE_EXTERNAL_MIDI_CLOCK && !MODE_ABLETON_LINK_SYNC && !MODE_INTERNAL_CLOCK)
		MODE_INTERNAL_CLOCK = true;
#else	
	if (!MODE_EXTERNAL_MIDI_CLOCK && !MODE_INTERNAL_CLOCK)
		MODE_INTERNAL_CLOCK = true;
#endif

	if (MODE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		PLAYING_State = !PLAYING_State;
		//bIsPlaying = PLAYING_State;

		ofLogNotice(__FUNCTION__) << "PLAY internal: " << PLAYING_State;
	}
#ifdef USE_ofxAbletonLink
	else if (MODE_ABLETON_LINK_SYNC && ENABLE_CLOCKS)
	{
		LINK_Play = !LINK_Play;
		ofLogNotice(__FUNCTION__) << "LINK_Play: " << LINK_Play;
	}
#endif
	else
	{
		ofLogNotice(__FUNCTION__) << "Skip setTogglePlay";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::beatTick_MONITOR(int _beat)
{
	// trigs ball drawing and sound ticker
	ofLogVerbose(__FUNCTION__) << "> BeatTick : " << Beat_string;

#ifdef USE_ofxAbletonLink
	if (ENABLE_CLOCKS && (MODE_INTERNAL_CLOCK || MODE_EXTERNAL_MIDI_CLOCK || MODE_ABLETON_LINK_SYNC))
#else
	if (ENABLE_CLOCKS && (MODE_INTERNAL_CLOCK || MODE_EXTERNAL_MIDI_CLOCK))
#endif
	{
		BeatTick = true;

		//gui engine preview
		circleBeat.bang();

		//-

		//play metronome sound
		if (ENABLE_sound)
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
		//if (ENABLE_sound && !bTap_Running)
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
float ofxBeatClock::getBPM()
{
	return BPM_Global;
}

//--------------------------------------------------------------
int ofxBeatClock::getTimeBar()
{
	return BPM_Global_TimeBar;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_Params(ofAbstractParameter &e) //patch change
{
	string name = e.getName();
	if (name != BPM_Global.getName() &&
		name != BPM_Global_TimeBar.getName() &&
		name != PLAYING_External_State.getName()
		)
		ofLogNotice(__FUNCTION__) << name << " : " << e;

	//-

	if (false) {}

	//-

	else if (name == MODE_Editor.getName())
	{
		ofLogNotice(__FUNCTION__) << name << (MODE_Editor ? "TRUE" : "FALSE");
		if (MODE_Editor.get())
		{
			rPreview.enableEdit();
		}
		else
		{
			rPreview.disableEdit();
		}
	}

	else if (name == bGui_PreviewClockNative.getName())
	{
		//TODO:
		// update not implemented
		//if (bGui_PreviewClockNative.get())
		//{
		//	guiManager.AddStyle(SHOW_Editor, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, true, 2, 0);
		//	guiManager.AddStyle(MODE_Editor, SurfingImGuiTypes::OFX_IM_TOGGLE_SMALL, false, 2, 0);
		//}
		//else
		//{
		//	guiManager.AddStyle(SHOW_Editor, SurfingImGuiTypes::OFX_IM_HIDDEN, true, 2, 0);
		//	guiManager.AddStyle(MODE_Editor, SurfingImGuiTypes::OFX_IM_HIDDEN, false, 2, 0);
		//}
	}

	else if (name == PLAYING_State.getName())//button for internal only
	{
		ofLogNotice(__FUNCTION__) << "PLAYING_State: " << (PLAYING_State ? "TRUE" : "FALSE");

		// worklow
		// put one mode active. default is internal
#ifdef USE_ofxAbletonLink
		if (!MODE_EXTERNAL_MIDI_CLOCK && !MODE_ABLETON_LINK_SYNC && !MODE_INTERNAL_CLOCK) MODE_INTERNAL_CLOCK = true;
#else
		if (!MODE_EXTERNAL_MIDI_CLOCK && !MODE_INTERNAL_CLOCK) MODE_INTERNAL_CLOCK = true;
#endif
		//-

		if (MODE_ABLETON_LINK_SYNC)
		{
			// workflow
			if (MODE_ABLETON_LINK_SYNC) {
				if (!LINK_Enable) LINK_Enable = true;
				LINK_Play = PLAYING_State;

				// play
				if (PLAYING_State)
				{
					start();
				}

				// stop
				else
				{
					stop();
				}
			}
		}

		// clocks are disabled or using external midi clock. dont need playing, just to be enabled
		else if ((MODE_EXTERNAL_MIDI_CLOCK || !ENABLE_CLOCKS || !MODE_INTERNAL_CLOCK) && PLAYING_State)
		{
			PLAYING_State = false; // skip and restore to false the play button
		}

		else if (MODE_INTERNAL_CLOCK && ENABLE_CLOCKS)
		{
			// play
			if (PLAYING_State)
			{
				start();
			}

			// stop
			else
			{
				stop();
			}
		}
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (name == LINK_Play.getName()) {
		if (!MODE_ABLETON_LINK_SYNC) MODE_ABLETON_LINK_SYNC = true;
		if (MODE_ABLETON_LINK_SYNC)
		{
			if (!LINK_Enable) LINK_Enable = true;
			PLAYING_State = LINK_Play;
		}
	}
#endif

	//-

	else if (name == BPM_Global.getName())
	{
		ofLogVerbose(__FUNCTION__) << "GLOBAL BPM   : " << BPM_Global;

		//calculate bar duration to the new tempo
		BPM_Global_TimeBar = 60000.0f / (float)BPM_Global;//60,000 / BPM = one beat in milliseconds

		//---

		//NOTE: this update maybe can cause a kind of loop,
		//because clockInternal_Bpm, BPM_Global and LINK_Bpm
		//are updated each others too...
		//TODO: we need some flag to avoid multiple updates when they are not required?

		//TODO:
		clockInternal_Bpm = BPM_Global;

		//-

		//TODO:
#ifdef USE_ofxAbletonLink
		if (MODE_ABLETON_LINK_SYNC) LINK_Bpm = BPM_Global;
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

	//-

	else if (name == BPM_Global_TimeBar.getName())
	{
		ofLogVerbose(__FUNCTION__) << "BAR ms: " << BPM_Global_TimeBar;

		BPM_Global = 60000.0f / (float)BPM_Global_TimeBar;

		//NOTE: this update maybe can cause a kind of loop,
		//because clockInternal_Bpm, BPM_Global and LINK_Bpm
		//are updated each others too...
		//TODO: we need some flag to avoid multiple updates when they are not required?
	}

	//--

	//source clock type

	else if (name == MODE_INTERNAL_CLOCK.getName())
	{
		ofLogNotice(__FUNCTION__) << "CLOCK INTERNAL: " << MODE_INTERNAL_CLOCK;

		if (MODE_INTERNAL_CLOCK)
		{
			MODE_EXTERNAL_MIDI_CLOCK = false;
#ifdef USE_ofxAbletonLink
			MODE_ABLETON_LINK_SYNC = false;
#endif
			//-

			//display text
			clockActive_Type = MODE_INTERNAL_CLOCK.getName();
			clockActive_Info = "";
			strClock = clockActive_Type;

			//-
#ifdef USE_OFX_GUI_EXTENDED2
			group_INTERNAL->maximize();
#endif
		}
		else
		{
			//workflow
			//autostop
			if (PLAYING_State) PLAYING_State = false;
			//disable internal
			if (clockInternal_Active) clockInternal_Active = false;

			//display text
			Bar_string = "0";
			Beat_string = "0";
			Tick_16th_string = "0";

			if (!MODE_INTERNAL_CLOCK)
			{
				//display text
				//clockActive_Type = "NONE";
				clockActive_Type = "";
				clockActive_Info = "";
				strClock = clockActive_Type;
			}

			//-
#ifdef USE_OFX_GUI_EXTENDED2
			group_INTERNAL->minimize();
#endif
		}
	}

	//-

	else if (name == MODE_EXTERNAL_MIDI_CLOCK.getName())
	{
		ofLogNotice(__FUNCTION__) << "CLOCK EXTERNAL MIDI-IN: " << MODE_EXTERNAL_MIDI_CLOCK;

		if (MODE_EXTERNAL_MIDI_CLOCK)
		{
			MODE_INTERNAL_CLOCK = false;
#ifdef USE_ofxAbletonLink
			MODE_ABLETON_LINK_SYNC = false;
#endif
			//-

			////workflow
			////autostop
			//if (PLAYING_State)
			//	PLAYING_State = false;
			////disable internal
			//if (clockInternal_Active)
			//	clockInternal_Active = false;

			//-

			clockInternal.stop();//TODO: trying to avoid log exceptions 
			//FF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
			//	The thread 0xc9d0 has exited with code 0 (0x0).
			//	Exception thrown at 0x00007FFF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
			//	The thread 0x17fa0 has exited with code 0 (0x0).
			//PLAYING_State = false;
			//clockInternal_Active = false;

			//-

			//display text
			clockActive_Type = MODE_EXTERNAL_MIDI_CLOCK.getName();
			clockActive_Info = "PORT:\n";
			clockActive_Info += "'" + midiIn.getName() + "'";
			//clockActive_Info += ofToString(midiIn.getPort());

			strClock = clockActive_Type;

			midiIn_PortName = midiIn.getName();

			//-
#ifdef USE_OFX_GUI_EXTENDED2
			group_EXTERNAL_MIDI->maximize();
#endif
		}
		else
		{
			//display text
			Bar_string = "0";
			Beat_string = "0";
			Tick_16th_string = "0";

			if (!MODE_EXTERNAL_MIDI_CLOCK)
			{
				clockActive_Type = "";
				//clockActive_Type = "NONE";
				clockActive_Info = "";

				strClock = clockActive_Type;
			}

			//-
#ifdef USE_OFX_GUI_EXTENDED2
			group_EXTERNAL_MIDI->minimize();
#endif
		}
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (name == MODE_ABLETON_LINK_SYNC.getName())
	{
		ofLogNotice(__FUNCTION__) << "ABLETON LINK: " << MODE_ABLETON_LINK_SYNC;

		if (MODE_ABLETON_LINK_SYNC)
		{
			ofLogNotice(__FUNCTION__) << "Add link listeners";
			ofAddListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
			ofAddListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
			ofAddListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

			MODE_INTERNAL_CLOCK = false;
			MODE_EXTERNAL_MIDI_CLOCK = false;

			//-

			////TODO:
			////this could be deleted/simplififed
			////workflow
			////autostop
			//if (PLAYING_State)
			//	PLAYING_State = false;
			////disable internal
			//if (clockInternal_Active)
			//	clockInternal_Active = false;

			clockInternal.stop();//TODO: trying to avoid log exceptions 
			//FF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
			//	The thread 0xc9d0 has exited with code 0 (0x0).
			//	Exception thrown at 0x00007FFF91DDA799 in 2_ofxBeatClock_example.exe: Microsoft C++ exception : std::system_error at memory location 0x000000000AB1F1F0.
			//	The thread 0x17fa0 has exited with code 0 (0x0).
			//PLAYING_State = false;
			//clockInternal_Active = false;

			//-

			//display text
			clockActive_Type = MODE_ABLETON_LINK_SYNC.getName();
			//clockActive_Type = "3 ABLETON LINK";
			clockActive_Info = "";

			strClock = clockActive_Type;

			//-

#ifdef USE_OFX_GUI_EXTENDED2
			//workflow
			group_LINK->maximize();
#endif
		}
		else
		{
			ofLogNotice(__FUNCTION__) << "Remove link listeners";
			ofRemoveListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
			ofRemoveListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
			ofRemoveListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

			//-

#ifdef USE_OFX_GUI_EXTENDED2
			//workflow
			group_LINK->minimize();
#endif
		}
	}
#endif

	//--

	//TODO:
	//should check this better
	else if (name == ENABLE_CLOCKS.getName())
	{
		ofLogNotice(__FUNCTION__) << "ENABLE: " << ENABLE_CLOCKS;

		//workflow
		if (!ENABLE_CLOCKS)
		{
			//internal clock
			if (MODE_INTERNAL_CLOCK)
			{
				//autostop
				if (PLAYING_State) PLAYING_State = false;

				//disable internal
				if (clockInternal_Active) clockInternal_Active = false;
			}
		}

		//-

		//workflow
//#ifdef USE_ofxAbletonLink
//		if (MODE_ABLETON_LINK_SYNC)
//		{
//			link.setLinkEnable(ENABLE_CLOCKS);
//		}
//#endif

		//-

		//workflow
		//if none clock mode is selected, we select the internal clock by default
#ifndef USE_ofxAbletonLink
		if (!MODE_INTERNAL_CLOCK && !MODE_EXTERNAL_MIDI_CLOCK)
#else
		if (!MODE_INTERNAL_CLOCK && !MODE_EXTERNAL_MIDI_CLOCK && !MODE_ABLETON_LINK_SYNC)
#endif
		{
			MODE_INTERNAL_CLOCK = true;//default state / clock selected
		}
	}

	//---

	//external midi clock
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

		//workflow
		//stop to avoid freeze bug
		PLAYING_State = false;

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

	//-

	//helpers
	else if (name == RESET_BPM_Global.getName())
	{
		ofLogNotice(__FUNCTION__) << "BPM RESET";

		if (RESET_BPM_Global)
		{
			RESET_BPM_Global = false;//TODO: could be a button not toggle to better gui workflow

			clockInternal_Bpm = 120.00f;

			//not required bc
			//BPM_Global will be updated on the clockInternal_Bpm callback..etc

			//clockInternal.setBpm(clockInternal_Bpm);
			//BPM_Global = 120.00f;

			ofLogNotice(__FUNCTION__) << "BPM_Global: " << BPM_Global;
		}
	}
	else if (name == BPM_double_TRIG.getName())
	{
		if (BPM_double_TRIG)
		{
			ofLogNotice(__FUNCTION__) << "DOUBLE BPM";
			BPM_double_TRIG = false;

			clockInternal_Bpm = clockInternal_Bpm * 2.0f;
		}
	}
	else if (name == BPM_half_TRIG.getName())
	{
		if (BPM_half_TRIG)
		{
			ofLogNotice(__FUNCTION__) << "HALF BPM";
			BPM_half_TRIG = false;

			clockInternal_Bpm = clockInternal_Bpm / 2.0f;
		}
	}
	else if (name == bGui_BpmClock.getName())
	{
		ofLogNotice(__FUNCTION__) << bGui_BpmClock;

#ifdef USE_OFX_GUI_EXTENDED2
		if (bGui_BpmClock)
		{
			group_Advanced->maximize();
		}
		else
		{
			group_Advanced->minimize();
	}
#endif
	}

	//-

	//metronome ticks volume
	else if (name == volumeSound.getName())
	{
		ofLogNotice(__FUNCTION__) << "VOLUME: " << ofToString(volumeSound, 1);

		bpmTapTempo.setVolume(volumeSound);
	}
	else if (name == ENABLE_sound.getName())
	{
		ofLogNotice(__FUNCTION__) << "TICK SOUND: " << (ENABLE_sound ? "ON" : "OFF");
		bpmTapTempo.setEnableSound(ENABLE_sound);
	}

	//-

	//tap
	else if (name == BPM_Tap_Tempo_TRIG.getName())
	{
		ofLogNotice(__FUNCTION__) << "TAP BUTTON";

		if ((BPM_Tap_Tempo_TRIG == true) && (MODE_INTERNAL_CLOCK))
		{
			BPM_Tap_Tempo_TRIG = false;

			ofLogNotice(__FUNCTION__) << "BPM_Tap_Tempo_TRIG: " << BPM_Tap_Tempo_TRIG;

			tap_Trig();
		}
	}

	//-

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
void ofxBeatClock::Changed_midiIn_BeatsInBar(int &beatsInBar)
{
	ofLogVerbose(__FUNCTION__) << beatsInBar;

	//only used in midiIn clock sync 
	//this function trigs when any midi tick (beatsInBar) updating, so we need to filter if (we want beat or 16th..) has changed.
	//problem is maybe that beat param refresh when do not changes too, jus because it's accesed..

	if (ENABLE_pattern_limits)
	{
		midiIn_BeatsInBar = beatsInBar % pattern_BEAT_limit;
	}

	//-

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

		//trig ball drawing and sound ticker

		beatTick_MONITOR(Beat_current);

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
//	//PLAYING_State = true;
//}

//--------------------------------------------------------------
void ofxBeatClock::Changed_ClockInternal_Bpm(float &value)
{
	clockInternal.setBpm(value);

	//TODO:
	clockInternal.resetTimer();//(?) is this required (?)

	//-

	BPM_Global = value;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_ClockInternal_Active(bool &value)
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
void ofxBeatClock::newMidiMessage(ofxMidiMessage &message)
{
	if (MODE_EXTERNAL_MIDI_CLOCK && ENABLE_CLOCKS)
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

				//clockInternal_Bpm = (float)midiIn_Clock_Bpm;

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

		PLAYING_External_State = bMidiInClockRunning;
		//TODO:
		//BUG:
		//sometimes, when external sequencer is stopped, 
		//bMidiInClockRunning do not updates to false!
	}
}

//--------------------------------------------------------------
void ofxBeatClock::saveSettings(std::string path)
{
	ofxSurfingHelpers::CheckFolder(path);

#ifdef USE_OFX_GUI_EXTENDED2
	pos_Gui = glm::vec2(panel_BeatClock->getPosition().x, panel_BeatClock->getPosition().y);
#endif

	//save settings
	ofLogNotice(__FUNCTION__) << path;

	ofXml settings1;
	ofSerialize(settings1, params_CONTROL);
	settings1.save(path + file_BeatClock);

	ofXml settings2;
	ofSerialize(settings2, params_EXTERNAL_MIDI);
	settings2.save(path + file_Midi);

	ofXml settings3;
	ofSerialize(settings3, params_App);
	settings3.save(path + file_App);
}

//--------------------------------------------------------------
void ofxBeatClock::loadSettings(std::string path)
{
	//load settings
	ofLogNotice(__FUNCTION__) << path;

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
	ofDeserialize(settings3, params_App);

#ifdef USE_OFX_GUI_EXTENDED2
	panel_BeatClock->setPosition(pos_Gui.get().x, pos_Gui.get().y);
#endif
}

//--------------------------------------------------------------

// not used and callbacks are disabled on audioBuffer timer mode(?)

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int &bar)
{
	ofLogVerbose(__FUNCTION__) << "Internal Clock - BAR: " << bar;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (ENABLE_CLOCKS && MODE_INTERNAL_CLOCK)
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
void ofxBeatClock::onBeatEvent(int &beat)
{
	ofLogVerbose(__FUNCTION__) << "Internal Clock - BEAT: " << beat;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (ENABLE_CLOCKS && MODE_INTERNAL_CLOCK)
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

			beatTick_MONITOR(Beat_current);

			//-
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::onSixteenthEvent(int &sixteenth)
{
	//ofLogVerbose(__FUNCTION__) << "Internal Clock - 16th: " << sixteenth;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (ENABLE_CLOCKS && MODE_INTERNAL_CLOCK)
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

	//if (MODE_INTERNAL_CLOCK)
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
void ofxBeatClock::tap_Trig()
{
	if (MODE_INTERNAL_CLOCK)//extra verified, not mandatory
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
	clockInternal_Bpm = bpm;
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
void ofxBeatClock::audioOut(ofSoundBuffer &buffer)
{
	if (PLAYING_State && MODE_AudioBufferTimer)
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

				if (ENABLE_CLOCKS && MODE_INTERNAL_CLOCK)
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
						beatTick_MONITOR(Beat_current);

						//ofLogNotice(__FUNCTION__) << "[audioBuffer] BEAT: " << Beat_string;
					}
				}
			}
		}
	}
}
#endif


//--------------------------------------------------------------
void ofxBeatClock::windowResized(int _w, int _h)
{
	//	//if (!DISABLE_Callbacks)
	//	//{
	//	window_W = _w;
	//	window_H = _h;
	//	//window_X = ofGetWindowPositionX();
	//	//window_Y = ofGetWindowPositionY();
	//
	////	ofLogNotice(__FUNCTION__) << "size: " << window_W << "," << window_H;;
	////	ofLogNotice(__FUNCTION__) << "position: " << window_X << "," << window_Y;;
	//
	////	//--
	//
	////	//gui
	//
	////	//--
	//
	////	//1. video player
	////	hapSkipper.windowResized(window_W, window_H);
	////}
}

//--------------------------------------------------------------
void ofxBeatClock::keyPressed(ofKeyEventArgs &eventArgs)
{
	if (!bKeys) return;

	const int key = eventArgs.key;

	// modifiers
	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

	ofLogNotice(__FUNCTION__) << " : " << key;

	switch (key)
	{
		// toggle play / stop
	case ' ':
		setTogglePlay();
		break;

		// trig tap-tempo 4 times 
		// when using internal clock 
		// only to set the bpm on the fly
	case 't':
		tap_Trig();
		break;

		// get some beatClock info. look api methods into ofxBeatClock.h
	case OF_KEY_RETURN:
		ofLogWarning(__FUNCTION__) << "BPM     : " << getBPM() << " beats per minute";
		ofLogWarning(__FUNCTION__) << "BAR TIME: " << getTimeBar() << " ms";
		break;

		// debug
	case 'd':
		toggleDebug_Clock(); // clock debug
		toggleDebug_Layout(); // layout debug
		break;

		// show gui controls
#ifdef USE_OFX_GUI_EXTENDED2
	case 'g':
		toggleVisible_GuiPanel();
		break;
#endif
	case 'g':
		toggleVisibleGui();
		break;

		// show gui previews
	case 'G':
		setToggleVisible_GuiPreview();
		break;

		// edit gui preview
	case 'e':
		MODE_Editor = !MODE_Editor;
		break;

	case '-':
		BPM_Global -= 1.0;
		break;

	case '+':
		BPM_Global += 1.0;
		break;
	}
}