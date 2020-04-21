#include "ofxBeatClock.h"

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
	ofSetLogLevel("ofxBeatClock", OF_LOG_NOTICE);

	Reset_clockValuesAndStop();

	//--

#pragma mark - EXTERNAL_MIDI_IN_CLOCK

	setup_MIDI_CLOCK();
	//should be defined before rest of gui to list midi ports being included on gui

	//--

#pragma mark - CONTROL AND INTERNAL CLOCK

	//--

	//1. DAW METRO. CLOCK SYSTEM

	//add bar/beat/sixteenth listener on demand
	dawMetro.addBeatListener(this);
	dawMetro.addBarListener(this);
	dawMetro.addSixteenthListener(this);

	DAW_bpm.addListener(this, &ofxBeatClock::Changed_DAW_bpm);
	DAW_active.addListener(this, &ofxBeatClock::Changed_DAW_active);
	DAW_bpm.set("BPM", BPM_INIT, 30, 300);
	DAW_active.set("Active", false);

	dawMetro.setBpm(DAW_bpm);

	//--

	//2. CONTROL

	params_CONTROL.setName("CONTROL");
	params_CONTROL.add(ENABLE_CLOCKS.set("ENABLE", true));
	params_CONTROL.add(DAW_bpm.set("BPM", BPM_INIT, 30, 300));
	params_CONTROL.add(ENABLE_INTERNAL_CLOCK.set("INTERNAL", false));
	params_CONTROL.add(ENABLE_EXTERNAL_CLOCK.set("EXTERNAL", true));
	params_CONTROL.add(ENABLE_sound.set("SOUND TICK", false));
	params_CONTROL.add(volumeSound.set("VOLUME", 0.5f, 0.f, 1.f));
	params_CONTROL.add(SHOW_Extra.set("SHOW EXTRA", false));

	//TODO:
#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	MODE_AudioBufferTimer.set("MODE AUDIO BUFFER", false);
	params_CONTROL.add(MODE_AudioBufferTimer);
#endif

	params_INTERNAL.setName("INTERNAL CLOCK");
	params_INTERNAL.add(PLAYER_state.set("PLAY", false));
	params_INTERNAL.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
	//TODO:
	//params_INTERNAL.add(bSync_Trig.set("SYNC", false));///trig resync

	params_EXTERNAL.setName("EXTERNAL CLOCK");
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
	tapCount = 0;
	tapIntervals.clear();

	//----

#pragma mark - DEFAULT POSITIONS

	//default config. to be setted after with .setPosition_Gui

	gui_Panel_W = 200;
	gui_Panel_posX = 5;
	gui_Panel_posY = 5;
	gui_Panel_padW = 5;

	//-

	string strFont;
	strFont = "ofxBeatClock/fonts/telegrama_render.otf";
	//strFont = "ofxBeatClock/fonts/mono.ttf";

	TTF_small.load(strFont, 7);
	TTF_medium.load(strFont, 10);
	TTF_big.load(strFont, 13);

	//-

	//TODO:
	//workaround to anticipate or detect if loading of theme will fail 
	if (!TTF_small.isLoaded())
	{
		ofLogError("ofxBeatClock") << "ERROR LOADING FONT " << strFont;
		ofLogError("ofxBeatClock") << "WILL FAIL TO LOAD JSON THEME TO ofxGuiExteneded. EDIT FILE TO CORRECT FOONT FILENAME!";
		ofLogError("ofxBeatClock") << "theme/theme_bleurgh.json";
	}

	//--

	//TRANSPORT GUI

	setup_Gui();

	//-

	//default positions
	setPosition_Gui(gui_Panel_posX, gui_Panel_posY, gui_Panel_W);

	//--

