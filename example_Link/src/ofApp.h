
/*	ABOUT:
	This examples show how to sync with Ableton Live using Link
	To run the example out-of-the-box, go into: ofxBeatClock.h",
	and uncomment the below line (to enable Link): 
	#define USE_ofxAbletonLink
	
*/

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
