
#include "ofxBeatClock.h"

//--------------------------------------------------------------
void ofxBeatClock::setup()
{
#ifdef MODE_ENABLE_BPM_ENGINE
    tappet_division_SELECTED = 0;
    BPM_gotBeat = false;
    PLAYER_state = false; // true: playing
    PLAYER_start_TRIG = false; // true: starting play
    PLAYER_stop_TRIG = false; // true: stoping trig
    BPM_of_PLAYER = BPM_INIT;
    BPM_TimeBar = 60000 / BPM_of_PLAYER;
    PLAYER_state.addListener(this, &ofxBeatClock::PLAYER_state_Changed);//engine bpm player
    beatsInBar.addListener(this, &ofxBeatClock::beatsInBar_Changed);
#endif
    
    
#ifdef MODE_ENABLE_BPM_ENGINE
    
    // SOUNDS
    
    mySound1.load("sounds/click1.wav");
    mySound1.setVolume(1.0f);
    mySound1.setMultiPlay(false);
    
    mySound2.load("sounds/click2.wav");
    mySound2.setVolume(0.25f);
    mySound2.setMultiPlay(false);
    
    //--
    
    // BPM ENGINE:
    
    // use the `setup(bpm, beatPerBpm)` function to initialize the bpm and beatPerBpm
    bpm.setup(BPM_of_PLAYER, 4);
    
    // other initialisation method
    //bpm.setBpm(120);
    //bpm.setBeatPerBar(4);
    
    // use the `start` function to start the bpm
    //bpm.start();
    bpm.stop();
    
    // add a listener on the beatEvent to listen to every beat
    ofAddListener(bpm.beatEvent, this, &ofxBeatClock::onBeatEvent);
    
    //-
    
    // tap tempo
    bpmTapper.setBpm(BPM_of_PLAYER);
    
    // 60,000 / BPM = one beat in milliseconds
//    myTubesBlinker.LFO_timeCycle = 60000 / bpm.getBpm();
    
    //-
    
    BPM_MASTER_CLOCK = true;//enable clock sync
    
    //-
    
    BPM_of_PLAYER.addListener(this, &ofxBeatClock::BPM_of_PLAYER_Changed);
#endif

    
        gui_set_BPM();
    
     setup_MIDI_CLOCK();
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


//----------------------
void ofxBeatClock::gui_set_BPM()
{
    int pad_wid = 25;
    
    gui_BPM = new ofxUISuperCanvas(" ");
    
    //gui_BPM->setGlobalCanvasWidth(100);
    gui_BPM->setGlobalButtonDimension(30);
    gui_BPM->setGlobalSliderHeight(30);
    
    gui_BPM->addSpacer(0, pad_wid / 2);
    gui_BPM->addLabel("MASTER TIME CLOCK", OFX_UI_FONT_MEDIUM);
    gui_BPM->addSpacer(0, pad_wid);
    gui_BPM->setWidgetFontSize(OFX_UI_FONT_LARGE);
    gui_BPM->addLabel("BPM_LABEL");
    gui_BPM->addSpacer(0, pad_wid*0.1);
    
    int gui_Width = 200;
    gui_BPM->addIntSlider("1-BAR TIME", 251, 1538, 500, gui_Width, 5);//240 to 40 bpm. 120 default
                                                                      //gui_BPM->setWidgetFontSize(OFX_UI_FONT_LARGE);
                                                                      //gui_BPM->addLabelToggle("PLAY", false, 50, 50);
    gui_BPM->addSpacer(0, pad_wid);
    gui_BPM->setWidgetFontSize(OFX_UI_FONT_SMALL);
    
    //gui_BPM->addLabelToggle("CLOCK SYNC", false);
    
    ////gui_BPM->addSpacer(0, pad_wid * 0.25);
    //gui_BPM->addLabelButton("TAP TEMPO!", false);
    ////gui_BPM->addSpacer(0, pad_wid * 0.5);
    //gui_BPM->setWidgetFontSize(OFX_UI_FONT_SMALL);
    ////gui_BPM->setWidgetSpacing(10.0);
    //gui_BPM->addToggle("  METRONOME", false);
    //gui_BPM->addSpacer(0, pad_wid);
    //gui_BPM->addSpacer(0, 225);
    
    gui_BPM->setWidgetColor(OFX_UI_WIDGET_COLOR_BACK, ofColor(16));
    gui_BPM->setWidgetColor(OFX_UI_WIDGET_COLOR_FILL, ofColor(255, 128));
    //gui_BPM->setWidgetColor(OFX_UI_WIDGET_COLOR_FILL_HIGHLIGHT, ofColor::red);
    //gui_BPM->setWidgetColor(OFX_UI_WIDGET_COLOR_OUTLINE, ofColor::green);
#ifdef COLOR_THEME_RED
    gui_PRESETS->setWidgetColor(OFX_UI_WIDGET_COLOR_OUTLINE_HIGHLIGHT, ofColor::red);
#endif
#ifdef COLOR_THEME_BLACK_AND_WHITE
    gui_PRESETS->setWidgetColor(OFX_UI_WIDGET_COLOR_OUTLINE_HIGHLIGHT, ofColor(255));
#endif
    
    gui_BPM->autoSizeToFitWidgets();
    gui_BPM->setPosition(20, 216);
    ofAddListener(gui_BPM->newGUIEvent, this, &ofxBeatClock::gui_Event);
    
    //((ofxUIToggle*)gui_BPM->getWidget("CLOCK SYNC"))->setValue(BPM_MASTER_CLOCK);
    //((ofxUIToggle*)gui_BPM->getWidget("  METRONOME"))->setValue(BPM_Metronome);
}

//--------------------------------------------------------------
void ofxBeatClock::update()
{
    
    //-----------------------------------------------------------------------------
    
    // MIDI CLOCK
    
    // read bpm with a clock refresh or every frame if not defined time-clock-refresh:
#ifdef BPM_MIDI_CLOCK_REFRESH_RATE
    if (ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime >= BPM_MIDI_CLOCK_REFRESH_RATE)
    {
#endif
        //ofLogNotice() << "BPM UPDATED" << ofGetElapsedTimeMillis() - bpm_CheckUpdated_lastTime;
        
        //-
        
        // blinker update
        BPM_TimeBar = 60000 / bpm_CLOCK;// 60,000 / BPM = one beat in milliseconds
//        myTubesBlinker.LFO_timeCycle = BPM_TimeBar;// 1-bar for lfo
        
        ((ofxUIIntSlider*)gui_BPM->getWidget("1-BAR TIME"))->setValue(BPM_TimeBar);//update ui:
        ((ofxUILabel*)gui_BPM->getWidget("BPM_LABEL"))->setLabel("BPM: " + ofToString((int)bpm_CLOCK));//update ui
        
        //-
        
#ifdef BPM_MIDI_CLOCK_REFRESH_RATE
        bpm_CheckUpdated_lastTime = ofGetElapsedTimeMillis();
    }
#endif
    
    //--
    
#ifdef MODE_ENABLE_BPM_ENGINE
    // BPM ENGINE:
    
    // TODO: SE ESTA USANDO EN DOS LUGARES LA VARIABLE FLAG!!
    if (BPM_gotBeat == true)
    {
        //----
        
        //test
        BPM_LAST_Tick_Time_ELLAPSED_PRE = BPM_LAST_Tick_Time_ELLAPSED;
        BPM_LAST_Tick_Time_ELLAPSED = ofGetElapsedTimeMillis() - BPM_LAST_Tick_Time_LAST;//test
        BPM_LAST_Tick_Time_LAST = ofGetElapsedTimeMillis();//test
        ELLAPSED_diff = BPM_LAST_Tick_Time_ELLAPSED_PRE - BPM_LAST_Tick_Time_ELLAPSED;
        //----
        
        if (bpm.barIndex == 0)
        {
            //if (myTubesBlinker.OUT_BPM_TAP && bpm.barIndex == 0) {
            ////ofLogNotice() << "BEAT ! " << bpmTapper.beatPerc();
            
            if (BPM_Metronome)
                mySound1.play();
            
            //ofLogNotice() << "| ! BEAT ! |";
            ofLogNotice() << "| ! BEAT ! | " << BPM_LAST_Tick_Time_ELLAPSED << " | " << ELLAPSED_diff;//test
        }
        
        else
        {
            //else if (myTubesBlinker.OUT_BPM_TAP) {
            if (BPM_Metronome)
                mySound2.play();
            
            //ofLogNotice() << "|   BEAT   |";
            ofLogNotice() << "|   BEAT   | " << BPM_LAST_Tick_Time_ELLAPSED << " | " << ELLAPSED_diff;//test
            
        }
        
        BPM_gotBeat = false;
    }
    
    //-
    
    ofSoundUpdate();
    bpmTapper.update();
#endif
}

//--------------------------------------------------------------
void ofxBeatClock::draw()
{
}

//-----------

void ofxBeatClock::draw_MIDI_IN_CLOCK(){
    
    int interline = 12; // line heigh
                        //    int i = 0; // line number
                        //    int pad_Rect = 8; // rectangle up
                        //    int size_Rect = interline / 4; // rectangle up
                        //    int total_Lines = 15;
                        //    int midi_DEBUG_h = ofGetHeight() - 110 - total_Lines * interline;
                        //    ofPushMatrix();
                        //    ofTranslate(20, midi_DEBUG_h);
                        //    ofSetColor(ofColor::white);
    
    //--
    
    // MIDI CLOCK
    
    int px = 24;
    int py = 80;
    int paddingAlign = 98;
    ofPushMatrix();
    ofTranslate(px, py);
    
    int i = 0;
    
    TTF_message = "MIDI CLOCK IN port: " + ofToString(midiIn_CLOCK.getPort());
    TTF_small.drawString(TTF_message, 0, interline * i++);
    TTF_message = ofToString("'" + ofToString(midiIn_CLOCK.getName()) + "'");
    TTF_small.drawString(TTF_message, 0, interline * i++); i++;
    
    ofTranslate(paddingAlign, 7);
    //TTF_message = (clockRunning ? "clock: running" : "MIDI clock: stopped");
    //TTF_small.drawString(TTF_message, 0, interline * i++);
    //TTF_message = ("beats: " + ofToString(beats));
    //TTF_small.drawString(TTF_message, 0, interline * i++);
    //TTF_message = ("seconds: " + ofToString((int)seconds));
    //TTF_small.drawString(TTF_message, 0, interline * i++);
    TTF_message = ("BPM: " + ofToString(round(bpm_CLOCK)));
    TTF_small.drawString(TTF_message, 0, interline * i++);
    
    // a MIDI beat is a 16th note, so do a little math to convert to a time signature:
    // 4/4 -> 4 notes per bar & quarter note = 1 beat, add 1 to count from 1 instead of 0
    quarters = beats / 4; // convert total # beats to # quarters
    bars = (quarters / 4) + 1; // compute # of bars
    beatsInBar = (quarters % 4) + 1; // compute remainder as # notes within the current bar
    TTF_message = ("4/4 BARS: " + ofToString(bars + 53));
    TTF_small.drawString(TTF_message, 0, interline * i++);
    TTF_message = ("BEAT: " + ofToString(beatsInBar));
    TTF_small.drawString(TTF_message, 0, interline * i++);
    ofPopMatrix();
    
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
    
    //--
    
#ifdef MODE_ENABLE_BPM_ENGINE
    // BPM MIDI CLOCK:
    
    ofPushStyle();
    ofSetColor(16); // ball background
                    //ofSetColor(ofColor::yellow); // ball background
    metronome_ball_radius = 25;
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
#endif
    
    //-
}
//--------------------------------------------------------------
void ofxBeatClock::exit()
{
    
#ifdef MODE_ENABLE_BPM_ENGINE
    bpm.stop();
#endif
    
    
    midiIn_CLOCK.closePort();
    midiIn_CLOCK.removeListener(this);

    delete gui_BPM;
}



//-------------------------------------
void ofxBeatClock::gui_Event(ofxUIEventArgs &e)
{
    string name = e.getName();
    int kind = e.getKind();
    //ofLogNotice() << "got event from: " << name;
    
    if (false) {
    }
    
    //else if (name == "CLOCK SYNC")
    //{
    //    ofxUIToggle *toggle = (ofxUIToggle *)e.getToggle();
    //    BPM_MASTER_CLOCK = toggle->getValue();
    //    ofLogNotice() << "|- BPM_MASTER_CLOCK: " << BPM_MASTER_CLOCK;
    //}
    
    //else if (name == "  METRONOME")
    //{
    //    ofxUIToggle *toggle = (ofxUIToggle *)e.getToggle();
    //    BPM_Metronome = toggle->getValue();
    //    ofLogNotice() << "|- BPM_Metronome : " << BPM_Metronome;
    //}
    
    //else if (name == "TAP TEMPO!")
    //{
    //    ofxUIButton *button = (ofxUIButton *)e.getButton();
    //    BPM_Tap_Tempo_TRIG = button->getValue();
    
    //    if (BPM_Tap_Tempo_TRIG)
    //    {
    //        ofLogNotice() << "|- BPM_Tap_Tempo_TRIG: " << BPM_Tap_Tempo_TRIG;
    
    //        bpmTapper.tap();
    //        BPM_of_PLAYER = bpmTapper.bpm();
    //        bpm.setBpm(BPM_of_PLAYER);
    //    }
    //}
    
    //else if (name == "PLAY")
    //{
    //    ofxUIToggle *toggle = (ofxUIToggle *)e.getToggle();
    //    PLAYER_state = toggle->getValue();
    //    ofLogNotice() << "|- PLAYER_state: " << PLAYER_state;
    
    //    if (PLAYER_state == true)//play
    //    {
    //        PLAYER_START();
    //    }
    
    //    else//stop
    //    {
    //        PLAYER_STOP();
    //    }
    //}
}

//--------------------------------------------------------

// BPM ENGINE:

#ifdef MODE_ENABLE_BPM_ENGINE

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_START()
{
    ofLogNotice() << "|--------------------- PLAYER_START ---------------------|";
    //ofLogNotice() << "|- PLAYER_START";
    
    PLAYER_state = true;//redundante si viene de click gui. no si viene de key pressed
    if (!bpm.isPlaying)
    {
        bpm.start();
        //sequencer.start();
        
        //-
        
        //refresh tempos
        BPM_TimeBar = 60000 / bpm.getBpm();//60,000 / BPM = one beat in milliseconds
//        myTubesBlinker.LFO_timeCycle = BPM_TimeBar;
//        myTubesFixture.CURVE_Anim_Duration_TIME = BPM_TimeBar * 4;
//
//        // trig preset
//        TRIG_PRESET_Bool = true;
        
        //-
    }
}

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_STOP()
{
    //ofLogNotice() << "|- PLAYER_STOP";
    ofLogNotice() << "|--------------------- PLAYER_STOP ---------------------|";
    
    PLAYER_state = false;//redundante si viene de click gui. no si viene de key pressed
    if (bpm.isPlaying)
    {
        bpm.stop();
        //sequencer.stop();
        bpm.reset();
        //sequencer.reset();
        //bpmTapper.startFresh();
        
//        // stop preset
//        //STOP_PRESET_Bool = true;
//        STOP_PRESET_Function();
    }
}

//--------------------------------------------------------------
void ofxBeatClock::PLAYER_state_Changed(bool & PLAYER_state)
{
    ofLogNotice() << "|- ENGINE BPM PLAYER_state changed: " << PLAYER_state;
    //myTubesBlinker.PRESET_Playing = PLAYER_state;
    //myTubesFixture.PRESET_Playing = PLAYER_state;
}

////--------------------------------------------------
//void ofxBeatClock::bpm_CLOCK_Changed(float & bpm_CLOCK) {
//    ofLogNotice() << "bpm_CLOCK: " << bpm_CLOCK;
//
//    BPM_TimeBar = 60000 / bpm_CLOCK;// 60,000 / BPM = one beat in milliseconds
//    ofLogNotice() << "TIME BEAT : " << BPM_TimeBar << "ms";
//    ofLogNotice() << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";
//
//    //-
//
//    // TODO: BPM:
//    myTubesBlinker.LFO_timeCycle = BPM_TimeBar;
//    //myTubesFixture.CURVE_Anim_Duration_TIME = BPM_TimeBar * 4;
//
//    //-
//
//}

void ofxBeatClock::beatsInBar_Changed(int & beatsInBar) {
    if (beatsInBar != beatsInBar_PRE) {
        //ofLogNotice() << "BPM CLOCK TICK! " << beatsInBar;
        bpm_beat_TICKER = true;
        /*beatsInBar_PRE = beatsInBar;*/
    }
}

//--------------------------------------------------
void ofxBeatClock::BPM_of_PLAYER_Changed(float & BPM_of_PLAYER) {
    ofLogNotice() << "NEW BPM   : " << BPM_of_PLAYER;
    
    BPM_TimeBar = 60000 / bpm.getBpm();// 60,000 / BPM = one beat in milliseconds
    ofLogNotice() << "TIME BEAT : " << BPM_TimeBar << "ms";
    ofLogNotice() << "TIME BAR  : " << 4 * BPM_TimeBar << "ms";
    
    //-
    
//    // TODO: BPM:
//    myTubesBlinker.LFO_timeCycle = BPM_TimeBar;
//    myTubesFixture.CURVE_Anim_Duration_TIME = BPM_TimeBar * 4;
//
    //-
    
}
#endif



//-------------------------------
void ofxBeatClock::keyPressed(int key) {
#ifdef MODE_ENABLE_SHORTCUTS
    
    //ofLogNotice(); ofLogNotice() << "|--------------------- PRESSED KEY " << key << " ---------------------|";
    
    //--
    
    switch (key) {


            //-----
            
            
#ifdef MODE_ENABLE_BPM_ENGINE
            
            
            // BPM PLAYER:
            
        case OF_KEY_RETURN:
            if (PLAYER_state == false)//play from stopped
            {
                PLAYER_state = true;
                PLAYER_START();
                //                ((ofxUIToggle*)gui_BPM->getWidget("PLAY"))->setValue(PLAYER_state);
            }
            
            else//stop from playing
            {
                PLAYER_state = false;
                PLAYER_STOP();
                //                ((ofxUIToggle*)gui_BPM->getWidget("PLAY"))->setValue(PLAYER_state);
            }
            break;
            //--
            
        case '-':
            BPM_of_PLAYER--;
            bpm.setBpm(BPM_of_PLAYER);
            bpmTapper.setBpm(BPM_of_PLAYER);
            //sequencer.setBpm(BPM_of_PLAYER);
            break;
            
        case '+':
            BPM_of_PLAYER++;
            bpm.setBpm(BPM_of_PLAYER);
            bpmTapper.setBpm(BPM_of_PLAYER);
            //sequencer.setBpm(BPM_of_PLAYER);
            break;
            
        case 't': case 'T':
            bpmTapper.tap();
            BPM_of_PLAYER = bpmTapper.bpm();
            bpm.setBpm(BPM_of_PLAYER);
            //sequencer.setBpm(BPM_of_PLAYER);
            break;
            
            // case 'r':
            // bpm.reset();
            // break;
#endif
            
        default:
            break;
            
    }
    
    //---

#endif//MODE_ENABLE_SHORTCUTS
    
}

#ifdef MODE_ENABLE_BPM_ENGINE
void ofxBeatClock::onBeatEvent() {
    BPM_gotBeat = true;
}
#endif

//--------------------------------------------------------------
void ofxBeatClock::newMidiMessage(ofxMidiMessage& message) {
    
    //--
    
    // 1. MIDI CLOCK:
    
    //if ( ofxMidiMessage::getStatusString(message.status) == "Time Clock" )
    if ((message.status == MIDI_TIME_CLOCK) ||
        (message.status == MIDI_SONG_POS_POINTER) ||
        //(message.status == MIDI_ACTIVE_SENSING) ||
        (message.status == MIDI_START) ||
        (message.status == MIDI_CONTINUE) ||
        (message.status == MIDI_STOP))
    {
        
        //    midiCLOCK_Message = message;
        
        // 1. MIDI CLOCK
        
        // update the clock length and song pos in beats
        if (clock.update(message.bytes)) {
            // we got a new song pos
            beats = clock.getBeats();
            seconds = clock.getSeconds();
        }
        
        // compute the seconds and bpm
        switch (message.status) {
                
                // compute seconds and bpm live, you may or may not always need this
                // which is why it is not integrated into the ofxMidiClock parser class
            case MIDI_TIME_CLOCK:
                seconds = clock.getSeconds();
                bpm_CLOCK += (clock.getBpm() - bpm_CLOCK) / 5; // average the last 5 bpm values
                                                               // no break here so the next case statement is checked,
                                                               // this way we can set clockRunning if we've missed a MIDI_START
                                                               // ie. master was running before we started this example
                
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
