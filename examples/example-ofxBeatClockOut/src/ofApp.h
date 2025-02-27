#pragma once
#include "ofMain.h"

#include "ofxWindowApp.h" // -> Not required. Can be removed.

#include "ofxBeatClock.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void draw();
	void exit();

	ofxBeatClock beatClock;
	ofEventListener listenerBeat;
	void Changed_Tick();

	ofxWindowApp w;
};
