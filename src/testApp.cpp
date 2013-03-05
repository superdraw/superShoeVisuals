#include "testApp.h"
#include "Poco/RegularExpression.h"

#define kFBORenderScale .5
#define kFBOWidth 1920
#define kFBOHeight 1080


#define kTextRenderAlpha 110


using Poco::RegularExpression;



vector < string > getMatchedStrings (string stringToParse, string regex );
vector < string > getMatchedStrings (string contents, string regex, int & lastPos ){
    
    vector < string > results;
    RegularExpression regEx(regex);
    RegularExpression::Match match;
    
    lastPos = 0;
    while(regEx.match(contents, match) != 0) {
        
        // we get the sub string from the content
        // and then trim the content so that we
        // can continue to search
        string foundStr = contents.substr(match.offset, match.length);
        contents = contents.substr(match.offset + match.length);
        
        
        lastPos += match.offset + match.length;
        
        results.push_back(foundStr);
        
    }
    return results;
}

//--------------------------------------------------------------
void testApp::setup(){
    // listen on the given port
	cout << "listening for osc messages on port " << kOscListenPort << "\n";
	receiver.setup(kOscListenPort);
    current_msg_string = "0";
    
    // setup serial:
    serial.listDevices();
    //std::exit(0);
    ofSetLogLevel(OF_LOG_VERBOSE);
    // TODO: get this from config file!
    // MARK: SERIAL INIT (now just on spacebar)
//    serialInitSuccess = serial.setup("cu.FireFly-AD9F-SPP", 57600);
    
    
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofBackground(0);
	ofSetVerticalSync(false );
    // something
    // allocate FBO
//    rgbaFbo.allocate(1080, 960, GL_RGBA); // with alpha, 8 bits red, 8 bits green, 8 bits blue, 8 bits alpha, from 0 to 255 in 256 steps
    rgbaFbo.allocate(kFBOWidth, kFBOHeight, GL_RGBA32F_ARB); // with alpha, 32 bits red, 32 bits green, 32 bits blue, 32 bits alpha, from 0 to 1 in 'infinite' steps
    // we can also define the fbo with ofFbo::Settings.
	// this allows us so set more advanced options the width (400), the height (200) and the internal format like this
	/*
	 ofFbo::Settings s;
	 s.width			= 400;
	 s.height			= 200;
	 s.internalformat   = GL_RGBA;
	 s.useDepth			= true;
	 // and assigning this values to the fbo like this:
	 rgbFbo.allocate(s);
	 */
    
    
    // TODO: load config textfile here for BGCOLOR:
    ;
    bgColor.r = 0;
    bgColor.g = 120;
    bgColor.b = 255;
    fadeAmt = 100;
    // we have to clear all the fbos so that we don't see any artefacts
	// the clearing color does not matter here, as the alpha value is 0, that means the fbo is cleared from all colors
	// whenever we want to draw/update something inside the fbo, we have to write that inbetween fbo.begin() and fbo.end()
    
    rgbaFbo.begin();
	ofClear(255,255,255, 255);
    rgbaFbo.end();

	ofEnableAlphaBlending();
		
	//we load a font and tell OF to make outlines so we can draw it as GL shapes rather than textures
    int fontSteps = 3;
    for (int i=0; i<fontSteps; i++) {
        float size = 100+ ((float)i/(fontSteps-1))*100;
        cout << "loading fontsize " << size << "\n";
        ofTrueTypeFont tempFont;
        tempFont.loadFont("type/OpenSans-ExtraBold.ttf", size, true, false, true, 0.2, 100);
        font.push_back(tempFont);
    }
    
	shader.load("shaders/noise.vert", "shaders/noise.frag");
    // just use the same one for now:
    FBOshader.load("shaders/fboShader.vert","shaders/fboShader.frag");
//    FBOshader.load("shaders/")
    
    gplusLabel.allocate(152, 152, OF_IMAGE_COLOR_ALPHA);

    gplusLabel.loadImage("images/gplus_corner.png");
    gplusLabel.resize(152*kFBORenderScale, 152*kFBORenderScale);
	
	doShader = true;
    
    useFbo = true;
    
    forceTextSize = true;
    
    currentShoeDataObject.force = 0;
    
    
    // load the phrases from text file
    ofBuffer buffer = ofBufferFromFile("shoeSayings.txt");
    
    if(buffer.size()) {
        
        // we now keep grabbing the next line
        // until we reach the end of the file
        while(buffer.isLastLine() == false) {
            
            // move on to the next line
            string line = buffer.getNextLine();
            
            // copy the line to draw later
            // make sure its not a empty line
            if(line.empty() == false) {
                phrases.push_back(line);
            }
            
            // print out the line
           // cout << line << endl;
            
        }
    }else{
        
        cout << "something went wrong with textfile load!";
    }
    currentPhraseIndex = 0;
    oldPhraseIndex = -1;
    // maybe the reserve isn't necessary?
//    for (int i =0; i<kObjectBufferSize; i++) {
//        <#statements#>
//    }
//    
//    dataObjects.reserve(kObjectBufferSize);
}

