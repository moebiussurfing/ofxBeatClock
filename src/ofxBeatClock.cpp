#include "ofxBeatClock.h"

//--------------------------------------------------------------
ofxBeatClock::ofxBeatClock() {
	ofSetLogLevel("ofxBeatClock", OF_LOG_NOTICE);
	path_Global = "ofxBeatClock/settings/";

	// subscribed to auto run update and draw without required 'manual calls'
	ofAddListener(ofEvents().update, this, &ofxBeatClock::update);
	ofAddListener(ofEvents().draw, this, &ofxBeatClock::draw);
}

//--------------------------------------------------------------
 ofxBeatClock::~ofxBeatClock() {
	 exit();

	 ofRemoveListener(ofEvents().update, this, &ofxBeatClock::update);
	 ofRemoveListener(ofEvents().draw, this, &ofxBeatClock::draw);
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

	//1.1 controls

	RESET_BPM_Global.set("RESET BPM", false);

	params_CONTROL.setName("CONTROL");
	params_CONTROL.add(ENABLE_CLOCKS.set("ENABLE", true));
	//params_CONTROL.add(PLAYING_Global_State.set("PLAY", false));//TEST
	params_CONTROL.add(clockInternal_Bpm.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_CONTROL.add(RESET_BPM_Global);

	params_CONTROL.add(ENABLE_INTERNAL_CLOCK.set("INTERNAL", false));
	params_CONTROL.add(ENABLE_EXTERNAL_MIDI_CLOCK.set("EXTERNAL MIDI", true));
#ifdef USE_ofxAbletonLink
	params_CONTROL.add(ENABLE_LINK_SYNC.set("ABLETON LINK", false));
#endif
	params_CONTROL.add(SHOW_Extra.set("SHOW PREVIEW", true));
	params_CONTROL.add(SHOW_Advanced.set("SHOW ADVANCED", false));

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
	params_INTERNAL.add(PLAYING_State.set("PLAY", false));
	params_INTERNAL.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
	//TODO: should better gui-behavior-feel being a button not toggle
	
	//-

	//TODO:
	///trig resync to closest beat? (not so simple as to go to bar start)
	//params_INTERNAL.add(bSync_Trig.set("SYNC", false));

	//-

	//1.3 external midi
	params_EXTERNAL_MIDI.setName("EXTERNAL MIDI CLOCK");
	params_EXTERNAL_MIDI.add(PLAYING_External_State.set("PLAY SYNC", false));
	params_EXTERNAL_MIDI.add(midiIn_Port_SELECT.set("MIDI INPUT PORT", 0, 0, midiIn_numPorts - 1));
	params_EXTERNAL_MIDI.add(midiIn_PortName);

	//-

	//1.4 ableton link

	//-

#ifdef USE_ofxAbletonLink
#pragma mark - ABLETON_LINK
	LINK_setup();
#endif

	//-

#ifdef USE_ofxAbletonLink
	params_LINK.setName("ABLETON LINK");
	params_LINK.add(LINK_Enable.set("LINK", true));
	params_LINK.add(LINK_Peers_string.set("PEERS", " "));
	params_LINK.add(LINK_Beat_string.set("BEAT", ""));
	params_LINK.add(LINK_Play.set("PLAY", false));
	params_LINK.add(LINK_Phase.set("PHASE", 0.f, 0.f, 4.f));
	params_LINK.add(LINK_Bpm.set("BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_LINK.add(LINK_RestartBeat.set("RESYNC", false));
	params_LINK.add(LINK_ResetBeats.set("FORCE RESET", false));
	//params_LINK.add(LINK_Beat_Selector.set("GO BEAT", 0, 0, 100));//TODO: TEST

	//not required..
	LINK_Enable.setSerializable(false);
	LINK_Bpm.setSerializable(false);
	LINK_Play.setSerializable(false);
	LINK_Phase.setSerializable(false);
	LINK_Beat_string.setSerializable(false);
	LINK_RestartBeat.setSerializable(false);
	LINK_ResetBeats.setSerializable(false);
	LINK_Peers_string.setSerializable(false);
	//LINK_Beat_Selector.setSerializable(false);
#endif

	//--

	//1.5 extra and advanced settings

#pragma mark - ADVANCED_PANEL

	//this smoothed (or maybe slower refreshed than fps) clock will be sended to target sequencer outside the class. see BPM_MIDI_CLOCK_REFRESH_RATE.
	params_Advanced.setName("ADVANCED");
	params_Advanced.add(BPM_Global.set("GLOBAL BPM", BPM_INIT, BPM_INIT_MIN, BPM_INIT_MAX));
	params_Advanced.add(BPM_Global_TimeBar.set("BAR ms", (int)60000 / BPM_Global, 100, 2000));
	params_Advanced.add(RESET_BPM_Global);

	//added to group in other method (?)
	BPM_half_TRIG.set("HALF", false);
	BPM_double_TRIG.set("DOUBLE", false);
	params_Advanced.add(BPM_half_TRIG);
	params_Advanced.add(BPM_double_TRIG);

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	MODE_AudioBufferTimer.set("MODE AUDIO BUFFER", false);
	params_Advanced.add(MODE_AudioBufferTimer);
#endif

	params_Advanced.add(ENABLE_sound.set("SOUND TICK", false));
	params_Advanced.add(volumeSound.set("VOLUME", 0.5f, 0.f, 1.f));

	//--

	//other settings only to storing
	params_App.setName("AppSettings");
	params_App.add(ENABLE_sound);
	params_App.add(volumeSound);

	//-

	//exclude
	midiIn_PortName.setSerializable(false);
	PLAYING_External_State.setSerializable(false);
	RESET_BPM_Global.setSerializable(false);

	//--

	//tap tempo engine
	bTap_Running = false;
	tap_Count = 0;
	tap_Intervals.clear();

	//----

#pragma mark - DEFAULT_LAYOUT_POSITIONS

	//default config. to be setted after with .setPosition_GuiPanel

	gui_Panel_Width = 200;
	gui_Panel_posX = 5;
	gui_Panel_posY = 5;

	//-

	//display text
	std::string strFont;
	strFont = "assets/fonts/telegrama_render.otf";

	fontSmall.load(strFont, 7);
	fontMedium.load(strFont, 10);
	fontBig.load(strFont, 13);

	//-

	if (!fontSmall.isLoaded())
	{
		ofLogError(__FUNCTION__) << "ERROR LOADING FONT " << strFont;
		ofLogError(__FUNCTION__) << "Check your font file into /data/assets/fonts/";
	}

	//--

	//add already initiated params from transport and all controls to the gui panel
	setup_GuiPanel();

	//---

#pragma mark - LAYOUT_GUI_ELEMENTS

	//default gui panel position
	setPosition_GuiPanel(gui_Panel_posX, gui_Panel_posY, gui_Panel_Width);

	//--

	//default gui extra position
	//pos_Global = glm::vec2(5, 720);//TODO:
	setPosition_GuiExtra(5, 770);

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
	BeatTick_TRIG = false;

	//---

#pragma mark - METRONOME_SOUNDS

	//beat 1 sound
	tic.load("ofxBeatClock/sounds/click1.wav");
	tic.setVolume(1.0f);
	tic.setMultiPlay(false);

	//beats 2-3-4 sound
	tac.load("ofxBeatClock/sounds/click2.wav");
	tac.setVolume(0.25f);
	tac.setMultiPlay(false);

	//tap tempo measure done sound
	tapBell.load("ofxBeatClock/sounds/tapBell.wav");
	tapBell.setVolume(1.0f);
	tapBell.setMultiPlay(false);

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
	//load last session settings

	//folder to both (control and midi input port) settings files
	loadSettings(path_Global);

	//-

	//rectanglePresetClicker.width = getPresetClicker_Width() + 2 * _RectClick_Pad + _RectClick_w;
	//rectanglePresetClicker.height = getPresetClicker_Height() + 2 * _RectClick_Pad;
	//rectanglePresetClicker.x = getPresetClicker_Position().x - _RectClick_Pad - _RectClick_w;
	//rectanglePresetClicker.y = getPresetClicker_Position().y - _RectClick_Pad;
	//_rectRatio = rectanglePresetClicker.width / rectanglePresetClicker.height;

	//// load settings
	//rectanglePresetClicker.loadSettings(path_RectanglePresetClicker, path_UserKit_Folder + "/" + path_ControlSettings + "/", false);
	//clicker_Pos.x = rectanglePresetClicker.x + _RectClick_Pad + _RectClick_w;
	//clicker_Pos.y = rectanglePresetClicker.y + _RectClick_Pad;

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

	//-----
}

//--------------------------------------------------------------
void ofxBeatClock::setup_GuiPanel()
{
	//to customize some widgets
	confg_Sliders =
	{
		{"height", 40}
	};
	confg_Button =
	{
		{"type", "fullsize"},
		{"text-align", "center"},
		{"height", 40},
	};
	confg_ButtonSmall =
	{
		{"type", "fullsize"},
		{"text-align", "center"},
		{"height", 23},
	};

	//--

	//1. main panel
	group_BeatClock = gui.addGroup("BEAT CLOCK");//main container

	//1.1 control
	group_Controls = group_BeatClock->addGroup(params_CONTROL);

	//1.2 internal
	group_INTERNAL = group_BeatClock->addGroup("INTERNAL CLOCK");
	group_INTERNAL->add(params_INTERNAL);

	//1.3 external midi
	group_EXTERNAL_MIDI = group_BeatClock->addGroup("EXTERNAL MIDI CLOCK");
	group_EXTERNAL_MIDI->add(params_EXTERNAL_MIDI);

	//1.4 link
#ifdef USE_ofxAbletonLink
	group_LINK = group_BeatClock->addGroup("ABLETON LINK");
	group_LINK->add(params_LINK);
#endif

	//1.5 extra and advanced settings
	group_Advanced = group_BeatClock->addGroup(params_Advanced);

	//--

	//customize

	//1.1
	group_Controls->getControl("ENABLE")->setConfig(confg_Button);
	//group_Controls->getToggle("PLAY")->setConfig(confg_Button);//global play//TEST
	group_Controls->getControl("INTERNAL")->setConfig(confg_Button);
	group_Controls->getControl("EXTERNAL MIDI")->setConfig(confg_Button);
#ifdef USE_ofxAbletonLink
	group_Controls->getControl("ABLETON LINK")->setConfig(confg_Button);
#endif
	group_Controls->getFloatSlider("BPM")->setConfig(confg_Sliders);
	group_Controls->getFloatSlider("BPM")->setPrecision(2);

	//1.2
	group_INTERNAL->getToggle("PLAY")->setConfig(confg_Button);
	group_INTERNAL->getToggle("TAP")->setConfig(confg_Button);

	//1.3
	group_EXTERNAL_MIDI->getToggle("PLAY SYNC")->setConfig(confg_Button);
	group_EXTERNAL_MIDI->getControl("PLAY SYNC")->unregisterMouseEvents();

	//1.4
#ifdef USE_ofxAbletonLink
	group_LINK->getToggle("LINK")->setConfig(confg_ButtonSmall);
	group_LINK->getToggle("PLAY")->setConfig(confg_Button);
	group_LINK->getToggle("RESYNC")->setConfig(confg_ButtonSmall);
	group_LINK->getToggle("FORCE RESET")->setConfig(confg_ButtonSmall);
	group_LINK->getFloatSlider("PHASE")->setConfig(ofJson{
		{"precision", 1},
		{"fill-color", "rgb(128,128,0)"}
		});
#endif

	//1.5
	group_Advanced->getFloatSlider("GLOBAL BPM")->setPrecision(2);

	//-

	//listeners
	ofAddListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_INTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_EXTERNAL_MIDI.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_Advanced.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//--

	//theme
	path_Theme = "assets/theme/";
	path_Theme += "theme_ofxGuiExtended2_01.json";
	loadTheme(path_Theme);

	////customize panel width over the loaded json theme
	//group_BeatClock->setWidth(gui_Panel_Width);
	//group_INTERNAL->setWidth(gui_Panel_Width);
	//group_Controls->setWidth(gui_Panel_Width);
	//group_Advanced->setWidth(gui_Panel_Width);

	//--

	//expand/collapse pannels
	group_BeatClock->maximize();
	group_Controls->maximize();
	group_Advanced->minimize();

	if (ENABLE_INTERNAL_CLOCK)
	{
		group_INTERNAL->maximize();
	}
	else
	{
		group_INTERNAL->minimize();
	}

	if (ENABLE_EXTERNAL_MIDI_CLOCK)
	{
		group_EXTERNAL_MIDI->maximize();
	}
	else
	{
		group_EXTERNAL_MIDI->minimize();
	}

#ifdef USE_ofxAbletonLink
	if (ENABLE_LINK_SYNC)
	{
		group_LINK->maximize();
	}
	else
	{
		group_LINK->minimize();
	}
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::refresh_Gui()
{
	//kind of init state after loaded settings just in case not open correct(?)

	if (ENABLE_INTERNAL_CLOCK)
	{
		ENABLE_EXTERNAL_MIDI_CLOCK = false;
#ifdef USE_ofxAbletonLink
		ENABLE_LINK_SYNC = false;
#endif
		//display text
		clockActive_Type = "INTERNAL";
		clockActive_Info = "";
	}

	//-

	else if (ENABLE_EXTERNAL_MIDI_CLOCK)
	{
		ENABLE_INTERNAL_CLOCK = false;
#ifdef USE_ofxAbletonLink
		ENABLE_LINK_SYNC = false;
#endif

		if (PLAYING_State)
			PLAYING_State = false;
		if (clockInternal_Active)
			clockInternal_Active = false;

		//display text
		clockActive_Type = "EXTERNAL MIDI";
		clockActive_Info = "MIDI PORT: ";
		clockActive_Info += "'" + midiIn.getName() + "'";
		//clockActive_Info += ofToString(midiIn.getPort());
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (ENABLE_LINK_SYNC)
	{
		ENABLE_INTERNAL_CLOCK = false;
		ENABLE_EXTERNAL_MIDI_CLOCK = false;

		if (PLAYING_State)
			PLAYING_State = false;
		if (clockInternal_Active)
			clockInternal_Active = false;

		//display text
		clockActive_Type = "ABLETON LINK";
		clockActive_Info = "";
	}
#endif

	//--

	//gui workflow

	if (ENABLE_INTERNAL_CLOCK)
	{
		group_INTERNAL->maximize();
	}
	else
	{
		group_INTERNAL->minimize();
	}

	if (ENABLE_EXTERNAL_MIDI_CLOCK)
	{
		group_EXTERNAL_MIDI->maximize();
	}
	else
	{
		group_EXTERNAL_MIDI->minimize();
	}

#ifdef USE_ofxAbletonLink
	if (ENABLE_LINK_SYNC)
	{
		group_LINK->maximize();
	}
	else
	{
		group_LINK->minimize();
	}
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MidiIn_Clock()
{
	//external midi clock

	ofLogNotice(__FUNCTION__) << "setup_MidiIn_Clock()";

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
	clockActive_Type = "EXTERNAL MIDI";
	clockActive_Info = "MIDI PORT: ";
	clockActive_Info += "'" + midiIn.getName() + "'";
	//clockActive_Info += ofToString(midiIn.getPort());

	ofLogNotice(__FUNCTION__) << "Connected to MIDI IN CLOCK Port: " << midiIn.getPort();
}

#pragma mark - UPDATE
//--------------------------------------------------------------
void ofxBeatClock::update(ofEventArgs & args)
{
	//--

	//tap engine
	if (bTap_Running) tap_Update();

	//--

#ifdef USE_ofxAbletonLink
	if (ENABLE_LINK_SYNC) LINK_update();
#endif

	//--

	//TODO:
	//smooth clock from received MIDI CLOCK...
//	//read bpm with a spaced clock refresh or in every frame if not defined time-clock-refresh:
//
//#ifdef BPM_MIDI_CLOCK_REFRESH_RATE
//	if (ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime >= (int)BPM_MIDI_CLOCK_REFRESH_RATE)
//	{
//#endif
//		ofLogNotice(__FUNCTION__) << "BPM UPDATED" << ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime;
//
//		//-
//
//#ifdef BPM_MIDI_CLOCK_REFRESH_RATE
//		bpm_CheckUpdated_lastTime = ofGetElapsedTimeMillis();
//	}
//#endif

	//-

	//BPM_LAST_Tick_Time_ELLAPSED_PRE = BPM_LAST_Tick_Time_ELLAPSED;
	//BPM_LAST_Tick_Time_ELLAPSED = ofGetElapsedTimeMillis() - BPM_LAST_Tick_Time_LAST;//test
	//BPM_LAST_Tick_Time_LAST = ofGetElapsedTimeMillis();//test
	//ELLAPSED_diff = BPM_LAST_Tick_Time_ELLAPSED_PRE - BPM_LAST_Tick_Time_ELLAPSED;

	//--

	//metronome sound ticks
	ofSoundUpdate();

	//--
}


#pragma mark - DRAW

//--------------------------------------------------------------
void ofxBeatClock::drawPreviewExtra()
{

	//-

	//// bg rectangle editor
	//if (SHOW_BackGround_EditPresetClicker)
	//{
	//	ofFill();
	//	ofSetColor(_colorBg);
	//	rectanglePresetClicker.draw();
	//	ofDrawRectRounded(rectanglePresetClicker, _round);
	//}

	//// get clicker position from being edited rectangle
	//if (MODE_EditPresetClicker)
	//{
	//	_RectClick_w = getGroupNamesWidth();
	//	clicker_Pos.x = rectanglePresetClicker.x + _RectClick_Pad + _RectClick_w;
	//	clicker_Pos.y = rectanglePresetClicker.y + _RectClick_Pad;
	//	//rectanglePresetClicker.width = MIN(getPresetClicker_Width() + 2 * _RectClick_Pad + _RectClick_w, 1000);
	//	//rectanglePresetClicker.height = rectanglePresetClicker.width / _rectRatio;
	//}

	//-

	draw_BpmInfo(pos_BpmInfo.x, pos_BpmInfo.y);
	draw_ClockInfo(pos_ClockInfo.x, pos_ClockInfo.y);
	draw_BeatBoxes(pos_BeatBoxes_x, pos_BeatBoxes_y, pos_BeatBoxes_width);
	draw_BeatBall(pos_BeatBall_x, pos_BeatBall_y, pos_BeatBall_radius);
}

//--------------------------------------------------------------
void ofxBeatClock::draw(ofEventArgs & args)
{
	//TODO: maybe could improve performance with fbo drawings for all BeatBoxes/text/ball?

	if (SHOW_Extra)
	{
		drawPreviewExtra();

		//-

#ifdef USE_ofxAbletonLink
		if (ENABLE_LINK_SYNC && DEBUG_moreInfo)
		{
			LINK_draw();
		}
#endif
	}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BeatBoxes(int px, int py, int w)///draws text info and boxes
{
	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	//sizes and paddings
	int squaresW;//squares beats size
	squaresW = w / 4;//width of each square box
	int xPad = 5;//x margin
	int interline = 15; //text line heigh
	int i = 0;//vertical spacer to accumulate text lines

	//colors
	int colorBoxes = 192;//grey
	int alphaBoxes = 200;//alpha of several above draw fills

	//----

	//2. BEATS [4] SQUARES

	ofPushStyle();

	for (int i = 0; i < 4; i++)
	{
		ofPushStyle();

		//define squares colors:
#ifdef USE_ofxAbletonLink
		if (ENABLE_CLOCKS && (ENABLE_EXTERNAL_MIDI_CLOCK || ENABLE_INTERNAL_CLOCK || ENABLE_LINK_SYNC))
#else
		if (ENABLE_CLOCKS && (ENABLE_EXTERNAL_MIDI_CLOCK || ENABLE_INTERNAL_CLOCK))
#endif
		{
			//ligther color (colorBoxes) for beat squares that "has been passed [left]"
			if (i <= (Beat_current - 1))
			{
#ifdef USE_ofxAbletonLink
				//TEST:
				if (ENABLE_LINK_SYNC && LINK_Enable)
					/*(link.isPlaying()) ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);*/
					(LINK_Play) ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);

				else if (ENABLE_INTERNAL_CLOCK)
					PLAYING_State ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);
#else
				if (ENABLE_INTERNAL_CLOCK)
					PLAYING_State ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);
#endif
				else if (ENABLE_EXTERNAL_MIDI_CLOCK)
					bMidiInClockRunning ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);
			}

			//dark color if beat square "is comming [right]"..
			else ofSetColor(32, alphaBoxes);
		}

		//-

		else//disabled all the 3 source clock modes
		{
			ofSetColor(32, alphaBoxes);//black
		}

		//--

		//DRAW EACH i SQUARE

		//filled rectangle
		////optional:
		////adds alpha faded 16th blink, hearth-pulse mode for visual feedback only
		//if (i == Beat_current - 1 && Tick_16th_current % 2 == 0)
		//{
		//	ofSetColor(colorBoxes, alphaBoxes - 64);
		//}
		ofFill();
		ofDrawRectRounded(px + i * squaresW, py, squaresW, squaresW, 3.0f);

		//rectangle with border only
		ofNoFill();
		ofSetColor(8, alphaBoxes);
		ofSetLineWidth(2.0f);
		ofDrawRectRounded(px + i * squaresW, py, squaresW, squaresW, 3.0f);

		//--

		ofPopStyle();
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_ClockInfo(int px, int py)
{
	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	//this method draws: tap debug, clock source, clock info, bar-beat-tick16th clock 
	int xPad = 5;
	int interline = 15; //text line heigh
	int i = 0;//vertical spacer to accumulate text lines
	int h = fontBig.getSize();//font spacer to respect px, py anchor

	//--

	//clock info:

	ofPushStyle();
	ofSetColor(colorText);

	//-

	ofPushMatrix();
	{
		ofTranslate(px, py);

		//-

		//1.2. tap mode

		int speed = 10;//blink speed when measuring taps
		bool b = (ofGetFrameNum() % (2 * speed) < speed);
		ofSetColor(255, b ? 128 : 255);
		if (bTap_Running)
		{
			messageInfo = "     TAP " + ofToString(ofMap(tap_Count, 1, 4, 3, 0));
		}
		else
		{
			messageInfo = "";
		}
		fontBig.drawString(messageInfo, xPad, h + interline * i++);

		i++;//one extra line vertical spacer
		i++;

		//-

		ofSetColor(colorText);

		//1.3. clock

		//show clock source (internal, external or link)
		messageInfo = "CLOCK: " + clockActive_Type;
		fontSmall.drawString(messageInfo, xPad, interline * i++);

		//-

		//A. external midi clock
		//midi in port. id number and name
		if (ENABLE_EXTERNAL_MIDI_CLOCK)
		{
			messageInfo = ofToString(clockActive_Info);
			fontSmall.drawString(messageInfo, xPad, interline * i++);
		}

		//-

		//B. internal clock
		else if (ENABLE_INTERNAL_CLOCK)
		{
			i++;//empty line
		}

		//-

#ifdef USE_ofxAbletonLink
		//C. Ableton Link
		else if (ENABLE_LINK_SYNC)
		{
			messageInfo = ofToString(clockActive_Info);
			fontSmall.drawString(messageInfo, xPad, interline * i++);
		}
#endif
		//-

		else//space in case no clock source is selected to mantain layout "static" below
		{
			i++;
		}

		//-

		//1.5 main big clock: [bar:beat:tick16th]
		//TODO: could split this and allow to draw independently
		ofSetColor(colorText);
		int yPad = 10;
		i = i + 2;//spacer

		draw_BigClockTime(xPad, yPad + interline * i);

	}
	ofPopMatrix();

	//--

	//1.4 more debug info
	if (DEBUG_moreInfo)
	{
		int xpos = 240;
		int ypos = 750;

		i = 0;

		messageInfo = ("BPM: " + ofToString(BPM_Global, 2));
		fontMedium.drawString(messageInfo, xpos, ypos + interline * i++);

		messageInfo = ("BAR: " + Bar_string);
		fontMedium.drawString(messageInfo, xpos, ypos + interline * i++);

		messageInfo = ("BEAT: " + Beat_string);
		fontMedium.drawString(messageInfo, xpos, ypos + interline * i++);

		messageInfo = ("Tick 16th: " + Tick_16th_string);
		fontMedium.drawString(messageInfo, xpos, ypos + interline * i);
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BpmInfo(int px, int py)
{
	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	int xPad = 5;
	int h = fontBig.getSize();

	ofPushStyle();
	ofSetColor(colorText);

	messageInfo = "BPM: " + ofToString(BPM_Global.get(), 2);
	fontBig.drawString(messageInfo, px + xPad, py + h);

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BeatBall(int px, int py, int _radius)
{
	if (DEBUG_Layout) ofxSurfingHelpers::draw_Anchor(px, py);

	ofPushStyle();

	//-

	//tick ball:

	metronome_ball_radius = _radius;
	py += metronome_ball_radius;
	metronome_ball_pos.x = px + metronome_ball_radius;
	metronome_ball_pos.y = py;

	//highlight 1st beat
	ofColor c;
	if (Beat_current == 1)
		c = (ofColor::red);
	else
		c = (ofColor::white);

	//-

	//background black ball
	if (!bTap_Running)
	{
		ofSetColor(16, 200);//ball background when not tapping
	}
	else
	{
		//white alpha fade when measuring tapping
		float t = (ofGetElapsedTimeMillis() % 1000);
		float fade = sin(ofMap(t, 0, 1000, 0, 2 * PI));
		ofLogVerbose(__FUNCTION__) << "fade: " << fade << endl;
		int alpha = (int)ofMap(fade, -1.0f, 1.0f, 0, 50) + 205;
		ofSetColor(ofColor(96), alpha);
	}
	ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);

	//-

	//beat circle

	float radius = metronome_ball_radius;
	int alphaMax = 128;

#ifdef USE_VISUAL_FEEDBACK_FADE
	ofPushStyle();
	//alpha tick is tweened to kid of "heartbeat feeling"..
	fadeOut_animCounter += 4.0f*dt;//fade out timed speed
	fadeOut_animRunning = fadeOut_animCounter <= 1;
	float alpha = 0.0f;
	if (fadeOut_animRunning && !bTap_Running)
	{
		circlePos.set(metronome_ball_pos.x, metronome_ball_pos.y);

		ofFill();
		alpha = ofMap(fadeOut_animCounter, 0, 1, alphaMax, 0);
		ofSetColor(c, alpha);//faded alpha
		ofDrawCircle(circlePos, fadeOut_animCounter * radius);
	}

	//border circle
	ofNoFill();
	ofSetLineWidth(2.0f);
	ofSetColor(255, alphaMax * 0.5f + alpha * 0.5f);
	ofDrawCircle(circlePos, radius);
	ofPopStyle();

#else//TODO:
	//not doing the candy-fading-out, but it seems that do not improve fps much neither...
	if (!bTap_Running)
	{
		ofPushStyle();

		circlePos.set(metronome_ball_pos.x, metronome_ball_pos.y);

		ofFill();
		ofSetColor(c, alphaMax);
		ofDrawCircle(circlePos, radius);

		//border circle
		ofNoFill();
		ofSetLineWidth(2.0f);
		ofSetColor(255, alphaMax * 0.5f);
		ofDrawCircle(circlePos, radius);
		ofPopStyle();
	}
#endif

	//-

	//first beat circle of bar is drawed red. other ones are white
	//this circle flahses will not be drawed when measuring tap tempo engine

	if (ENABLE_CLOCKS && !bTap_Running &&
#ifndef USE_ofxAbletonLink
	(ENABLE_EXTERNAL_MIDI_CLOCK || ENABLE_INTERNAL_CLOCK))
#else
		(ENABLE_EXTERNAL_MIDI_CLOCK || ENABLE_INTERNAL_CLOCK || ENABLE_LINK_SYNC))
#endif
	{
		//this trigs to draw a flashing circle for a frame only
		if (BeatTick_TRIG)
		{
			BeatTick_TRIG = false;

			//TODO: this could be the problem of confusing when beat 1/2 are
			//highlight 1st beat
			//if (Beat_current == 1)
			if (lastBeatFlash == 1)
				ofSetColor(ofColor::red);
			else
				ofSetColor(ofColor::white);

			ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
		}
	}

	//-

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BigClockTime(int x, int y)
{
	ofPushStyle();
	ofSetColor(colorText);

	int xpad = 12;
	std::string timePos = ofToString(Bar_string, 3, ' ') + " : " + ofToString(Beat_string);

	//only internal clock mode gets 16th ticks (beat / 16 duration)
	//midi sync and link modes do not gets the 16th ticks, so we hide the (last) number to avoid confusion..

#ifndef USE_ofxAbletonLink
	if (!ENABLE_EXTERNAL_MIDI_CLOCK)
#else
	if (!ENABLE_EXTERNAL_MIDI_CLOCK && !ENABLE_LINK_SYNC)
#endif
	{
		timePos += " : " + ofToString(Tick_16th_string);
	}

	fontBig.drawString(timePos, x + xpad, y);

	ofPopStyle();
}

#pragma mark - SETTERS_FOR_LAYOUT_POSITIONS

//--------------------------------------------------------------
void ofxBeatClock::setPosition_GuiGlobal(int x, int y)
{
	setPosition_GuiPanel(x, y, 200);
	setPosition_GuiExtra(x, y + 750);
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_GuiPanel(int _x, int _y, int _w)
{
	gui_Panel_posX = _x;
	gui_Panel_posY = _y;
	gui_Panel_Width = _w;

	group_BeatClock->setPosition(ofPoint(gui_Panel_posX, gui_Panel_posY));

	group_BeatClock->setWidth(gui_Panel_Width);
	group_INTERNAL->setWidth(gui_Panel_Width);
	group_Controls->setWidth(gui_Panel_Width);
	group_Advanced->setWidth(gui_Panel_Width);
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_GuiExtra(int x, int y)
{
	//will be used if they are not redefined

	//pos global
	//main anchor to reference all other gui elements
	pos_Global = glm::vec2(x, y);

	//bpm big text info
	pos_BpmInfo = glm::vec2(pos_Global.x, pos_Global.y);

	//clock info
	pos_ClockInfo = glm::vec2(pos_Global.x, pos_BpmInfo.y + 20);

	//beat boxes
	pos_BeatBoxes_width = 200;
	pos_BeatBoxes_x = pos_Global.x;
	pos_BeatBoxes_y = pos_ClockInfo.y + 130;

	//beat ball
	pos_BeatBall_radius = 30;
	pos_BeatBall_x = pos_Global.x + pos_BeatBoxes_width * 0.5f - pos_BeatBall_radius;
	pos_BeatBall_y = pos_BeatBoxes_y + 62;
}

//--------------------------------------------------------------
ofPoint ofxBeatClock::getPosition_GuiPanel()
{
	ofPoint p;
	//p = group_BeatClock->getShape().getTopLeft();
	p = ofPoint(gui_Panel_posX, gui_Panel_posY);
	return p;
}

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
void ofxBeatClock::setVisible_BeatBall(bool b)
{
	SHOW_Extra = b;
}

//--------------------------------------------------------------
void ofxBeatClock::setVisible_GuiPanel(bool b)
{
	gui.getVisible().set(b);
}

//--------------------------------------------------------------
void ofxBeatClock::exit()
{
	ofLogNotice(__FUNCTION__);

	//rectanglePresetClicker.saveSettings(path_RectanglePresetClicker, path_UserKit_Folder + "/" + path_ControlSettings + "/", false);

	//-

	//default desired settings when it will opened
	PLAYING_State = false;
#ifdef USE_ofxAbletonLink
	if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_MIDI_CLOCK && !ENABLE_LINK_SYNC)
#else
	if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_MIDI_CLOCK)
#endif
		ENABLE_INTERNAL_CLOCK = true;//force to enable one mode, this mode by default

	//-

	saveSettings(path_Global);

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
	ofRemoveListener(params_Advanced.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//--
}

#pragma mark - PLAYER_MANAGER

//--------------------------------------------------------
void ofxBeatClock::start()//only used in internal and link modes
{
	ofLogNotice(__FUNCTION__);

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice(__FUNCTION__) << "START (internal clock)";

		if (!PLAYING_State)
			PLAYING_State = true;
		if (!clockInternal_Active)
			clockInternal_Active = true;

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
	else if (ENABLE_LINK_SYNC && ENABLE_CLOCKS)
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

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice(__FUNCTION__) << "STOP";

		if (PLAYING_State)
			PLAYING_State = false;
		if (clockInternal_Active)
			clockInternal_Active = false;

		//bIsPlaying = false;

		reset_ClockValues();//set gui display text clock to 0:0:0
	}
#ifdef USE_ofxAbletonLink
	else if (ENABLE_LINK_SYNC && ENABLE_CLOCKS)
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

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		PLAYING_State = !PLAYING_State;
		//bIsPlaying = PLAYING_State;

		ofLogNotice(__FUNCTION__) << "PLAY internal: " << PLAYING_State;
	}
#ifdef USE_ofxAbletonLink
	else if (ENABLE_LINK_SYNC && ENABLE_CLOCKS)
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
	//trigs ball drawing and sound ticker
	ofLogVerbose(__FUNCTION__) << "> BeatTick : " << Beat_string;

#ifdef USE_ofxAbletonLink
	if (ENABLE_CLOCKS && (ENABLE_INTERNAL_CLOCK || ENABLE_EXTERNAL_MIDI_CLOCK || ENABLE_LINK_SYNC))
#else
	if (ENABLE_CLOCKS && (ENABLE_INTERNAL_CLOCK || ENABLE_EXTERNAL_MIDI_CLOCK))
#endif
	{
		//-

		//flash beat ball
		BeatTick_TRIG = true;
		lastBeatFlash = _beat;
		//this trigs to draw a flashing circle for a frame only

#ifdef USE_VISUAL_FEEDBACK_FADE
		//bit ball alpha fade setted to transparent, no alpha
		fadeOut_animCounter = 0.0f;
#endif

		//-

		//TODO: 
		//BUG: 
		//why sometimes sound 1st tick its trigged at second beat??

		//play tic on the first beat of a bar
		//set metronome silent when measuring tap tempo engine
		if (ENABLE_sound && !bTap_Running)
		{
			//BUG:
			//sometimes metronome ticks goes on beat 2 instead 1.
			//works better with 0 and 4 detectors, but why?
			//!!
			//we must check better all the beat%limit bc should be the problem!!
			//must check each source clock type what's the starting beat: 0 or 1!!

			//if (_beat == 0 || _beat == 4 )
			if (Beat_current == 1)
			{
				tic.play();
			}
			else
			{
				tac.play();
			}
		}
	}
}

#pragma mark - API

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

#pragma mark - CHANGED LISTENERS

//--------------------------------------------------------------
void ofxBeatClock::Changed_Params(ofAbstractParameter &e) //patch change
{
	string name = e.getName();
	ofLogVerbose(__FUNCTION__) << name << " : " << e;

	//-

	if (false) {}

	//-

	//else if (name == MODE_EditPresetClicker.getName())
	//{
	//	if (MODE_EditPresetClicker.get())
	//	{
	//		rectanglePresetClicker.enableEdit();

	//		// workflow
	//		//SHOW_BackGround_EditPresetClicker = true;
	//		if (!SHOW_Panel_Click) SHOW_Panel_Click = true;
	//	}
	//	else
	//	{
	//		rectanglePresetClicker.disableEdit();
	//	}
	//}

	else if (name == "PLAY")//button for internal only
	{
		ofLogNotice(__FUNCTION__) << "PLAYING_State: " << (PLAYING_State ? "TRUE" : "FALSE");

		//clocks are disabled or using external midi clock. dont need playing, just to be enabled

		if ((ENABLE_EXTERNAL_MIDI_CLOCK || !ENABLE_CLOCKS || !ENABLE_INTERNAL_CLOCK)
			&& PLAYING_State)
		{
			PLAYING_State = false; //skip and restore to false the play button
		}

		else if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
		{
			//play
			if (PLAYING_State == true)
			{
				start();
			}

			//stop
			else
			{
				stop();
			}
		}
	}

	//-

	else if (name == "GLOBAL BPM")
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
		if (ENABLE_LINK_SYNC) LINK_Bpm = BPM_Global;
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

	else if (name == "BAR ms")
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

	else if (name == "INTERNAL")
	{
		ofLogNotice(__FUNCTION__) << "CLOCK INTERNAL: " << ENABLE_INTERNAL_CLOCK;

		if (ENABLE_INTERNAL_CLOCK)
		{
			ENABLE_EXTERNAL_MIDI_CLOCK = false;
#ifdef USE_ofxAbletonLink
			ENABLE_LINK_SYNC = false;
#endif
			//-

			//display text
			clockActive_Type = "INTERNAL";
			clockActive_Info = "";

			//-

			group_INTERNAL->maximize();
		}
		else
		{
			//workflow
			//autostop
			if (PLAYING_State)
				PLAYING_State = false;
			//disable internal
			if (clockInternal_Active)
				clockInternal_Active = false;

			//display text
			Bar_string = "0";
			Beat_string = "0";
			Tick_16th_string = "0";

			if (!ENABLE_INTERNAL_CLOCK)
			{
				//display text
				clockActive_Type = "NONE";
				clockActive_Info = "";
			}

			//-

			group_INTERNAL->minimize();
		}
	}

	//-

	else if (name == "EXTERNAL MIDI")
	{
		ofLogNotice(__FUNCTION__) << "CLOCK EXTERNAL MIDI-IN: " << ENABLE_EXTERNAL_MIDI_CLOCK;

		if (ENABLE_EXTERNAL_MIDI_CLOCK)
		{
			ENABLE_INTERNAL_CLOCK = false;
#ifdef USE_ofxAbletonLink
			ENABLE_LINK_SYNC = false;
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
			clockActive_Type = "EXTERNAL MIDI";
			clockActive_Info = "MIDI PORT: ";
			clockActive_Info += "'" + midiIn.getName() + "'";
			//clockActive_Info += ofToString(midiIn.getPort());

			midiIn_PortName = midiIn.getName();

			//-

			group_EXTERNAL_MIDI->maximize();
		}
		else
		{
			//display text
			Bar_string = "0";
			Beat_string = "0";
			Tick_16th_string = "0";

			if (!ENABLE_EXTERNAL_MIDI_CLOCK)
			{
				clockActive_Type = "NONE";
				clockActive_Info = "";
			}

			//-

			group_EXTERNAL_MIDI->minimize();
		}
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (name == "ABLETON LINK")
	{
		ofLogNotice(__FUNCTION__) << "ABLETON LINK: " << ENABLE_LINK_SYNC;

		if (ENABLE_LINK_SYNC)
		{
			ofLogNotice(__FUNCTION__) << "Add link listeners";
			ofAddListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
			ofAddListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
			ofAddListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

			ENABLE_INTERNAL_CLOCK = false;
			ENABLE_EXTERNAL_MIDI_CLOCK = false;

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
			clockActive_Type = "ABLETON LINK";
			clockActive_Info = "";

			//-

			//workflow
			group_LINK->maximize();
		}
		else
		{
			ofLogNotice(__FUNCTION__) << "Remove link listeners";
			ofRemoveListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
			ofRemoveListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
			ofRemoveListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

			//-

			//workflow
			group_LINK->minimize();
		}
	}
#endif

	//--

	//TODO:
	//should check this better
	else if (name == "ENABLE")
	{
		ofLogNotice(__FUNCTION__) << "ENABLE: " << ENABLE_CLOCKS;

		//workflow
		if (!ENABLE_CLOCKS)
		{
			//internal clock
			if (ENABLE_INTERNAL_CLOCK)
			{
				//autostop
				if (PLAYING_State)
					PLAYING_State = false;
				//disable internal
				if (clockInternal_Active)
					clockInternal_Active = false;
			}
		}

		//-

		//workflow
//#ifdef USE_ofxAbletonLink
//		if (ENABLE_LINK_SYNC)
//		{
//			link.setLinkEnable(ENABLE_CLOCKS);
//		}
//#endif

		//-

		//workflow
		//if none clock mode is selected, we select the internal clock by default
#ifndef USE_ofxAbletonLink
		if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_MIDI_CLOCK)
#else
		if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_MIDI_CLOCK && !ENABLE_LINK_SYNC)
#endif
		{
			ENABLE_INTERNAL_CLOCK = true;//default state / clock selected
		}
	}

	//---

	//external midi clock
	else if (name == "MIDI INPUT PORT")
	{
		ofLogNotice(__FUNCTION__) << "midiIn_Port_SELECT: " << midiIn_Port_SELECT;

		if (midiIn_Port_PRE != midiIn_Port_SELECT)
		{
			midiIn_Port_PRE = midiIn_Port_SELECT;

			ofLogNotice(__FUNCTION__) << "LIST PORTS:";
			midiIn.listInPorts();

			ofLogNotice(__FUNCTION__) << "OPENING PORT: " << midiIn_Port_SELECT;
			setup_MidiIn_Port(midiIn_Port_SELECT);

			midiIn_PortName = midiIn.getName();
			ofLogNotice(__FUNCTION__) << "PORT NAME: " << midiIn_PortName;
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
	else if (name == "RESET BPM")
	{
	ofLogNotice(__FUNCTION__) << "RESET BPM";

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
	else if (name == "DOUBLE")
	{
	ofLogNotice(__FUNCTION__) << "DOUBLE BPM";
		if (BPM_double_TRIG)
		{
			BPM_double_TRIG = false;

			clockInternal_Bpm = clockInternal_Bpm * 2.0f;
		}
	}
	else if (name == "HALF")
	{
	ofLogNotice(__FUNCTION__) << "HALF BPM";
		if (BPM_half_TRIG)
		{
			BPM_half_TRIG = false;

			clockInternal_Bpm = clockInternal_Bpm / 2.0f;
		}
	}
	else if (name == "SHOW ADVANCED")
	{
		ofLogNotice(__FUNCTION__) << "SHOW ADVANCED: " << SHOW_Advanced;

		if (SHOW_Advanced)
		{
			group_Advanced->maximize();
		}
		else
		{
			group_Advanced->minimize();
		}
	}

	//-

	//metronome ticks volume
	else if (name == "VOLUME")
	{
		ofLogNotice(__FUNCTION__) << "VOLUME: " << ofToString(volumeSound, 1);
		tic.setVolume(volumeSound);
		tac.setVolume(volumeSound);
		tapBell.setVolume(volumeSound);
	}

	//-

	//tap
	else if (name == "TAP")
	{
		ofLogNotice(__FUNCTION__) << "TAP BUTTON";

		if ((BPM_Tap_Tempo_TRIG == true) && (ENABLE_INTERNAL_CLOCK))
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


#pragma mark - MIDI_IN_EXTERNAL_CLOCK_MESSAGE

//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage &message)
{
	if (ENABLE_EXTERNAL_MIDI_CLOCK && ENABLE_CLOCKS)
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

#pragma mark - XML_SETTINGS

//--------------------------------------------------------------
void ofxBeatClock::saveSettings(std::string path)
{
	if (DEBUG_Layout) ofxSurfingHelpers::CheckFolder(path);

	//save settings
	ofLogNotice(__FUNCTION__) << path;

	ofXml settings;
	ofSerialize(settings, params_CONTROL);
	settings.save(path + filenameControl);

	ofXml settings2;
	ofSerialize(settings2, params_EXTERNAL_MIDI);
	settings2.save(path + filenameMidiPort);

	ofXml settings3;
	ofSerialize(settings3, params_App);
	settings3.save(path + filenameApp);
}

//--------------------------------------------------------------
void ofxBeatClock::loadSettings(std::string path)
{
	//load settings
	ofLogNotice(__FUNCTION__) << path;

	ofXml settings;
	settings.load(path + filenameControl);
	ofLogNotice(__FUNCTION__) << path + filenameControl << " : " << settings.toString();
	ofDeserialize(settings, params_CONTROL);

	ofXml settings2;
	settings2.load(path + filenameMidiPort);
	ofLogNotice(__FUNCTION__) << path + filenameMidiPort << " : " << settings2.toString();
	ofDeserialize(settings2, params_EXTERNAL_MIDI);

	ofXml settings3;
	settings3.load(path + filenameApp);
	ofLogNotice(__FUNCTION__) << path + filenameApp << " : " << settings3.toString();
	ofDeserialize(settings3, params_App);
}

//--------------------------------------------------------------

#pragma mark - INTERNAL_CLOCK_ofxDawMetro
//not used and callbacks are disabled on audioBuffer timer mode(?)

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int &bar)
{
	ofLogVerbose(__FUNCTION__) << "Internal Clock - BAR: " << bar;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
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
		if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
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
		if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
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

	//if (ENABLE_INTERNAL_CLOCK)
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

#pragma mark - TAP MACHINE

//TODO: implement bpm divider / multiplier x2 x4 /2 /4
//--------------------------------------------------------------
void ofxBeatClock::tap_Trig()
{
	if (ENABLE_INTERNAL_CLOCK)//extra verified, not mandatory
	{
		bTap_Running = true;

		//-

		////workflow
		////disable sound to better flow
		//if (tap_Count == 0 && ENABLE_sound)
		//{
		//	ENABLE_sound = false;
		//	SOUND_wasDisabled = true;
		//}

		//-

		int time = ofGetElapsedTimeMillis();
		tap_Count++;
		ofLogNotice(">TAP<") << "TRIG: " << tap_Count;

		if (tap_Count != 4)
			tic.play();
		else
			tapBell.play();

		tap_Intervals.push_back(time - tap_LastTime);
		tap_LastTime = time;

		if (tap_Count > 3)//4th tap
		{
			tap_Intervals.erase(tap_Intervals.begin());
			tap_AvgBarMillis = accumulate(tap_Intervals.begin(), tap_Intervals.end(), 0) / tap_Intervals.size();

			if (tap_AvgBarMillis == 0)
			{
				tap_AvgBarMillis = 1000;
				ofLogError(__FUNCTION__) << "Divide by 0!";
				ofLogError(__FUNCTION__) << "tap_AvgBarMillis: " << ofToString(tap_AvgBarMillis);
			}

			tap_BPM = 60 * 1000 / (float)tap_AvgBarMillis;

			ofLogNotice(">TAP<") << "NEW Tap BPM: " << tap_BPM;

			//TODO: target bpm could be tweened..

			//-

			tap_Intervals.clear();
			tap_Count = 0;
			bTap_Running = false;

			//-

			//TODO:
			//workflow
			//if (SOUND_wasDisabled)//sound disbler to better flow
			//{
			//	ENABLE_sound = true;
			//	SOUND_wasDisabled = false;
			//}

			//-

			//finally, we set the obtained bpm after the 4 trigged-by-user measurements
			clockInternal_Bpm = tap_BPM;
		}
	}
	else if (tap_Count > 1)
	{
		//TODO:
		//temp update to last interval...
		float val = (float)tap_Intervals[tap_Intervals.size() - 1];
		if (val == 0)
		{
			val = 1000.0f;
			ofLogError(__FUNCTION__) << "Divide by 0!";
			ofLogError(__FUNCTION__) << "val: " << ofToString(val);
		}
		tap_BPM = 60 * 1000 / val;
		ofLogNotice(__FUNCTION__) << "> TAP < : NEW BPM Tap : " << tap_BPM;

		//finally, we set the obtained bpm after the 4 trigged-by-user measurements
		clockInternal_Bpm = tap_BPM;
	}

	//-
}

//--------------------------------------------------------------
void ofxBeatClock::tap_Update()
{
	int time = ofGetElapsedTimeMillis();
	if (tap_Intervals.size() > 0 && (time - tap_LastTime > 3000))
	{
		ofLogNotice(__FUNCTION__) << ">TAP< TIMEOUT: clear tap logs";
		tap_Intervals.clear();

		tap_Count = 0;
		bTap_Running = false;

		//-

		//workflow
		//if (SOUND_wasDisabled)//sound disbler to better flow
		//{
		//	ENABLE_sound = true;
		//	SOUND_wasDisabled = false;
		//}
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

				if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
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