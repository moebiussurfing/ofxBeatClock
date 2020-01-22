#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(32);
    ofSetFrameRate(25);
    ofSetVerticalSync(true);

    //-
    
    CLOCKER.setup();

    // set positions and sizes
    CLOCKER.setPosition_Gui(10, 10, 100);
    CLOCKER.setPosition_Squares(10, 500, 190);
    CLOCKER.setPosition_Ball(80, 635, 20);

    //-

    // OSC OUTPUT
    
    out_ip = "localhost";
    out_port = 9000;
    ofxPublishOsc(out_ip, out_port, "/bpmBarTime", CLOCKER.BPM_GLOBAL_TimeBar);
    ofxPublishOsc(out_ip, out_port, "/bpmTempo", CLOCKER.BPM_Global);
    ofxPublishOsc(out_ip, out_port, "/beatNum", CLOCKER.BPM_beat_current);
    ofxPublishOsc(out_ip, out_port, "/tick", CLOCKER.TRIG_TICK);

    // TODO: other way
    //    CLOCKER.BPM_beat_current.addListener(this, &ofApp::Changed_OSC_beats);

    //-

}
////--------------------------------------------------------------
//void ofApp::Changed_OSC_beats(int & beatsInBar) {
//    ofLogNotice() << "OSC tick";
//
//
//}

//--------------------------------------------------------------
void ofApp::update(){
    CLOCKER.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    CLOCKER.draw();

    //-

    // OSC DEBUG

    ofPushMatrix();
    ofTranslate(20,685);

    int i = 1;
    string str;
    int line = 20;
    str = "OSC OUT";
    ofDrawBitmapStringHighlight(str, 0, i++ * line);
    str = "IP: " + out_ip;
    ofDrawBitmapStringHighlight(str, 0, i++ * line);
    str = "PORT: " + ofToString(out_port);
    ofDrawBitmapStringHighlight(str, 0, i++ * line);

    ofPopMatrix();

    //-
}

//--------------------------------------------------------------
void ofApp::exit(){
    CLOCKER.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    
    switch (key) {

        case ' ':
            CLOCKER.PLAYER_TOGGLE();
            break;

        case 't':
            CLOCKER.Tap_Trig();
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
