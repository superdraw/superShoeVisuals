//
//  TextWordBlock.cpp
//  superShoeVisuals
//
//  Created by josh ott on 3/1/13.
//
//

#include "TextWordBlock.h"


void TextWordBlock::update(float animationSpeed){
    currentPosition.x += easeToInc(currentPosition.x, goalPosition.x, animationSpeed, .00001);
    currentPosition.y += easeToInc(currentPosition.y, goalPosition.y, animationSpeed, .00001);
    
    offset.x+=easeToInc(offset.x, 0, animationSpeed, .00001);
    offset.y+=easeToInc(offset.y, 0, animationSpeed, .00001);
}
void TextWordBlock::draw(float x, float y){
    
    ofFill();
    font.drawStringAsShapes(word, x, y);
    // draw bounds for our info
//    ofNoFill();
//    ofRect(x, y+bounds.y, bounds.width, bounds.height);
}
void TextWordBlock::initParams(string theWord,ofTrueTypeFont theFont){
    word = theWord;
    font = theFont;
    // will this work anywhere? or does it need to be in the draw loop.
    bounds = font.getStringBoundingBox(word, 0, 0);
}

