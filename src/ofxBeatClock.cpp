
#include "ofxBeatClock.h"


//---------------------------
void ofxBeatClock::setup_Gui_CLOCKER(){
    
    //-
    
    int panelW, x, y, padW;
    panelW = 200;
    x = 300;
    y = 10;
    padW = 5;
    
    //-
    
    // 1. control
    
    params_control.setName("CLOCK CONTROL");
    params_control.add(enable_CLOCK.set("ENABLE", true));
    params_control.add(PLAYER_state.set("PLAY", false));
    params_control.add(ENABLE_INTERNAL_CLOCK.set("INTERNAL", false));
    params_control.add(ENABLE_MIDI_CLOCK.set("MIDI-IN CLOCK", true));
    ofAddListener(params_control.parameterChangedE(), this, &ofxBeatClock::Changed_gui_CLOCKER);
    //-
    
    // 2. monitor
    
    params_clocker.setName("BPM CONTROL");
    params_clocker.add(BPM_Global.set("BPM", BPM_INIT, 60, 300));
    params_clocker.add(BPM_TimeBar.set("ms", 60000 / BPM_Global, 1, 5000));
    params_clocker.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
    ofAddListener(params_clocker.parameterChangedE(), this, &ofxBeatClock::Changed_gui_CLOCKER);

    //-
    
    // 3. daw metro
    
    // add bar/beat/sixteenth listener on demand
    metro.addBeatListener(this);
    //metro.addBarListener(this);
    //metro.addSixteenthListener(this);
    
    DAW_bpm.addListener(this, &ofxBeatClock::Changed_DAW_bpm);
    DAW_active.addListener(this, &ofxBeatClock::Changed_DAW_active);
    
    params_daw.setName("DAW CONTROL");
    DAW_bpm.set("BPM", 120, 30, 240);
    DAW_active.set("Active", false);
    params_daw.add(DAW_bpm);
    params_daw.add(DAW_active);
    
    metro.setBpm(DAW_bpm);
    
    //--
    
    container_controls = gui_CLOCKER.addGroup(params_control);
    container_controls->setPosition(ofPoint(x, y));
    
    container_clocker = gui_CLOCKER.addGroup(params_clocker);
    container_clocker->setPosition(ofPoint(x + (panelW + padW), y));
    
    container_daw = gui_CLOCKER.addGroup(params_daw);
    container_daw->setPosition(ofPoint(x + 2 * (panelW + padW), y));
    
    //-

    // LOAD LAST SETTINGS
    
    pathSettings = "CLOCKER_settings.xml";//default
    loadSettings(pathSettings);
    
    //--
    
    
}

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
    //-
    
    // DRAW STUFF
    
    TTF_small.load("fonts/mono.ttf", 8);
    TTF_medium.load("fonts/mono.ttf", 12);
    TTF_big.load("fonts/mono.ttf", 18);
    
    metronome_ball_pos.set(400, 700);
    metronome_ball_radius = 25;
    
    //---
    
    // SOUNDS
    
    tic.load("sounds/click1.wav");
    tic.setVolume(1.0f);
    tic.setMultiPlay(false);
    
    tac.load("sounds/click2.wav");
    tac.setVolume(0.25f);
    tac.setMultiPlay(false);
    
    //--
    
    // MIDI IN CLOCK
    
    setup_MIDI_CLOCK();
    beatsInBar.addListener(this, &ofxBeatClock::Changed_MIDI_beatsInBar);
    
    
    //----
    
    // TAP TEMPO
    
    tapMachine = make_shared<ofxTapMachine>();
    ofAddListener(tapMachine->bar.event, this, &ofxBeatClock::barFunc);
    ofAddListener(tapMachine->minim.event, this, &ofxBeatClock::minimFunc);
    ofAddListener(tapMachine->crochet.event, this, &ofxBeatClock::crochetFunc);
    
    //-----
    

    
    //--
    
    setup_Gui_CLOCKER();
}

