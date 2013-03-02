//
//  TextWordBlock.cpp
//  superShoeVisuals
//
//  Created by josh ott on 3/1/13.
//
//

#include "TextWordBlock.h"


void TextWordBlock::draw(float x, float y){
    font.drawStringAsShapes(word, x, y);
}

void TextWordBlock::initParams(string theWord,ofTrueTypeFont theFont){
    word = theWord;
    font = theFont;
    // will this work anywhere? or does it need to be in the draw loop.
    bounds = font.getStringBoundingBox(word, 0, 0);
}

