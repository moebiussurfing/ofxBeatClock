
#pragma once

#include "ofMain.h"

#include "ofxWindowApp.h" // -> optional. can be removed.

/*
	NOTE:
	This example shows how to sync with Ableton Live using Link.
	To run the example out-of-the-box, go into the top of the file: 
	ofxBeatClock.h", after OPTIONAL DEFINES
	and uncomment the below line (to enable Link): 
	#define USE_ofxAbletonLink
	If you want to exclude the Link feature/add-on, just comment the line!
	//#define USE_ofxAbletonLink
*/

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
