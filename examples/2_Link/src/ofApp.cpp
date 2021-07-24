#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	//ofSetFrameRate(60);
	//ofSetWindowPosition(1920, 20);

	ofBackground(ofColor::orangeRed);

	beatClock.setup();

	listenerBeat = beatClock.BeatTick.newListener([&](bool&) {this->Changed_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_BeatTick() // callback to receive BeatTicks
{
	ofLogNotice(__FUNCTION__) << "BeatTick ! #" << beatClock.getBeat();
}
