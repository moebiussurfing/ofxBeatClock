//
//  ofxDawMetro.h
//  ofxDawMetro
//
//  Created by Ean Jee on 11/14/16.
//
// +

#ifndef ofxDawMetro_h
#define ofxDawMetro_h

#include "ofThread.h"
#include "ofTimer.h"
#include "ofEvent.h"

class ofxDawMetro : public ofThread {
public:

	class MetroListener {
	public:
		virtual void onBarEvent(int & bar) {}
		virtual void onBeatEvent(int & beat) {}
		virtual void onSixteenthEvent(int & sixteenth) {}
	};

	ofxDawMetro();
	virtual ~ofxDawMetro();

	void setBpm(float bpm);

	int getBar() { return mBarCount; }
	int getBeat() { return mBeatCount; }
	int getSixteenth() { return mSixteenthCount; }

	void start();
	void stop();
	void toggle();
	void resetTimer();

	void addBarListener(MetroListener * listener);
	void addBeatListener(MetroListener * listener);
	void addSixteenthListener(MetroListener * listener);

	void removeBarListener(MetroListener * listener);
	void removeBeatListener(MetroListener * listener);
	void removeSixteenthListener(MetroListener * listener);

protected:

	virtual void threadedFunction() override;

	ofEvent<int> mBarEvent;
	ofEvent<int> mBeatEvent;
	ofEvent<int> mSixteenthEvent;

	int mBarCount;
	int mBeatCount;
	int mSixteenthCount;

	ofTimer mTimer;

	// timer period for 16th TARGET_NOTES_params in nanoseconds
	uint64_t mPeriod;

	std::vector<MetroListener *> mBarListeners;
	std::vector<MetroListener *> mBeatListeners;
	std::vector<MetroListener *> mSixteenthListeners;
};

#endif /* ofxDawMetro_h */
