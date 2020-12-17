//
//  interactiveRect.cpp
//
//  Created by Roy Macdonald on 8/15/12.
//  Copyright (c) 2012 micasa. All rights reserved.
//
//  Updated by Aaron Richards on 13/05/2014.
//

#include "ofxInteractiveRect.h"



//--------------------------------------------------------------
ofxInteractiveRect::ofxInteractiveRect(string nombre)
{
    bIsEditing = false;
    bMove = false;
    bLeft = false;
    bRight = false;
    bUp = false;
    bDown = false;
	bIsOver = false;
    this->nombre = nombre;
    this->path = "";
}
//--------------------------------------------------------------
ofxInteractiveRect::~ofxInteractiveRect()
{
}

//--------------------------------------------------------------
void ofxInteractiveRect::enableEdit(bool enable)
{
    
	ofLogVerbose("ofxInteractiveRect::enableEdit") << "interactiveRect " << this->nombre <<" enableEdit " << (string)(enable?"true":"false");
    
    if (enable != bIsEditing)
    {
        if (enable)
        {
            ofRegisterMouseEvents(this);
        }
        else
        {
            ofUnregisterMouseEvents(this);
          //  saveSettings();
        }
        bIsEditing = enable;
        
    }
    
}

//--------------------------------------------------------------
void ofxInteractiveRect::disableEdit()
{
    enableEdit(false);
}	

//--------------------------------------------------------------
void ofxInteractiveRect::toggleEdit()
{
    enableEdit(!bIsEditing);
}

//--------------------------------------------------------------
ofRectangle ofxInteractiveRect::getRect()
{
    return ofRectangle(x, y, width, height);
}

//--------------------------------------------------------------
void ofxInteractiveRect::saveSettings(string nombre, string path, bool saveJson)
{
    
    if(nombre!="")
    {
        this->nombre = nombre;
    }
    
    if (path!="")
    {
        this->path = path;
    }

	string filename = this->path + "settings" + this->nombre ;
	 
	if(saveJson){
		filename += ".json";
		ofSaveJson(filename, toJson());
	}else{
		filename += ".xml";
		toXml().save(filename);
	}
	
	
    
	ofLogVerbose("ofxInteractiveRect::saveSettings") << "saved settings: "<< filename;
    
}


ofJson ofxInteractiveRect::toJson()
{
	ofJson j;//("interactiveRect");
	

	j["x"] = this->ofRectangle::x;
	j["y"] = this->ofRectangle::y;
	j["width"] =  this->ofRectangle::width;
	j["height"] =  this->ofRectangle::height;
	j["nombre"] =  this->nombre;
	j["path"] =  this->path;
	j["isEditing"] = this->bIsEditing;
	return j;
}

void ofxInteractiveRect::fromJson(const ofJson& j)
{
	

	j["x"].get_to(this->ofRectangle::x);
	j["y"].get_to(this->ofRectangle::y);
	j["width"].get_to(this->ofRectangle::width);
	j["height"].get_to(this->ofRectangle::height);
	j["nombre"].get_to(this->nombre);
	j["path"].get_to(this->path);
	bool editing;
	
	j["isEditing"].get_to(editing);
	enableEdit(editing);
	
}

ofXml ofxInteractiveRect::toXml()
{
	ofXml xml;
	auto r = xml.appendChild("interactiveRect");
	
	r.appendChild("path").set(this->path);
	r.appendChild("x").set(this->ofRectangle::x);
	r.appendChild("y").set(this->ofRectangle::y);
	r.appendChild("width").set(this->ofRectangle::width);
	r.appendChild("height").set(this->ofRectangle::height);
	r.appendChild("nombre").set(this->nombre);
	r.appendChild("isEditing").set(this->bIsEditing);

	return xml;
}

bool ofxInteractiveRect::fromXml(const ofXml& xml)
{
	
	auto r = xml.getChild("interactiveRect");
	if(r)
	{
		this->path = r.getChild("path").getValue();
		this->ofRectangle::x = r.getChild("x").getFloatValue();
		this->ofRectangle::y = r.getChild("y").getFloatValue();
		this->ofRectangle::width = r.getChild("width").getFloatValue();
		this->ofRectangle::height = r.getChild("height").getFloatValue();
		this->nombre = r.getChild("nombre").getValue();
		enableEdit(r.getChild("isEditing").getBoolValue());
		
		return true;
	}
	
	
	return false;
}





