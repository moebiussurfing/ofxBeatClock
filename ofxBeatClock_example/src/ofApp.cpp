#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    ofSetFrameRate(60);
    
    CLOCKER.setup();
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
//            if ( CLOCKER.PLAYER_state == false )
//            {
//                CLOCKER.PLAYER_state = true;
//                
//            }
//            else
//            {
//                CLOCKER.PLAYER_state = false;
//            }
            break;

        case '-':
            break;

        case '+':
            break;

        case 't':
            CLOCKER.BPM_Tap_Tempo_TRIG = true;
            break;

         case 'r':
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
