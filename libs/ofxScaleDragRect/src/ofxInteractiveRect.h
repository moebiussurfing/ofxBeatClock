//
//  interactiveRect.h
//
//  Created by Roy Macdonald on 8/15/12.
//  Copyright (c) 2012 micasa. All rights reserved.
//
//  Updated by Aaron Richards on 13/05/2014.
//

#pragma once

#include "ofMain.h"



#define MARGEN 10

class ofxInteractiveRect : public ofRectangle
{
    
public:
    ofxInteractiveRect(string nombre);
    virtual ~ofxInteractiveRect();
        
    void enableEdit(bool enable = true);
    void disableEdit();
    void toggleEdit();
	bool isEditing(){return bIsEditing;}
	
	
    void draw();
    
    bool loadSettings(string nombre = "", string path = "", bool loadJson = false);
    void saveSettings(string nombre = "", string path = "", bool saveJson = false);
    
    
    void mouseMoved(ofMouseEventArgs & mouse);
    void mousePressed(ofMouseEventArgs & mouse);
    void mouseDragged(ofMouseEventArgs & mouse);
    void mouseReleased(ofMouseEventArgs & mouse);

	void mouseScrolled(ofMouseEventArgs & mouse);
	void mouseEntered(ofMouseEventArgs & mouse);
	void mouseExited(ofMouseEventArgs & mouse);

    float getRectX(){return x;}
    float getRectY(){return y;}
    float getRectWidth(){return width;}
    float getRectHeight(){return height;}
    string nombre;
    string path;
    
    ofRectangle getRect();
    void setRect(float x, float y, float width, float height)
    {
        this->x = x; 
        this->y = y;
        this->width = width;
        this->height = height;
    }
    
	
	ofJson toJson();
	void fromJson(const ofJson& j);
	
	ofXml toXml();
	bool fromXml(const ofXml& x);
	
	ofColor handleColor = {ofColor::yellow};
	
protected:
    bool bIsEditing;
    bool bMove;
	bool bIsOver;
    bool bLeft, bRight, bUp, bDown;
	bool bPressed;
    glm::vec2 mousePrev;
   
   
    
};