//--------------------------------------------------------------
bool ofxInteractiveRect::loadSettings(string nombre, string path, bool loadJson)
{
    if(nombre!="")
    {
        this->nombre = nombre;
    }
    
    if (path!="")
    {
        this->path = path;
    }

	string filename = this->path + "settings" + this->nombre ;
	 
	if(loadJson)
	{
		filename += ".json";
		
		fromJson(ofLoadJson(filename));
		return true;
		
	}else{
		filename += ".xml";
		ofXml xml;
		if(xml.load(filename))
		{
			if(fromXml(xml))
			{
				return true;
			}
		}
	}
	ofLogVerbose("ofxInteractiveRect::loadSettings") << "unable to load : "<< filename;
	
	return false;
        
}
    

//--------------------------------------------------------------
void ofxInteractiveRect::draw()
{
    
	if (bIsEditing)
    {
    
        ofPushStyle();
		if (bIsOver)
        {
			if (bPressed)
            {
				ofSetColor(50, 200);				
			}
            else
            {
				ofSetColor(50, 70);				
			}
			ofNoFill();
			ofDrawRectangle(*this);
			
		}
        
		ofFill();
        if (bMove)
        {
            ofSetColor(127, 127);
            ofDrawRectangle(*this);
        }
        else
        {
            ofSetColor(handleColor, 150);
            
            if (bUp)
            {
                ofDrawRectangle(x, y, width, MARGEN);
            }
            else if(bDown)
            {
                ofDrawRectangle(x, y + height - MARGEN, width, MARGEN);
            }
            
            if (bLeft)
            {
                ofDrawRectangle(x, y, MARGEN, height);
            }
            else if(bRight)
            {
                ofDrawRectangle(x + width - MARGEN, y, MARGEN, height);
            }
        }
        ofPopStyle();
        
	}
    
}

//--------------------------------------------------------------
void ofxInteractiveRect::mouseMoved(ofMouseEventArgs & mouse)
{
    
	if (!bPressed)
    {
		
		bIsOver = inside(mouse.x, mouse.y);

		bLeft = false;
		bRight = false;
		bUp = false;
		bDown = false;
	
        if (bIsOver)
        {
			bMove = true;
		
            if (mouse.x < x+MARGEN && mouse.x > x)
            {
				bLeft = true;
				bMove = false;
			}
            else if(mouse.x < x + width && mouse.x > x + width - MARGEN)
            {
				bRight = true;
				bMove = false;
			}
			
            if (mouse.y > y && mouse.y < y + MARGEN)
            {
				bUp = true;
				bMove = false;
			}
            else if(mouse.y > y + height - MARGEN && mouse.y < y + height)
            {
				bDown = true;
				bMove = false;
			}
		}
        else
        {
			bMove = false;
		}

	}
    
}
//--------------------------------------------------------------
void ofxInteractiveRect::mousePressed(ofMouseEventArgs & mouse)
{
    
	mousePrev = mouse;
	bPressed = true;
    bIsOver = inside(mouse.x, mouse.y);
    
    bLeft = false;
    bRight = false;
    bUp = false;
    bDown = false;
    
    if (bIsOver)
    {
        bMove = true;
        if (mouse.x < x+MARGEN && mouse.x > x)
        {
            bLeft = true;
            bMove = false;
        }
        else if(mouse.x < x + width && mouse.x > x + width - MARGEN)
        {
            bRight = true;
            bMove = false;
        }
        if (mouse.y > y && mouse.y < y + MARGEN)
        {
            bUp = true;
            bMove = false;
        }
        else if(mouse.y > y + height - MARGEN && mouse.y < y + height)
        {
            bDown = true;
            bMove = false;
        }
    }
    else
    {
        bMove = false;
    }

}
//--------------------------------------------------------------
void ofxInteractiveRect::mouseDragged(ofMouseEventArgs & mouse)
{
    
    if (bUp)
    {
        y += mouse.y - mousePrev.y;
        height += mousePrev.y- mouse.y;
    }
    else if (bDown)
    {
        height += mouse.y - mousePrev.y;
    }
    if (bLeft)
    {
        x += mouse.x - mousePrev.x;
        width += mousePrev.x - mouse.x;
    }
    else if (bRight)
    {
        width += mouse.x - mousePrev.x;
    }
    if (bMove)
    {
        x += mouse.x - mousePrev.x;
        y += mouse.y - mousePrev.y;
    }
    
	mousePrev = mouse;
    
}
//--------------------------------------------------------------
void ofxInteractiveRect::mouseReleased(ofMouseEventArgs & mouse)
{
    
    bMove = false;
    bLeft = false;
    bRight = false;
    bUp = false;
    bDown = false;
	bPressed = false;
    
}



//--------------------------------------------------------------
void ofxInteractiveRect::mouseScrolled(ofMouseEventArgs & mouse) {}
void ofxInteractiveRect::mouseEntered(ofMouseEventArgs & mouse) {}
void ofxInteractiveRect::mouseExited(ofMouseEventArgs & mouse) {}
