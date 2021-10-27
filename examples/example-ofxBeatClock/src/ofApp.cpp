#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(ofColor::orangeRed);

	beatClock.setup();

	listenerBeat = beatClock.BeatTick.newListener([&](bool&) {this->Changed_Tick(); });
}

//--------------------------------------------------------------
void ofApp::draw()
{
	beatClock.draw();
}

//--------------------------------------------------------------
void ofApp::Changed_Tick() // Callback to receive BeatTicks
{
	ofLogNotice(__FUNCTION__) << "Beat! #" << beatClock.getBeat();
}
