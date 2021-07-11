#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetFrameRate(60);
	ofBackground(ofColor::orangeRed);

	beatClock.setup();

	listenerBeat = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->Changed_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_BeatTick()// callback to receive BeatTicks
{
	if (beatClock.BeatTick_TRIG) ofLogNotice(__FUNCTION__) << "BeatTick ! #" << beatClock.Beat_current;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch (key)
	{
		// toggle play / stop
	case ' ':
		beatClock.setTogglePlay();
		break;

		// trig tap-tempo 4 times 
		// when using internal clock 
		// only to set the bpm on the fly
	case 't':
		beatClock.tap_Trig();
		break;

		// get some beatClock info. look api methods into ofxBeatClock.h
	case OF_KEY_RETURN:
		ofLogWarning(__FUNCTION__) << "BPM     : " << beatClock.getBPM() << " beats per minute";
		ofLogWarning(__FUNCTION__) << "BAR TIME: " << beatClock.getTimeBar() << " ms";
		break;

	//	// debug
	//case 'd':
	//	beatClock.toggleDebug_Clock();//clock debug
	//	beatClock.toggleDebug_Layout();//layout debug
	//	break;

		// show gui controls
		#ifdef USE_OFX_GUI_EXTENDED2
	case 'g':
		beatClock.toggleVisible_GuiPanel();
		break;
#endif

		// show gui previews
	case 'G':
		beatClock.setToggleVisible_GuiPreview();
		break;
	}
}