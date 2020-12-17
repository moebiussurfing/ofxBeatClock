
/*	ABOUT:
	This examples show how to sync with Ableton Live using Link
	To run the example out-of-the-box, uncomment the line: 
	#define USE_ofxAbletonLink
	into: ofxBeatClock.h", to enable Link.
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
