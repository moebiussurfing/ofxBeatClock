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

//            // BPM PLAYER:
//            
//        case OF_KEY_RETURN:
//            if ( CLOCKER.PLAYER_state == false)//play from stopped
//            {
//                CLOCKER.PLAYER_state = true;
//                 CLOCKER.PLAYER_START();
//                //                ((ofxUIToggle*)gui_BPM->getWidget("PLAY"))->setValue(PLAYER_state);
//            }
//            
//            else//stop from playing
//            {
//                CLOCKER.PLAYER_state = false;
//                CLOCKER.PLAYER_STOP();
//                //                ((ofxUIToggle*)gui_BPM->getWidget("PLAY"))->setValue(PLAYER_state);
//            }
//            break;
//            
//            //--
//            
//        case '-':
//            CLOCKER.BPM_of_PLAYER--;
//            CLOCKER.bpm.setBpm(CLOCKER.BPM_of_PLAYER);
//            CLOCKER.bpmTapper.setBpm(CLOCKER.BPM_of_PLAYER);
//            break;
//            
//        case '+':
//            CLOCKER.BPM_of_PLAYER++;
//            CLOCKER.bpm.setBpm(CLOCKER.BPM_of_PLAYER);
//            CLOCKER.bpmTapper.setBpm(CLOCKER.BPM_of_PLAYER);
//            break;
//            
//        case 't': case 'T':
//            CLOCKER.bpmTapper.tap();
//            CLOCKER.BPM_of_PLAYER = CLOCKER.bpmTapper.bpm();
//            CLOCKER.bpm.setBpm(CLOCKER.BPM_of_PLAYER);
//            break;
//            
//            // case 'r':
//            // CLOCKER.bpm.reset();
//            // break;
            
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
