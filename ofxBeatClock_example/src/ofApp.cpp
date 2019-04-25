#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
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
void ofApp::keyPressed(int key) {

    
    switch (key) {

        case ' ':
            if ( CLOCKER.PLAYER_state == false )
            {
                CLOCKER.PLAYER_state = true;
                
            }
            else
            {
                CLOCKER.PLAYER_state = false;
            }
            break;

        case '-':
            break;

        case '+':
            break;

        case 't':
        case 'T':
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
