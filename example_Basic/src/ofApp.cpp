#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	////log level
	//ofSetLogLevel("ofxBeatClock", OF_LOG_VERBOSE);
	//ofSetLogLevel("ofApp", OF_LOG_VERBOSE);

	ofBackground(ofColor::orangeRed);

	//-

	//window
	float fps = 60.f;
	ofSetFrameRate(fps);
	ofSetVerticalSync(true);

	//-

	beatClock.setup();

	//callback to receive BeatTicks
	listenerBeat = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->Changed_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_BeatTick()
{
	if (beatClock.BeatTick_TRIG) ofLogWarning("ofApp") << "BeatTick ! " << beatClock.Beat_current;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch (key)
	{
		//toggle play/stop
	case ' ':
		beatClock.setTogglePlay();
		break;

		//trig tap-tempo 4 times 
		//when using internal clock 
		//only to set the bpm on the fly
	case 't':
		beatClock.tap_Trig();
		break;

		//get some beatClock info. look api methods into ofxBeatClock.h
	case OF_KEY_RETURN:
		ofLogWarning("ofApp") << "BPM     : " << beatClock.getBPM() << " beats per minute";
		ofLogWarning("ofApp") << "BAR TIME: " << beatClock.getTimeBar() << "ms";
		break;

		//debug
	case 'd':
		beatClock.toggleDebug_Clock();//clock debug
		beatClock.toggleDebug_Layout();//layout debug
		break;

		//show/hide gui
	case 'g':
		beatClock.toggleVisible_GuiPanel();
		break;
	}
}