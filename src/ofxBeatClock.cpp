#include "ofxBeatClock.h"

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
	ofSetLogLevel("ofxBeatClock", OF_LOG_NOTICE);

	reset_clockValuesAndStop();

	//--

#pragma mark - EXTERNAL_MIDI_IN_CLOCK

	setup_MIDI_IN_Clock();
	//should be defined before rest of gui to list midi ports being included on gui

	//-

#ifdef USE_ofxAbletonLink
#pragma mark - ABLETON_LINK
	LINK_setup();
#endif

	//--

#pragma mark - CONTROL AND INTERNAL CLOCK

	//--

	//1. DAW METRO. CLOCK SYSTEM

	//add bar/beat/sixteenth listener on demand
	clockInternal.addBeatListener(this);
	clockInternal.addBarListener(this);
	clockInternal.addSixteenthListener(this);

	bpm_ClockInternal.addListener(this, &ofxBeatClock::Changed_ClockInternal_Bpm);
	clockInternal_Active.addListener(this, &ofxBeatClock::Changed_ClockInternal_Active);
	bpm_ClockInternal.set("BPM", BPM_INIT, 30, 300);
	clockInternal_Active.set("Active", false);

	clockInternal.setBpm(bpm_ClockInternal);

	//--

	//2. CONTROL

	params_CONTROL.setName("CONTROL");
	params_CONTROL.add(ENABLE_CLOCKS.set("ENABLE", true));
	params_CONTROL.add(bpm_ClockInternal.set("BPM", BPM_INIT, 30, 300));
	params_CONTROL.add(ENABLE_INTERNAL_CLOCK.set("INTERNAL", false));
	params_CONTROL.add(ENABLE_EXTERNAL_CLOCK.set("EXTERNAL MIDI", true));
#ifdef USE_ofxAbletonLink
	params_CONTROL.add(ENABLE_LINK_SYNC.set("ABLETON LINK", false));
#endif
	params_CONTROL.add(ENABLE_sound.set("SOUND TICK", false));
	params_CONTROL.add(volumeSound.set("VOLUME", 0.5f, 0.f, 1.f));
	params_CONTROL.add(SHOW_Extra.set("SHOW EXTRA", false));

	//TODO:
#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	MODE_AudioBufferTimer.set("MODE AUDIO BUFFER", false);
	params_CONTROL.add(MODE_AudioBufferTimer);
#endif

	params_INTERNAL.setName("INTERNAL CLOCK");
	params_INTERNAL.add(PLAYING_State.set("PLAY", false));
	params_INTERNAL.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
	//TODO:
	//params_INTERNAL.add(bSync_Trig.set("SYNC", false));///trig resync

	params_EXTERNAL.setName("EXTERNAL MIDI CLOCK");
	params_EXTERNAL.add(MIDI_Port_SELECT.set("MIDI PORT", 0, 0, num_MIDI_Ports - 1));
	params_EXTERNAL.add(midiPortName);
	midiPortName.setSerializable(false);

	//--

	//3.

#pragma mark - MONITOR GLOBAL TARGET

	//this smoothed (or maybe slower refreshed than fps) clock will be sended to target sequencer outside the class. see BPM_MIDI_CLOCK_REFRESH_RATE.
	params_BpmTarget.setName("ADVANCED");
	params_BpmTarget.add(BPM_Global.set("GLOBAL BPM", BPM_INIT, 30, 300));
	params_BpmTarget.add(BPM_GLOBAL_TimeBar.set("BAR ms", 60000 / BPM_Global, 100, 2000));
	params_BpmTarget.add(RESET_BPM_Global.set("RESET BPM", false));

	BPM_half_TRIG.set("HALF", false);
	BPM_double_TRIG.set("DOUBLE", false);

	//--

	//4. TAP TEMPO
	bTap_running = false;
	tap_Count = 0;
	tap_Intervals.clear();

	//----

#pragma mark - DEFAULT POSITIONS

	//default config. to be setted after with .setPosition_GuiPanel

	gui_Panel_W = 200;
	gui_Panel_posX = 5;
	gui_Panel_posY = 5;
	gui_Panel_padW = 5;

	//-

	string strFont;
	strFont = "ofxBeatClock/fonts/telegrama_render.otf";
	//strFont = "ofxBeatClock/fonts/mono.ttf";

	fontSmall.load(strFont, 7);
	fontMedium.load(strFont, 10);
	fontBig.load(strFont, 13);

	//-

	//TODO:
	//workaround to anticipate or detect if loading of theme will fail 
	if (!fontSmall.isLoaded())
	{
		ofLogError("ofxBeatClock") << "ERROR LOADING FONT " << strFont;
		ofLogError("ofxBeatClock") << "WILL FAIL TO LOAD JSON THEME TO ofxGuiExteneded. EDIT FILE TO CORRECT FOONT FILENAME!";
		ofLogError("ofxBeatClock") << "theme/theme_bleurgh.json";
	}

	//--

	//TRANSPORT GUI

	setup_Gui();

	//---

	//default positions
	setPosition_GuiPanel(gui_Panel_posX, gui_Panel_posY, gui_Panel_W);

	//--

#pragma mark - LAYOUT_GUI_ELEMENTS

	//MONITOR DEFAULT POSITIONS

	////TODO:
	////improve layout system
	//int sw, sh;
	//sw = 1920;
	//sh = 1080;
	//position_BeatBoxes.set("position_BeatBoxes", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));
	//position_BeatBall.set("position_BeatBall", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));
	//position_ClockInfo.set("position_ClockInfo", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));
	//position_BpmInfo.set("position_BpmInfo", glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(sw, sh));

	//-

	//pos_Global = glm::vec2(5, 720);//TODO:
	setPosition_GuiExtra(5, 720);

	//-

	colorText = ofColor(255, 255);

	//---

	//DRAW BALL TRIGGER

	BeatTick_TRIG = false;

	//---

