#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);
	ofBackground(ofColor::orangeRed);

	beatClock.setup();

	listenerBeat = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->Changed_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_BeatTick() // callback to receive BeatTicks
{
	if (beatClock.BeatTick_TRIG) ofLogNotice("ofApp") << "BeatTick ! #" << beatClock.Beat_current;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	beatClock.keyPressed(key);
}