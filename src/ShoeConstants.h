//
//  ShoeDataObject.h
//  superShoeVisuals
//
//  Created by josh ott on 2/27/13.
//
//
#pragma once

#ifndef superShoeVisuals_ShoeDataObject_h
#define superShoeVisuals_ShoeDataObject_h
#include <iostream>
#include <vector>

#define kShoeMaxTrackedValues 7

typedef struct{
    float values [kShoeMaxTrackedValues];
//    vector <float> values;
    float ax;
    float ay;
    float az;
    
    float gx;
    float gy;
    float gz;
    
    float force;
    int textIndex;
    float time;
    
    float singleTap;
    float freeFall;
    float activity;
float stepsPerMinute;

}ShoeDataObject;



#endif
