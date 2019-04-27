
#include "ofxBeatClock.h"

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    //-
    
    // DRAW STUFF
    
    TTF_small.load("fonts/mono.ttf", 8);
    TTF_medium.load("fonts/mono.ttf", 12);
    TTF_big.load("fonts/mono.ttf", 18);

    //--

    // MONITOR

    // default pos
    posMon_x = 10;
    posMon_Y = 10;

    //---
    
    // SOUNDS
    
    tic.load("sounds/click1.wav");
    tic.setVolume(1.0f);
    tic.setMultiPlay(false);
    
    tac.load("sounds/click2.wav");
    tac.setVolume(0.25f);
    tac.setMultiPlay(false);
    
    //-
    
    // TEXT DISPLAY
    
    BPM_bar_str = "0";
    BPM_beat_str = "0";
    BPM_sixteen_str = "0";
    
    //--
    
    // EXTERNAL MIDI IN CLOCK
    
    setup_MIDI_CLOCK();
   
    //-
    
    // CONTROL AND INTERNAL CLOCK
    
    // 1. control
    
    params_control.setName("CLOCK CONTROL");
    params_control.add(ENABLE_CLOCKS.set("ENABLE", true));
    params_control.add(ENABLE_INTERNAL_CLOCK.set("INTERNAL", false));
    params_control.add(PLAYER_state.set("PLAY", false));
    params_control.add(ENABLE_EXTERNAL_CLOCK.set("EXTERNAL", true));
    params_control.add(MIDI_Port.set("MIDI PORT", 0, 0, num_MIDI_Ports));
    params_control.add(ENABLE_sound.set("TICK", false));
    ofAddListener(params_control.parameterChangedE(), this, &ofxBeatClock::Changed_Params);

    //-
    
    // 2. monitor
    
    params_clocker.setName("BPM CONTROL");
    params_clocker.add(BPM_Global.set("BPM", BPM_INIT, 60, 300));
    params_clocker.add(BPM_TimeBar.set("BAR ms", 60000 / BPM_Global, 1, 5000));
    params_clocker.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
    ofAddListener(params_clocker.parameterChangedE(), this, &ofxBeatClock::Changed_Params);
    
    //-
    
    // 3.1

    // TAP TEMPO

    Tap_running = false;
    tapCount = 0;
    intervals.clear();

    //-
    
    // 3.2
    
    // DAW METRO
    
    // add bar/beat/sixteenth listener on demand
    metro.addBeatListener(this);
    metro.addBarListener(this);
    metro.addSixteenthListener(this);
    
    DAW_bpm.addListener(this, &ofxBeatClock::Changed_DAW_bpm);
    DAW_active.addListener(this, &ofxBeatClock::Changed_DAW_active);
    DAW_bpm.set("BPM", BPM_INIT, 30, 240);
    DAW_active.set("Active", false);
    
    params_daw.setName("DAW CONTROL");
    params_daw.add(DAW_bpm);
    params_daw.add(DAW_active);
    
    metro.setBpm(DAW_bpm);
    
    //--
    
    setup_Gui();
    setPosition_Gui(300, 50, 200);
    setPosition_Draw(300, 500);

    //--

    TRIG_Ball_draw = false;
}

