#pragma once
#include "ofMain.h"

#include "ofxBeatClock.h"

class ofApp : public ofBaseApp 
{
public:
	void setup();
	void keyPressed(int key);

	ofxBeatClock beatClock;

	ofEventListener listenerBeat;
	void Changed_BeatTick();
};
