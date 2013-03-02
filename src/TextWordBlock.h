//
//  TextWordBlock.h
//  superShoeVisuals
//
//  Created by josh ott on 3/1/13.
//
//
#pragma once

#include "ofMain.h"

#ifndef __superShoeVisuals__TextWordBlock__
#define __superShoeVisuals__TextWordBlock__

#include <iostream>

#endif /* defined(__superShoeVisuals__TextWordBlock__) */


class TextWordBlock{
	
public:
    void initParams(string theWord,ofTrueTypeFont theFont);
    void draw(float x, float y);
    ofRectangle bounds;
    string word;
    int size;
    ofTrueTypeFont font;

};
