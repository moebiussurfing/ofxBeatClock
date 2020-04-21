#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofBackground(32);

	//-

#ifdef USE_ofxWindowApp
	//required when opening for first time, when no json settings created yet.
	windowApp.setSettingsFps(60);
#else
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
#endif

	//-

	beatClock.setup();

	////customize positions and sizes
	//beatClock.setPosition_Gui(10, 10, 100);
	//beatClock.setPosition_BeatBoxes(400, 100, 300);
	//beatClock.setPosition_BeatBall_Auto(false);//must disable auto to customize pos/size
	//beatClock.setPosition_BeatBall(400, 400, 50);

	//-

	//callback beat tick
	listener = beatClock.TRIG_TICK.newListener([&](bool&) {this->callback_Tick(); });
}

//--------------------------------------------------------------
void ofApp::callback_Tick()
{
	//TODO:
	//improve callback to get bool value too to avoid to check state like this..
	if (beatClock.TRIG_TICK)
		ofLogWarning("ofApp") << "TICK! " << beatClock.BPM_beat_current;
}

//--------------------------------------------------------------
void ofApp::update() {
	beatClock.update();

	////add a log line every x frames to spread received and logged TICK callbacks
	//if (ofGetFrameNum() % 15 == 0 && true)
	//	ofLogWarning("ofApp") << "";
}

//--------------------------------------------------------------
void ofApp::draw() {
	beatClock.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
	beatClock.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	switch (key) {

	case ' ':
		beatClock.PLAYER_TOGGLE();
		break;

	case 't':
		beatClock.Tap_Trig();
		break;

	case 'g':
		beatClock.toggle_Gui_visible();
		break;

	case OF_KEY_RETURN:
		ofLogWarning("ofApp") << "BPM: " << beatClock.get_BPM();
		ofLogWarning("ofApp") << "BAR TIME: " << beatClock.get_TimeBar() << "ms";
		break;
	
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