//--------------------------------------------------------------
void testApp::update(){

    // ______________________________________SERIAL UPDATING
    // MARK: serial update
    // zach's code:
    
    //------------------------------------------------------------ read serial
    // todo: work here for good serial read and monitor connection.

    unsigned char bytes[100];
    if(serialInitSuccess){
        int howMany = serial.readBytes(bytes, 100);
        
        for (int i = 0; i < howMany; i++){
            message += (bytes[i]);
        }
    }
    
    //------------------------------------------------------------ regular expression against serial
    int pos = 0;
    vector < string > strs  = getMatchedStrings(message, "\\$.*?#", pos);
    
    
    //------------------------------------------------------------ chomp the message data if we got good values
    
    if (message.size() > 0){
        message = message.substr(pos, message.size());
    }
    
    //------------------------------------------------------------ iterate through the messages, break them up by commas
    
    for (int i = 0; i < strs.size(); i++){
        
        string val = strs[i].substr(1, strs[i].size()-2);
        vector < string > valStrings = ofSplitString(val, ",");         // split on a comma
        
        
        int count = valStrings.size();                                  // count the number of values we are recording
        
//        if (displays.size() != count/2){                                // this is the dynamic part :)
//            displays.resize(count/2);
//        }
        // populate the data displays
        // make a new shoedata object:
        ShoeDataObject newShoeData;
        string name = "";
        for (int j = 0; j < count; j++){
            
            if (j % 2 == 0) name = valStrings[j];
            else {
              //  displays[j/2].name = name;
                bool isStringVal = false;
                if (name.size() > 4){
                    if (name.substr(name.size()-4, name.size()-1) == "_str"){
                        isStringVal = true;
                    }
                }
                
                if (!isStringVal){
                    float val = ofToFloat(valStrings[j]);
                    // TODO: check against names of stuff here and populate our object:
//                    cout<< name << " = " << val << "\n";
                    if(name == "fsr"){
                        // FORCE
                        newShoeData.force = val;
                        newShoeData.values[0] = val;
                    }else if (name == "accel_x"){
                        newShoeData.ax = val;
                        newShoeData.values[1] = val;
                    }else if (name == "accel_z"){
                        newShoeData.az = val;
                        newShoeData.values[2] = val;
                    }else if (name == "playingId"){
                        newShoeData.textIndex = val;
                    }else if (name == "singleTap"){
                        newShoeData.singleTap = val;
                        newShoeData.values[3] = val;
                    }else if (name == "freeFall"){
                        newShoeData.freeFall = val;
                        newShoeData.values[4] = val;
                    }else if (name == "activity"){
                        newShoeData.activity = val;
                        newShoeData.values[5] = val;
                    }else if (name == "stepsPerMinute"){
                        newShoeData.stepsPerMinute = val;
                        newShoeData.values[6] = val;
                    }
//                    displays[j/2].addValue(val);
//                    displays[j/2].bAmStringData = false;
                }else {
                    // we're not going to do anything with stringval stuff
//                    displays[j/2].bAmStringData = true;
//                    displays[j/2].curValueString =  valStrings[j];
                    
                }
                //cout << data.data[j]  << " , ";
            }
        }
        // now do something with our new shoeDataObject
        updateShoeDataObjectWithData(newShoeData);
    }
    
    
    
    
    
    // update OSC listener:
    while(receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);
        
		// check for mouse moved message
        if(m.getAddress() == "/shoeData"){
            // 8 floats and an action text INT
            ShoeDataObject shoeData;
            shoeData.textIndex = 0;
            //force first:
            shoeData.time = m.getArgAsFloat(1);
            
            shoeData.force = m.getArgAsFloat(0);
            
            shoeData.ax = m.getArgAsFloat(2);
            shoeData.ay = m.getArgAsFloat(3);
            shoeData.az = m.getArgAsFloat(4);
            
            shoeData.gx = m.getArgAsFloat(5);
            shoeData.gy = m.getArgAsFloat(6);
            shoeData.gz = m.getArgAsFloat(7);
            int numArgs = m.getNumArgs();
            for (int i =0; i<kShoeMaxTrackedValues && i< numArgs; i++) {
                shoeData.values[i] = m.getArgAsFloat(i);
            }
            //shoeData.textIndex = m.getArgAsInt32(8);
//            cout << "shoe " << shoeData.force << " ay" << shoeData.ay << "\n";
            // TODO: call this from serial as well when not using OSC
            updateShoeDataObjectWithData(shoeData);
        }
        
        
	}
    

    float currentTime = ofGetElapsedTimef();
   
    //TODO: this should be coming from the shoe!
    
