#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ShoeConstants.h"

#define kOscListenPort 12345

#define kObjectBufferSize 10

class testApp : public ofBaseApp{
	
	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased (int key);

		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofTrueTypeFont font;
		ofShader shader;
		bool doShader;
    
    
    
    ofImage img;
    int frameSequenceCounter;
    float distortAmt;
    float distortAmt2;
    float distortAmt3;
    float distortAmt4;
    
    //8 bits red, 8 bits green, 8 bits blue, from 0 to 255 in 256 steps
    ofFbo rgbaFbo; // with alpha
    void drawFbo();
    void analyzeShoeData();
    int fadeAmt;
    bool useFbo;
    
    ofImage gplusLabel;
    
    // OSC
    ofxOscReceiver receiver;
    string current_msg_string;
    
    ShoeDataObject currentShoeDataObject;
    vector<ShoeDataObject> dataObjects;
    ShoeDataObject shoeDataObjectWorking;
    
    int dataObjectInsertionIndex;
    
    int currentPhraseIndex;
    int oldPhraseIndex;
    int phraseWordIndex;
    vector<string> phrases;
    vector<string> currentPhraseWords;
    string currentPhraseWordString;
    float lastPhraseWordIteratedTime;
    float lastPhraseSelectedTime;
    int renderedPhraseLineCount;
};