#pragma mark - SOUNDS

	tic.load("ofxBeatClock/sounds/click1.wav");
	tic.setVolume(1.0f);
	tic.setMultiPlay(false);

	tac.load("ofxBeatClock/sounds/click2.wav");
	tac.setVolume(0.25f);
	tac.setMultiPlay(false);

	tapBell.load("ofxBeatClock/sounds/tapBell.wav");
	tapBell.setVolume(1.0f);
	tapBell.setMultiPlay(false);

	//-

	//TEXT DISPLAY
	Bar_string = "0";
	Beat_string = "0";
	Tick_16th_string = "0";

	//--

	//PATTERN LIMITING. (VS LONG SONG MODE)
	//this only makes sense when syncing to external midi clock (playing a long song) 
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

	//-----

	//startup

	//LOAD LAST SETTINGS

	////add extra settings to group after gui creation, to include in xml settings
	//params_CONTROL.add(bpm_ClockInternal);

	pathSettings = "ofxBeatClock/settings/";//folder to both settings files

	loadSettings(pathSettings);

	//TODO:
	//to ensure gui workflow is update...bc later startup bugs..
	//it seems that changed_parmas callback is not called after loading settings?
	refresh_Gui();

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	setupAudioBuffer(0);
#endif

	//-----
}

//--------------------------------------------------------------
void ofxBeatClock::setup_Gui()
{
	//-

	//GUI POSITION AND SIZE
	//gui_slider_big_h = 40;
	//gui_button_h = 40;
	//gui_w = 200;
	//gui_slider_h = 18;

	//--

	//THEME
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
	//conf_Cont =
	//{
	//	//{"width", gui_w},
	//	//{"direction", "vertical"},
	//};

	//--

	//PANEL
	group_BEAT_CLOCK = gui.addGroup("BEAT CLOCK");
	group_Controls = group_BEAT_CLOCK->addGroup(params_CONTROL);
	group_INTERNAL = group_BEAT_CLOCK->addGroup("INTERNAL CLOCK");
	group_INTERNAL->add(params_INTERNAL);
	group_Controls->getControl("ENABLE")->setConfig(confg_Button);
	group_Controls->getControl("INTERNAL")->setConfig(confg_Button);
	group_Controls->getControl("EXTERNAL MIDI")->setConfig(confg_Button);
#ifdef USE_ofxAbletonLink
	group_Controls->getControl("ABLETON LINK")->setConfig(confg_Button);
#endif
	group_EXTERNAL = group_BEAT_CLOCK->addGroup("EXTERNAL MIDI CLOCK");
	group_EXTERNAL->add(params_EXTERNAL);
	group_BpmTarget = group_BEAT_CLOCK->addGroup(params_BpmTarget);

	//--

	//custom settings
	(group_INTERNAL->getToggle("PLAY"))->setConfig(confg_Button);
	(group_INTERNAL->getToggle("TAP"))->setConfig(confg_Button);
	(group_Controls->getFloatSlider("BPM"))->setConfig(confg_Sliders);
	(group_Controls->getFloatSlider("BPM"))->setPrecision(2);
	(group_BpmTarget->getFloatSlider("GLOBAL BPM"))->setPrecision(2);
	//(group_BpmTarget->getFloatSlider("GLOBAL BPM"))->setConfig(confg_Sliders);
	//(group_BpmTarget->getIntSlider("BAR ms"))->setConfig(confg_Sliders);
	//(group_BpmTarget->getToggle("RESET BPM"))->setConfig(confg_Sliders);
	//TODO:
	//(group_INTERNAL->getToggle("SYNC"))->setConfig(confg_Button);

	//-

	//half & double
	ofxGuiContainer *container_h;
	container_h = group_BpmTarget->addContainer();
	container_h->add<ofxGuiButton>(BPM_half_TRIG);
	container_h->add<ofxGuiButton>(BPM_double_TRIG);

	//-

	//listeners
	ofAddListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_INTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_EXTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_BpmTarget.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//--

	//customize gui theme
	ofLogNotice("ofxBeatClock") << "Trying to load JSON ofxGuiExtended2 Theme...";
	ofLogNotice("ofxBeatClock") << "'theme/theme_bleurgh.json'";
	loadTheme("theme/theme_bleurgh.json");

	//--

	//expand/collapse pannels
	group_Controls->maximize();
	group_BEAT_CLOCK->maximize();
	group_BpmTarget->minimize();

	if (ENABLE_INTERNAL_CLOCK)
	{
		group_INTERNAL->maximize();
	}
	else
	{
		group_INTERNAL->minimize();
	}

	if (ENABLE_EXTERNAL_CLOCK)
	{
		group_EXTERNAL->maximize();
	}
	else
	{
		group_EXTERNAL->minimize();
	}
}

