//
//  TextWordBlock.h
//  superShoeVisuals
//
//  Created by josh ott on 3/1/13.
//
//
#pragma once

#include "ofMain.h"
#include "AniUtils.h"

#ifndef __superShoeVisuals__TextWordBlock__
#define __superShoeVisuals__TextWordBlock__

#include <iostream>

#endif /* defined(__superShoeVisuals__TextWordBlock__) */


class TextWordBlock{
	
public:
    void initParams(string theWord,ofTrueTypeFont theFont);
    void draw(float x, float y);
    void update(float animationSpeed);
    ofRectangle bounds;
    string word;
    int size;
    float fontHeight;
    ofTrueTypeFont font;

    int lineNumber;
    ofVec2f goalPosition;
    ofVec2f currentPosition;
    ofVec2f offset;
};
