#pragma once
#include "ofMain.h"

#include "ofxBeatClock.h"

//uncomment if you added this addon to handle window settings
#define USE_ofxWindowApp
#ifdef USE_ofxWindowApp
#include "ofxWindowApp.h"
#endif

class ofApp : public ofBaseApp 
{
public:
	void setup();
	void update();
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
