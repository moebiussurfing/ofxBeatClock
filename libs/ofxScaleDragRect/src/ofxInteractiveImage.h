//
//  interactiveImage.h
//
//  Created by Roy Macdonald on 8/15/12.
//  Copyright (c) 2012 micasa. All rights reserved.
//
//  Updated by Aaron Richards on 13/05/2014.
//

#pragma once

#include "ofMain.h"
#include "ofxInteractiveRect.h"

class ofxInteractiveImage : public ofxInteractiveRect, public ofImage
{

public:
    
    ofxInteractiveImage(string nombre):ofxInteractiveRect::ofxInteractiveRect(nombre)
    {
        path = "";
	}
    
    ~ofxInteractiveImage(){}
    
    
    void setup(string path)
    {
		load(path);
        
    }
    void setup(string path, float x , float y, float w, float h)
    {
        
		load(path);
		
		this->ofxInteractiveRect::set(x,y,w,h);
		
    }
    
    bool load(string path)
    {
        this->path = path;
        if(ofImage::load(path))
        {
            this->ofxInteractiveRect::set(x,y,this->ofImage::getWidth(),this->ofImage::getHeight());
			ofLogVerbose("ofxInteractiveImage: setup") << "loaded \"" << path << "\" successfully!";
			return true;
        }
        else
        {
			ofLogVerbose("ofxInteractiveImage: setup") << " failed to load \"" << path ;
            
        }
		return false;
    }
    
    void draw()
    {
        draw(x, y);
    }
    
    void draw(float x, float y)
    {
        
        this->x = x;
        this->y = y;
        this->ofImage::draw(this->x, this->y, this->ofxInteractiveRect::width, this->ofxInteractiveRect::height);
        ofxInteractiveRect::draw();
        
    }
    
    void drawImgAtY(float y)
    {
        this->ofImage::draw(this->x, y, this->ofxInteractiveRect::width, this->ofxInteractiveRect::height );
    }
    
    void drawImgAtX(float x)
    {
        this->ofImage::draw(x, this->y, this->ofxInteractiveRect::width, this->ofxInteractiveRect::height );
    }
    
    bool isOver(float x, float y)
    {
        
        if (this->inside(x, y))
        {
            cout << this->ofImage::getColor(x, y) << endl;
            if(this->ofImage::getColor(x, y).a > 0 )
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
        
    }
};


