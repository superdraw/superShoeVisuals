#include "dataDisplay.h"



dataDisplay::dataDisplay(){
    minVal = 100000;
    maxVal = -1000000;
}


void dataDisplay::searchForAndDrawHighMinsAndMaxes(ofRectangle rect){
    
    
    
    for (int i = 0; i < values.size(); i++){
        
        float pct = ofMap(values[i], minVal - 0.0000001, maxVal + 0.000001, 0,1);
        ofPoint pt;
        pt.x = ofMap(i, 0, values.size()-1, rect.x, rect.x + rect.width);
        pt.y = ofMap(values[i], minVal - 0.0000001, maxVal + 0.000001, rect.y + rect.height,  rect.y);
        //cout << pt << endl;
        //ofVertex(pt.x, pt.y);
        if (pct > 0.95 || pct < 0.05){
            ofCircle(pt, 3);
        }
    }
}


void dataDisplay::searchForAndDrawPeaks(ofRectangle rect){
    
    // twin peaks!
    
    /*
     Between any two points in your data, (x(0),y(0)) and (x(n),y(n)), add up y(i+1)-y(i) for 0 <= i < n and call this T ("travel") and set R ("rise") to y(n)- y(0) + k for suitably small k. T/R > 1 indicates a peak. This works OK if large travel due to noise is unlikely or if noise distributes symmetrically around a base curve shape. For your application, accept the earliest peak with a score above a given threshold, or analyze the curve of travel per rise values for more interesting properties.
     */
    
//    int nSamples = values.size();
//    
//    int nSamplesToTest = 5;
//    if (nSamples > nSamplesToTest){
//        for (int i = 0; i < nSamples-nSamplesToTest; i++){
//            
//            int start = i;
//            int end = i+nSamplesToTest;
//            
//            float t = 0;  // travel;
//            float r = values[end] - values[start] + 0.1f;
//            for (int j = start; j < end-1; j++){
//                t += values[j+1] - values[j];
//            }
//            if (r > 0.00001){
//                float tToR = t / r;
//                
//                if (tToR > 1){
//                    ofPoint pt;
//                    pt.x = ofMap(end, 0, values.size()-1, rect.x, rect.x + rect.width);
//                    pt.y = ofMap(values[end], minVal - 0.0000001, maxVal + 0.000001, rect.y + rect.height,  rect.y);
//                    ofCircle(pt.x, pt.y, 3);
//                }
//            }
//        }
//    }
    
    
}
    
void dataDisplay::draw(ofRectangle rect){
    ofNoFill();
    ofSetColor(255);
    ofRect(rect.x, rect.y, rect.width, rect.height);
    
    ofFill();
    
    if (values.size() > 0){
    
    ofBeginShape();
    ofVertex(rect.x, rect.y + rect.height);
    for (int i = 0; i < values.size(); i++){
        ofPoint pt;
        pt.x = ofMap(i, 0, values.size()-1, rect.x, rect.x + rect.width);
        pt.y = ofMap(values[i], minVal - 0.0000001, maxVal + 0.000001, rect.y + rect.height,  rect.y);
        //cout << pt << endl;
        ofVertex(pt.x, pt.y);
    }
    ofVertex(rect.x + rect.width, rect.y + rect.height);
    ofEndShape(true);
    
    ofSetColor(100,100,100);
    
    ofPoint pt;
    pt.y = ofMap(mean, minVal - 0.0000001, maxVal + 0.000001, rect.y + rect.height,  rect.y);
    ofLine(rect.x, pt.y, rect.x + rect.width, pt.y);
        
        ofEnableAlphaBlending();
        ofSetColor(255,0,0,30);
        ofPoint ptA; ptA.y = ofMap(mean + dev, minVal - 0.0000001, maxVal + 0.000001, rect.y + rect.height,  rect.y);
        ofPoint ptB; ptB.y = ofMap(mean - dev, minVal - 0.0000001, maxVal + 0.000001, rect.y + rect.height,  rect.y);
        ofRect(rect.x, ptA.y,rect.width, ptB.y-ptA.y);

    }
    
//    
//    int count = values.size();
//    bool [count];
    
    
    searchForAndDrawPeaks(rect);
    searchForAndDrawHighMinsAndMaxes(rect);
    
   // ofDrawBitmapStringHighlight(name, ofPoint(rect.x + rect.width, rect.y + rect.height), ofColor::black, ofColor::cyan);
    
}

    
void dataDisplay::addValue(float val){
    
        currentValue = val;
    
    
        // cout << val << endl;
        values.push_back(val);
        if (values.size() > 80){
            values.erase(values.begin());
        }
        
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    
        computeStats(values.begin( ), values.end( ), sum, mean, var, dev, skew, kurt);
}