//--------------------------------------------------------------
void ofxBeatClock::setup_Gui(){

    //-

    // GUI POSITION AND SIZE

    // default config. to be setted after with .setPosition_Gui

    gui_Panel_W = 200;
    gui_Panel_posX = 300;
    gui_Panel_posY = 10;
    gui_Panel_padW = 5;

    //--

    container_controls = gui_CLOCKER.addGroup(params_control);
    container_controls->setPosition(ofPoint(gui_Panel_posX, gui_Panel_posY));

    container_daw = gui_CLOCKER.addGroup(params_daw);
    container_daw->setPosition(ofPoint(gui_Panel_posX + 1 * (gui_Panel_W + gui_Panel_padW), gui_Panel_posY));

    container_clocker = gui_CLOCKER.addGroup(params_clocker);
    container_clocker->setPosition(ofPoint(gui_Panel_posX + 2 * (gui_Panel_W + gui_Panel_padW), gui_Panel_posY));

    //-

    // LOAD LAST SETTINGS

    pathSettings = "CLOCKER_settings.xml";//default
    loadSettings(pathSettings);

    //--

    // init state just in cas not open correCt

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

        if (PLAYER_state) PLAYER_state = false;
        if (DAW_active) DAW_active = false;

        //-

        // TEXT DISPLAY
        BPM_input_str = "EXTERNAL" ;
        BPM_name_str = "MIDI PORT: ";
        BPM_name_str += ofToString( midiIn_CLOCK.getPort() );
        BPM_name_str += " - '" + midiIn_CLOCK.getName() + "'";
    }
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_Gui(int _x, int _y, int _w)
{
    gui_Panel_W = _w;
    gui_Panel_posX = _x;
    gui_Panel_posY = _y;
    gui_Panel_padW = 5;
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MIDI_CLOCK()
{
    
    // EXTERNAL MIDI CLOCK:
    
    ofLogNotice() << "==================================================";
    ofLogNotice() << "\t MIDI CLOCK IN:";
    ofSetLogLevel("ofxMidiClock", OF_LOG_NOTICE);
    
    midiIn_CLOCK.listInPorts();
    num_MIDI_Ports = midiIn_CLOCK.getNumInPorts();
    ofLogNotice() << "NUM MIDI-IN PORTS:" << num_MIDI_Ports;

    //midiIn_CLOCK.openPort(7);
    int midi_IN_Clock_PortSelected = 0;
    midiIn_CLOCK.openPort(midi_IN_Clock_PortSelected);
    //midiIn_CLOCK.openVirtualPort(midi_IN_Clock_PortSelected);
    
    // --------------------------------------------------------------------
    
    //midiIn_CLOCK.openPort("loopMIDI Port 1 7");
    //ofLogNotice() << "PORT NAME " << midiIn_CLOCK.getInPortName(7);
    
    ofLogNotice() << "\t connected to MIDI CLOCK IN port: " << midiIn_CLOCK.getPort();
    ofLogNotice() << "==================================================";
    
    // TODO: IGNORE SYSX
    midiIn_CLOCK.ignoreTypes(true, // sysex  <-- don't ignore timecode messages!
                             false, // timing <-- don't ignore clock messages!
                             true); // sensing
    
    // add ofxBeatClock as a listener
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
        //ofLogNotice() << "BPM UPDATED" << ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime;
        
        //-
        
#ifdef BPM_MIDI_CLOCK_REFRESH_RATE
        bpm_CheckUpdated_lastTime = ofGetElapsedTimeMillis();
    }
#endif
    
    //--
    
    // BPM ENGINE:
    
//    // TODO: SE ESTA USANDO EN DOS LUGARES LA VARIABLE FLAG!!
//    if (BPM_gotBeat == true)
//    {
//        //----
//
//        BPM_LAST_Tick_Time_ELLAPSED_PRE = BPM_LAST_Tick_Time_ELLAPSED;
//        BPM_LAST_Tick_Time_ELLAPSED = ofGetElapsedTimeMillis() - BPM_LAST_Tick_Time_LAST;//test
//        BPM_LAST_Tick_Time_LAST = ofGetElapsedTimeMillis();//test
//        ELLAPSED_diff = BPM_LAST_Tick_Time_ELLAPSED_PRE - BPM_LAST_Tick_Time_ELLAPSED;
//
//        //----
//
//        if (bpm.barIndex == 0)
//        {
//            if (ENABLE_sound)
//                mySound1.play();
//
//            //ofLogNotice() << "| ! BEAT ! |";
//            ofLogNotice() << "| ! BEAT ! | " << BPM_LAST_Tick_Time_ELLAPSED << " | " << ELLAPSED_diff;//test
//        }
//
//        else
//        {
//            if (ENABLE_sound)
//                mySound2.play();
//
//            //ofLogNotice() << "|   BEAT   |";
//            ofLogNotice() << "|   BEAT   | " << BPM_LAST_Tick_Time_ELLAPSED << " | " << ELLAPSED_diff;//test
//        }
//
//        BPM_gotBeat = false;
//    }
    
    //-

    ofSoundUpdate();
}

//--------------------------------------------------------------
void ofxBeatClock::draw()
{
    draw_MONITOR(posMon_x, posMon_Y);
}

