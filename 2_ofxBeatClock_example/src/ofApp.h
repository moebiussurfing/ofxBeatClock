#pragma once

#include "ofMain.h"

#include "ofxBeatClock.h"

//uncomment if you added this addon to handle window settings
//this addon auto stores/recall window size, position, fps and vsync between app sessions
//also it shows alert when fps drop the expected framerate.
#define USE_ofxWindowApp
#ifdef USE_ofxWindowApp
#include "ofxWindowApp.h"
#endif

class ofApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);


	//----

	ofxBeatClock beatClock;

	//callback to receive beat-clock beat-ticks
	ofEventListener beatListener;
	void callback_BeatTick();

	//----


#ifdef USE_ofxWindowApp
	ofxWindowApp windowApp;
#endif

	//-

	//debug mouse
	bool bDEBUG = false;
	//to show mouse position and x,y coordinates to debug layout
	void draw_Anchor(int x, int y)
	{
		ofPushStyle();
		ofFill();
		ofSetColor(ofColor::red);
		ofDrawCircle(x, y, 3);
		int pad;
		if (y < 15) pad = 20;
		else pad = -20;
		ofDrawBitmapStringHighlight(ofToString(x) + "," + ofToString(y), x, y + pad);
		ofPopStyle();
	}
};