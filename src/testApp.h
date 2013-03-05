#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ShoeConstants.h"
#include "TextWordBlock.h"
#include "IVTextLine.h"
#include "dataDisplay.h"

#define kOscListenPort 12345

#define kObjectBufferSize 40

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
		
		vector<ofTrueTypeFont> font;
        vector<TextWordBlock> wordBlocks;
        vector<IVTextLine> wordLines;
    int currentTotalLines;
    
//    TextWordBlock wordBlock1;
//    TextWordBlock wordBlock2;
    float textStartY;
		ofShader shader;
    ofShader FBOshader;
		bool doShader;
    
    ofColor bgColor;
    bool forceTextSize;
    ofImage img;
    int frameSequenceCounter;
    float distortAmt1;
    float distortAmt2;
    float distortAmt3;
    float distortAmt4;
    float distortAmt5;
    float distortAmt6;
    float distortAmt7;
    float distortAmt8;
    float distortAmt9;
    
    //8 bits red, 8 bits green, 8 bits blue, from 0 to 255 in 256 steps
    ofFbo rgbaFbo; // with alpha
    void drawFbo();
    void analyzeShoeData();
    int fadeAmt;
    bool useFbo;
    
    void drawBackground();
//    void drawLineFromData();
    void drawLineFromData(float x1, float x2, float y,float currentVal, float prevVal, float maxVal, float minVal, float height);
    
    ofImage gplusLabel;
    
    // OSC
    ofxOscReceiver receiver;
    string current_msg_string;
    
    // serial:
    ofSerial serial;
    bool serialInitSuccess;
    string message;
    
    void updateShoeDataObjectWithData(ShoeDataObject newData);
    ShoeDataObject currentShoeDataObject;
    ShoeDataObject currentShoeDataObjectSmoothed;
    ShoeDataObject maxValues;
    ShoeDataObject minValues;
    
    vector<ShoeDataObject> dataObjects;
    vector<dataDisplay>dataDisplays;
    
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
    
    int renderDataMode;
};