//--------------------------------------------------------------
void ofxBeatClock::draw_MONITOR(int px, int py){

    //-

    // heigh paddings

    int padToSquares = 0;
    int padToBigTime = 0;
    int padToBall = 10;
    int squaresW = 50;//squares beats size
    metronome_ball_radius = 30;
    int interline = 11; // line heigh

    bool SHOW_moreInfo = false;

    //-

    int i = 0;

    //--
    
    // 2. TEXT INFO:
    
    ofPushMatrix();
    ofTranslate(px, py);
    
    TTF_message = "CLOCK: " + BPM_input_str ;
    TTF_small.drawString(TTF_message, 0, interline * i++);
    
    TTF_message = ofToString( BPM_name_str );
    TTF_small.drawString(TTF_message, 0, interline * i++); i++;

    py = py + (interline * (i+2));// acumulate heigh distance

    //-

    if (SHOW_moreInfo)
    {
        //ofTranslate(paddingAlign, 7);

        TTF_message = ("BPM : " + ofToString( BPM_Global ));
        TTF_small.drawString(TTF_message, 0, interline * i++);
        
        TTF_message = ("BAR : " + BPM_bar_str );
        TTF_small.drawString(TTF_message, 0, interline * i++);
        
        TTF_message = ("BEAT: " + BPM_beat_str );
        TTF_small.drawString(TTF_message, 0, interline * i++);
        
        TTF_message = ("16th: " + BPM_sixteen_str );
        TTF_small.drawString(TTF_message, 0, interline * i++);

        //-

        py = py + (interline * i);// acumulate heigh distance
    }
    
    ofPopMatrix();

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

        if ( ENABLE_CLOCKS & (ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK) )
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

    //-

    py = py + (squaresW);// acumulate heigh distance

    //--

    // 3. TICK BALL:

    py += padToBall + metronome_ball_radius;
    metronome_ball_pos.x = px + metronome_ball_radius;
    metronome_ball_pos.y = py;

    ofPushStyle();
    
    ofSetColor(16); // ball background
    ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);

    //-

    if ( ENABLE_CLOCKS && ( ENABLE_EXTERNAL_CLOCK || ENABLE_INTERNAL_CLOCK ) )
    {
        if ( TRIG_Ball_draw )
        {
            TRIG_Ball_draw = false;

            if (BPM_beat_current == 1)
                ofSetColor(ofColor::red);
            else
                ofSetColor(ofColor::white);
            
            ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
        }
    }

    //-

    ofPopStyle();

    //-

    py = py + (2 * metronome_ball_radius);// acumulate heigh distance
    px = px - metronome_ball_radius;

    //-
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BigClockTime(int x, int y){

    // BIG LETTERS:

    // TODO: PERFORMANCE: reduce number of drawings..

    {
        std::string timePos =
        ofToString(BPM_bar_str, 3, ' ') + " : " +
        ofToString(BPM_beat_str) + " : " +
        ofToString(BPM_sixteen_str);

        TTF_big.drawString(timePos, x, y);
    }
}

//--------------------------------------------------------------
void ofxBeatClock::setPosition_Draw(int x, int y){
    posMon_x = x;
    posMon_Y = y;
}

