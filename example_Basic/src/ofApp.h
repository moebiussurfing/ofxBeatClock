#pragma once
#include "ofMain.h"

#include "ofxBeatClock.h"

class ofApp : public ofBaseApp 
{
public:
	void setup();
	void keyPressed(int key);

	//-

	ofxBeatClock beatClock;

	//callback to receive beat-ticks
	ofEventListener listenerBeat;
	void Changed_BeatTick();

	//-

#ifdef USE_ofxWindowApp
	ofxWindowApp windowApp;
#endif
};
