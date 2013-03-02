#include "testApp.h"

#define kFBORenderScale .5
#define kFBOWidth 1920
#define kFBOHeight 1080


#define kTextRenderAlpha 110
//--------------------------------------------------------------
void testApp::setup(){
    // listen on the given port
	cout << "listening for osc messages on port " << kOscListenPort << "\n";
	receiver.setup(kOscListenPort);
    current_msg_string = "0";
    
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
        ofTrueTypeFont tempFont;
        tempFont.loadFont("type/OpenSans-ExtraBold.ttf", size, true, false, true, 0.2, 100);
        font.push_back(tempFont);
    }
	
	shader.load("shaders/noise.vert", "shaders/noise.frag");
    
    gplusLabel.allocate(152, 152, OF_IMAGE_COLOR_ALPHA);

    gplusLabel.loadImage("images/gplus_corner.png");
    gplusLabel.resize(152*kFBORenderScale, 152*kFBORenderScale);
	
	doShader = true;
    distortAmt = 0;
    fadeAmt = 100;
    useFbo = true;
    
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
    
    dataObjects.reserve(kObjectBufferSize);
}

//--------------------------------------------------------------
void testApp::update(){
    // update OSC listener:
    while(receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);
        
		// check for mouse moved message
        if(m.getAddress() == "/shoeData"){
            // 8 floats and an action text INT
            ShoeDataObject shoeData;
            //force first:
            shoeData.time = m.getArgAsFloat(1);
            
            shoeData.force = m.getArgAsFloat(0);
            
            shoeData.ax = m.getArgAsFloat(2);
            shoeData.ay = m.getArgAsFloat(3);
            shoeData.az = m.getArgAsFloat(4);
            
            shoeData.gx = m.getArgAsFloat(5);
            shoeData.gy = m.getArgAsFloat(6);
            shoeData.gz = m.getArgAsFloat(7);
            
            //shoeData.textIndex = m.getArgAsInt32(8);
//            cout << "shoe " << shoeData.force << " ay" << shoeData.ay << "\n";
            // TODO: call this from serial as well when not using OSC
            updateShoeDataObjectWithData(shoeData);
        }
        
        
	}
    

    float currentTime = ofGetElapsedTimef();
   
    //TODO: this should be coming from the shoe!
    if(currentTime-lastPhraseSelectedTime>6){
        lastPhraseSelectedTime = currentTime;
        currentPhraseIndex++;
        currentPhraseIndex %= phrases.size();
    }
    
    // MARK: phrase and text timing info
    if(oldPhraseIndex != currentPhraseIndex){
        phraseWordIndex = 0;
        oldPhraseIndex = currentPhraseIndex;
        lastPhraseWordIteratedTime = currentTime;
        currentPhraseWords = ofSplitString(phrases[currentPhraseIndex], " ");
        
        for (int i=0; i<currentPhraseWords.size(); i++) {
            TextWordBlock wordBlock;
            int sizeIndex = ofRandom(0, font.size()-1);
            wordBlock.initParams(currentPhraseWords[i], font[sizeIndex]);
            if(i<wordBlocks.size()){
                wordBlocks[i] = wordBlock;
            }else{
                wordBlocks.push_back(wordBlock);
            }
        }
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

    if(dataObjects.size()<kObjectBufferSize){
        dataObjects.push_back(currentShoeDataObject);
    }else{
        dataObjectInsertionIndex++;
        if(dataObjectInsertionIndex>dataObjects.size()-1){
            dataObjectInsertionIndex = 0;
        }
        dataObjects[dataObjectInsertionIndex] = currentShoeDataObject;
    }
}
//--------------------------------------------------------------
void testApp::analyzeShoeData(){
    // MARK: THIS IS IT text distortion from data
    
    // look through the buffer and detect stuff:
    //float forceChange = 0;
    for (int i =1; i<dataObjects.size(); i++) {
        // look at rates of change over the last xx data objects:
        // let's look at force:
        shoeDataObjectWorking.force+= dataObjects[i].force - dataObjects[i-1].force;
        shoeDataObjectWorking.ay+= dataObjects[i].ay - dataObjects[i-1].ay;
    }
    
//    if(abs(shoeDataObjectWorking.ay)> 10000.){
//        cout << "force is big " << shoeDataObjectWorking.ay << "\n";
//        // now kill it so we retrigger. 
//        shoeDataObjectWorking.ay =0;
//        distortAmt2+=10;
//    }
    
    if(currentShoeDataObjectSmoothed.force>100){
        distortAmt += (currentShoeDataObjectSmoothed.force/5000);
    }
    if(currentShoeDataObject.ay<0){
        distortAmt4 += abs(currentShoeDataObject.ay)/1000;
    }

}
void testApp::draw(){
    ofEnableAlphaBlending();
    // just clear to this color:
    // TODO: this background color is set by which quad or place we're in!
    ofClear(0, 100, 255);
    
//font.loadFont("type/OpenSans-ExtraBold.ttf", mouseX+10, true, false, true, 0.2, 100);
    
	//lets draw some graphics into our two fbos
    if(useFbo) rgbaFbo.begin();
    drawFbo();
    if(useFbo) rgbaFbo.end();
    
   
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    if(useFbo){
        ofSetColor(255,255,255,255);
        rgbaFbo.draw(0,0,kFBOWidth*kFBORenderScale,kFBOHeight*kFBORenderScale);
 
;
    }
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
        gplusLabel.draw(0,0);
    ofDrawBitmapString("distortamt="+ofToString(distortAmt), 20, 40);
    ofDrawBitmapString("fps: " + ofToString((int)ofGetFrameRate()) + "\nPress 'O' to toggle fbo: " + (useFbo ? "ON" : "OFF"), 20, 10);
  
    
    
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
    ofTranslate(currentShoeDataObjectSmoothed.ax, currentShoeDataObjectSmoothed.ay, currentShoeDataObjectSmoothed.az);
    ofRotateX(currentShoeDataObjectSmoothed.gx);
    ofRotateY(currentShoeDataObjectSmoothed.gy);
    ofRotateZ(currentShoeDataObjectSmoothed.gz);
    ofSetColor(255,255,255,50);
    ofBox(200);
}
void testApp::drawFbo(){
   
    ofFill();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    
    
	ofSetColor(0,0,0, fadeAmt);
	ofRect(0,0,kFBOWidth,kFBOHeight);
    
    // now draw the regular scene:
        
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(255,255,255,kTextRenderAlpha);
	ofFill();
    float div = 20;
//    distortAmt3 = 1;
	distortAmt+=(0-distortAmt)/div;
    distortAmt2+=(0-distortAmt2)/div;
    
    distortAmt3+=(0-distortAmt3)/div;
    
    distortAmt4+=(0-distortAmt4)/div;
    distortAmt5+=(0-distortAmt5)/div;
    
	if( doShader ){
		shader.begin();
        //we want to pass in some varrying values to animate our type / color
        shader.setUniform1f("timeValX", ofGetElapsedTimef() * 10 );
        shader.setUniform1f("timeValY", -ofGetElapsedTimef() * 10.1 );
        shader.setUniform1f("distortAmount1", distortAmt );
        shader.setUniform1f("distortAmount2", distortAmt2 );
        shader.setUniform1f("distortAmount3", distortAmt3 );
        shader.setUniform1f("distortAmount4", distortAmt4 );
        shader.setUniform1f("distortAmount5", distortAmt5 );
        
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
//        ofSetColor(255, 0, 0,100);
//        ofRect(bounds);
//        cout << bounds.height <<"\n";
//                ofTranslate(0, );
//        ofNoFill();
        currentFont.drawStringAsShapes(phraseString, 0, 0);
        
        ofPopMatrix();
        ofFill();
        ofPoint startPoint;
        
        ofRectangle prevBounds;
        float totalMaxHeight =0;
        float currentMaxHeight =0;
        float space = 80;
        startPoint.y = centerPoint-199;
        startPoint.x = 176;
        for (int i =0; i<=phraseWordIndex; i++) {
            // translate by the size of the last shapes drawn???
            //font.drawStringAsShapes(currentPhraseWords[i], 0, i*144);
            
//            wordBlocks[i].draw(startPoint.x,startPoint.y);
            if(wordBlocks[i].bounds.height>currentMaxHeight) currentMaxHeight = wordBlocks[i].bounds.height;
            if(startPoint.x+wordBlocks[i].bounds.width+space<kFBOWidth){
                startPoint.x+=wordBlocks[i].bounds.width + space;
            }else{
                startPoint.x = 176;
                startPoint.y += currentMaxHeight;
                currentMaxHeight = wordBlocks[i].bounds.height;
            }
            prevBounds = wordBlocks[i].bounds;
            
        }
        textStartY = totalMaxHeight;
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
	
    if(key == '2'){
        distortAmt2 +=1;
    }else if(key == '3'){
        distortAmt3 +=1;
    }else if(key == '4'){
        distortAmt4 +=1;
    }else if(key == '5'){
        distortAmt5 +=1;
    }else if(key == '7'){
        distortAmt4 +=1;
    }else{
        distortAmt += 10;
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