//--------------------------------------------------------------
void ofxBeatClock::exit()
{

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

//--------------------------------------------------------
void ofxBeatClock::PLAYER_START()//only used in internal mode
{
    ofLogNotice() << "PLAYER_START";
    
    if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
    {
        ofLogNotice() << "START";
        
        if (!PLAYER_state) PLAYER_state = true;
        if (!DAW_active) DAW_active = true;
        
        isPlaying = PLAYER_state;
    }
    else
    {
        ofLogNotice() << "skip";
    }
}

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_STOP()//only used in internal mode
{
    ofLogNotice() << "PLAYER_STOP";
    
    if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
    {
        ofLogNotice() << "STOP";
        
        if (PLAYER_state) PLAYER_state = false;
        if (DAW_active) DAW_active = false;

        
        isPlaying = PLAYER_state;
    }
    else
    {
        ofLogNotice() << "skip";
    }
}

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_TOGGLE()//only used in internal mode
{
    ofLogNotice() << "PLAYER_TOGGLE";
    
    if (ENABLE_INTERNAL_CLOCK && ENABLE_CLOCKS)
    {
        ofLogNotice() << "TOOGLE PLAY";
        
        PLAYER_state = !PLAYER_state;
        isPlaying = PLAYER_state;
    }
    else
    {
        ofLogNotice() << "skip";
    }
}

//--------------------------------------------------------------
void ofxBeatClock::CLOCK_Tick_MONITOR(int beat)
{
    if (ENABLE_CLOCKS && (ENABLE_INTERNAL_CLOCK || ENABLE_EXTERNAL_CLOCK))
    {
        TRIG_Ball_draw = true;

        //-

        if (ENABLE_sound)
        {
            // play tic on the first beat of a bar
            if (beat == 1) {
                tic.play();
            }
            else {
                tac.play();
            }
        }
    }

}

//--------------------------------------------------------------
void ofxBeatClock::Changed_Params(ofAbstractParameter& e) // patch change
{
    string wid = e.getName();
    //    ofLogNotice() << "Changed_gui_CLOCKER '" << wid << "': " << e;

    if (wid == " ")
    {

    }

    else if (wid == "TAP" && BPM_Tap_Tempo_TRIG)
    {
        if ( (BPM_Tap_Tempo_TRIG == true) && (ENABLE_INTERNAL_CLOCK) )
        {
            BPM_Tap_Tempo_TRIG = false; //should be button

            ofLogNotice() << "BPM_Tap_Tempo_TRIG: " << BPM_Tap_Tempo_TRIG;

            Tap_Trig();
        }
    }

    else if (wid == "PLAY")
    {
        ofLogNotice() << "PLAYER_state: " << PLAYER_state;

        // clocks are disabled or using external midi clock. dont need playing, just to be enabled

        if ( (ENABLE_EXTERNAL_CLOCK || !ENABLE_CLOCKS || !ENABLE_INTERNAL_CLOCK)  && PLAYER_state )
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
        //ofLogNotice() << "NEW BPM   : " << BPM_Global;

        BPM_TimeBar = 60000 / BPM_Global;// 60,000 / BPM = one beat in milliseconds
                                         //
                                         //        ofLogNotice() << "TIME BEAT : " << BPM_TimeBar << "ms";
                                         //        ofLogNotice() << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";
    }

    else if (wid == "INTERNAL")
    {
        ofLogNotice() << "CLOCK INTERNAL: " << ENABLE_INTERNAL_CLOCK;

        if (ENABLE_INTERNAL_CLOCK)
        {
            ENABLE_EXTERNAL_CLOCK = false;

            //-

            // TEXT DISPLAY
            BPM_input_str = "INTERNAL";
            BPM_name_str = "";
        }
        else
        {
            if (PLAYER_state) PLAYER_state = false;
            if (DAW_active) DAW_active = false;

            // TEXT DISPLAY
            BPM_bar_str = "0";
            BPM_beat_str = "0";
            BPM_sixteen_str = "0";

            if(!ENABLE_INTERNAL_CLOCK)
            {
                // TEXT DISPLAY
                BPM_input_str = "NONE";
                BPM_name_str = "";
            }
        }
    }

    else if (wid == "EXTERNAL")
    {
        ofLogNotice() << "CLOCK EXTERNAL MIDI-IN: " << ENABLE_EXTERNAL_CLOCK;

        if (ENABLE_EXTERNAL_CLOCK)
        {
            ENABLE_INTERNAL_CLOCK = false;

            if (PLAYER_state) PLAYER_state = false;
            if (DAW_active) DAW_active = false;

            //-

            // TEXT DISPLAY
            BPM_input_str = "EXTERNAL" ;
            BPM_name_str = "MIDI PORT: ";
            BPM_name_str += ofToString( midiIn_CLOCK.getPort() );
            BPM_name_str += " - '" + midiIn_CLOCK.getName() + "'";
        }
        else
        {
            // TEXT DISPLAY
            BPM_bar_str = "0";
            BPM_beat_str = "0";
            BPM_sixteen_str = "0";

            if(!ENABLE_EXTERNAL_CLOCK)
            {
                // TEXT DISPLAY
                BPM_input_str = "NONE";
                BPM_name_str = "";
            }
        }
    }

    else if (wid == "ENABLE")
    {
        ofLogNotice() << "ENABLE: " << ENABLE_CLOCKS;

        if (!ENABLE_CLOCKS)
        {
            if (ENABLE_EXTERNAL_CLOCK)
            {
                ENABLE_EXTERNAL_CLOCK = false;
            }

            if (ENABLE_INTERNAL_CLOCK)
            {
                ENABLE_INTERNAL_CLOCK = false;
            }

            //PLAYER_STOP();
            if (PLAYER_state) PLAYER_state = false;
            if (DAW_active) DAW_active = false;
        }

        //        else if (ENABLE_CLOCKS)
        //        {
        //            if (!ENABLE_INTERNAL_CLOCK)
        //            {
        //                ENABLE_INTERNAL_CLOCK = true;
        //            }
        //        }
    }
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_MIDI_beatsInBar(int & beatsInBar) {

    // this function trigs when any midi tick (beatsInBar) updating, so we need to filter if (we want beat or 16th..) has changed.
    // problem is maybe that beat param refresh when do not changes too, jus because it's accesed..

    if (beatsInBar != beatsInBar_PRE)
    {
        ofLogVerbose() << "MIDI-IN CLOCK TICK! " << beatsInBar;

        //-

        BPM_bar_str = ofToString( MIDI_bars );
        BPM_beat_str = ofToString( beatsInBar );
        BPM_beat_current = beatsInBar;//MIDI_beatsInBar = beatsInBar
        BPM_sixteen_str = " ";//TODO: should compute too..

        //-
        
        CLOCK_Tick_MONITOR(BPM_beat_current);

        beatsInBar_PRE = beatsInBar;
    }
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_DAW_bpm(float & value) {
    metro.setBpm(value);
    metro.resetTimer();

    //-

    BPM_Global = value;
}

//--------------------------------------------------------------
void ofxBeatClock::Changed_DAW_active(bool & value) {
    if (value)
    {
        metro.start();
    }
    else
    {
        metro.stop();
    }
}


//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage& message) {
    
    if ( ENABLE_EXTERNAL_CLOCK && ENABLE_CLOCKS )
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
                MIDI_bars = ( MIDI_quarters / 4) + 1; // compute # of bars
                MIDI_beatsInBar = ( MIDI_quarters % 4) + 1; // compute remainder as # notes within the current bar
                
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
                    
                case MIDI_START: case MIDI_CONTINUE:
                    if (!clockRunning) {
                        clockRunning = true;
                        ofLog() << "clock started";
                    }
                    break;
                    
                case MIDI_STOP:
                    if (clockRunning) {
                        clockRunning = false;
                        ofLog() << "clock stopped";
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
}

//--------------------------------------------------------------
void ofxBeatClock::saveSettings(string path)
{
    pathSettings = path;//store default
    
    // save settings
    ofXml settings;
    ofSerialize(settings, params_control);
    settings.save(path);
}

//--------------------------------------------------------------
void ofxBeatClock::loadSettings(string path)
{
    pathSettings = path;//store default
    
    // load settings
    ofXml settings;
    settings.load(path);
    ofDeserialize(settings, params_control);
}

//--------------------------------------------------------------

// INTERNAL CLOCK - DAW METRO

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int & bar) {
    
    ofLogVerbose() << "DAW METRO - BAR: " << bar;
    
    if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
    {
        BPM_bar_str = ofToString(bar);
    }
}

//--------------------------------------------------------------
void ofxBeatClock::onBeatEvent(int & beat) {

    ofLogVerbose() << "DAW METRO - BEAT: " << beat;
    
    if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
    {
        BPM_beat_str = ofToString(beat);
        BPM_beat_current = beat;

        //-

        CLOCK_Tick_MONITOR(beat);
    }
}

//--------------------------------------------------------------
void ofxBeatClock::onSixteenthEvent(int & sixteenth) {
    
    ofLogVerbose() << "DAW METRO - 16th: " << sixteenth;
    
    if (ENABLE_CLOCKS && ENABLE_INTERNAL_CLOCK)
    {
        BPM_sixteen_str = ofToString(sixteenth);
    }
}

//--------------------------------------------------------------

// TAP MACHINE

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
        ofLogNotice(">TAP<") <<  "TRIG: " << tapCount;

        intervals.push_back(time - lastTime);
        lastTime = time;

        if (tapCount>3)
        {
            intervals.erase(intervals.begin());
            avgBarMillis = accumulate(intervals.begin(), intervals.end(), 0) / intervals.size();
            Tap_BPM = 60 * 1000 / (float)avgBarMillis;

            ofLogNotice(">TAP<" )<< "NEW Tap BPM: " << Tap_BPM;

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

            // SET BPM

            DAW_bpm = Tap_BPM;

            //-
        }
    }
}

//---------------------------
void ofxBeatClock::Tap_update()
{
    int time = ofGetElapsedTimeMillis();
    if( intervals.size() > 0 && (time - lastTime > 3000) )
    {
        ofLogVerbose(">TAP<") << "TIMEOUT: clear tap logs";
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

