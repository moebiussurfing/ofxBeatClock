#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(32);
    ofSetFrameRate(60);
    ofSetVerticalSync(true);

    //-
    
    CLOCKER.setup();

//    // set positions and sizes
//    CLOCKER.setPosition_Gui(10, 10, 100);
//    CLOCKER.setPosition_Squares(400, 100, 100);
//    CLOCKER.setPosition_Ball(400, 400, 50);
}

//--------------------------------------------------------------
void ofApp::update(){
    CLOCKER.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    CLOCKER.draw();
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
