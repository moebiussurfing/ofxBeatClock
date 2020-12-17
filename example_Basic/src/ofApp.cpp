#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	////log level
	//ofSetLogLevel("ofxBeatClock", OF_LOG_VERBOSE);
	//ofSetLogLevel("ofApp", OF_LOG_VERBOSE);

	//-

	//window
	float fps = 60.f;
#ifdef USE_ofxWindowApp
	windowApp.setFrameRate(fps);
	windowApp.setVerticalSync(true);
#else
	ofSetFrameRate(fps);
	ofSetVerticalSync(true);
#endif

	ofBackground(ofColor::orangeRed);

	//----

	beatClock.setup();

	//--

	//optional 
	//(you can uncomment one of the 3 blocks)

	////customize all layout elements "grouped" with default positions/distances/sizes
	//beatClock.setPosition_GuiGlobal(200, 50);

	////or customize positions with separated gui panel and gui extra elements
	//beatClock.setPosition_GuiExtra(500, 10);
	//beatClock.setPosition_GuiPanel(100, 100, 300);

	////or heavier customize positions and sizes element by element
	//beatClock.setPosition_GuiPanel(30, 400, 300);
	//beatClock.setPosition_BpmInfo(ofGetWidth()*0.5 - 100, ofGetHeight() - 50);
	//beatClock.setPosition_BeatBoxes(10, 10, ofGetWidth() - 20);
	//beatClock.setPosition_BeatBall(ofGetWidth()*0.5 - 100, ofGetHeight()*0.5 - 100, 200);
	//beatClock.setPosition_ClockInfo(ofGetWidth() - 200, ofGetHeight()*0.5 - 100);

	//-

	//callback to receive BeatTicks
	listenerBeat = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->Changed_BeatTick(); });
}

//--------------------------------------------------------------
void ofApp::Changed_BeatTick()
{
	//TODO:
	//could improve callback to get bool value too to avoid to check state like this..
	if (beatClock.BeatTick_TRIG && false)
		ofLogWarning("ofApp") << "BeatTick ! " << beatClock.Beat_current;
}

//--------------------------------------------------------------
void ofApp::update()
{
	beatClock.update();

	//add a log line every x frames to spread received and logged BeatTicks on callbacks
	if (ofGetFrameNum() % 15 == 0 && false) ofLogWarning("ofApp") << "";
}

//--------------------------------------------------------------
void ofApp::draw()
{
	beatClock.draw();

	//--

	if (bDEBUG) ofxSurfingHelpers::draw_Anchor(ofGetMouseX(), ofGetMouseY());
}

//--------------------------------------------------------------
void ofApp::exit()
{
	//beatClock.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch (key)
	{

		//toggle play/stop
	case ' ':
		beatClock.setTogglePlay();
		break;

		//trig tap-tempo 4 times 
		//when using internal clock 
		//only to set the bpm on the fly
	case 't':
		beatClock.tap_Trig();
		break;

		//get some beatClock info. look api methods into ofxBeatClock.h
	case OF_KEY_RETURN:
		ofLogWarning("ofApp") << "BPM     : " << beatClock.getBPM() << " beats per minute";
		ofLogWarning("ofApp") << "BAR TIME: " << beatClock.getTimeBar() << "ms";
		break;

		//enable debug
	case 'd':
		beatClock.toggleDebug_Clock();//clock debug
		//beatClock.toggleDebug_Layout();//layout debug
		break;

		//enable debug
	case 'D':
		bDEBUG = !bDEBUG;//to show mouse x,y position from ofApp
		break;

		//show/hide gui panel
	case 'g':
		beatClock.toggleVisible_GuiPanel();
		break;

	}
}