#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);
	ofBackground(ofColor::orangeRed);

	beatClock.setup();
	
	// callback to receive BeatTicks
	listenerBeat = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->Changed_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_BeatTick() // callback to receive BeatTicks
{
	if (beatClock.BeatTick_TRIG) ofLogNotice("ofApp") << "BeatTick ! #" << beatClock.Beat_current;
}