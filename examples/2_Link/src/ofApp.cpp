#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);
	ofSetWindowPosition(1920, 20);

	ofBackground(ofColor::orangeRed);

	beatClock.setup();

	listenerBeat = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->Changed_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_BeatTick() // callback to receive BeatTicks
{
	if (beatClock.BeatTick_TRIG) ofLogNotice(__FUNCTION__) << "BeatTick ! #" << beatClock.Beat_current;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
}