//    if(currentTime-lastPhraseSelectedTime>6){
//        lastPhraseSelectedTime = currentTime;
//        currentPhraseIndex++;
//        currentPhraseIndex %= phrases.size();
//    }
    
    
    currentPhraseIndex = currentShoeDataObject.textIndex;
    
    // MARK: phrase and text timing info
    if(oldPhraseIndex != currentPhraseIndex && currentPhraseIndex!=-1){
        phraseWordIndex = 0;
        oldPhraseIndex = currentPhraseIndex;
        lastPhraseWordIteratedTime = currentTime;
        currentPhraseWords = ofSplitString(phrases[currentPhraseIndex], " ");
        cout << "new phrase=" << phrases[currentPhraseIndex] << "\n";
        
        int lineNumber =0 ;
        ofPoint startPoint;
        startPoint.x = 176;
        float space = 80;
        float currentLineHeight = 0;
        for (int i=0; i<wordLines.size(); i++) {
            wordLines[i].maxHeight =0;
        }
        for (int i=0; i<currentPhraseWords.size(); i++) {
            TextWordBlock wordBlock;
            int sizeIndex = ofRandom(0, font.size());
            cout << " wordsize=" << sizeIndex;
            
            if(forceTextSize)sizeIndex = 1;
            wordBlock.initParams(currentPhraseWords[i], font[sizeIndex]);
            // set the line number of the block:
            // TODO: this is ridiculously lame:
            if(sizeIndex==0){
                wordBlock.fontHeight = 170;
            }else if (sizeIndex==1){
                wordBlock.fontHeight = 210;
            }else if (sizeIndex==2){
                wordBlock.fontHeight = 260;
            }
            wordBlock.currentPosition.x = wordBlock.goalPosition.x = startPoint.x;
            wordBlock.goalPosition.y = startPoint.y;
        
            
            if(startPoint.x+wordBlock.bounds.width+space<kFBOWidth){
                startPoint.x+=wordBlock.bounds.width + space;
                if(wordBlock.bounds.height>currentLineHeight) currentLineHeight = wordBlock.bounds.height;
            }else{
                startPoint.x = 176;
                // this should be calculated not here:
                currentLineHeight = wordBlock.bounds.height;
                startPoint.y += currentLineHeight;
                wordBlock.currentPosition.x = wordBlock.goalPosition.x = startPoint.x;
                wordBlock.goalPosition.y = startPoint.y;
                startPoint.x+=wordBlock.bounds.width + space;
                lineNumber++;
            }
            
            wordBlock.currentPosition.y = 1200;
            
            // MARK: set goal positions of each block of text:
            wordBlock.lineNumber = lineNumber;
            //wordBlock.goalPosition.y = lineNumber*150;
            if(i<wordBlocks.size()){
                wordBlocks[i] = wordBlock;
            }else{
                wordBlocks.push_back(wordBlock);
            }
        }
        currentTotalLines = lineNumber;
        // push the current wordblocks into vectors inside their lines:
        
        for (int i=0; i<currentPhraseWords.size(); i++) {
            IVTextLine line;
            int k = wordBlocks[i].lineNumber;
            if(k<wordLines.size()){
                wordLines[k] = line;
            }else{
                wordLines.push_back(line);
            }

        }
        cout << "\n";
//        wordBlock1.initParams(currentPhraseWords[0], font[0]);
//        wordBlock2.initParams(currentPhraseWords[1], font[font.size()-1]);
        
    };