//--------------------------------------------------------------
void ofxBeatClock::setup_MIDI_CLOCK()
{
    
    // EXTERNAL MIDI CLOCK:
    
    ofLogNotice() << "==================================================";
    ofLogWarning() << "\t MIDI CLOCK IN:";
    ofSetLogLevel("ofxMidiClock", OF_LOG_NOTICE);
    
    
    midiIn_CLOCK.listInPorts();
    //midiIn_CLOCK.openPort(7);
    int midi_IN_Clock_PortSelected = 0;
    midiIn_CLOCK.openPort(midi_IN_Clock_PortSelected);
    //midiIn_CLOCK.openVirtualPort(midi_IN_Clock_PortSelected);
    
    // --------------------------------------------------------------------
    
    //midiIn_CLOCK.openPort("loopMIDI Port 1 7");
    //ofLogNotice() << "PORT NAME " << midiIn_CLOCK.getInPortName(7);
    
    ofLogWarning() << "\t connected to MIDI CLOCK IN port: " << midiIn_CLOCK.getPort();
    ofLogNotice() << "==================================================";
    
    // TODO: IGNORE SYSX
    midiIn_CLOCK.ignoreTypes(true, // sysex  <-- don't ignore timecode messages!
                             false, // timing <-- don't ignore clock messages!
                             true); // sensing
    
    // add ofxBeatClock as a listener
    midiIn_CLOCK.addListener(this);
    
    clockRunning = false; //< is the clock sync running?
    beats = 0; //< song pos in beats
    seconds = 0; //< song pos in seconds, computed from beats
    bpm_CLOCK = 120; //< song tempo in bpm, computed from clock length
                     //bpm_CLOCK.addListener(this, &ofxBeatClock::bpm_CLOCK_Changed);
    
    //-
    
}


//--------------------------------------------------------------
void ofxBeatClock::Changed_gui_CLOCKER(ofAbstractParameter& e) // patch change
{
    string wid = e.getName();
    ofLogNotice() << "Changed_gui_CLOCKER '" << wid << "': " << e;
    
    if (wid == " ")
    {

    }
    
    else if (wid == "TAP" && BPM_Tap_Tempo_TRIG)
    {
        if (BPM_Tap_Tempo_TRIG == true)
        {
            BPM_Tap_Tempo_TRIG = false; //should be button
            
            ofLogNotice() << "BPM_Tap_Tempo_TRIG: " << BPM_Tap_Tempo_TRIG;
            
            tapMachine->tap();
        }

    }
    
    else if (wid == "PLAY")
    {
        ofLogNotice() << "PLAYER_state: " << PLAYER_state;

        if (PLAYER_state == true) //play
        {
            PLAYER_START();
        }

        else //stop
        {
            PLAYER_STOP();
        }
    }
    
    else if (wid == "BPM")
    {
        ofLogNotice() << "NEW BPM   : " << BPM_Global;
        
        BPM_TimeBar = 60000 / BPM_Global;// 60,000 / BPM = one beat in milliseconds
        
        ofLogNotice() << "TIME BEAT : " << BPM_TimeBar << "ms";
        ofLogNotice() << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";
        
        
    }
    
    else if (wid == "INTERNAL")
    {
        ofLogNotice() << "CLOCK INTERNAL: " << ENABLE_INTERNAL_CLOCK;
        
        if (ENABLE_INTERNAL_CLOCK)
        {
            ENABLE_MIDI_CLOCK = false;
        }
        else
        {
            if (PLAYER_state) PLAYER_state = false;
            if (DAW_active) DAW_active = false;
        }
        
    }
    
    else if (wid == "MIDI-IN CLOCK")
    {
        ofLogNotice() << "MIDI-IN CLOCK: " << ENABLE_MIDI_CLOCK;
        
        if (ENABLE_MIDI_CLOCK)
        {
            ENABLE_INTERNAL_CLOCK = false;
            
            //PLAYER_STOP();
            if (PLAYER_state) PLAYER_state = false;
            if (DAW_active) DAW_active = false;
        }
        
    }
    
    
    

    
    
    
}


