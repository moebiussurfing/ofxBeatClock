
#include "ofxBeatClock.h"

//TODO:
//clock by audio buffer. not timer
//--------------------------------------------------------------
void ofxBeatClock::audioOut(ofSoundBuffer &buffer)
{
	if (PLAYER_state)
	{

		//TODO:
		if (MODE_BufferTimer)
		{
			//            int ticksPerBeat = 4;
			int ticksPerBeat = 4;
			int samplesPerTick = (44100 * 60.0f) / BPM_Global / ticksPerBeat;
		}


		for (size_t i = 0; i < buffer.getNumFrames(); ++i)
		{
			if (++samples == samplesPerTick)
			{
				//            tick();
				ofLogError("ofxBeatClock") << "TICK";
				samples = 0;
			}

			// ...
		}

	}
}

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
	ofSetLogLevel("ofxBeatClock", OF_LOG_NOTICE);

	//--

//    wavePhase = 0;
//    pulsePhase = 0;

	// start the sound stream with a sample rate of 44100 Hz, and a buffer
	// size of 512 samples per audioOut() call
	ofSoundStreamSettings settings;
	settings.numOutputChannels = 2;
	settings.sampleRate = 44100;
	settings.bufferSize = 512;
	settings.numBuffers = 4;
	settings.setOutListener(this);
	soundStream.setup(settings);

	//--

#pragma mark - EXTERNAL MIDI IN CLOCK

	setup_MIDI_CLOCK();
	//should be defined before rest of gui to list midi ports being included on gui

	//--

#pragma mark - CONTROL AND INTERNAL CLOCK

	//-

	// 1. DAW METRO. CLOCK SYSTEM

	// add bar/beat/sixteenth listener on demand
	metro.addBeatListener(this);
	metro.addBarListener(this);
	metro.addSixteenthListener(this);

	DAW_bpm.addListener(this, &ofxBeatClock::Changed_DAW_bpm);
	DAW_active.addListener(this, &ofxBeatClock::Changed_DAW_active);
	DAW_bpm.set("BPM", BPM_INIT, 30, 300);
	DAW_active.set("Active", false);

	metro.setBpm(DAW_bpm);

	//-

	// 2. CONTROL

	params_control.setName("CONTROL");
	params_control.add(ENABLE_CLOCKS.set("ENABLE", true));
    params_control.add(DAW_bpm.set("BPM", BPM_INIT, 30, 300));
    params_control.add(ENABLE_INTERNAL_CLOCK.set("INTERNAL", false));
	params_control.add(ENABLE_EXTERNAL_CLOCK.set("EXTERNAL", true));
	params_control.add(ENABLE_sound.set("SOUND TICK", false));

	params_INTERNAL.setName("INTERNAL CLOCK");
	params_INTERNAL.add(PLAYER_state.set("PLAY", false));
	params_INTERNAL.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
	//    BPM_Tap_Tempo_TRIG = false;
	//    params_INTERNAL(BPM_Tap_Tempo_button.set("TAP"));

	params_EXTERNAL.setName("EXTERNAL CLOCK");
	params_EXTERNAL.add(MIDI_Port_SELECT.set("MIDI PORT", 0, 0, num_MIDI_Ports - 1));
    params_EXTERNAL.add(midiPortName);
    midiPortName.setSerializable(false);

	//-

	// 3.

#pragma mark - MONITOR GLOBAL TARGET

	// this smoothed (or maybe slower refreshed than fps) clock will be sended to target sequencer outside the class. see BPM_MIDI_CLOCK_REFRESH_RATE.
	params_clocker.setName("BPM TARGET");
	params_clocker.add(BPM_Global.set("GLOBAL BPM", BPM_INIT, 30, 300));
	params_clocker.add(BPM_GLOBAL_TimeBar.set("BAR ms", 60000 / BPM_Global, 100, 2000));

	params_clocker.add(RESET_BPM_Global.set("RESET BPM", false));

	//    params_clocker.add(BPM_half_TRIG.set("HALF", false));
	//    params_clocker.add(BPM_double_TRIG.set("DOUBLE", false));
	BPM_half_TRIG.set("HALF", false);
	BPM_double_TRIG.set("DOUBLE", false);

	//-

	// 4. TAP TEMPO

	Tap_running = false;
	tapCount = 0;
	intervals.clear();

	//-

#pragma mark - DEFAULT POSITIONS

	// default config. to be setted after with .setPosition_EDITOR

	gui_Panel_W = 200;
	gui_Panel_posX = 5;
	gui_Panel_posY = 5;
	gui_Panel_padW = 5;

	//--

	// TRANSPORT GUI

	setup_Gui();

	//-

	// default positions
	setPosition_Gui(gui_Panel_posX, gui_Panel_posY, gui_Panel_W);

	//--