#pragma mark - DRAW STUFF

	//MONITOR DEFAULT POSITIONS
	//will be used if they are not redefined

	//beat boxes
	pos_BeatBoxes_y = 640;
	pos_BeatBoxes_x = 5;
	pos_BeatBoxes_w = 200;

	//beat ball
	pos_BeatBall_y = 830;
	pos_BeatBall_x = 500;
	pos_BeatBall_w = 30;

	setPosition_BeatBoxes(pos_BeatBoxes_x, pos_BeatBoxes_y, pos_BeatBoxes_w);
	setPosition_BeatBall(pos_BeatBall_x, pos_BeatBall_y, pos_BeatBall_w);

	//-

	//DRAW BALL TRIGGER

	TRIG_TICK = false;

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
	BPM_bar_str = "0";
	BPM_beat_str = "0";
	BPM_16th_str = "0";

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
	//params_CONTROL.add(DAW_bpm);

	pathSettings = "ofxBeatClock/settings/";//folder to both settings files

	loadSettings(pathSettings);

	//TODO:
	//to ensure gui workflow is update...bc later startup bugs..
	//it seems that changed_parmas callback is not called after loading settings?
	refresh_Gui();

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	setupAudioBuffer(3);
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
	group_Controls->getControl("EXTERNAL")->setConfig(confg_Button);
	group_EXTERNAL = group_BEAT_CLOCK->addGroup("EXTERNAL CLOCK");
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

		//TEXT DISPLAY
		BPM_input_str = "INTERNAL";
		BPM_name_str = "";
	}

	else if (ENABLE_EXTERNAL_CLOCK)
	{
		ENABLE_INTERNAL_CLOCK = false;

		if (PLAYER_state)
			PLAYER_state = false;
		if (DAW_active)
			DAW_active = false;

		//TEXT DISPLAY
		BPM_input_str = "EXTERNAL";
		BPM_name_str = "MIDI PORT: ";
		//BPM_name_str += ofToString(midiIn_CLOCK.getPort());
		BPM_name_str += "'" + midiIn_CLOCK.getName() + "'";
	}

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
void ofxBeatClock::setup_MIDI_CLOCK()
{
	//EXTERNAL MIDI CLOCK:
	ofSetLogLevel("MIDI PORT", OF_LOG_NOTICE);
	ofLogNotice("MIDI PORT") << "setup_MIDI_CLOCK";

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

	clockRunning = false; //< is the clock sync running?
	MIDI_beats = 0; //< song pos in beats
	MIDI_seconds = 0; //< song pos in seconds, computed from beats
	MIDI_CLOCK_bpm = (double)BPM_INIT; //< song tempo in bpm, computed from clock length
	//bpm_CLOCK.addListener(this, &ofxBeatClock::bpm_CLOCK_Changed);

	//-

	MIDI_beatsInBar.addListener(this, &ofxBeatClock::Changed_MIDI_beatsInBar);

	//-
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MIDI_PORT(int p)
{
	ofLogNotice("MIDI PORT") << "setup_MIDI_PORT";

	midiIn_CLOCK.closePort();
	midiIn_CLOCK_port_OPENED = p;
	midiIn_CLOCK.openPort(midiIn_CLOCK_port_OPENED);
	ofLogNotice("MIDI PORT") << "PORT NAME " << midiIn_CLOCK.getInPortName(p);

	//TEXT DISPLAY
	BPM_input_str = "EXTERNAL";
	BPM_name_str = "MIDI PORT: ";
	//BPM_name_str += ofToString(midiIn_CLOCK.getPort());
	BPM_name_str += "'" + midiIn_CLOCK.getName() + "'";

	ofLogNotice("MIDI PORT") << "connected to MIDI CLOCK IN port: " << midiIn_CLOCK.getPort();
}

#pragma mark - UPDATE

//--------------------------------------------------------------
void ofxBeatClock::update()
{
	//--

	//TAP

	if (bTap_running)
		Tap_update();

	//--

	//TODO:
	//smooth clock
	//MIDI CLOCK

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

	//--

	//BPM ENGINE:

	//BPM_LAST_Tick_Time_ELLAPSED_PRE = BPM_LAST_Tick_Time_ELLAPSED;
	//BPM_LAST_Tick_Time_ELLAPSED = ofGetElapsedTimeMillis() - BPM_LAST_Tick_Time_LAST;//test
	//BPM_LAST_Tick_Time_LAST = ofGetElapsedTimeMillis();//test
	//ELLAPSED_diff = BPM_LAST_Tick_Time_ELLAPSED_PRE - BPM_LAST_Tick_Time_ELLAPSED;

	//--

	ofSoundUpdate();

	//-
}

#pragma mark - DRAW

//--------------------------------------------------------------
void ofxBeatClock::draw()
{
	//TODO: maybe could improve performance with fbo drawings for all BeatBoxes

	if (SHOW_Extra)
	{
		//beat boxes & text

		drawBeatBoxes(pos_BeatBoxes_x, pos_BeatBoxes_y, pos_BeatBoxes_w);

		//-

		//beat ball (visual feedback)

		if (bBallAutoPos)
		{
			//above squares auto position
			draw_BeatBall(pos_BeatBoxes_x + 0.5f*pos_BeatBoxes_w - pos_BeatBall_w,
				pos_BeatBall_y, pos_BeatBall_w);
		}
		else
		{
			//custom position mode
			draw_BeatBall(pos_BeatBall_x, pos_BeatBall_y, pos_BeatBall_w);
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::drawBeatBoxes(int px, int py, int w)///draws text info and boxes
{
	//TODO:
	//should make better all this "layout system"
	ofPushStyle();

	ofColor c;
	c = ofColor(255, 255);

	int colorBoxes = 192;//grey

	//heigh paddings
	int padToSquares = 0;
	int padToBigTime = 0;
	int squaresW;//squares beats size
	int interline = 15; //line heigh
	bool SHOW_moreInfo = false;//more debug
	int i = 0;

	int alpha = 200;
	squaresW = w / 4;

	//--

	//1. TEXT INFO:

	ofSetColor(c);
	int xPad = 5;

	//-

	ofPushMatrix();
	{
		ofTranslate(px, py);

		//bpm
		//i = 3;//spacer
		TTF_message = "BPM: " + ofToString(BPM_Global.get(), 2);
		TTF_big.drawString(TTF_message, xPad, interline * i++);
		//TTF_big.drawString(TTF_message, xPad, 0);
		//i++;//spacer

		//tap mode
		int speed = 10;
		bool b = (ofGetFrameNum() % (2 * speed) < speed);
		ofSetColor(255, b ? 128 : 255);
		if (bTap_running)
		{
			TTF_message = "TAP " + ofToString(ofMap(tapCount, 1, 4, 3, 0));
		}
		else
		{
			TTF_message = "";
		}
		i++;
		TTF_big.drawString(TTF_message, 3 * xPad, interline * i++);
		i++;//spacer

		//-

		ofSetColor(c);

		//clock
		TTF_message = "CLOCK: " + BPM_input_str;
		TTF_small.drawString(TTF_message, xPad, interline * i++);

		//midi port
		if (ENABLE_EXTERNAL_CLOCK)
		{
			TTF_message = ofToString(BPM_name_str);
			TTF_small.drawString(TTF_message, xPad, interline * i++);
		}

		//-

		py = py + (interline * (i + 2));//accumulate heigh distance

		//-

		if (SHOW_moreInfo)
		{
			//ofTranslate(paddingAlign, 7);

			TTF_message = ("BPM: " + ofToString(BPM_Global));
			TTF_small.drawString(TTF_message, 4 * xPad, interline * i++);

			TTF_message = ("BAR: " + BPM_bar_str);
			TTF_small.drawString(TTF_message, 4 * xPad, interline * i++);

			TTF_message = ("BEAT: " + BPM_beat_str);
			TTF_small.drawString(TTF_message, 4 * xPad, interline * i++);

			TTF_message = ("16th: " + BPM_16th_str);
			TTF_small.drawString(TTF_message, 4 * xPad, interline * i++);

			//-

			py = py + (interline * i);//acumulate heigh distance
		}

		//-
	}
	ofPopMatrix();//TODO: pop could be at function ending

	//--

	//py += padToBigTime;
	draw_BigClockTime(px, py);

	py = py + (TTF_big.getSize());//acumulate heigh distance

	ofPopStyle();

	//--

	//2. BEATS SQUARES

	py += padToSquares;

	for (int i = 0; i < 4; i++)
	{
		ofPushStyle();

		//--

		//DEFINE SQUARE COLORS:

		if (ENABLE_CLOCKS & (ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK))
		{
			if (i <= (BPM_beat_current - 1))
			{
				if (ENABLE_INTERNAL_CLOCK)
					PLAYER_state ? ofSetColor(colorBoxes, alpha) : ofSetColor(32, alpha);

				else if (ENABLE_EXTERNAL_CLOCK)
					clockRunning ? ofSetColor(colorBoxes, alpha) : ofSetColor(32, alpha);
			}
			else
				ofSetColor(32, alpha);
		}

		//-

		else//disabled both modes
		{
			ofSetColor(32, alpha);//black
		}

		//--

		//DRAW SQUARE

		//filled
		ofFill();

		////faded hearth-pulse alpha mode
		//if (i == BPM_beat_current - 1 && BPM_16th_current % 2 == 0)
		//{
		//	ofSetColor(colorBoxes, alpha - 64);
		//}

		ofDrawRectRounded(px + i * squaresW, py, squaresW, squaresW, 3.0f);
		//ofDrawRectangle(px + i * squaresW, py, squaresW, squaresW); 

		//border
		ofNoFill();
		ofSetColor(8, alpha);
		ofSetLineWidth(2.0f);

		ofDrawRectRounded(px + i * squaresW, py, squaresW, squaresW, 3.0f);
		//ofDrawRectangle(px + i * squaresW, py, squaresW, squaresW);

		ofPopStyle();

		//--
	}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BeatBall(int px, int py, int w)
{
	//3. TICK BALL:

	//TODO: alpha tick could be tweened to better heartbeat feeling..
	metronome_ball_radius = w;
	int padToBall = 0;

	py += padToBall + metronome_ball_radius;
	metronome_ball_pos.x = px + metronome_ball_radius;
	metronome_ball_pos.y = py;

	ofPushStyle();

	ofColor c;
	if (BPM_beat_current == 1)
		c = (ofColor::red);
	else
		c = (ofColor::white);

	//bg black ball
	if (!bTap_running)
	{
		ofSetColor(16, 200); //ball background when not tapping
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
	animCounter += 4.0f*dt;//speed
	animRunning = animCounter <= 1;
	float radius = metronome_ball_radius;
	int alphaMax = 128;
	float alpha = 0.0f;
	ofPushStyle();
	if (animRunning && !bTap_running)
	{
		circlePos.set(metronome_ball_pos.x, metronome_ball_pos.y);

		ofFill();
		alpha = ofMap(animCounter, 0, 1, alphaMax, 0);
		ofSetColor(c, alpha);//faded alpha
		ofDrawCircle(circlePos, animCounter * radius);
	}

	//border circle
	ofNoFill();
	ofSetLineWidth(2.0f);
	ofSetColor(255, alphaMax * 0.5f + alpha * 0.5f);
	ofDrawCircle(circlePos, radius);
	ofPopStyle();

	//-

	//we should disable this maybe...?

	if (ENABLE_CLOCKS && !bTap_running &&
		(ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK))
	{
		if (TRIG_TICK)
		{
			TRIG_TICK = false;

			if (BPM_beat_current == 1)
				ofSetColor(ofColor::red);
			else
				ofSetColor(ofColor::white);

			ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
		}
	}

	ofPopStyle();

	//-
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BigClockTime(int x, int y)
{
	//BIG LETTERS:
	{
		ofPushStyle();

		//ofSetColor(192);
		ofSetColor(ofColor::white);

		int pad = 12;
		std::string timePos =
			ofToString(BPM_bar_str, 3, ' ') + " : " +
			ofToString(BPM_beat_str) + " : " +
			ofToString(BPM_16th_str);

		TTF_big.drawString(timePos, x + pad, y);

		ofPopStyle();
	}
}

#pragma mark - SET POSITIONS

//--------------------------------------------------------------
void ofxBeatClock::setPosition_Gui(int _x, int _y, int _w)
{
	gui_Panel_W = _w;
	gui_Panel_posX = _x;
	gui_Panel_posY = _y;
	gui_Panel_padW = 5;

	group_BEAT_CLOCK->setPosition(ofPoint(gui_Panel_posX, gui_Panel_posY));
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_Gui_ALL(int _x, int _y, int _w)
{
	gui_Panel_W = _w;
	gui_Panel_posX = _x;
	gui_Panel_posY = _y;
	gui_Panel_padW = 5;

	//set positions and sizes
	int w;
	w = _w;
	int pad_Clock = 15;
	int pos_Clock_x = w + pad_Clock;

	//setPosition_Gui(pos_Clock_x, 0, w);
	setPosition_Gui(gui_Panel_posX, gui_Panel_posY, w);

	setPosition_BeatBoxes(pos_Clock_x, 500, w);
	setPosition_BeatBall(pos_Clock_x + 50, 650, 25);
}

//--------------------------------------------------------------
ofPoint ofxBeatClock::getPosition_Gui()
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
	pos_BeatBoxes_w = w;
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_BeatBall(int x, int y, int w)
{
	pos_BeatBall_x = x;
	pos_BeatBall_y = y;
	pos_BeatBall_w = w;
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_TextBpm(int x, int y)
{
	pos_TextBpm = glm::vec2(x, y);
}

//--------------------------------------------------------------
void ofxBeatClock::set_BeatBall_visible(bool b)
{
	SHOW_Extra = b;
}

//--------------------------------------------------------------
void ofxBeatClock::set_Gui_visible(bool b)
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
	PLAYER_state = false;
	if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_CLOCK)
		ENABLE_INTERNAL_CLOCK = true;

	saveSettings(pathSettings);

	//--

	//EXTERNAL

	midiIn_CLOCK.closePort();
	midiIn_CLOCK.removeListener(this);
	MIDI_beatsInBar.removeListener(this, &ofxBeatClock::Changed_MIDI_beatsInBar);

	//--

	//remove listeners

	ofRemoveListener(params_CONTROL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofRemoveListener(params_BpmTarget.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	DAW_bpm.removeListener(this, &ofxBeatClock::Changed_DAW_bpm);
	DAW_active.removeListener(this, &ofxBeatClock::Changed_DAW_active);

	//--
}

#pragma mark - PLAYER MANAGER

//--------------------------------------------------------
void ofxBeatClock::PLAYER_START()//only used in internal mode
{
	ofLogNotice("ofxBeatClock") << "PLAYER_START()";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "START";

		if (!PLAYER_state)
			PLAYER_state = true;
		if (!DAW_active)
			DAW_active = true;

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
void ofxBeatClock::PLAYER_STOP()//only used in internal mode
{
	ofLogNotice("ofxBeatClock") << "PLAYER_STOP()";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "STOP";

		if (PLAYER_state)
			PLAYER_state = false;
		if (DAW_active)
			DAW_active = false;

		bIsPlaying = false;

		Reset_clockValuesAndStop();
	}
	else
	{
		ofLogNotice("ofxBeatClock") << "skip";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_TOGGLE()//only used in internal mode
{
	ofLogNotice("ofxBeatClock") << "PLAYER_TOGGLE";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "TOOGLE PLAY";

		PLAYER_state = !PLAYER_state;
		bIsPlaying = PLAYER_state;
	}
	else
	{
		ofLogNotice("ofxBeatClock") << "skip";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::beatTick_MONITOR(int beat)
{
	if (ENABLE_CLOCKS && (ENABLE_INTERNAL_CLOCK || ENABLE_EXTERNAL_CLOCK))
	{
		ofLogNotice("ofxBeatClock") << ">BEAT: " << BPM_beat_str;

		TRIG_TICK = true;

		//bit ball alpha fade
		animCounter = 0.0f;

		//-

		if (ENABLE_sound && !bTap_running)
		{
			//play tic on the first beat of a bar
			if (beat == 1)
				//TODO: BUG: why tick at second beat?
				//if (beat == 4)
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
float ofxBeatClock::get_BPM()
{
	return BPM_Global;
}

//--------------------------------------------------------------
int ofxBeatClock::get_TimeBar()
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

			Tap_Trig();
		}
	}

	//-

	else if (name == "PLAY")
	{
		ofLogNotice("ofxBeatClock") << "PLAYER_state: " << (PLAYER_state ? "TRUE" : "FALSE");

		//clocks are disabled or using external midi clock. dont need playing, just to be enabled

		if ((ENABLE_EXTERNAL_CLOCK || !ENABLE_CLOCKS || !ENABLE_INTERNAL_CLOCK) && PLAYER_state)
		{
			PLAYER_state = false; //skip and restore to false the play button
		}

		else if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
		{
			if (PLAYER_state == true) //play
			{
				PLAYER_START();
			}

			else //stop
			{
				PLAYER_STOP();
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
			DAW_bpm = 120.00f;
			dawMetro.setBpm(DAW_bpm);
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
			DAW_bpm = DAW_bpm / 2.0f;
			//BPM_Global = tempBPM;
			//DAW_bpm = tempBPM;
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
			DAW_bpm = DAW_bpm * 2.0f;
			//BPM_Global = tempBPM;
			//DAW_bpm = tempBPM;
		}
	}

	//-

	else if (name == "GLOBAL BPM")
	{
		ofLogVerbose("ofxBeatClock") << "GLOBAL BPM   : " << BPM_Global;

		BPM_GLOBAL_TimeBar = 60000.0f / (float)BPM_Global;//60,000 / BPM = one beat in milliseconds

		//TODO:
		DAW_bpm = BPM_Global;

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

	//source clock

	else if (name == "INTERNAL")
	{
		ofLogNotice("ofxBeatClock") << "CLOCK INTERNAL: " << ENABLE_INTERNAL_CLOCK;

		if (ENABLE_INTERNAL_CLOCK)
		{
			ENABLE_EXTERNAL_CLOCK = false;

			//-

			//TEXT DISPLAY
			BPM_input_str = "INTERNAL";
			BPM_name_str = "";

			group_INTERNAL->maximize();
		}
		else
		{
			if (PLAYER_state)
				PLAYER_state = false;
			if (DAW_active)
				DAW_active = false;

			//TEXT DISPLAY
			BPM_bar_str = "0";
			BPM_beat_str = "0";
			BPM_16th_str = "0";

			if (!ENABLE_INTERNAL_CLOCK)
			{
				//TEXT DISPLAY
				BPM_input_str = "NONE";
				BPM_name_str = "";
			}

			//-

			if (!ENABLE_EXTERNAL_CLOCK)
				ENABLE_EXTERNAL_CLOCK = true;

			group_INTERNAL->minimize();
		}
	}

	//-

	else if (name == "EXTERNAL")
	{
		ofLogNotice("ofxBeatClock") << "CLOCK EXTERNAL MIDI-IN: " << ENABLE_EXTERNAL_CLOCK;

		if (ENABLE_EXTERNAL_CLOCK)
		{
			ENABLE_INTERNAL_CLOCK = false;

			if (PLAYER_state)
				PLAYER_state = false;
			if (DAW_active)
				DAW_active = false;

			//-

			//TEXT DISPLAY
			BPM_input_str = "EXTERNAL";
			BPM_name_str = "MIDI PORT: ";
			//BPM_name_str += ofToString(midiIn_CLOCK.getPort());
			BPM_name_str += "'" + midiIn_CLOCK.getName() + "'";

			midiPortName = midiIn_CLOCK.getName();

			group_EXTERNAL->maximize();
		}
		else
		{
			//TEXT DISPLAY
			BPM_bar_str = "0";
			BPM_beat_str = "0";
			BPM_16th_str = "0";

			if (!ENABLE_EXTERNAL_CLOCK)
			{
				BPM_input_str = "NONE";
				BPM_name_str = "";
			}

			//-

			if (!ENABLE_INTERNAL_CLOCK)
				ENABLE_INTERNAL_CLOCK = true;

			group_EXTERNAL->minimize();
		}
	}

	//--

	else if (name == "ENABLE")
	{
		ofLogNotice("ofxBeatClock") << "ENABLE: " << ENABLE_CLOCKS;

		if (!ENABLE_CLOCKS)
		{
			if (PLAYER_state)
				PLAYER_state = false;
			if (DAW_active)
				DAW_active = false;
		}

		//-

		//ENABLE CLOCKS true
		else if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_CLOCK)
		{
			ENABLE_INTERNAL_CLOCK = true;//default state when enabling clocks
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
			setup_MIDI_PORT(MIDI_Port_SELECT);

			midiPortName = midiIn_CLOCK.getName();
		}
	}

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	else if (name == "MODE AUDIO BUFFER")
	{
		ofLogNotice("ofxBeatClock") << "AUDIO BUFFER: " << MODE_AudioBufferTimer;

		//workflow: stop to avoid freeze bug
		PLAYER_state = false;

		//this is required only if we want to use both internal modes together
		//(dawMetro and audioBuffer) 
		//but it seems that causes problems when connecting/disconnectings ports..
		//if (MODE_AudioBufferTimer)
		//{
		//	setupAudioBuffer(3);

		//	//if (dawMetro.is)//?
		//	//disable daw metro
		//	dawMetro.removeBeatListener(this);
		//	dawMetro.removeBarListener(this);
		//	dawMetro.removeSixteenthListener(this);
		//}
		//else
		//{
		//	closeAudioBuffer();

		//	//enable daw metro
		//	dawMetro.addBeatListener(this);
		//	dawMetro.addBarListener(this);
		//	dawMetro.addSixteenthListener(this);
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
			BPM_beat_current = MIDI_beatsInBar;
			BPM_beat_str = ofToString(BPM_beat_current);
			BPM_bar_current = MIDI_bars % pattern_BAR_limit;
			BPM_bar_str = ofToString(BPM_bar_current);
			BPM_16th_current = 0;//TODO: should compute too.. but we are ignoring now
			BPM_16th_str = ofToString(BPM_16th_current);
		}
		else
		{
			BPM_beat_current = MIDI_beatsInBar;
			BPM_beat_str = ofToString(BPM_beat_current);
			BPM_bar_current = MIDI_bars;
			BPM_bar_str = ofToString(BPM_bar_current);
			BPM_16th_current = 0;//TODO: should compute too..
			BPM_16th_str = ofToString(BPM_16th_current);
		}

		//-

		//TRIG BALL DRAWING AND SOUND TICKER

		beatTick_MONITOR(BPM_beat_current);

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
	//dawMetro.resetTimer();
	//dawMetro.stop();
	//dawMetro.start();
	//PLAYER_state = true;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_DAW_bpm(float &value)
{
	dawMetro.setBpm(value);
	dawMetro.resetTimer();

	//-

	BPM_Global = value;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_DAW_active(bool &value)
{
	ofLogVerbose("ofxBeatClock") << "Changed_DAW_active" << value;

	if (value)
	{
		dawMetro.start();
	}
	else
	{
		dawMetro.stop();
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
				//this way we can set clockRunning if we've missed a MIDI_START
				//ie. master was running before we started this example

				//-

				BPM_Global = (float)MIDI_CLOCK_bpm;

				//-

				DAW_bpm = (float)MIDI_CLOCK_bpm;

				//-

				//transport control

			case MIDI_START:
			case MIDI_CONTINUE:
				if (!clockRunning)
				{
					clockRunning = true;
					ofLogVerbose("ofxBeatClock") << "clock started";
				}
				break;

			case MIDI_STOP:
				if (clockRunning)
				{
					clockRunning = false;
					ofLogVerbose("ofxBeatClock") << "clock stopped";
				}
				break;

			default:
				break;
			}
		}
	}
}

#pragma mark - XML SETTINGS

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

#pragma mark - INTERNAL_CLOCK_-_DAW_METRO
//not used and callbacks disabled on audioBuffer timer mode!

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int &bar)
{
	ofLogVerbose("ofxBeatClock") << "DAW METRO - BAR: " << bar;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
		{
			if (ENABLE_pattern_limits)
			{
				BPM_bar_current = bar % pattern_BAR_limit;
			}
			else
			{
				BPM_bar_current = bar;
			}

			BPM_bar_str = ofToString(BPM_bar_current);
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::onBeatEvent(int &beat)
{
	ofLogVerbose("ofxBeatClock") << "DAW METRO - BEAT: " << beat;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{
		if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
		{
			if (ENABLE_pattern_limits)
			{
				BPM_beat_current = beat % pattern_BEAT_limit;//limited to 16 beats

			}
			else
			{
				BPM_beat_current = beat;
			}

			BPM_beat_str = ofToString(BPM_beat_current);

			//-

			beatTick_MONITOR(BPM_beat_current);

			//-
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::onSixteenthEvent(int &sixteenth)
{
	ofLogVerbose("ofxBeatClock") << "DAW METRO - 16th: " << sixteenth;

#ifdef USE_AUDIO_BUFFER_TIMER_MODE
	if (!MODE_AudioBufferTimer)
#endif
	{

		if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
		{
			BPM_16th_current = sixteenth;
			BPM_16th_str = ofToString(BPM_16th_current);
		}
	}
}

//-

//--------------------------------------------------------------
void ofxBeatClock::Reset_clockValuesAndStop()
{
	ofLogVerbose("ofxBeatClock") << "Reset_clockValuesAndStop";

	dawMetro.stop();
	dawMetro.resetTimer();

	BPM_bar_current = 0;
	BPM_beat_current = 0;
	BPM_16th_current = 0;
	BPM_beat_str = ofToString(BPM_beat_current);
	BPM_bar_str = ofToString(BPM_bar_current);
	BPM_16th_str = ofToString(BPM_16th_current);
}

//--------------------------------------------------------------

#pragma mark - TAP MACHINE

//TODO: implement bpm divider / multiplier x2 x4 /2 /4
//--------------------------------------------------------------
void ofxBeatClock::Tap_Trig()
{
	if (ENABLE_INTERNAL_CLOCK)
	{
		bTap_running = true;

		//-

		////disable sound to better flow
		//if (tapCount == 0 && ENABLE_sound)
		//{
		//	ENABLE_sound = false;
		//	SOUND_wasDisabled = true;
		//}

		//-

		int time = ofGetElapsedTimeMillis();
		tapCount++;
		ofLogNotice(">TAP<") << "TRIG: " << tapCount;

		if (tapCount != 4)
			tic.play();
		else
			tapBell.play();

		tapIntervals.push_back(time - lastTime);
		lastTime = time;

		if (tapCount > 3)//4th tap
		{
			tapIntervals.erase(tapIntervals.begin());
			avgBarMillis = accumulate(tapIntervals.begin(), tapIntervals.end(), 0) / tapIntervals.size();

			if (avgBarMillis == 0)
			{
				avgBarMillis = 1000;
				ofLogError("ofxBeatClock") << "Divide by 0!";
				ofLogError("ofxBeatClock") << "avgBarMillis: " << ofToString(avgBarMillis);
			}

			Tap_BPM = 60 * 1000 / (float)avgBarMillis;

			ofLogNotice(">TAP<") << "NEW Tap BPM: " << Tap_BPM;

			//TODO: target bpm could be tweened..

			//-

			tapIntervals.clear();
			tapCount = 0;
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
			DAW_bpm = Tap_BPM;
		}
	}
	else if (tapCount > 1)
	{
		//TODO
		//temp update to last interval...
		float val = (float)tapIntervals[tapIntervals.size() - 1];
		if (val == 0)
		{
			val = 1000;
			ofLogError("ofxBeatClock") << "Divide by 0!";
			ofLogError("ofxBeatClock") << "val: " << ofToString(val);
		}
		Tap_BPM = 60 * 1000 / val;
		ofLogNotice("ofxBeatClock") << "> TAP < : NEW BPM Tap : " << Tap_BPM;

		//SET OBTAINED BPM
		DAW_bpm = Tap_BPM;
	}

	//-
}

//---------------------------
void ofxBeatClock::Tap_update()
{
	int time = ofGetElapsedTimeMillis();
	if (tapIntervals.size() > 0 && (time - lastTime > 3000))
	{
		ofLogNotice("ofxBeatClock") << ">TAP< TIMEOUT: clear tap logs";
		tapIntervals.clear();

		tapCount = 0;
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
void ofxBeatClock::set_DAW_bpm(float bpm)
{
	DAW_bpm = bpm;
}

//----

//TODO:
#ifdef USE_AUDIO_BUFFER_TIMER_MODE
//clock by audio buffer. not normal threaded dawMetro timer

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

	//deviceOut = 3;//wasapi (line-1 out)
	devicesOut = soundStream.getDeviceList(ofSoundDevice::Api::MS_WASAPI);
	settings.setApi(ofSoundDevice::Api::MS_WASAPI);

	//deviceOut = 0;//ds
	//devicesOut = soundStream.getDeviceList(ofSoundDevice::Api::MS_DS);
	//settings.setApi(ofSoundDevice::Api::MS_DS);

	//deviceOut = 0;//asio
	//devicesOut = soundStream.getDeviceList(ofSoundDevice::Api::MS_ASIO);
	//settings.setApi(ofSoundDevice::Api::MS_ASIO);

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
	if (PLAYER_state && MODE_AudioBufferTimer)
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
					BPM_16th_current++;
					BPM_16th_str = ofToString(BPM_16th_current);

					//accumulate ticks until ticksPerBeat (default 4), then a 16th is filled
					//a beat tick happens
					if (BPM_16th_current == ticksPerBeat)
					{
						if (DEBUG_bufferAudio)
						{
							DEBUG_ticks = 0;
						}

						BPM_16th_current = 0;
						BPM_16th_str = ofToString(BPM_16th_current);

						//-

						//beat
						BPM_beat_current++;
						BPM_beat_str = ofToString(BPM_beat_current);

						//-

						if (BPM_beat_current == 5)
						{
							BPM_beat_current = 1;
							BPM_beat_str = ofToString(BPM_beat_current);

							BPM_bar_current++;
							BPM_bar_str = ofToString(BPM_bar_current);
						}

						if (BPM_bar_current == 5)
						{
							BPM_bar_current = 1;
							BPM_bar_str = ofToString(BPM_bar_current);
						}

						//-

						//trig beatTick for sound and gui feedback
						beatTick_MONITOR(BPM_beat_current);

						//ofLogNotice("ofxBeatClock") << "[audioBuffer] BEAT: " << BPM_beat_str;
					}
				}
			}
		}
	}
}
#endif