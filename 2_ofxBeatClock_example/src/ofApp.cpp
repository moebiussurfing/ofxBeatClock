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
	//beatClock.setPosition_GuiPanel(10, 10, 100);
	//beatClock.setPosition_BeatBoxes(400, 100, 300);
	//beatClock.setPosition_BeatBall_Auto(false);//must disable auto to customize pos/size
	//beatClock.setPosition_BeatBall(400, 400, 50);

	//-

	//callback beat tick
	listener = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->callback_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::callback_BeatTick()
{
	//TODO:
	//improve callback to get bool value too to avoid to check state like this..
	if (beatClock.BeatTick_TRIG)
		ofLogWarning("ofApp") << "TICK! " << beatClock.Beat_current;
}

//--------------------------------------------------------------
void ofApp::update() {
	beatClock.update();

	////add a log line every x frames to spread received and logged BEAT TICK callbacks
	//if (ofGetFrameNum() % 15 == 0 && true)
	//	ofLogWarning("ofApp") << "";
}

//--------------------------------------------------------------
void ofApp::draw() {
	beatClock.draw();

	draw_Anchor(ofGetMouseX(), ofGetMouseY());
}

//--------------------------------------------------------------
void ofApp::exit() {
	beatClock.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	beatClock.LINK_keyPressed(key);

	//-

	switch (key) 
	{

	case ' '://toggle play
		beatClock.togglePlay();
		break;

	case 't'://trig tap when using internal clock only
		beatClock.tap_Trig();
		break;

	case 'g':
		beatClock.toggle_Gui_visible();
		break;

	case OF_KEY_RETURN://get some beatClock info
		ofLogWarning("ofApp") << "BPM: " << beatClock.getBPM();
		ofLogWarning("ofApp") << "BAR TIME: " << beatClock.getTimeBar() << "ms";
		break;
	
	case 'd':
		DEBUG_Layout = !DEBUG_Layout;
		beatClock.setDebug(DEBUG_Layout);
		//beatClock.toggleDebugMode();
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
