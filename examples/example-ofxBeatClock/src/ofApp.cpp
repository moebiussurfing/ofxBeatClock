#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(ofColor::orangeRed);

	beatClock.setup();

	// Beat Tick Callback
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
	if (beatClock.getBeat() == 1)ofLogNotice(__FUNCTION__) << "--------";
	ofLogNotice(__FUNCTION__) << "Beat! #" << beatClock.getBeat();
}
