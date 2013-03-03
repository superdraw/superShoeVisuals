//
//  IVTextLine.h
//  superShoeVisuals
//
//  Created by josh ott on 3/2/13.
//
//
#pragma once

#ifndef __superShoeVisuals__IVTextLine__
#define __superShoeVisuals__IVTextLine__

#include <iostream>

#endif /* defined(__superShoeVisuals__IVTextLine__) */

#include "TextWordBlock.h"


class IVTextLine{
	
    public:

    float maxHeight;
    float descenderHeight;
    vector<TextWordBlock>words;
    
    
};