//--------------------------------------------------------------
void ofxBeatClock::refresh_Gui()
{
	//--

	//init state after loaded settings just in case not open correct

	if (ENABLE_INTERNAL_CLOCK)
	{
		ENABLE_EXTERNAL_CLOCK = false;
#ifdef USE_ofxAbletonLink
		ENABLE_LINK_SYNC = false;
#endif
		//TEXT DISPLAY
		clockActive_Type = "INTERNAL";
		clockActive_Info = "";
	}

	else if (ENABLE_EXTERNAL_CLOCK)
	{
		ENABLE_INTERNAL_CLOCK = false;
#ifdef USE_ofxAbletonLink
		ENABLE_LINK_SYNC = false;
#endif

		if (PLAYING_State)
			PLAYING_State = false;
		if (clockInternal_Active)
			clockInternal_Active = false;

		//TEXT DISPLAY
		clockActive_Type = "EXTERNAL MIDI";
		clockActive_Info = "MIDI PORT: ";
		clockActive_Info += "'" + midiIn_CLOCK.getName() + "'";
		//clockActive_Info += ofToString(midiIn_CLOCK.getPort());
	}

#ifdef USE_ofxAbletonLink
	else if (ENABLE_LINK_SYNC)
	{
		ENABLE_INTERNAL_CLOCK = false;
		ENABLE_EXTERNAL_CLOCK = false;

		if (PLAYING_State)
			PLAYING_State = false;
		if (clockInternal_Active)
			clockInternal_Active = false;

		//TEXT DISPLAY
		clockActive_Type = "ABLETON LINK";
		clockActive_Info = "";
	}
#endif

	//--

	//TODO:
	//gui workflow
	if (ENABLE_INTERNAL_CLOCK)
	{
		group_INTERNAL->maximize();
	}
	else
	{
		group_INTERNAL->minimize();
	}

	if (ENABLE_EXTERNAL_CLOCK)
	{
		group_EXTERNAL->maximize();
	}
	else
	{
		group_EXTERNAL->minimize();
	}
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MIDI_IN_Clock()
{
	//EXTERNAL MIDI CLOCK:
	ofSetLogLevel("MIDI PORT", OF_LOG_NOTICE);
	ofLogNotice("MIDI PORT") << "setup_MIDI_IN_Clock";

	ofLogNotice("MIDI PORT") << "LIST PORTS:";
	midiIn_CLOCK.listInPorts();

	num_MIDI_Ports = midiIn_CLOCK.getNumInPorts();
	ofLogNotice("MIDI PORT") << "NUM MIDI-IN PORTS:" << num_MIDI_Ports;

	midiIn_CLOCK_port_OPENED = 0;
	midiIn_CLOCK.openPort(midiIn_CLOCK_port_OPENED);

	ofLogNotice("MIDI PORT") << "connected to MIDI CLOCK IN port: " << midiIn_CLOCK.getPort();

	//TODO: IGNORE SYSX
	midiIn_CLOCK.ignoreTypes(
		true, //sysex  <-- don't ignore timecode messages!
		false, //timing <-- don't ignore clock messages!
		true
	); //sensing

	midiIn_CLOCK.addListener(this);

	bMidiClockRunning = false; //< is the clock sync running?
	MIDI_beats = 0; //< song pos in beats
	MIDI_seconds = 0; //< song pos in seconds, computed from beats
	MIDI_CLOCK_bpm = (double)BPM_INIT; //< song tempo in bpm, computed from clock length
	//bpm_CLOCK.addListener(this, &ofxBeatClock::bpm_CLOCK_Changed);

	//-

	MIDI_beatsInBar.addListener(this, &ofxBeatClock::Changed_MIDI_beatsInBar);

	//-
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MidiIn_Port(int p)
{
	ofLogNotice("MIDI PORT") << "setup_MidiIn_Port";

	midiIn_CLOCK.closePort();
	midiIn_CLOCK_port_OPENED = p;
	midiIn_CLOCK.openPort(midiIn_CLOCK_port_OPENED);
	ofLogNotice("MIDI PORT") << "PORT NAME " << midiIn_CLOCK.getInPortName(p);

	//TEXT DISPLAY
	clockActive_Type = "EXTERNAL MIDI";
	clockActive_Info = "MIDI PORT: ";
	clockActive_Info += "'" + midiIn_CLOCK.getName() + "'";
	//clockActive_Info += ofToString(midiIn_CLOCK.getPort());

	ofLogNotice("MIDI PORT") << "connected to MIDI CLOCK IN port: " << midiIn_CLOCK.getPort();
}

#pragma mark - UPDATE

//--------------------------------------------------------------
void ofxBeatClock::update()
{
	//--

	//TAP

	if (bTap_running) tap_Update();

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
//		ofLogNotice("ofxBeatClock") << "BPM UPDATED" << ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime;
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

#ifdef USE_ofxAbletonLink
//--------------------------------------------------------------
void ofxBeatClock::LINK_update()
{
	if (ENABLE_LINK_SYNC)//not required but prophylactic
	{
		//TEXT DISPLAY
		clockActive_Type = "ABLETON LINK";

		//clockActive_Info + = ""
		clockActive_Info = "BEAT: " + ofToString(link.getBeat(), 2);
		clockActive_Info += "\n";
		clockActive_Info += "PHASE:  " + ofToString(link.getPhase(), 2);
		clockActive_Info += "\n";
		clockActive_Info += "PEERS:  " + ofToString(link.getNumPeers());

		//	<< "bpm:   " << link.getBPM() << std::endl
		//	<< "beat:  " << link.getBeat() << std::endl
		//	<< "phase: " << link.getPhase() << std::endl
		//	<< "peers: " << link.getNumPeers() << std::endl
		//	<< "play?: " << (link.isPlaying() ? "play" : "stop");

		//BPM_Global = (float)link.getBPM();
		//bpm_ClockInternal = BPM_Global;

		Beat_float_current = (float)link.getBeat() + 1.0f;
		Beat_float_string = ofToString(Beat_float_current, 2);

		if (ENABLE_pattern_limits)
		{
			Beat_current = ((int)Beat_float_current) % pattern_BEAT_limit;//limited to 16 beats
		}
		else
		{
			Beat_current = ((int)Beat_float_current);
		}
		Beat_string = ofToString(Beat_current);

		if (Beat_current != Beat_current_PRE)
		{
			cout << "LINK beat changed:" << Beat_current << endl;
			Beat_current_PRE = Beat_current;

			//-

			beatTick_MONITOR(Beat_current);

			//-
		}
	}
}
#endif

#pragma mark - DRAW

//--------------------------------------------------------------
void ofxBeatClock::draw()
{
	//TODO: maybe could improve performance with fbo drawings for all BeatBoxes?

	if (SHOW_Extra)
	{
		draw_BpmInfo(pos_BpmInfo.x, pos_BpmInfo.y);
		draw_ClockInfo(pos_ClockInfo.x, pos_ClockInfo.y);
		draw_BeatBoxes(pos_BeatBoxes_x, pos_BeatBoxes_y, pos_BeatBoxes_width);
		draw_BeatBall(pos_BeatBall_x, pos_BeatBall_y, pos_BeatBall_radius);

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
	draw_Anchor(px, py);

	//sizes and paddings
	int squaresW;//squares beats size
	squaresW = w / 4;//width of each square box
	int xPad = 5;//x margin
	int interline = 15; //text line heigh
	int i = 0;//vertical spacer to accumulate text lines

	//colors
	int colorBoxes = 192;//grey
	int alphaBoxes = 200;//alpha of several above draw fills

	ofPushStyle();

	//----

	//2. BEATS SQUARES

	for (int i = 0; i < 4; i++)
	{
		ofPushStyle();

		//DEFINE SQUARES COLORS:
#ifdef USE_ofxAbletonLink
		if (ENABLE_CLOCKS & (ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK || ENABLE_LINK_SYNC))
#else
		if (ENABLE_CLOCKS & (ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK))
#endif
		{
			if (i <= (Beat_current - 1))
			{
#ifdef USE_ofxAbletonLink
				if (ENABLE_LINK_SYNC || ENABLE_INTERNAL_CLOCK)
					PLAYING_State ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);
#else
				if (ENABLE_INTERNAL_CLOCK)
					PLAYING_State ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);
#endif
				else if (ENABLE_EXTERNAL_CLOCK)
					bMidiClockRunning ? ofSetColor(colorBoxes, alphaBoxes) : ofSetColor(32, alphaBoxes);
			}
			else
				ofSetColor(32, alphaBoxes);
		}

		//-

		else//disabled both modes
		{
			ofSetColor(32, alphaBoxes);//black
		}

		//--

		//DRAW SQUARE

		//filled
		ofFill();
		////adds alpha faded 16th blink, hearth-pulse mode for visual feedback only
		//if (i == Beat_current - 1 && Tick_16th_current % 2 == 0)
		//{
		//	ofSetColor(colorBoxes, alphaBoxes - 64);
		//}
		ofDrawRectRounded(px + i * squaresW, py, squaresW, squaresW, 3.0f);

		//only border
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
	draw_Anchor(px, py);

	//this method draws: tap debug, clock source, clock info, bar-beat-tick16th clock 
	int xPad = 5;
	int interline = 15; //text line heigh
	int i = 0;//vertical spacer to accumulate text lines
	int h = fontBig.getSize();//font spacer to respect px, py anchor

	//--

	//CLOCK INFO:

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
		if (bTap_running)
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

		//external clock
		//midi in port. id number and name
		if (ENABLE_EXTERNAL_CLOCK)
		{
			messageInfo = ofToString(clockActive_Info);
			fontSmall.drawString(messageInfo, xPad, interline * i++);
		}

		//-

		//internal clock
		else if (ENABLE_INTERNAL_CLOCK)
		{
			i++;//empty line
		}

		//-

#ifdef USE_ofxAbletonLink
		//ableton link
		else if (ENABLE_LINK_SYNC)
		{
			messageInfo = ofToString(clockActive_Info);
			fontSmall.drawString(messageInfo, xPad, interline * i++);
		}
#endif
		//-

		else//space in case no clock source is selected to mantain layout "static"
		{
			i++;
		}

		//-

		//1.5 main big clock: [bar:beat:tick16th]

		ofSetColor(colorText);
		i = i + 2;//spacer
		int yPad = 10;
		draw_BigClockTime(xPad, yPad + interline * i);

	}
	ofPopMatrix();

	//--

	//1.4 more debug info
	if (DEBUG_moreInfo)
	{
		i = 4;
		int padding = 250;

		messageInfo = ("BPM: " + ofToString(BPM_Global));
		fontSmall.drawString(messageInfo, padding, interline * i++);

		messageInfo = ("BAR: " + Bar_string);
		fontSmall.drawString(messageInfo, padding, interline * i++);

		messageInfo = ("BEAT: " + Beat_string);
		fontSmall.drawString(messageInfo, padding, interline * i++);

		messageInfo = ("16th: " + Tick_16th_string);
		fontSmall.drawString(messageInfo, padding, interline * i);
	}

	ofPopStyle();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BpmInfo(int px, int py)
{
	draw_Anchor(px, py);

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
	draw_Anchor(px, py);

	ofPushStyle();

	//-

	//tick ball:

	//TODO: alpha tick could be tweened to better heartbeat feeling..
	metronome_ball_radius = _radius;
	int padToBall = 0;
	py += padToBall + metronome_ball_radius;
	metronome_ball_pos.x = px + metronome_ball_radius;
	metronome_ball_pos.y = py;

	ofColor c;//beat 1 mark color
	if (Beat_current == 1)
		c = (ofColor::red);
	else
		c = (ofColor::white);

	//-

	//background black ball
	if (!bTap_running)
	{
		ofSetColor(16, 200);//ball background when not tapping
	}
	else
	{
		//white alpha fade when measuring tapping
		float t = (ofGetElapsedTimeMillis() % 1000);
		float fade = sin(ofMap(t, 0, 1000, 0, 2 * PI));
		ofLogVerbose("ofxBeatClock") << "fade: " << fade << endl;
		int alpha = (int)ofMap(fade, -1.0f, 1.0f, 0, 50) + 205;
		ofSetColor(ofColor(96), alpha);
	}
	ofDrawCircle(metronome_ball_pos.x,
		metronome_ball_pos.y,
		metronome_ball_radius);

	//-

	//beat circle
	fadeOut_animCounter += 4.0f*dt;//fade out timed speed
	fadeOut_animRunning = fadeOut_animCounter <= 1;
	float radius = metronome_ball_radius;
	int alphaMax = 128;
	float alpha = 0.0f;
	ofPushStyle();
	if (fadeOut_animRunning && !bTap_running)
	{
		circlePos.set(metronome_ball_pos.x, metronome_ball_pos.y);

		ofFill();
		alpha = ofMap(fadeOut_animCounter, 0, 1, alphaMax, 0);
		ofSetColor(c, alpha);//faded alpha
		ofDrawCircle(circlePos, fadeOut_animCounter * radius);
	}

	//-

	//border circle
	ofNoFill();
	ofSetLineWidth(2.0f);
	ofSetColor(255, alphaMax * 0.5f + alpha * 0.5f);
	ofDrawCircle(circlePos, radius);
	ofPopStyle();

	//-

	//first beat circle of bar is drawed red. other ones are white
	if (ENABLE_CLOCKS && !bTap_running &&
#ifndef USE_ofxAbletonLink
	(ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK))
#else
		(ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK || ENABLE_LINK_SYNC))
#endif
	{
		if (BeatTick_TRIG)
		{
			BeatTick_TRIG = false;

			if (Beat_current == 1)
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

	int pad = 12;
	std::string timePos =
		ofToString(Bar_string, 3, ' ') + " : " +
		ofToString(Beat_string) + " : " +
		ofToString(Tick_16th_string);

	fontBig.drawString(timePos, x + pad, y);

	ofPopStyle();
}

#pragma mark - SETTERS_FOR_LAYOUT_POSITIONS

//--------------------------------------------------------------
void ofxBeatClock::setPosition_GuiPanel(int _x, int _y, int _w)
{
	gui_Panel_W = _w;
	gui_Panel_posX = _x;
	gui_Panel_posY = _y;
	gui_Panel_padW = 5;

	group_BEAT_CLOCK->setPosition(ofPoint(gui_Panel_posX, gui_Panel_posY));
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
	pos_BeatBoxes_y = pos_ClockInfo.y + 140;

	//beat ball
	pos_BeatBall_radius = 30;
	pos_BeatBall_x = pos_Global.x + pos_BeatBoxes_width * 0.5f - pos_BeatBall_radius;
	pos_BeatBall_y = pos_BeatBoxes_y + 80;
}

//--------------------------------------------------------------
ofPoint ofxBeatClock::getPosition_GuiPanel()
{
	ofPoint p;
	//p = group_BEAT_CLOCK->getShape().getTopLeft();
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
	ofLogNotice("ofxBeatClock") << "exit";

	//ofLogNotice("ofxBeatClock") << ofSetDataPathRoot();
	//ofLogNotice("ofxBeatClock") << ofRestoreWorkingDirectoryToDefault();
	//ofRestoreWorkingDirectoryToDefault();

	//default desired settings when it will opened
	PLAYING_State = false;
	if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_CLOCK)
		ENABLE_INTERNAL_CLOCK = true;

	saveSettings(pathSettings);

	//--

	//external

	midiIn_CLOCK.closePort();
	midiIn_CLOCK.removeListener(this);
	MIDI_beatsInBar.removeListener(this, &ofxBeatClock::Changed_MIDI_beatsInBar);

	//--

	//link
	ofLogNotice("ofxBeatClock") << "remove LINK listeners";
	ofRemoveListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
	ofRemoveListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
	ofRemoveListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

	//--

	//remove more listeners

	ofRemoveListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofRemoveListener(params_BpmTarget.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	bpm_ClockInternal.removeListener(this, &ofxBeatClock::Changed_ClockInternal_Bpm);
	clockInternal_Active.removeListener(this, &ofxBeatClock::Changed_ClockInternal_Active);

	//--
}

#pragma mark - PLAYER_MANAGER

//--------------------------------------------------------
void ofxBeatClock::start()//only used in internal mode
{
	ofLogNotice("ofxBeatClock") << "start()";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "START";

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
		{
			bIsPlaying = true;
		}
	}
	else
	{
		ofLogNotice("ofxBeatClock") << "skip";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::stop()//only used in internal mode
{
	ofLogNotice("ofxBeatClock") << "stop()";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "STOP";

		if (PLAYING_State)
			PLAYING_State = false;
		if (clockInternal_Active)
			clockInternal_Active = false;

		bIsPlaying = false;

		reset_clockValuesAndStop();
	}
	else
	{
		ofLogNotice("ofxBeatClock") << "skip";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::togglePlay()//only used in internal mode
{
	ofLogNotice("ofxBeatClock") << "togglePlay";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "TOOGLE PLAY";

		PLAYING_State = !PLAYING_State;
		bIsPlaying = PLAYING_State;
	}
	else
	{
		ofLogNotice("ofxBeatClock") << "Skip";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::beatTick_MONITOR(int beat)
{
#ifdef USE_ofxAbletonLink
	if (ENABLE_CLOCKS && (ENABLE_INTERNAL_CLOCK || ENABLE_EXTERNAL_CLOCK || ENABLE_LINK_SYNC))
#else
	if (ENABLE_CLOCKS && (ENABLE_INTERNAL_CLOCK || ENABLE_EXTERNAL_CLOCK))
#endif
	{
		ofLogNotice("ofxBeatClock") << ">BEAT: " << Beat_string;

		BeatTick_TRIG = true;

		//bit ball alpha fade setted to transparent, no alpha
		fadeOut_animCounter = 0.0f;

		//-

		if (ENABLE_sound && !bTap_running)
		{
			//play tic on the first beat of a bar
			//TODO: BUG: why tick at second beat?
			//if (beat == 4)
			if (beat == 1)
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
	return BPM_GLOBAL_TimeBar;
}

#pragma mark - CHANGED LISTENERS

//--------------------------------------------------------------
void ofxBeatClock::Changed_Params(ofAbstractParameter &e) //patch change
{
	string name = e.getName();
	ofLogNotice("ofxBeatClock") << "Changed_Params '" << name << "': " << e;

	if (name == "TAP")
	{
		ofLogNotice("ofxBeatClock") << "TAP BUTTON";
		//BPM_Tap_Tempo_TRIG = !BPM_Tap_Tempo_TRIG;

		if ((BPM_Tap_Tempo_TRIG == true) && (ENABLE_INTERNAL_CLOCK))
		{
			BPM_Tap_Tempo_TRIG = false; //TODO: should be button not toggle

			ofLogNotice("ofxBeatClock") << "BPM_Tap_Tempo_TRIG: " << BPM_Tap_Tempo_TRIG;

			tap_Trig();
		}
	}

	//-

	else if (name == "PLAY")
	{
		ofLogNotice("ofxBeatClock") << "PLAYING_State: " << (PLAYING_State ? "TRUE" : "FALSE");

		//clocks are disabled or using external midi clock. dont need playing, just to be enabled

		if ((ENABLE_EXTERNAL_CLOCK || !ENABLE_CLOCKS || !ENABLE_INTERNAL_CLOCK) && PLAYING_State)
		{
			PLAYING_State = false; //skip and restore to false the play button
		}

		else if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
		{
			if (PLAYING_State == true) //play
			{
				start();
			}

			else //stop
			{
				stop();
			}
		}
	}

	//-

	//else if (name == "BPM")
	//{
	//	//ofLogVerbose"ofxBeatClock") << "NEW BPM: " << BPM_Global;
	//}

	else if (name == "RESET BPM")
	{
		ofLogNotice("ofxBeatClock") << "RESET BPM: " << RESET_BPM_Global;

		if (RESET_BPM_Global)
		{
			RESET_BPM_Global = false;//should be a button not toggle
			BPM_Global = 120.00f;
			bpm_ClockInternal = 120.00f;
			clockInternal.setBpm(bpm_ClockInternal);
			ofLogNotice("ofxBeatClock") << "BPM_Global: " << BPM_Global;
		}
	}
	else if (name == "HALF")
	{
		ofLogNotice("ofxBeatClock") << "HALF: " << BPM_half_TRIG;
		if (BPM_half_TRIG)
		{
			//float tempBPM = BPM_Global / 2;
			//cout << "temp bpm: " << tempBPM << endl;
			//BPM_Global = BPM_Global / 2;
			bpm_ClockInternal = bpm_ClockInternal / 2.0f;
			//BPM_Global = tempBPM;
			//bpm_ClockInternal = tempBPM;
		}
	}
	else if (name == "DOUBLE")
	{
		ofLogNotice("ofxBeatClock") << "DOUBLE: " << BPM_double_TRIG;
		if (BPM_double_TRIG)
		{
			//float tempBPM = BPM_Global * 2;
			//cout << "temp bpm: " << tempBPM << endl;
			//BPM_Global = BPM_Global * 2;
			bpm_ClockInternal = bpm_ClockInternal * 2.0f;
			//BPM_Global = tempBPM;
			//bpm_ClockInternal = tempBPM;
		}
	}

	//-

	else if (name == "GLOBAL BPM")
	{
		ofLogVerbose("ofxBeatClock") << "GLOBAL BPM   : " << BPM_Global;

		BPM_GLOBAL_TimeBar = 60000.0f / (float)BPM_Global;//60,000 / BPM = one beat in milliseconds

		//TODO:
		bpm_ClockInternal = BPM_Global;

		//ofLogNotice("ofxBeatClock") << "TIME BEAT : " << BPM_TimeBar << "ms";
		//ofLogNotice("ofxBeatClock") << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";

		//TODO:
#ifdef USE_AUDIO_BUFFER_TIMER_MODE
		samplesPerTick = (sampleRate * 60.0f) / BPM_Global / ticksPerBeat;
		ofLogNotice("ofxBeatClock") << "samplesPerTick: " << samplesPerTick;
#endif
	}

	//-

	else if (name == "BAR ms")
	{
		ofLogVerbose("ofxBeatClock") << "BAR ms: " << BPM_GLOBAL_TimeBar;

		BPM_Global = 60000.0f / (float)BPM_GLOBAL_TimeBar;
	}

	//--

	//source clock type

	else if (name == "INTERNAL")
	{
		ofLogNotice("ofxBeatClock") << "CLOCK INTERNAL: " << ENABLE_INTERNAL_CLOCK;

		if (ENABLE_INTERNAL_CLOCK)
		{
			ENABLE_EXTERNAL_CLOCK = false;
#ifdef USE_ofxAbletonLink
			ENABLE_LINK_SYNC = false;
#endif
			//-

			//TEXT DISPLAY
			clockActive_Type = "INTERNAL";
			clockActive_Info = "";

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

			//TEXT DISPLAY
			Bar_string = "0";
			Beat_string = "0";
			Tick_16th_string = "0";

			if (!ENABLE_INTERNAL_CLOCK)
			{
				//TEXT DISPLAY
				clockActive_Type = "NONE";
				clockActive_Info = "";
			}

			//-

			//workflow
			//if (!ENABLE_EXTERNAL_CLOCK)
			//	ENABLE_EXTERNAL_CLOCK = true;

			group_INTERNAL->minimize();
		}
	}

	//-

	else if (name == "EXTERNAL MIDI")
	{
		ofLogNotice("ofxBeatClock") << "CLOCK EXTERNAL MIDI-IN: " << ENABLE_EXTERNAL_CLOCK;

		if (ENABLE_EXTERNAL_CLOCK)
		{
			ENABLE_INTERNAL_CLOCK = false;
#ifdef USE_ofxAbletonLink
			ENABLE_LINK_SYNC = false;
#endif
			if (PLAYING_State)
				PLAYING_State = false;
			if (clockInternal_Active)
				clockInternal_Active = false;

			//-

			//TEXT DISPLAY
			clockActive_Type = "EXTERNAL MIDI";
			clockActive_Info = "MIDI PORT: ";
			clockActive_Info += "'" + midiIn_CLOCK.getName() + "'";
			//clockActive_Info += ofToString(midiIn_CLOCK.getPort());

			midiPortName = midiIn_CLOCK.getName();

			group_EXTERNAL->maximize();
		}
		else
		{
			//TEXT DISPLAY
			Bar_string = "0";
			Beat_string = "0";
			Tick_16th_string = "0";

			if (!ENABLE_EXTERNAL_CLOCK)
			{
				clockActive_Type = "NONE";
				clockActive_Info = "";
			}

			//-

			//workflow
			//if (!ENABLE_INTERNAL_CLOCK)
			//	ENABLE_INTERNAL_CLOCK = true;

			group_EXTERNAL->minimize();
		}
	}

	//-

#ifdef USE_ofxAbletonLink
	else if (name == "ABLETON LINK")
	{
		ofLogNotice("ofxBeatClock") << "ABLETON LINK: " << ENABLE_LINK_SYNC;
		if (ENABLE_LINK_SYNC)
		{
			ofLogNotice("ofxBeatClock") << "add link listeners";
			ofAddListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
			ofAddListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
			ofAddListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);

			ENABLE_INTERNAL_CLOCK = false;
			ENABLE_EXTERNAL_CLOCK = false;

			if (PLAYING_State)
				PLAYING_State = false;
			if (clockInternal_Active)
				clockInternal_Active = false;

			//TEXT DISPLAY
			clockActive_Type = "ABLETON LINK";
			clockActive_Info = "";

			//TODO:
		}
		else
		{
			ofLogNotice("ofxBeatClock") << "remove link listeners";
			ofRemoveListener(link.bpmChanged, this, &ofxBeatClock::LINK_bpmChanged);
			ofRemoveListener(link.numPeersChanged, this, &ofxBeatClock::LINK_numPeersChanged);
			ofRemoveListener(link.playStateChanged, this, &ofxBeatClock::LINK_playStateChanged);
		}
	}
#endif

	//--

	//TODO:
	//should check this better
	else if (name == "ENABLE")
	{
		ofLogNotice("ofxBeatClock") << "ENABLE: " << ENABLE_CLOCKS;

		//workflow
		if (!ENABLE_CLOCKS)
		{
			if (PLAYING_State)
				PLAYING_State = false;
			if (clockInternal_Active)
				clockInternal_Active = false;
		}

		//-

		//workflow: if none clock mode is selected, selectect internal by default
#ifndef USE_ofxAbletonLink
		else if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_CLOCK)
#else
		else if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_CLOCK && !ENABLE_LINK_SYNC)
#endif
		{
			ENABLE_INTERNAL_CLOCK = true;//default state / clock selected
		}
	}

	//-

	else if (name == "MIDI PORT")
	{
		ofLogNotice("ofxBeatClock") << "MIDI_Port_SELECT: " << MIDI_Port_SELECT;

		if (MIDI_Port_PRE != MIDI_Port_SELECT)
		{
			MIDI_Port_PRE = MIDI_Port_SELECT;

			//ofLogNotice("MIDI PORT") << "SELECTED PORT BY GUI";

			ofLogNotice("MIDI PORT") << "LIST PORTS:";
			midiIn_CLOCK.listInPorts();

			ofLogNotice("MIDI PORT") << "OPENING PORT: " << MIDI_Port_SELECT;
			setup_MidiIn_Port(MIDI_Port_SELECT);

			midiPortName = midiIn_CLOCK.getName();
		}
	}

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	else if (name == "MODE AUDIO BUFFER")
	{
		ofLogNotice("ofxBeatClock") << "AUDIO BUFFER: " << MODE_AudioBufferTimer;

		//workflow: stop to avoid freeze bug
		PLAYING_State = false;

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

	//metronome ticks volume
	else if (name == "VOLUME")
	{
		ofLogNotice("ofxBeatClock") << "VOLUME: " << ofToString(volumeSound, 1);
		tic.setVolume(volumeSound);
		tac.setVolume(volumeSound);
		tapBell.setVolume(volumeSound);
	}

	//TODO:
	//else if (name == "SYNC")
	//{
	//	ofLogNotice("ofxBeatClock") << "bSync_Trig: " << bSync_Trig;
	//	if (bSync_Trig)
	//	{
	//		bSync_Trig = false;
	//		reSync();
	//	}
	//}
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_MIDI_beatsInBar(int &beatsInBar)
{
	ofLogVerbose("ofxBeatClock") << "Changed_MIDI_beatsInBar: " << beatsInBar;

	//only used in midiIn clock sync 
	//this function trigs when any midi tick (beatsInBar) updating, so we need to filter if (we want beat or 16th..) has changed.
	//problem is maybe that beat param refresh when do not changes too, jus because it's accesed..

	if (ENABLE_pattern_limits)
	{
		MIDI_beatsInBar = beatsInBar % pattern_BEAT_limit;
	}

	//-

	if (MIDI_beatsInBar != beatsInBar_PRE)
	{
		ofLogVerbose("ofxBeatClock") << "MIDI-IN CLOCK BEAT TICK! " << beatsInBar;

		//-

		if (ENABLE_pattern_limits)
		{
			Beat_current = MIDI_beatsInBar;
			Bar_current = MIDI_bars % pattern_BAR_limit;
			Beat_string = ofToString(Beat_current);
			Bar_string = ofToString(Bar_current);

			//TODO: should compute too?.. better precision maybe? but we are ignoring now
			Tick_16th_current = 0;
			Tick_16th_string = ofToString(Tick_16th_current);
		}
		else
		{
			Beat_current = MIDI_beatsInBar;
			Bar_current = MIDI_bars;
			Beat_string = ofToString(Beat_current);
			Bar_string = ofToString(Bar_current);

			//TODO: should compute too?.. better precision maybe? but we are ignoring now
			Tick_16th_current = 0;
			Tick_16th_string = ofToString(Tick_16th_current);
		}

		//-

		//TRIG BALL DRAWING AND SOUND TICKER

		beatTick_MONITOR(Beat_current);

		//if (pattern_limits)
		//{
		//    beatsInBar_PRE = beatsInBar;
		//}
		//else
		//{
		beatsInBar_PRE = MIDI_beatsInBar;
		//}
	}
}

//TODO:
//--------------------------------------------------------------
void ofxBeatClock::reSync()
{
	ofLogVerbose("ofxBeatClock") << "reSync";
	//clockInternal.resetTimer();
	//clockInternal.stop();
	//clockInternal.start();
	//PLAYING_State = true;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_ClockInternal_Bpm(float &value)
{
	clockInternal.setBpm(value);
	clockInternal.resetTimer();

	//-

	BPM_Global = value;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_ClockInternal_Active(bool &value)
{
	ofLogVerbose("ofxBeatClock") << "Changed_ClockInternal_Active" << value;

	if (value)
	{
		clockInternal.start();
	}
	else
	{
		clockInternal.stop();
	}
}

#pragma mark - MIDI MESSAGE

//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage &message)
{

	if (ENABLE_EXTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		//1. MIDI CLOCK:

		if ((message.status == MIDI_TIME_CLOCK) ||
			(message.status == MIDI_SONG_POS_POINTER) ||
			(message.status == MIDI_START) ||
			(message.status == MIDI_CONTINUE) ||
			(message.status == MIDI_STOP))
		{

			//midiCLOCK_Message = message;

			//1. MIDI CLOCK

			//update the clock length and song pos in beats
			if (MIDI_clock.update(message.bytes))
			{
				//we got a new song pos
				MIDI_beats = MIDI_clock.getBeats();
				MIDI_seconds = MIDI_clock.getSeconds();

				//-

				MIDI_quarters = MIDI_beats / 4; //convert total # beats to # quarters
				MIDI_bars = (MIDI_quarters / 4) + 1; //compute # of bars
				MIDI_beatsInBar = (MIDI_quarters % 4) + 1; //compute remainder as # TARGET_NOTES_params within the current bar

				//-
			}

			//compute the seconds and bpm

			switch (message.status)
			{

			case MIDI_TIME_CLOCK:
				MIDI_seconds = MIDI_clock.getSeconds();
				MIDI_CLOCK_bpm += (MIDI_clock.getBpm() - MIDI_CLOCK_bpm) / 5.0;

				//average the last 5 bpm values
				//no break here so the next case statement is checked,
				//this way we can set bMidiClockRunning if we've missed a MIDI_START
				//ie. master was running before we started this example

				//-

				BPM_Global = (float)MIDI_CLOCK_bpm;

				//-

				bpm_ClockInternal = (float)MIDI_CLOCK_bpm;

				//-

				//transport control

			case MIDI_START:
			case MIDI_CONTINUE:
				if (!bMidiClockRunning)
				{
					bMidiClockRunning = true;
					ofLogVerbose("ofxBeatClock") << "clock started";
				}
				break;

			case MIDI_STOP:
				if (bMidiClockRunning)
				{
					bMidiClockRunning = false;
					ofLogVerbose("ofxBeatClock") << "clock stopped";
				}
				break;

			default:
				break;
			}
		}
	}
}

#pragma mark - XML_SETTINGS

//--------------------------------------------------------------
void ofxBeatClock::saveSettings(string path)
{
	ofLogNotice("ofxBeatClock") << "saveSettings";
	ofLogNotice("ofxBeatClock") << path;

	//save settings
	ofXml settings;
	ofSerialize(settings, params_CONTROL);
	settings.save(path + filenameControl);

	ofXml settings2;
	ofSerialize(settings2, params_EXTERNAL);
	settings2.save(path + filenameMidiPort);
}

//--------------------------------------------------------------
void ofxBeatClock::loadSettings(string path)
{
	//load settings
	ofLogNotice("ofxBeatClock") << "LOAD SETTINGS";
	ofLogNotice("ofxBeatClock") << path;

	ofXml settings;
	settings.load(path + filenameControl);
	ofLogNotice("ofxBeatClock") << path + filenameControl << " : " << settings.toString();
	ofDeserialize(settings, params_CONTROL);

	ofXml settings2;
	settings2.load(path + filenameMidiPort);
	ofLogNotice("ofxBeatClock") << path + filenameMidiPort << " : " << settings2.toString();
	ofDeserialize(settings2, params_EXTERNAL);

}

//--------------------------------------------------------------

#pragma mark - INTERNAL_CLOCK_ofxDawMetro
//not used and callbacks disabled on audioBuffer timer mode!

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int &bar)
{
	ofLogVerbose("ofxBeatClock") << "Internal Clock - BAR: " << bar;

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
	ofLogVerbose("ofxBeatClock") << "Internal Clock - BEAT: " << beat;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
		{
			if (ENABLE_pattern_limits)
			{
				Beat_current = beat % pattern_BEAT_limit;//limited to 16 beats

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
	ofLogVerbose("ofxBeatClock") << "Internal Clock - 16th: " << sixteenth;

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
void ofxBeatClock::reset_clockValuesAndStop()
{
	ofLogVerbose("ofxBeatClock") << "reset_clockValuesAndStop";

	clockInternal.stop();
	clockInternal.resetTimer();

	Bar_current = 0;
	Beat_current = 0;
	Tick_16th_current = 0;
	Beat_string = ofToString(Beat_current);
	Bar_string = ofToString(Bar_current);
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
		bTap_running = true;

		//-

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
				ofLogError("ofxBeatClock") << "Divide by 0!";
				ofLogError("ofxBeatClock") << "tap_AvgBarMillis: " << ofToString(tap_AvgBarMillis);
			}

			tap_BPM = 60 * 1000 / (float)tap_AvgBarMillis;

			ofLogNotice(">TAP<") << "NEW Tap BPM: " << tap_BPM;

			//TODO: target bpm could be tweened..

			//-

			tap_Intervals.clear();
			tap_Count = 0;
			bTap_running = false;

			//-

			//TODO:
			//if (SOUND_wasDisabled)//sound disbler to better flow
			//{
			//	ENABLE_sound = true;
			//	SOUND_wasDisabled = false;
			//}

			//-

			//SET OBTAINED BPM
			bpm_ClockInternal = tap_BPM;
		}
	}
	else if (tap_Count > 1)
	{
		//TODO
		//temp update to last interval...
		float val = (float)tap_Intervals[tap_Intervals.size() - 1];
		if (val == 0)
		{
			val = 1000;
			ofLogError("ofxBeatClock") << "Divide by 0!";
			ofLogError("ofxBeatClock") << "val: " << ofToString(val);
		}
		tap_BPM = 60 * 1000 / val;
		ofLogNotice("ofxBeatClock") << "> TAP < : NEW BPM Tap : " << tap_BPM;

		//SET OBTAINED BPM
		bpm_ClockInternal = tap_BPM;
	}

	//-
}

//---------------------------
void ofxBeatClock::tap_Update()
{
	int time = ofGetElapsedTimeMillis();
	if (tap_Intervals.size() > 0 && (time - tap_LastTime > 3000))
	{
		ofLogNotice("ofxBeatClock") << ">TAP< TIMEOUT: clear tap logs";
		tap_Intervals.clear();

		tap_Count = 0;
		bTap_running = false;

		//-

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
	bpm_ClockInternal = bpm;
}

//----

//TODO:
#ifdef USE_AUDIO_BUFFER_TIMER_MODE
//clock by audio buffer. not normal threaded clockInternal timer

//--------------------------------------------------------------
void ofxBeatClock::closeAudioBuffer()
{
	ofLogNotice("ofxBeatClock") << "closeAudioBuffer()";
	soundStream.stop();
	soundStream.close();
}

//--------------------------------------------------------------
void ofxBeatClock::setupAudioBuffer(int _device)
{
	ofLogNotice("ofxBeatClock") << "setupAudioBuffer()";

	//--

	//TODO:
	//start the sound stream with a sample rate of 44100 Hz, and a buffer
	//size of 512 samples per audioOut() call
	sampleRate = 44100;
	bufferSize = 512;
	//sampleRate = 48000;
	//bufferSize = 256;

	ofLogNotice("ofxBeatClock") << "OUTPUT devices";
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

	ofLogNotice("ofxBeatClock") << "connecting to device: " << deviceOut;

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
	ofLogNotice("ofxBeatClock") << "samplesPerTick: " << samplesPerTick;

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
					ofLogNotice("ofxBeatClock") << "[audioBuffer] Samples-TICK [" << DEBUG_ticks << "]";
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

						if (Beat_current == 5)
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

						//ofLogNotice("ofxBeatClock") << "[audioBuffer] BEAT: " << Beat_string;
					}
				}
			}
		}
	}
}
#endif