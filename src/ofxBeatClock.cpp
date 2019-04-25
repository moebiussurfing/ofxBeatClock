
#include "ofxBeatClock.h"


//---------------------------
void ofxBeatClock::gui_CLOCKER_setup(){
    
    int panelW, x, y;
    panelW = 200;
    x = 500;
    y = 10;
    
    // 1. control
    params_control.setName("CLOCK CONTROL");
    params_control.add(enable_CLOCK.set("ENABLE", true));
    params_control.add(PLAYER_state.set("PLAY", false));
    params_control.add(internal_CLOCK.set("INTERNAL", false));
    params_control.add(ENABLEB_MIDI_CLOCK.set("MIDI-IN CLOCK", true));
    
    // 2. monitor
    params_clocker.setName("BPM CONTROL");
    params_clocker.add(BPM_value.set("BPM", BPM_INIT, 60, 300));
    params_clocker.add(BPM_TimeBar.set("ms", 60000 / BPM_value, 1, 5000));
    params_clocker.add(BPM_Tap_Tempo_TRIG.set("TAP", false));
    
    
    container_controls = gui_CLOCKER.addGroup(params_control);
    container_controls->setPosition(ofPoint(x, y));
    
    container_clocker = gui_CLOCKER.addGroup(params_clocker);
    container_clocker->setPosition(ofPoint(x + panelW, y));
    
    ofAddListener(params_control.parameterChangedE(), this, &ofxBeatClock::Changed_gui_CLOCKER);
    ofAddListener(params_clocker.parameterChangedE(), this, &ofxBeatClock::Changed_gui_CLOCKER);
    
    
}

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
    //-
    
    TTF_small.load("fonts/mono.ttf", 8);
    TTF_medium.load("fonts/mono.ttf", 12);
    TTF_big.load("fonts/mono.ttf", 18);
    
    metronome_ball_pos.set(400, 700);
    metronome_ball_radius = 25;
    
    
    
    //---
    
    // SOUNDS
    
    mySound1.load("sounds/click1.wav");
    mySound1.setVolume(1.0f);
    mySound1.setMultiPlay(false);
    
    mySound2.load("sounds/click2.wav");
    mySound2.setVolume(0.25f);
    mySound2.setMultiPlay(false);
    
    //--

    
    // 60,000 / BPM = one beat in milliseconds
    
    //--
    
    setup_MIDI_CLOCK();
    beatsInBar.addListener(this, &ofxBeatClock::beatsInBar_Changed);

    
    gui_CLOCKER_setup();
    
    //--
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
            ofLogNotice() << "BPM_Tap_Tempo_TRIG: " << BPM_Tap_Tempo_TRIG;
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
        ofLogNotice() << "NEW BPM   : " << BPM_value;
        
        BPM_TimeBar = 60000 / BPM_value;// 60,000 / BPM = one beat in milliseconds
        
        ofLogNotice() << "TIME BEAT : " << BPM_TimeBar << "ms";
        ofLogNotice() << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";
        
        
    }
}


//--------------------------------------------------------------
void ofxBeatClock::update()
{
    
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
    
//    bpmTapper.update();

}

//--------------------------------------------------------------
void ofxBeatClock::draw()
{
    draw_MIDI_IN_CLOCK();
}

//--------------------------------------------------------------
void ofxBeatClock::draw_MIDI_IN_CLOCK(){
    
    int interline = 12; // line heigh
    int i = 0;
    int px = 10;
    int py = 10;
    int paddingAlign = 98;
    
    ofPushMatrix();
    
    ofTranslate(px, py);
    TTF_message = "MIDI CLOCK IN port: " + ofToString(midiIn_CLOCK.getPort());
    TTF_small.drawString(TTF_message, 0, interline * i++);
    TTF_message = ofToString("'" + ofToString(midiIn_CLOCK.getName()) + "'");
    TTF_small.drawString(TTF_message, 0, interline * i++); i++;
    
    ofTranslate(paddingAlign, 7);
    TTF_message = ("BPM: " + ofToString(BPM_value));
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
        ofFill();
        if (i <= (beatsInBar - 1))
            clockRunning ? ofSetColor(192) : ofSetColor(32);
        else
            ofSetColor(32);
        ofDrawRectangle(px + i * w, py, w, w);
        ofNoFill();
        ofSetColor(8);
        ofSetLineWidth(2);
        ofDrawRectangle(px + i * w, py, w, w);
        ofPopStyle();
    }
    
    //--
    
    // TICK BALL:
    
    ofPushStyle();
    
    ofSetColor(16); // ball background
    //ofSetColor(ofColor::yellow); // ball background
    ofDrawCircle(metronome_ball_pos.x, metronome_ball_pos.y, metronome_ball_radius);
    
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
    
    ofPopStyle();
    
    //-
}
//--------------------------------------------------------------
void ofxBeatClock::exit()
{


    midiIn_CLOCK.closePort();
    midiIn_CLOCK.removeListener(this);
}

//--------------------------------------------------------

void ofxBeatClock::PLAYER_START()
{
    ofLogNotice() << "PLAYER_START";
    
    if (internal_CLOCK)
    {

    }
    else
    {
        ofLogNotice() << "skip. already playing or internal clock disabled";
    }
}

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_STOP()
{
    ofLogNotice() << "PLAYER_STOP";
    
    if (!internal_CLOCK)
    {


    }
    else
    {
        ofLogNotice() << "skip. not playing playing or internal clock disabled";
    }
}


//--------------------------------------------------------------
void ofxBeatClock::beatsInBar_Changed(int & beatsInBar) {
    if (beatsInBar != beatsInBar_PRE) {
        ofLogVerbose() << "MIDI CLOCK TICK! " << beatsInBar;
        bpm_beat_TICKER = true;
    }
}


//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage& message) {
    
    if ( ENABLEB_MIDI_CLOCK )
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
                    
                    BPM_value = bpm_CLOCK;
                    
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