//--------------------------------------------------------------
void ofxBeatClock::update()
{
    
    //-
    
    // INTERNAL
    
//    if (ENABLE_INTERNAL_CLOCK){
//        BPM_value = tapMachine->getBPM();
//    }
    
    //------------------------------------------------------------------------
    
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
    draw_BPM_CLOCK();
    
    draw_Tapper();
    
    draw_DAW();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_BPM_CLOCK(){
    
    int interline = 12; // line heigh
    int i = 0;
    int px = 10;
    int py = 10;
    int paddingAlign = 98;
    
    //--
    
    // TEXT INFO:
    
    ofPushMatrix();
    
    ofTranslate(px, py);
    TTF_message = "MIDI CLOCK IN port: " + ofToString(midiIn_CLOCK.getPort());
    TTF_small.drawString(TTF_message, 0, interline * i++);
    TTF_message = ofToString("'" + ofToString(midiIn_CLOCK.getName()) + "'");
    TTF_small.drawString(TTF_message, 0, interline * i++); i++;
    
    ofTranslate(paddingAlign, 7);
    TTF_message = ("BPM: " + ofToString(BPM_Global));
    TTF_small.drawString(TTF_message, 0, interline * i++);

    quarters = beats / 4; // convert total # beats to # quarters
    bars = (quarters / 4) + 1; // compute # of bars
    beatsInBar = (quarters % 4) + 1; // compute remainder as # notes within the current bar
    
    TTF_message = ("4/4 BARS: " + ofToString(bars + 53));
    TTF_small.drawString(TTF_message, 0, interline * i++);
    TTF_message = ("BEAT: " + ofToString(beatsInBar));
    TTF_small.drawString(TTF_message, 0, interline * i++);
    
    ofPopMatrix();
    
    //--
    
    // BEATS SQUEARES
    
    py += 95;
    int w = 50;//squares beats size
    
    for (int i = 0; i < 4; i++) {
        ofPushStyle();
        
        //--
        
        if (ENABLE_MIDI_CLOCK)
        {
            if (i <= (beatsInBar - 1))
                clockRunning ? ofSetColor(192) : ofSetColor(32);
            else
                ofSetColor(32);
        }
        
        //--
        
        else if (ENABLE_INTERNAL_CLOCK)
        {
            if ( metro.getBeat() > i )
            {
                PLAYER_state ? ofSetColor(192) : ofSetColor(32);
            }
            
            else
                ofSetColor(32);
            
//            if ( (tapMachine->bar.count % 4) >= i )
//            {
//                //PLAYER_state ? ofSetColor(192) : ofSetColor(32);
//                ofSetColor(192);//for tap mode..
//            }
//
//            else
//                ofSetColor(32);
        }
        
        //-
        
        else //disabled both modes
        {
            ofSetColor(32);//black
        }
        
        //--
        
        
        ofFill();
        ofDrawRectangle(px + i * w, py, w, w);
        ofNoFill();
        ofSetColor(8);
        ofSetLineWidth(2);//border
        ofDrawRectangle(px + i * w, py, w, w);
        
        ofPopStyle();
    }
    
    //--
    
    // TICK BALL:
    
    ofPushStyle();
    
    ofSetColor(16); // ball background
    ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
    
    if (ENABLE_MIDI_CLOCK)
    {
        if (bpm_beat_TICKER == true)
        {
            if (beatsInBar == 1)
                ofSetColor(ofColor::red);
            else
                ofSetColor(ofColor::white);
            
            ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
            
            bpm_beat_TICKER = false;
            
            beatsInBar_PRE = beatsInBar;//test
        }
    }
    
    else if (ENABLE_INTERNAL_CLOCK && beat_changed)
    {
        beat_changed = false;
        
        if (tapMachine->bar.count % 4 == 0)
            ofSetColor(ofColor::red);
        else
            ofSetColor(ofColor::white);
        
        ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
    }
    
    ofPopStyle();
    
    //-
}
//--------------------------------------------------------------
void ofxBeatClock::exit()
{

    PLAYER_state = false;
    saveSettings(pathSettings);

    midiIn_CLOCK.closePort();
    midiIn_CLOCK.removeListener(this);
}

//--------------------------------------------------------

void ofxBeatClock::PLAYER_START()//only used in internal mode
{
    ofLogNotice() << "PLAYER_START";
    
    if (ENABLE_INTERNAL_CLOCK)
    {
        ofLogNotice() << "START";
        
        if (!PLAYER_state) PLAYER_state = true;
        if (!DAW_active) DAW_active = true;
        
        //        tapMachine->toggleStart();
        
//        metro.start();
    }
    else
    {
        ofLogNotice() << "skip. already playing or internal clock disabled";
    }
}

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_STOP()//only used in internal mode
{
    ofLogNotice() << "PLAYER_STOP";
    
    if (ENABLE_INTERNAL_CLOCK)
    {
        ofLogNotice() << "STOP";
        
        if (PLAYER_state) PLAYER_state = false;
        if (DAW_active) DAW_active = false;
        
        ////        if ( tapMachine->isStart() )
        //        tapMachine->toggleStart();
        
//        metro.stop();
    }
    else
    {
        ofLogNotice() << "skip. not playing playing or internal clock disabled";
    }
}


//--------------------------------------------------------------
void ofxBeatClock::Changed_MIDI_beatsInBar(int & beatsInBar) {
    if (beatsInBar != beatsInBar_PRE) {
        ofLogVerbose() << "MIDI IN CLOCK TICK! " << beatsInBar;
        bpm_beat_TICKER = true;
        
        // TODO:
        if (ENABLE_MIDI_CLOCK)
        {
            // play tic on the first beat of a bar
            if (beatsInBar == 1) {
                tic.play();
            }
            else {
                tac.play();
            }
        }
    }
}


//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage& message) {
    
    if ( ENABLE_MIDI_CLOCK )
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
            if (clock.update(message.bytes))
            {
                // we got a new song pos
                beats = clock.getBeats();
                seconds = clock.getSeconds();
            }
            
            // compute the seconds and bpm
            
            switch (message.status)
            {
                    // compute seconds and bpm live, you may or may not always need this
                    // which is why it is not integrated into the ofxMidiClock parser class
                    
                case MIDI_TIME_CLOCK:
                    seconds = clock.getSeconds();
                    bpm_CLOCK += (clock.getBpm() - bpm_CLOCK) / 5; // average the last 5 bpm values
                   // no break here so the next case statement is checked,
                   // this way we can set clockRunning if we've missed a MIDI_START
                   // ie. master was running before we started this example
                    
                    BPM_Global = bpm_CLOCK;
                    
                    //-
                    
                    if (DAW_SlaveToInternal)
                        DAW_bpm = bpm_CLOCK;
                    
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
void ofxBeatClock::barFunc(int &count){
    cout<<"barCount : "<<count<<endl;
    beat_changed = true;
}
void ofxBeatClock::minimFunc(int &count){
    cout<<"minimCount : "<<count<<endl;
}
void ofxBeatClock::crochetFunc(int &count){
    cout<<"crochetCount : "<<count<<endl;
}

//--------------------------------------------------------------
void ofxBeatClock::draw_Tapper(){
    
    ofPushMatrix();
    ofTranslate(0, 300);
    ofPushStyle();
    
//    ofSetColor(255, 0, 0);
//    ofDrawCircle(50+(ofGetWidth()-100)*tapMachine->bar.normalized, 200, 10);
//
//    ofSetColor(0, 255, 0);
//    ofDrawCircle(50+(ofGetWidth()-100)*tapMachine->minim.normalized, 250, 10);
//
//    ofSetColor( 0, 0, 255);
//    ofDrawCircle(50+(ofGetWidth()-100)*tapMachine->crochet.normalized, 300, 10);
//
//
//    ofSetColor(255, 255, 0);
//    ofDrawCircle(50+(ofGetWidth()-100)*tapMachine->twoBar.normalized, 350, 10);
//    ofSetColor(0, 255, 255);
//    ofDrawCircle(50+(ofGetWidth()-100)*tapMachine->fourBar.normalized, 400, 10);
    
    
    string msg="===============[ ofxTapMachine example ]===============\n";
    msg += "press space bar more than 3 times to get average BPM.\n";
//    msg += "FPS        : " + ofToString(ofGetFrameRate(), 2) + "\n";
    msg += "BPM        : " + ofToString(tapMachine->getBPM(),2) + "\n";

//    msg += "bar        : " + ofToString(tapMachine->bar.count) + "\n";
//    msg += "minim      : " + ofToString(tapMachine->minim.count) + "\n";
//    msg += "crochet    : " + ofToString(tapMachine->crochet.count) + "\n";
//    msg += "quaver     : " + ofToString(tapMachine->quaver.count) + "\n";
    
    msg += "bar        : " + ofToString(tapMachine->bar.count % 4) + "\n";
    msg += "minim      : " + ofToString(tapMachine->minim.count % 8) + "\n";
    msg += "crochet    : " + ofToString(tapMachine->crochet.count % 16) + "\n";
    msg += "quaver     : " + ofToString(tapMachine->quaver.count % 32) + "\n";
    
    ofDrawBitmapStringHighlight(msg, 10, 10);
    
    ofPopStyle();
    ofPopMatrix();
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

// DAW METRO

//--------------------------------------------------------------
void ofxBeatClock::onBarEvent(int & bar) {
}

//--------------------------------------------------------------
void ofxBeatClock::onBeatEvent(int & beat) {
    
    if (ENABLE_INTERNAL_CLOCK)
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

//--------------------------------------------------------------
void ofxBeatClock::onSixteenthEvent(int & sixteenth) {
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
    if (value) {
        metro.start();
    }
    else {
        metro.stop();
    }
}

//--------------------------------------------------------------
void ofxBeatClock::draw_DAW(){

    std::string timePos = ofToString(metro.getBar(), 3, ' ') + " : " + ofToString(metro.getBeat()) + " : " +
    ofToString(metro.getSixteenth());
    
    TTF_big.drawString(timePos, 500, 350);

}
