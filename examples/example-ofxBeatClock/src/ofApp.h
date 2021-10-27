
#pragma once

#include "ofMain.h"

#include "ofxWindowApp.h" // -> optional. can be removed.

#include "ofxBeatClock.h"

class ofApp : public ofBaseApp 
{
public:
	void setup();
	void draw();

	ofxBeatClock beatClock;

	ofEventListener listenerBeat;
	void Changed_Tick();

	ofxWindowApp windowApp;
};
