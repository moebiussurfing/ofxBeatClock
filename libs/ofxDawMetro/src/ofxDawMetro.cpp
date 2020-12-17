//
//  ofxDawMetro.cpp
//  ofxDawMetro
//
//  Created by Ean Jee on 11/14/16.
//
//

#include "ofxDawMetro.h"
#include "ofEventUtils.h"
#include <algorithm>

ofxDawMetro::ofxDawMetro() {

}

ofxDawMetro::~ofxDawMetro() {
	for (auto & listener : mBarListeners) {
		ofRemoveListener(mBarEvent, listener, &MetroListener::onBarEvent);
	}

	for (auto & listener : mBeatListeners) {
		ofRemoveListener(mBeatEvent, listener, &MetroListener::onBeatEvent);
	}

	for (auto & listener : mSixteenthListeners) {
		ofRemoveListener(mSixteenthEvent, listener, &MetroListener::onSixteenthEvent);
	}

	stop();
}

void ofxDawMetro::setBpm(float bpm) {
	mPeriod = (uint64_t)(60 / bpm / 4 * 1000 * 1000 * 1000);
}

void ofxDawMetro::start() {

	mBarCount = mBeatCount = mSixteenthCount = 1;
	if (!isThreadRunning()) {

		ofNotifyEvent(mSixteenthEvent, mSixteenthCount);
		ofNotifyEvent(mBeatEvent, mBeatCount);
		ofNotifyEvent(mBarEvent, mBarCount);
		mTimer.setPeriodicEvent(mPeriod);

		startThread();
	}
}

void ofxDawMetro::stop() {
	if (isThreadRunning()) {
		stopThread();
		waitForThread();
	}
}

void ofxDawMetro::toggle() {
	if (isThreadRunning()) {
		stop();
	}
	else {
		start();
	}
}

void ofxDawMetro::resetTimer() {
	mTimer.setPeriodicEvent(mPeriod);
}

void ofxDawMetro::threadedFunction() {
	while (isThreadRunning()) {
		mTimer.waitNext();

		mSixteenthCount++;

		if (mSixteenthCount > 4) {
			mSixteenthCount = 1;
			mBeatCount++;
			
			if (mBeatCount > 4) {
				mBeatCount = 1;
				mBarCount++;

				ofNotifyEvent(mBarEvent, mBarCount);
			}

			ofNotifyEvent(mBeatEvent, mBeatCount);
		}

		ofNotifyEvent(mSixteenthEvent, mSixteenthCount);
	}
}

void ofxDawMetro::addBarListener(MetroListener * listener) {
	ofAddListener(mBarEvent, listener, &MetroListener::onBarEvent);
	mBarListeners.push_back(listener);
}

void ofxDawMetro::addBeatListener(MetroListener * listener) {
	ofAddListener(mBeatEvent, listener, &MetroListener::onBeatEvent);
	mBeatListeners.push_back(listener);
}

void ofxDawMetro::addSixteenthListener(MetroListener * listener) {
	ofAddListener(mSixteenthEvent, listener, &MetroListener::onSixteenthEvent);
	mSixteenthListeners.push_back(listener);
}

void ofxDawMetro::removeBarListener(MetroListener * listener) {
	ofRemoveListener(mBarEvent, listener, &MetroListener::onBarEvent);
	std::remove(mBarListeners.begin(), mBarListeners.end(), listener);
}

void ofxDawMetro::removeBeatListener(MetroListener * listener) {
	ofRemoveListener(mBeatEvent, listener, &MetroListener::onBeatEvent);
	std::remove(mBeatListeners.begin(), mBeatListeners.end(), listener);
}

void ofxDawMetro::removeSixteenthListener(MetroListener * listener) {
	ofRemoveListener(mSixteenthEvent, listener, &MetroListener::onSixteenthEvent);
	std::remove(mSixteenthListeners.begin(), mSixteenthListeners.end(), listener);
}