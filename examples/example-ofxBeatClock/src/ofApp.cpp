#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(ofColor::orangeRed);

	beatClock.setup();

	listenerBeat = beatClock.BeatTick.newListener([&](bool&) {this->Changed_Tick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_Tick() // callback to receive BeatTicks
{
	ofLogNotice(__FUNCTION__) << "Beat! #" << beatClock.getBeat();
}