#pragma mark - DRAW STUFF

	string strFont;
//    strFont = "assets/fonts/mono.ttf";
    strFont = "assets/fonts/telegrama_render.otf";
    //strFont = "assets/fonts/telegrama_render.otf";

	TTF_small.load(strFont, 9);
	TTF_medium.load(strFont, 11);
	TTF_big.load(strFont, 13);


	//-

	// MONITOR DEFAULT POS

	// squares
	pos_Squares_x = 10;
	pos_Squares_y = 600;
	pos_Squares_w = 200;

	// ball
	pos_Ball_x = 500;
	pos_Ball_y = 600;
	pos_Ball_w = 50;

	setPosition_Squares(pos_Squares_x, pos_Squares_y, pos_Squares_w);
	setPosition_Ball(pos_Ball_x, pos_Ball_y, pos_Ball_w);

	//-

	// DRAW BALL TRIGGER

	TRIG_TICK = false;

	//---

#pragma mark - SOUNDS

	tic.load("assets/sounds/click1.wav");
	tic.setVolume(1.0f);
	tic.setMultiPlay(false);

	tac.load("assets/sounds/click2.wav");
	tac.setVolume(0.25f);
	tac.setMultiPlay(false);

	//-

	// TEXT DISPLAY

	BPM_bar_str = "0";
	BPM_beat_str = "0";
	BPM_16th_str = "0";

	//--

	// PATTERN LIMITING. (VS LONG SONG MODE)

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

    //expand gui

    container_controls->maximize();
    group_BEAT_CLOCK->maximize();

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
void ofxBeatClock::setup_Gui()
{

	//-

	// GUI POSITION AND SIZE

	gui_w = 200;
	gui_slider_h = 18;
	gui_slider_big_h = 18;
	gui_button_h = 40;

	//--

	// THEME

	conf_Cont =
	{
		//{"width", gui_w},
		//{"direction", "vertical"},
	};

	confg_Sliders =
	{
		//{"height", gui_slider_h},
	};

	confg_Button =
	{
        {"type", "fullsize"},
        {"text-align", "center"},
        {"height", gui_button_h},
	};

	//--

	// PANEL

	group_BEAT_CLOCK = gui.addGroup("BEAT CLOCK", conf_Cont);

    //transport
    container_controls = group_BEAT_CLOCK->addGroup(params_control);

    group_INTERNAL = group_BEAT_CLOCK->addGroup("INTERNAL CLOCK", conf_Cont);
    group_INTERNAL->add(params_INTERNAL);

	container_controls->getControl("ENABLE")->setConfig(confg_Button);
    container_controls->getControl("INTERNAL")->setConfig(confg_Button);
    container_controls->getControl("EXTERNAL")->setConfig(confg_Button);

    group_EXTERNAL = group_BEAT_CLOCK->addGroup("EXTERNAL CLOCK", conf_Cont);
	group_EXTERNAL->add(params_EXTERNAL);


	//-

	container_clocker = group_BEAT_CLOCK->addGroup(params_clocker);

	//-

	//group_SEQ_editor = gui_CLOCKER.addGroup(params_control);
	//container_clocker = gui_CLOCKER.addGroup(params_clocker);

	//--

	//custom settings

	(container_controls->getFloatSlider("BPM"))->setPrecision(2);
	(container_controls->getFloatSlider("BPM"))->setConfig(confg_Sliders);

	(container_clocker->getFloatSlider("GLOBAL BPM"))->setPrecision(2);
	(container_clocker->getFloatSlider("GLOBAL BPM"))->setConfig(confg_Sliders);
	(container_clocker->getIntSlider("BAR ms"))->setConfig(confg_Sliders);
	(container_clocker->getToggle("RESET BPM"))->setConfig(confg_Sliders);

	(group_INTERNAL->getToggle("PLAY"))->setConfig(confg_Button);
	(group_INTERNAL->getToggle("TAP"))->setConfig(confg_Button);

	//-

	ofxGuiContainer *container_h;//half double
	//container_h = group_BEAT_CLOCK->addContainer();
	container_h = container_clocker->addContainer();

	ofJson containerConfig_browse =
	{
		//{"direction", "horizontal"},
		//{"background-color", "transparent"},
	};
	ofJson itemConfig_browse =
	{
		//{"height", gui_slider_h},
	};

	container_h->setConfig(containerConfig_browse);
	container_h->add<ofxGuiButton>(BPM_half_TRIG, itemConfig_browse);
	container_h->add<ofxGuiButton>(BPM_double_TRIG, itemConfig_browse);

	// TODO: BUG: not sure why do not requires to add to group whith listener function
	// maybe it's already added when adding the container to gui ?
	// if it's not commented, trigs arrive twice...
	//    // add to use listeners because are buttons not default toggles
	//    params_clocker.add(BPM_half_TRIG);
	//    params_clocker.add(BPM_double_TRIG);

	//-

	// LISTENERS

	ofAddListener(params_control.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_INTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_EXTERNAL.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofAddListener(params_clocker.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	//-

	// LOAD LAST SETTINGS

	////add extra settings to group after gui creation, to include in xml settings
	//params_control.add(DAW_bpm);

	ofLogNotice("ofxBeatClock") << "LOAD SETTINGS";
	ofLogNotice("ofxBeatClock") << pathSettings;

	pathSettings = "assets/settings/CLOCKER_settings.xml";//default

	loadSettings(pathSettings);

	//--

	// init state after loaded settings just in cas not open correct

	if (ENABLE_INTERNAL_CLOCK)
	{
		ENABLE_EXTERNAL_CLOCK = false;

		//-

		// TEXT DISPLAY
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

		//-

		// TEXT DISPLAY
		BPM_input_str = "EXTERNAL";
		BPM_name_str = "MIDI PORT: ";
		BPM_name_str += ofToString(midiIn_CLOCK.getPort());
		BPM_name_str += " '" + midiIn_CLOCK.getName() + "'";
	}

    //--

	ofLogNotice("ofxBeatClock") << "loadTheme";
	group_BEAT_CLOCK->loadTheme("theme/theme_bleurgh.json");
	container_controls->loadTheme("theme/theme_bleurgh.json");
	container_clocker->loadTheme("theme/theme_bleurgh.json");
	group_INTERNAL->loadTheme("theme/theme_bleurgh.json");
	group_EXTERNAL->loadTheme("theme/theme_bleurgh.json");
	//container_h->loadTheme("theme/theme_bleurgh.json");

    //-


//    //expand
//    container_controls->maximize();
//    group_BEAT_CLOCK->maximize();
//
//    if (ENABLE_INTERNAL_CLOCK)
//    {
//        group_INTERNAL->maximize();
//    }
//    else
//    {
//        group_INTERNAL->minimize();
//    }
//
//    if (ENABLE_EXTERNAL_CLOCK)
//    {
//        group_EXTERNAL->maximize();
//    }
//    else
//    {
//        group_EXTERNAL->minimize();
//    }
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MIDI_CLOCK()
{

	// EXTERNAL MIDI CLOCK:
	ofSetLogLevel("MIDI PORT", OF_LOG_NOTICE);
	ofLogNotice("MIDI PORT") << "== setup_MIDI_CLOCK";

	ofLogNotice("MIDI PORT") << "LIST PORTS:";
	midiIn_CLOCK.listInPorts();

	num_MIDI_Ports = midiIn_CLOCK.getNumInPorts();
	ofLogNotice("MIDI PORT") << "NUM MIDI-IN PORTS:" << num_MIDI_Ports;

	midiIn_CLOCK_port_OPENED = 0;
	midiIn_CLOCK.openPort(midiIn_CLOCK_port_OPENED);

	ofLogNotice("MIDI PORT") << "connected to MIDI CLOCK IN port: " << midiIn_CLOCK.getPort();

	// TODO: IGNORE SYSX
	midiIn_CLOCK.ignoreTypes(
		true, // sysex  <-- don't ignore timecode messages!
		false, // timing <-- don't ignore clock messages!
		true
	); // sensing

	midiIn_CLOCK.addListener(this);

	clockRunning = false; //< is the clock sync running?
	MIDI_beats = 0; //< song pos in beats
	MIDI_seconds = 0; //< song pos in seconds, computed from beats
	MIDI_CLOCK_bpm = BPM_INIT; //< song tempo in bpm, computed from clock length
	//bpm_CLOCK.addListener(this, &ofxBeatClock::bpm_CLOCK_Changed);

	//-

	MIDI_beatsInBar.addListener(this, &ofxBeatClock::Changed_MIDI_beatsInBar);

	//-
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MIDI_PORT(int p)
{
	ofLogNotice("MIDI PORT") << "== setup_MIDI_PORT";

	midiIn_CLOCK.closePort();
	midiIn_CLOCK_port_OPENED = p;
	midiIn_CLOCK.openPort(midiIn_CLOCK_port_OPENED);
	ofLogNotice("MIDI PORT") << "PORT NAME " << midiIn_CLOCK.getInPortName(p);

	// TEXT DISPLAY
	BPM_input_str = "EXTERNAL";
	BPM_name_str = "MIDI PORT: ";
	BPM_name_str += ofToString(midiIn_CLOCK.getPort());
	BPM_name_str += " '" + midiIn_CLOCK.getName() + "'";

	ofLogNotice("MIDI PORT") << "connected to MIDI CLOCK IN port: " << midiIn_CLOCK.getPort();
}

#pragma mark - UPDATE

//--------------------------------------------------------------
void ofxBeatClock::update()
{


	//-

	// TAP

	if (Tap_running)
		Tap_update();

	//--

	// MIDI CLOCK

	// read bpm with a clock refresh or every frame if not defined time-clock-refresh:

#ifdef BPM_MIDI_CLOCK_REFRESH_RATE
	if (ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime >= BPM_MIDI_CLOCK_REFRESH_RATE)
	{
#endif
		//ofLogNotice("ofxBeatClock") << "BPM UPDATED" << ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime;

		//-

#ifdef BPM_MIDI_CLOCK_REFRESH_RATE
		bpm_CheckUpdated_lastTime = ofGetElapsedTimeMillis();
	}
#endif

	//--

	// BPM ENGINE:

	//#define BPM_MIDI_CLOCK_REFRESH_RATE 200
	//    //refresh received MTC by clock. disabled/commented to "realtime" by frame update
	// //this smoothed (or maybe slower refreshed than fps) clock will be sended to target sequencer outside the class. see BPM_MIDI_CLOCK_REFRESH_RATE.

	//BPM_LAST_Tick_Time_ELLAPSED_PRE = BPM_LAST_Tick_Time_ELLAPSED;
	//BPM_LAST_Tick_Time_ELLAPSED = ofGetElapsedTimeMillis() - BPM_LAST_Tick_Time_LAST;//test
	//BPM_LAST_Tick_Time_LAST = ofGetElapsedTimeMillis();//test
	//ELLAPSED_diff = BPM_LAST_Tick_Time_ELLAPSED_PRE - BPM_LAST_Tick_Time_ELLAPSED;

	//-

	ofSoundUpdate();

	//-

}

#pragma mark - DRAW

//--------------------------------------------------------------
void ofxBeatClock::draw()
{
	//TODO: improve performance with fbo drawing

	//boxes
	draw_SQUARES(pos_Squares_x, pos_Squares_y, pos_Squares_w);

	//ball
//    draw_BALL(pos_Ball_x, pos_Ball_y, pos_Ball_w);
	draw_BALL(pos_Squares_x + 0.5f*pos_Squares_w - pos_Ball_w,
		pos_Ball_y, pos_Ball_w);
}

//--------------------------------------------------------------
void ofxBeatClock::draw_SQUARES(int px, int py, int w)
{

	//-

	// heigh paddings

	int padToSquares = 0;
	int padToBigTime = 0;
	int squaresW;//squares beats size
	squaresW = w / 4;

	int interline = 15; // line heigh

	bool SHOW_moreInfo = false;

	//-

	int i = 0;

	//--

	// 2. TEXT INFO:

	ofPushMatrix();
	int pad = 10;
	ofTranslate(px, py);

	TTF_message = "BPM: " + ofToString(BPM_Global.get(), 2);

	TTF_big.drawString(TTF_message, pad, interline * i++);
	i++;

	TTF_message = "CLOCK: " + BPM_input_str;
	TTF_small.drawString(TTF_message, pad, interline * i++);

	TTF_message = ofToString(BPM_name_str);
	TTF_small.drawString(TTF_message, pad, interline * i++);

	py = py + (interline * (i + 2));// accumulate heigh distance

	//-

	if (SHOW_moreInfo)
	{
		//ofTranslate(paddingAlign, 7);

		TTF_message = ("BPM : " + ofToString(BPM_Global));
		TTF_small.drawString(TTF_message, 0, interline * i++);

		TTF_message = ("BAR : " + BPM_bar_str);
		TTF_small.drawString(TTF_message, 0, interline * i++);

		TTF_message = ("BEAT: " + BPM_beat_str);
		TTF_small.drawString(TTF_message, 0, interline * i++);

		TTF_message = ("16th: " + BPM_16th_str);
		TTF_small.drawString(TTF_message, 0, interline * i++);

		//-

		py = py + (interline * i);// acumulate heigh distance
	}

	ofPopMatrix();//TODO: pop could be at function ending

	//--

	py += padToBigTime;
	draw_BigClockTime(px, py);
	py = py + (TTF_big.getSize());// acumulate heigh distance

	//--

	// 2. BEATS SQUARES

	py += padToSquares;

	for (int i = 0; i < 4; i++)
	{
		ofPushStyle();

		//--

		// DEFINE SQUARE COLORS:

		if (ENABLE_CLOCKS & (ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK))
		{
			if (i <= (BPM_beat_current - 1))
			{
				if (ENABLE_INTERNAL_CLOCK)
					PLAYER_state ? ofSetColor(192) : ofSetColor(32);

				else if (ENABLE_EXTERNAL_CLOCK)
					clockRunning ? ofSetColor(192) : ofSetColor(32);
			}

			else
				ofSetColor(32);
		}

		//-

		else //disabled both modes
		{
			ofSetColor(32);//black
		}

		//--

		// DRAW SQUARE

		ofFill();
		ofDrawRectangle(px + i * squaresW, py, squaresW, squaresW); //border

		ofNoFill();
		ofSetColor(8);
		ofSetLineWidth(2);
		ofDrawRectangle(px + i * squaresW, py, squaresW, squaresW); //filled

		ofPopStyle();

		//--
	}
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BALL(int px, int py, int w)
{

	// 3. TICK BALL:

	// TODO: alpha tick could be tweened to better heartbeat feeling..
	metronome_ball_radius = w;
	int padToBall = 0;

	py += padToBall + metronome_ball_radius;
	metronome_ball_pos.x = px + metronome_ball_radius;
	metronome_ball_pos.y = py;

	ofPushStyle();

	if (!Tap_running)
	{
		ofSetColor(16); // ball background when not tapping
	}
	else
	{
		// aloha fade when measuring tpaping
		float t = (ofGetElapsedTimeMillis() % 1000);
		float fade = sin(ofMap(t, 0, 1000, 0, 2 * PI));
		cout << "fade: " << fade << endl;
		int alpha = (int)ofMap(fade, -1.0f, 1.0f, 0, 127) + 128;
		ofSetColor(ofColor(96), alpha);
	}

	ofDrawCircle(metronome_ball_pos.x,
		metronome_ball_pos.y,
		metronome_ball_radius);

	//-

	if (ENABLE_CLOCKS && !Tap_running &&
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

	// BIG LETTERS:

	// TODO: PERFORMANCE: reduce number of drawings..

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

	// set positions and sizes
	int w;
	w = _w;
	int pad_Clock = 15;
	int pos_Clock_x = w + pad_Clock;

	//    setPosition_EDITOR(pos_Clock_x, 0, w);
	setPosition_Gui(gui_Panel_posX, gui_Panel_posY, w);

	setPosition_Squares(pos_Clock_x, 500, w);
	setPosition_Ball(pos_Clock_x + 50, 650, 25);
}

//--------------------------------------------------------------
ofPoint ofxBeatClock::getPosition_Gui()
{
	ofPoint p;
	//    p = group_BEAT_CLOCK->getShape().getTopLeft();
	p = ofPoint(gui_Panel_posX, gui_Panel_posY);
	return p;
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_Squares(int x, int y, int w)
{
	pos_Squares_x = x;
	pos_Squares_y = y;
	pos_Squares_w = w;
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_Ball(int x, int y, int w)
{
	pos_Ball_x = x;
	pos_Ball_y = y;
	pos_Ball_w = w;
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

	//    ofLogNotice("ofxBeatClock") << ofSetDataPathRoot();
	//    ofLogNotice("ofxBeatClock") << ofRestoreWorkingDirectoryToDefault();
	//    ofRestoreWorkingDirectoryToDefault();

	// default desired settings when it will opened
	PLAYER_state = false;
	if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_CLOCK)
		ENABLE_INTERNAL_CLOCK = true;

	saveSettings(pathSettings);

	//--

	// EXTERNAL

	midiIn_CLOCK.closePort();
	midiIn_CLOCK.removeListener(this);
	MIDI_beatsInBar.removeListener(this, &ofxBeatClock::Changed_MIDI_beatsInBar);

	//-

	// remove listeners

	ofRemoveListener(params_control.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
	ofRemoveListener(params_clocker.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

	DAW_bpm.removeListener(this, &ofxBeatClock::Changed_DAW_bpm);
	DAW_active.removeListener(this, &ofxBeatClock::Changed_DAW_active);

	//-
}

#pragma mark - PLAYER MANAGER

//--------------------------------------------------------
void ofxBeatClock::PLAYER_START()//only used in internal mode
{
	ofLogNotice("ofxBeatClock") << "PLAYER_START";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "START";

		if (!PLAYER_state)
			PLAYER_state = true;
		if (!DAW_active)
			DAW_active = true;


		//TODO:
		if (MODE_BufferTimer)
		{
			samples = 0;
		}
		else
		{
			isPlaying = true;
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
	ofLogNotice("ofxBeatClock") << "PLAYER_STOP";

	if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		ofLogNotice("ofxBeatClock") << "STOP";

		if (PLAYER_state)
			PLAYER_state = false;
		if (DAW_active)
			DAW_active = false;

		isPlaying = false;

		RESET_clockValues();


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
		isPlaying = PLAYER_state;
	}
	else
	{
		ofLogNotice("ofxBeatClock") << "skip";
	}
}

//--------------------------------------------------------------
void ofxBeatClock::CLOCK_Tick_MONITOR(int beat)
{
	if (ENABLE_CLOCKS && (ENABLE_INTERNAL_CLOCK || ENABLE_EXTERNAL_CLOCK))
	{
		TRIG_TICK = true;

		//-

		if (ENABLE_sound)
		{
			// play tic on the first beat of a bar
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
void ofxBeatClock::Changed_Params(ofAbstractParameter &e) // patch change
{
	string wid = e.getName();
	//ofLogNotice("ofxBeatClock") << "Changed_gui_CLOCKER '" << wid << "': " << e;

	if (wid == "TAP")
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

	else if (wid == "PLAY")
	{
		ofLogNotice("ofxBeatClock") << "PLAYER_state: " << PLAYER_state;

		// clocks are disabled or using external midi clock. dont need playing, just to be enabled

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

	else if (wid == "BPM")
	{
		// ofLogVerbose"ofxBeatClock") << "NEW BPM: " << BPM_Global;
	}

	else if (wid == "RESET BPM")
	{
		ofLogNotice("ofxBeatClock") << "RESET BPM: " << RESET_BPM_Global;

		if (RESET_BPM_Global)
		{
			RESET_BPM_Global = false;//should be a button not toggle
			BPM_Global = 120.00;
			DAW_bpm = 120.00;
			metro.setBpm(DAW_bpm);
			ofLogNotice("ofxBeatClock") << "BPM_Global: " << BPM_Global;
		}
	}
	else if (wid == "HALF")
	{
		ofLogNotice("ofxBeatClock") << "HALF: " << BPM_half_TRIG;
		if (BPM_half_TRIG)
		{
			//            float tempBPM = BPM_Global / 2;
			//            cout << "temp bpm: " << tempBPM << endl;
			//            BPM_Global = BPM_Global / 2;
			DAW_bpm = DAW_bpm / 2;
			//            BPM_Global = tempBPM;
			//            DAW_bpm = tempBPM;
		}
	}
	else if (wid == "DOUBLE")
	{
		ofLogNotice("ofxBeatClock") << "DOUBLE: " << BPM_double_TRIG;
		if (BPM_double_TRIG)
		{
			//            float tempBPM = BPM_Global * 2;
			//            cout << "temp bpm: " << tempBPM << endl;
			//            BPM_Global = BPM_Global * 2;
			DAW_bpm = DAW_bpm * 2;
			//            BPM_Global = tempBPM;
			//            DAW_bpm = tempBPM;
		}
	}

	else if (wid == "GLOBAL BPM")
	{
		ofLogNotice("ofxBeatClock") << "GLOBAL BPM: " << BPM_Global;
		// ofLogVerbose("ofxBeatClock") << "GLOBAL BPM   : " << BPM_Global;

		BPM_GLOBAL_TimeBar = 60000 / BPM_Global;// 60,000 / BPM = one beat in milliseconds

        //TODO:
        DAW_bpm = BPM_Global;
        
		//ofLogNotice("ofxBeatClock") << "TIME BEAT : " << BPM_TimeBar << "ms";
		//ofLogNotice("ofxBeatClock") << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";
	}
    else if (wid == "BAR ms")
    {
        ofLogNotice("ofxBeatClock") << "BAR ms: " << BPM_GLOBAL_TimeBar;
        BPM_Global = 60000 / BPM_GLOBAL_TimeBar;
    }

	else if (wid == "INTERNAL")
	{
		ofLogNotice("ofxBeatClock") << "CLOCK INTERNAL: " << ENABLE_INTERNAL_CLOCK;

		if (ENABLE_INTERNAL_CLOCK)
		{
			ENABLE_EXTERNAL_CLOCK = false;

			//-

			// TEXT DISPLAY
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

			// TEXT DISPLAY
			BPM_bar_str = "0";
			BPM_beat_str = "0";
			BPM_16th_str = "0";

			if (!ENABLE_INTERNAL_CLOCK)
			{
				// TEXT DISPLAY
				BPM_input_str = "NONE";
				BPM_name_str = "";
			}

			//-

			if (!ENABLE_EXTERNAL_CLOCK)
				ENABLE_EXTERNAL_CLOCK = true;

			group_INTERNAL->minimize();
		}
	}

	else if (wid == "EXTERNAL")
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

			// TEXT DISPLAY
			BPM_input_str = "EXTERNAL";
			BPM_name_str = "MIDI PORT: ";
			BPM_name_str += ofToString(midiIn_CLOCK.getPort());
			BPM_name_str += " '" + midiIn_CLOCK.getName() + "'";

            midiPortName = midiIn_CLOCK.getName();

			group_EXTERNAL->maximize();
		}
		else
		{
			// TEXT DISPLAY
			BPM_bar_str = "0";
			BPM_beat_str = "0";
			BPM_16th_str = "0";

			if (!ENABLE_EXTERNAL_CLOCK)
			{
				// TEXT DISPLAY
				BPM_input_str = "NONE";
				BPM_name_str = "";
			}

			//-

			if (!ENABLE_INTERNAL_CLOCK)
				ENABLE_INTERNAL_CLOCK = true;

			group_EXTERNAL->minimize();
		}
	}

	else if (wid == "ENABLE")
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

		// ENABLE CLOCKS true
		else if (!ENABLE_INTERNAL_CLOCK && !ENABLE_EXTERNAL_CLOCK)
		{
			ENABLE_INTERNAL_CLOCK = true;//default state when enabling clocks
		}
	}

	else if (wid == "MIDI PORT")
	{
		if (MIDI_Port_PRE != MIDI_Port_SELECT)
		{
			MIDI_Port_PRE = MIDI_Port_SELECT;

			ofLogNotice("MIDI PORT") << "== GUI SELECTED PORT";

			ofLogNotice("MIDI PORT") << "LIST PORTS:";
			midiIn_CLOCK.listInPorts();

			ofLogNotice("MIDI PORT") << "OPENING PORT: " << MIDI_Port_SELECT;
			setup_MIDI_PORT(MIDI_Port_SELECT);

            midiPortName = midiIn_CLOCK.getName();
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_MIDI_beatsInBar(int &beatsInBar)
{

	// this function trigs when any midi tick (beatsInBar) updating, so we need to filter if (we want beat or 16th..) has changed.
	// problem is maybe that beat param refresh when do not changes too, jus because it's accesed..

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
			BPM_16th_current = 0;//TODO: should compute too..
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

		// TRIG BALL DRAWING AND SOUND TICKER

		CLOCK_Tick_MONITOR(BPM_beat_current);

		//        if (pattern_limits)
		//        {
		//            beatsInBar_PRE = beatsInBar;
		//        }
		//        else
		//        {

		beatsInBar_PRE = MIDI_beatsInBar;

		//        }

	}
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_DAW_bpm(float &value)
{
	metro.setBpm(value);
	metro.resetTimer();

	//-

	BPM_Global = value;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_DAW_active(bool &value)
{
	if (value)
	{
		metro.start();
	}
	else
	{
		metro.stop();
	}
}

#pragma mark - MIDI MESSAGE

//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage &message)
{

	if (ENABLE_EXTERNAL_CLOCK && ENABLE_CLOCKS)
	{
		// 1. MIDI CLOCK:

		if ((message.status == MIDI_TIME_CLOCK) ||
			(message.status == MIDI_SONG_POS_POINTER) ||
			(message.status == MIDI_START) ||
			(message.status == MIDI_CONTINUE) ||
			(message.status == MIDI_STOP))
		{

			//    midiCLOCK_Message = message;

			// 1. MIDI CLOCK

			// update the clock length and song pos in beats
			if (MIDI_clock.update(message.bytes))
			{
				// we got a new song pos
				MIDI_beats = MIDI_clock.getBeats();
				MIDI_seconds = MIDI_clock.getSeconds();

				//-

				MIDI_quarters = MIDI_beats / 4; // convert total # beats to # quarters
				MIDI_bars = (MIDI_quarters / 4) + 1; // compute # of bars
				MIDI_beatsInBar = (MIDI_quarters % 4) + 1; // compute remainder as # TARGET_NOTES_params within the current bar

				//-

			}

			// compute the seconds and bpm

			switch (message.status)
			{

			case MIDI_TIME_CLOCK:
				MIDI_seconds = MIDI_clock.getSeconds();
				MIDI_CLOCK_bpm += (MIDI_clock.getBpm() - MIDI_CLOCK_bpm) / 5;

				// average the last 5 bpm values
				// no break here so the next case statement is checked,
				// this way we can set clockRunning if we've missed a MIDI_START
				// ie. master was running before we started this example

				//-

				BPM_Global = MIDI_CLOCK_bpm;

				//-

				DAW_bpm = MIDI_CLOCK_bpm;

				//-

				// transport control

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
	//ofToDataPath( "dreadlock_ride2.wav"
	// many debugger (included gdb and xcode) change the base path
	// when you run your app in debug mode
	// so the best is to use an absolute path

	//    pathSettings = path;//store default

	// save settings
	ofXml settings;
	ofSerialize(settings, params_control);
	settings.save(path);

	ofLogNotice("ofxBeatClock") << "saved settings";
	ofLogNotice("ofxBeatClock") << path;
}

//--------------------------------------------------------------
void ofxBeatClock::loadSettings(string path)
{
	//pathSettings = path;//store default

	// load settings
	ofXml settings;
	settings.load(path);
	ofDeserialize(settings, params_control);
}

//--------------------------------------------------------------

#pragma mark - INTERNAL CLOCK - DAW METRO

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int &bar)
{

	ofLogVerbose("ofxBeatClock") << "DAW METRO - BAR: " << bar;

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

//--------------------------------------------------------------
void ofxBeatClock::onBeatEvent(int &beat)
{

	ofLogVerbose("ofxBeatClock") << "DAW METRO - BEAT: " << beat;

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

		CLOCK_Tick_MONITOR(BPM_beat_current);

		//-
	}
}

//--------------------------------------------------------------
void ofxBeatClock::onSixteenthEvent(int &sixteenth)
{

	ofLogVerbose("ofxBeatClock") << "DAW METRO - 16th: " << sixteenth;

	if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
	{
		BPM_16th_current = sixteenth;
		BPM_16th_str = ofToString(BPM_16th_current);
	}
}

//--------------------------------------------------------------
void ofxBeatClock::RESET_clockValues()
{
	metro.stop();
	metro.resetTimer();

	BPM_bar_current = 0;
	BPM_beat_current = 0;
	BPM_16th_current = 0;
	BPM_beat_str = ofToString(BPM_beat_current);
	BPM_bar_str = ofToString(BPM_bar_current);
	BPM_16th_str = ofToString(BPM_16th_current);
}

//--------------------------------------------------------------

#pragma mark - TAP MACHINE

// TODO: implement bpm divider / multiplier x2 x4 /2 /4
//--------------------------------------------------------------
void ofxBeatClock::Tap_Trig()
{
	if (ENABLE_INTERNAL_CLOCK)
	{
		Tap_running = true;

		//-

		// disable sound to better flow
		if (tapCount == 0 && ENABLE_sound)
		{
			ENABLE_sound = false;
			SOUND_wasDisabled = true;
		}

		//-

		int time = ofGetElapsedTimeMillis();
		tapCount++;
		ofLogNotice(">TAP<") << "TRIG: " << tapCount;

		intervals.push_back(time - lastTime);
		lastTime = time;

		if (tapCount > 3)
		{
			intervals.erase(intervals.begin());
			avgBarMillis = accumulate(intervals.begin(), intervals.end(), 0) / intervals.size();
			Tap_BPM = 60 * 1000 / (float)avgBarMillis;

			ofLogNotice(">TAP<") << "NEW Tap BPM: " << Tap_BPM;

			// TODO: target bpm could be tweened..

			//-

			intervals.clear();
			tapCount = 0;
			Tap_running = false;

			//-

			if (SOUND_wasDisabled)// sound disbler to better flow
			{
				ENABLE_sound = true;
				SOUND_wasDisabled = false;
			}

			//-

			// SET OBTAINED BPM

			DAW_bpm = Tap_BPM;

			//-
		}
	}
}

//---------------------------
void ofxBeatClock::Tap_update()
{
	int time = ofGetElapsedTimeMillis();
	if (intervals.size() > 0 && (time - lastTime > 3000))
	{
		ofLogNotice("ofxBeatClock") << ">TAP< TIMEOUT: clear tap logs";
		intervals.clear();

		tapCount = 0;
		Tap_running = false;

		//-

		if (SOUND_wasDisabled)// sound disbler to better flow
		{
			ENABLE_sound = true;
			SOUND_wasDisabled = false;
		}
	}
}

//--------------------------------------------------------------
void ofxBeatClock::set_DAW_bpm(float bpm)
{
	DAW_bpm = bpm;
}
