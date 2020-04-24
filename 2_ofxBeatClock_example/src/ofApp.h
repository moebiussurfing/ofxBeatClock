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

class ofApp: public ofBaseApp{
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

    ofxBeatClock beatClock;

	//callback to receive clock ticks
	ofEventListener listener;
	void callback_Tick();

#ifdef USE_ofxWindowApp
	ofxWindowApp windowApp;
#endif
};
