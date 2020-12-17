//
//  ofxInteractiveVideo.h
//
//  Created by Aaron Richards on 13/05/2014.
//
//

#ifndef scaleDrag_ofxInteractiveVideo_h
#define scaleDrag_ofxInteractiveVideo_h

#include "ofMain.h"
#include "ofxInteractiveRect.h"

class ofxInteractiveVideo : public ofxInteractiveRect, public ofVideoPlayer
{
    
public:
    
    ofxInteractiveVideo(string nombre):ofxInteractiveRect::ofxInteractiveRect(nombre)
    {
        path = "";
	}
    
    ~ofxInteractiveVideo()
    {
        ofRemoveListener(ofEvents().update, this, &ofxInteractiveVideo::updateVideo);
    }
    
    
    void setup(string path)
    {
        
		load(path);
        
    }
    
    void setup(string path, float x, float y, float w, float h)
    {
        if(load(path))
        {
            this->ofxInteractiveRect::set(x,y,w,h);
        }
    }
	
	bool load(string path)
    {
        this->path = path;
        if(ofVideoPlayer::load(path))
        {
            this->ofxInteractiveRect::set(x,y,this->ofVideoPlayer::getWidth(),this->ofVideoPlayer::getHeight());
			ofLogVerbose("ofxInteractiveVideo::load") << "loaded \"" << path << "\" successfully!";
			ofAddListener(ofEvents().update, this, &ofxInteractiveVideo::updateVideo);
			return true;
        }
        else
        {
			ofLogVerbose("ofxInteractiveVideo::load") << " failed to load \"" << path ;
            
        }
		return false;
    }
	
    
    void updateVideo(ofEventArgs &e)
    {
        this->update();
    }
    
    void draw()
    {
        draw(x, y);
    }
    
    void draw(float x, float y)
    {
        
        this->x = x;
        this->y = y;
        this->ofVideoPlayer::draw(this->x, this->y, this->ofxInteractiveRect::width, this->ofxInteractiveRect::height);
        ofxInteractiveRect::draw();
        
    }
    
    void drawImgAtY(float y)
    {
        this->ofVideoPlayer::draw(this->x, y, this->ofxInteractiveRect::width, this->ofxInteractiveRect::height );
    }
    
    void drawImgAtX(float x)
    {
        this->ofVideoPlayer::draw(x, this->y, this->ofxInteractiveRect::width, this->ofxInteractiveRect::height );
    }
    
    
};

#endif