//     cout << phraseWordIndex << "\n";
    // cycle through worcount:
    float wordTime = .07;
    // add a per character time so that larger words take a bit longer.

    if(currentPhraseWords[phraseWordIndex].length()>0){
        wordTime += currentPhraseWords[phraseWordIndex].length()*.02;
        char lastChar =currentPhraseWords[phraseWordIndex].at(currentPhraseWords[phraseWordIndex].length()-1);
        if(lastChar ==','){
            wordTime*=3;
        }else if (lastChar == '.'){
            wordTime*=4;
        }
    }
    if(currentTime-lastPhraseWordIteratedTime>wordTime){
        if(phraseWordIndex<currentPhraseWords.size()-1)phraseWordIndex++;
        lastPhraseWordIteratedTime = currentTime;
    };
    
    analyzeShoeData();

}
void testApp::updateShoeDataObjectWithData(ShoeDataObject newData){
    currentShoeDataObject = newData;
    // make us a smoothed version:
    float smoothingDiv =10.;
    currentShoeDataObjectSmoothed.ax+=(currentShoeDataObject.ax-currentShoeDataObjectSmoothed.ax)/smoothingDiv;
    currentShoeDataObjectSmoothed.ay+=(currentShoeDataObject.ay-currentShoeDataObjectSmoothed.ay)/smoothingDiv;
    currentShoeDataObjectSmoothed.az+=(currentShoeDataObject.az-currentShoeDataObjectSmoothed.az)/smoothingDiv;
    currentShoeDataObjectSmoothed.gx+=(currentShoeDataObject.gx-currentShoeDataObjectSmoothed.gx)/smoothingDiv;
    currentShoeDataObjectSmoothed.gy+=(currentShoeDataObject.gy-currentShoeDataObjectSmoothed.gy)/smoothingDiv;
    currentShoeDataObjectSmoothed.gz+=(currentShoeDataObject.gz-currentShoeDataObjectSmoothed.gz)/smoothingDiv;
    
    currentShoeDataObjectSmoothed.force +=(currentShoeDataObject.force-currentShoeDataObjectSmoothed.force)/smoothingDiv;

//    dataDisplays.push_back(<#const value_type &__x#>)
    
    // this is better for doing this:
    dataObjects.push_back(currentShoeDataObject);
    if (dataObjects.size() > kObjectBufferSize){
        dataObjects.erase(dataObjects.begin());
    }
//    if(dataObjects.size()<kObjectBufferSize){
//        dataObjects.push_back(currentShoeDataObject);
//    }else{
//        dataObjectInsertionIndex++;
//        if(dataObjectInsertionIndex>dataObjects.size()-1){
//            dataObjectInsertionIndex = 0;
//        }
//        dataObjects[dataObjectInsertionIndex] = currentShoeDataObject;
//    }
}
//--------------------------------------------------------------
void testApp::analyzeShoeData(){
    // MARK: THIS IS IT text distortion from data
    
    // look through the buffer and detect stuff:
    //float forceChange = 0;
//    ShoeDataObject min;
//    ShoeDataObject max;
    // add values for current shoeData
    for (int k =0;k<kShoeMaxTrackedValues; k++) {
        float val = currentShoeDataObject.values[k];
        if(val<minValues.values[k]) minValues.values[k] =val;
        if(val>maxValues.values[k]) maxValues.values[k] = val;
    }
    for (int i =1; i<dataObjects.size(); i++) {
        // look at rates of change over the last xx data objects:
        // let's look at force:
    
        // move through the data values:
        
//        if(dataObjects[i].force<minValues.force) minValues.force = dataObjects[i].force;
//        if(dataObjects[i].force>maxValues.force) maxValues.force = dataObjects[i].force;
//
        shoeDataObjectWorking.force+= dataObjects[i].force - dataObjects[i-1].force;
        shoeDataObjectWorking.ay+= dataObjects[i].ay - dataObjects[i-1].ay;
    }
//    minValues = min;
//    maxValues = max;
    
    if(currentShoeDataObject.freeFall!=0){
//        distortAmt3+=.2;
        distortAmt5+=.1;
    }
    if(currentShoeDataObject.singleTap!=0){
        distortAmt1+=.2;
        distortAmt7 +=.3;
    }
//    distortAmt4=shoeDataObjectWorking.ax;
    
//    if(abs(shoeDataObjectWorking.ay)> 10000.){
//        cout << "force is big " << shoeDataObjectWorking.ay << "\n";
//        // now kill it so we retrigger. 
//        shoeDataObjectWorking.ay =0;
//        distortAmt2+=10;
//    }
    
//    if(currentShoeDataObjectSmoothed.force>100){
//        distortAmt1 += (currentShoeDataObjectSmoothed.force/5000);
//    }
//    if(currentShoeDataObject.ay<0){
//        distortAmt4 += abs(currentShoeDataObject.ay)/1000;
//    }

}
void testApp::draw(){
    ofEnableAlphaBlending();
    // just clear to this color:

    ofClear(bgColor);
    
//font.loadFont("type/OpenSans-ExtraBold.ttf", mouseX+10, true, false, true, 0.2, 100);
    
	//lets draw some graphics into our two fbos
    if(useFbo) rgbaFbo.begin();
    drawFbo();
    if(useFbo) rgbaFbo.end();
    
    drawBackground();
    // draw on the background:
   
    
    if(useFbo){
//        ofTexture tex0 = rgbaFbo.getTextureReference();
//        FBOshader.setUniformTexture("tex0", tex0, 0);
//        FBOshader.begin();
//        FBOshader.setUniform1f("timeValX", ofGetElapsedTimef() * 10 );
//        FBOshader.setUniform1f("timeValY", -ofGetElapsedTimef() * 10.1 );
//        FBOshader.setUniform1f("distortAmount1", distortAmt1 );
//        FBOshader.setUniform1f("distortAmount2", distortAmt2 );
//        FBOshader.setUniform1f("distortAmount3", distortAmt3 );
//        FBOshader.setUniform1f("distortAmount4", distortAmt4 );
//        FBOshader.setUniform1f("distortAmount5", distortAmt5 );
//        FBOshader.setUniform1f("distortAmount6", distortAmt6 );
//        FBOshader.setUniform1f("distortAmount7", distortAmt7 );
//        FBOshader.setUniform1f("distortAmount8", distortAmt8 );
//        FBOshader.setUniform1f("distortAmount9", distortAmt9 );
        
        ofSetColor(255,255,255,255);
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        rgbaFbo.draw(0,0,kFBOWidth*kFBORenderScale,kFBOHeight*kFBORenderScale);
//        FBOshader.end();
;
    }
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
        gplusLabel.draw(0,0);
   // /*
    ofDrawBitmapString("fps: " + ofToString((int)ofGetFrameRate()) +
                       "\nPress 'O' to toggle fbo: " + (useFbo ? "ON" : "OFF")+
                       "\npress 'f' to toggle forceTextSize: " + (forceTextSize ? "ON" : "OFF")+
                       "\npress 'r' for renderDataMode=" + (ofToString(renderDataMode))+
                       "\n distortamt8="+ofToString(distortAmt8)
                       , 20,10);
  
   // */
    
    // save frame?
    bool bSnapshot = false;
    if (bSnapshot == true){
		// grab a rectangle at 200,200, width and height of 300,180
		img.grabScreen(0,0,ofGetWidth(),ofGetHeight());
        
		string fileName = "snapshot_"+ofToString(10000+frameSequenceCounter)+".png";
		img.saveImage(fileName);
//		sprintf(snapString, "saved %s", fileName.c_str());
		frameSequenceCounter++;
		bSnapshot = false;
	}
    
    ofNoFill();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofTranslate(currentShoeDataObject.ax, currentShoeDataObject.ay);
//    ofRotateX(currentShoeDataObjectSmoothed.gx);
//    ofRotateY(currentShoeDataObjectSmoothed.gy);
//    ofRotateZ(currentShoeDataObjectSmoothed.gz);
    ofSetColor(255,255,255,100);
//    ofBox(200);
}
void testApp::drawBackground(){
    // draw the data from each shoedata:
    float width = ofGetWidth();
    float height = ofGetHeight();
    int max = dataObjects.size();
    float h = height/kShoeMaxTrackedValues;
    for (int i =1; i<max; i++) {
        // for each one, draw a line
        
        float p = (float)i/(max-1);
        float p2 =(float)(i-1)/(max-1);
//        float x = p*ofGetWidth();
        //float ybase = 100;
//        float data = dataObjects[i].force;
//        float data2 = dataObjects[i-1].force;

        for (int k = 0; k<kShoeMaxTrackedValues; k++) {
            float val1 = dataObjects[i].values[k];
            float val2 = dataObjects[i-1].values[k];
            float ybase = ((float)k/kShoeMaxTrackedValues)*height;
            if(renderDataMode!=0)drawLineFromData(p*width, p2*width, ybase, val1, val2, maxValues.values[k], minValues.values[k],h);
        }
        
        // this is so crappy!
        
    }
}
void testApp::drawLineFromData(float x1, float x2, float y,float currentVal, float prevVal, float maxVal, float minVal,float height){
    float p = (currentVal-minVal)/((maxVal-minVal)+.000001);
    float p2 =(prevVal-minVal)/((maxVal-minVal)+.000001);
    
    if(renderDataMode==1){
        ofLine(x1, y+p*height, x2,y+p2*height);
    }else if(renderDataMode==2){
        ofSetColor(255, 255, 255,p*100);
        ofRect(x1, y, x2-x1, height);
    }
    
}
void testApp::drawFbo(){
   
    ofFill();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    
    ofSetColor(bgColor, fadeAmt);
//	ofSetColor(0,100,255, fadeAmt);
	ofRect(0,0,kFBOWidth,kFBOHeight);
    
    // now draw the regular scene:
        
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(255,255,255,kTextRenderAlpha);
	ofFill();
    float div = 20;
    float tol = .0001;
//    distortAmt3 = 1;
	distortAmt1+=easeToInc(distortAmt1, 0, div, tol);
    distortAmt2+=easeToInc(distortAmt2, 0, div, tol);
    distortAmt3+=easeToInc(distortAmt3, 0, div, tol);
    distortAmt4+=easeToInc(distortAmt4, 0, div, tol);
    distortAmt5+=easeToInc(distortAmt5, 0, div, tol);
    distortAmt6+=easeToInc(distortAmt6, 0, div, tol);
    distortAmt7+=easeToInc(distortAmt7, 0, div, tol);
    distortAmt8+=easeToInc(distortAmt8, 0, div, tol);
    distortAmt9+=easeToInc(distortAmt9, 0, div, tol);
    
	if( doShader ){
		shader.begin();
        //we want to pass in some varrying values to animate our type / color
        shader.setUniform1f("timeValX", ofGetElapsedTimef() * 10 );
        shader.setUniform1f("timeValY", -ofGetElapsedTimef() * 10.1 );

        shader.setUniform1f("distortAmount1", distortAmt1 );
        shader.setUniform1f("distortAmount2", distortAmt2 );
        shader.setUniform1f("distortAmount3", distortAmt3 );
        shader.setUniform1f("distortAmount4", distortAmt4 );
        shader.setUniform1f("distortAmount5", distortAmt5 );
        shader.setUniform1f("distortAmount6", distortAmt6 );
        shader.setUniform1f("distortAmount7", distortAmt7 );
        shader.setUniform1f("distortAmount8", distortAmt8 );
        shader.setUniform1f("distortAmount9", distortAmt9 );

        //we also pass in the mouse position
        //we have to transform the coords to what the shader is expecting which is 0,0 in the center and y axis flipped.
        //shader.setUniform2f("mouse", mouseX - ofGetWidth()/2, ofGetHeight()/2-mouseY ;)
        shader.setUniform2f("mouse", 0, currentShoeDataObjectSmoothed.ay);
        
	}
	
    //finally draw our text
//    int whichSize = ((float)mouseX/ofGetWidth())*(font.size()-1);
//    if (whichSize>font.size()-1)whichSize = font.size()-1;
    int whichSize = font.size()-2;
    ofTrueTypeFont currentFont = font[1];
    currentFont.setLetterSpacing(.95);
    currentFont.setSpaceSize(.6);

    //    font.setLineHeight(.2);
    //    font.setGlobalDpi(123);
    
//    ofStyle style;
//    style.smoothing = true;
//    style.bFill = false;
//    style.curveResolution = 200;
//    style.circleResolution = 200;
////    style.lineWidth =10;
//    ofSetStyle(style);
    
   // 3lines font.drawStringAsShapes("really?\nis that all\nyou got?", 90, 260);
   // font.drawStringAsShapes("really? is that\nall you got?", 90, 260);
  //   font.drawStringAsShapes("you can\njump higher\nthan that.", 90, 600);
//    font.drawStringAsShapes("that's what\ni'm talking\nabout. come\non faster.\ndo it faster\nfaster faster\nfaster", 90, 260);
    // float offset = -abs(sin(ofGetElapsedTimef()*2))*200;
    float centerPoint =(kFBOHeight)/2;
   

    //font.drawStringAsShapes("go! go!\ngo! go! go!", 0,0);
    string phraseString;
    if(currentPhraseWords.size()>0){
        int lineLength = 0;
            renderedPhraseLineCount =0;
        for (int i =0; i<=phraseWordIndex; i++) {
            // translate by the size of the last shapes drawn???
            //font.drawStringAsShapes(currentPhraseWords[i], 0, i*144);
            
            lineLength +=currentPhraseWords[i].length()+1;
            if (lineLength>15) {
                phraseString.append("\n");
                lineLength = currentPhraseWords[i].length();
                renderedPhraseLineCount++;
            }else{
                if(i>0)phraseString.append(" ");
            }
            phraseString.append(currentPhraseWords[i]);
            
        }
        float lineHeight = currentFont.getStringBoundingBox("X", 0, 0).height;
        //ofTranslate(176, centerPoint-(renderedPhraseLineCount*lineHeight/2));
        
        
        ofRectangle bounds =        currentFont.getStringBoundingBox(phraseString, 0, 0);
        ofPushMatrix();
        ofTranslate(176,(centerPoint+lineHeight)-bounds.height/2);

//        ofNoFill();
//        currentFont.drawStringAsShapes(phraseString, 0, 0);
        
        ofPopMatrix();
        ofFill();
        ofPoint startPoint;
        
        ofRectangle prevBounds;
        float totalMaxHeight =0;
        float currentMaxHeight =0;
        
        startPoint.y = centerPoint-199;
        startPoint.x = 176;
        // MARK: draw the words
        int currentLineNumber = -1;
        float lineOffset =0;
        float firstLineHeight = 0;
        for (int i =0; i<=phraseWordIndex; i++) {
            // translate by the size of the last shapes drawn???
            //font.drawStringAsShapes(currentPhraseWords[i], 0, i*144);
            TextWordBlock* wordBlock = &wordBlocks[i];

        
            if(wordBlock->lineNumber>currentLineNumber){
                // calculate this line's max height:
//                cout << "lineheight=" << wordBlock->font.getLineHeight() << "\n";
                currentLineNumber = wordBlock->lineNumber;
//                float myMaxLineHeight = wordBlock->bounds.height;
                float myMaxLineHeight = wordBlock->fontHeight;
//                float myMaxDescender = (wordBlock->bounds.height+wordBlock->bounds.y);
                for (int k=i+1; k<=phraseWordIndex; k++) {
                    // look forward to find the largest lineheight on THIS line:
                    TextWordBlock* nextBlock = &wordBlocks[k];
                    if(nextBlock->lineNumber == currentLineNumber){
//                        float h =nextBlock->bounds.height;
                        float h = nextBlock->fontHeight;
                        if(h>myMaxLineHeight) myMaxLineHeight = h;
                    }else{
                        break;
                    }
                }
                totalMaxHeight+=myMaxLineHeight;//+nextDescender;//+oldDescender;///2;
                
            }

            if(currentLineNumber==0)firstLineHeight = totalMaxHeight;
            
           // float nextDescender = currentLineNumber-1>=0 ? wordLines[currentLineNumber].descenderHeight : 0;
            wordBlock->goalPosition.y = totalMaxHeight;
//            if(i+1<=phraseWordIndex && wordBlocks[i+1].lineNumber!=currentLineNumber){
//                // we're going to be a new line, add the old maxlineheight to the lineOffset:
//                lineOffset+=myMaxLineHeight;
//            }
            
//            wordBlock->offset.y = myMaxLineHeight;
            wordBlock->update(10);
            if(distortAmt6!=0){
                float amt =distortAmt6*1000;
                if(i%4<2){
                    amt *= i%2==1 ? 1:-1;
                    wordBlock->offset.x=amt;
                }else{
                    amt *= i%2==1 ? 1:-1;
                    wordBlock->offset.y=amt;
                }
            }
            // search forward to get the nextLines max height
            wordBlock->draw(wordBlock->currentPosition.x+wordBlock->offset.x,wordBlock->currentPosition.y+wordBlock->offset.y+textStartY);
            // we're on a new line, add the maxheight to the current thing.
            //            if(wordBlock->bounds.height >currentMaxHeight){
//                wordBlock->bounds.height = currentMaxHeight;
//            }
            
            
            
        }
        if(totalMaxHeight<kFBOHeight){
            textStartY = centerPoint-(60)-(totalMaxHeight/2);
        }else{
            textStartY = kFBOHeight-(totalMaxHeight)-100;
        }

//        ofTranslate(0,wordBlock1.bounds.height);
//        wordBlock1.draw();
//        ofTranslate(wordBlock1.bounds.width,0);
//        wordBlock2.draw();
    }
    // draw the second set: (this is really really wasteful!)
    //    if(doShader){
    //        shader.setUniform1f("distortAmount1",distortAmt/2);
    //            font.drawStringAsShapes("really? is that\nall you got?", 90, 260);
    //        shader.setUniform1f("distortAmount1",distortAmt/4);
    //            font.drawStringAsShapes("really? is that\nall you got?", 90, 260);
    //        shader.setUniform1f("distortAmount1",distortAmt/6);
    //            font.drawStringAsShapes("really? is that\nall you got?", 90, 260);
    //        shader.setUniform1f("distortAmount1",distortAmt/8);
    //            font.drawStringAsShapes("really? is that\nall you got?", 90, 260);
    //        shader.setUniform1f("distortAmount1",distortAmt/10);
    //            font.drawStringAsShapes("really? is that\nall you got?", 90, 260);
    //    }
    //    ofSetColor(255);
    //    font.drawStringAsShapes("really? is that all you got?", 90, 260);
    
    //    font.drawString("This is the second test looks better", 90, 500);
	if( doShader ){
		shader.end();
	}

}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
    
    if (key == ' '){
        // MARK: serial setup!
        serial.listDevices();
        serialInitSuccess = serial.setup("cu.FireFly-AD9F-SPP", 57600);
    }
    if(key== 'r'){
        renderDataMode+=1;
        if(renderDataMode==3)renderDataMode = 0;
    }
//	if( key == 's' ){
//		doShader = !doShader;
//	}
    ofLog(OF_LOG_VERBOSE, "Key [%c] pressed", key);
//    cout << "Output sentence";
    if(key == 'O'){
        useFbo= !useFbo;
    }
//    ofImage::saveImage("test.png");
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 

    if(key == '1'){
        distortAmt1 += 10;
    }else if(key == '2'){
        distortAmt2 +=1;
    }else if(key == '3'){
        distortAmt3 +=1;
    }else if(key == '4'){
        distortAmt4 +=1;
    }else if(key == '5'){
        distortAmt5 +=1;
    }else if(key == '6'){
        distortAmt6 +=1;
    }else if(key == '7'){
        distortAmt7 +=1;
    }else if (key == '8'){
        distortAmt8 +=1;
    }else if (key == '9'){
        distortAmt9 +=1;
    }
    if(key =='f'){
        forceTextSize=!forceTextSize;
    }
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	//distortAmt3+=1;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

