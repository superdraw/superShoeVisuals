

#pragma once

#include "ofMain.h"
#include "statistics.h"




//---------------------------------------------------------------
class dataDisplay {
    
public:
    
    
    float minVal, maxVal;
    vector < float > values;
    
    string name;
    string curValueString;
    bool bAmStringData;
    
    
    float currentValue;
    
    dataDisplay();
    void draw(ofRectangle rect);
    void addValue (float val);
    
    double sum, mean, var, dev, skew, kurt;
    
    void searchForAndDrawPeaks(ofRectangle rect);
    void searchForAndDrawHighMinsAndMaxes(ofRectangle rect);
        
    //computeStats(v.begin( ), v.end( ), sum, mean, var, dev, skew, kurt);

};
//---------------------------------------------------------------
