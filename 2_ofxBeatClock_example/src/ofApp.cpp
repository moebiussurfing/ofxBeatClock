#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	//ofBackground(64);
	ofBackground(ofColor::orangeRed);

	//-

#ifdef USE_ofxWindowApp
	windowApp.setSettingsFps(60);//required when opening for first time, when no json settings created yet.
#else
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
#endif

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
	beatListener = beatClock.BeatTick_TRIG.newListener([&](bool&) {this->callback_BeatTick(); });

	//-

	//set lower level log
	ofSetLogLevel("ofxBeatClock", OF_LOG_VERBOSE);
}

//--------------------------------------------------------------
void ofApp::callback_BeatTick()
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
	if (ofGetFrameNum() % 15 == 0 && false)
		ofLogWarning("ofApp") << "";
}

//--------------------------------------------------------------
void ofApp::draw()
{
	beatClock.draw();

	//--

	if (bDEBUG) draw_Anchor(ofGetMouseX(), ofGetMouseY());
}

//--------------------------------------------------------------
void ofApp::exit()
{
	beatClock.exit();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch (key)
	{

	case ' '://toggle play/stop
		beatClock.togglePlay();
		break;

	case 't'://trig tap 4 times when using internal clock only to manually set the bpm on the fly
		beatClock.tap_Trig();
		break;

	case 'g'://show/hide gui panel
		beatClock.toggleVisible_GuiPanel();
		break;

	case OF_KEY_RETURN://get some beatClock info. look api methods into ofxBeatClock.h/.cpp
		ofLogWarning("ofApp") << "BPM: " << beatClock.getBPM();
		ofLogWarning("ofApp") << "BAR TIME: " << beatClock.getTimeBar() << "ms";
		break;

	case 'd'://enable debug mode
		beatClock.toggleDebug_Clock();//clock debug

		//beatClock.toggleDebug_Layout();//layout debug
		//bDEBUG = !bDEBUG;//to show mouse x,y position from ofApp
		break;

	default:
		break;

	}
}

void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
