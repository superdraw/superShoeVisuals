#include "testApp.h"

#define kFBORenderScale 1
#define kFBOWidth 1920
#define kFBOHeight 1080

//--------------------------------------------------------------
void testApp::setup(){
    // listen on the given port
	cout << "listening for osc messages on port " << kOscListenPort << "\n";
	receiver.setup(kOscListenPort);
    current_msg_string = "0";
    
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofBackground(0);
	ofSetVerticalSync(false );
    
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
	font.loadFont("type/OpenSans-ExtraBold.ttf", 100, true, false, true, 0.2, 100);
	shader.load("shaders/noise.vert", "shaders/noise.frag");
    
    gplusLabel.allocate(256, 256, OF_IMAGE_COLOR_ALPHA);

    gplusLabel.loadImage("images/gplus_corner.png");
    gplusLabel.resize(256*kFBORenderScale, 256*kFBORenderScale);
	
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
            currentShoeDataObject = shoeData;
            
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
        
		if(m.getAddress() == "/mouse/position"){
			// both the arguments are int32's
            ofLog(OF_LOG_NOTICE,"mouse x=%i y=%i",m.getArgAsInt32(0),m.getArgAsInt32(1));
        }
		// check for mouse button message
		else if(m.getAddress() == "/mouse/button"){
			// the single argument is a string
			//mouseButtonState = m.getArgAsString(0);
            distortAmt+=1;  
            cout << "got a mouse button thingy";
		}
		else{
			// unrecognized message: display on the bottom of the screen
			string msg_string;
			msg_string = m.getAddress();
			msg_string += ": ";
			for(int i = 0; i < m.getNumArgs(); i++){
				// get the argument type
				msg_string += m.getArgTypeName(i);
				msg_string += ":";
				// display the argument - make sure we get the right type
				if(m.getArgType(i) == OFXOSC_TYPE_INT32){
					msg_string += ofToString(m.getArgAsInt32(i));
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
					msg_string += ofToString(m.getArgAsFloat(i));
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
					msg_string += m.getArgAsString(i);
				}
				else{
					msg_string += "unknown";
				}
			}
			// add to the list of strings to display
//			msg_strings[current_msg_string] = msg_string;
//			timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
//			current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
//			// clear the next line
//			msg_strings[current_msg_string] = "";
            current_msg_string = msg_string;
            
		}
        
	}
    

    float currentTime = ofGetElapsedTimef();
   
    //TODO: this should be coming from the shoe!
    if(currentTime-lastPhraseSelectedTime>4){
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
    };
//     cout << phraseWordIndex << "\n";
    // cycle through worcount:
    float wordTime = .15;
    if(currentPhraseWords[phraseWordIndex].length()>0 && currentPhraseWords[phraseWordIndex].at(currentPhraseWords[phraseWordIndex].length()-1)==','){
        wordTime*=3;
    }
    if(currentTime-lastPhraseWordIteratedTime>wordTime){
        if(phraseWordIndex<currentPhraseWords.size()-1)phraseWordIndex++;
        lastPhraseWordIteratedTime = currentTime;
    };
    
    analyzeShoeData();

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
    
    if(currentShoeDataObject.force>100){
        distortAmt += (currentShoeDataObject.force/5000);
    }
    if(currentShoeDataObject.ay<0){
        distortAmt3 += abs(currentShoeDataObject.ay)/1000;
    }

}
void testApp::draw(){
    ofEnableAlphaBlending();
    // just clear to this color:
    // TODO: this background color is set by which quad or place we're in!
    ofClear(0, 0, 255);
    

    
   
    

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
    ofTranslate(currentShoeDataObject.ax, currentShoeDataObject.ay, currentShoeDataObject.az);
    ofRotateX(currentShoeDataObject.gx);
    ofRotateY(currentShoeDataObject.gy);
    ofRotateZ(currentShoeDataObject.gz);
    ofSetColor(255,255,255,50);
    ofBox(100);
}
void testApp::drawFbo(){
   
    ofFill();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    
    
	ofSetColor(0,0,0, fadeAmt);
	ofRect(0,0,kFBOWidth,kFBOHeight);
    

    // now draw the regular scene:
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(255,255,255,255);
	ofFill();
    float div = 20;
	distortAmt+=(0-distortAmt)/div;
    distortAmt2+=(0-distortAmt2)/div;
    
    distortAmt3+=(0-distortAmt3)/div;
    
    distortAmt4+=(0-distortAmt4)/div;
    
	if( doShader ){
		shader.begin();
        //we want to pass in some varrying values to animate our type / color
        shader.setUniform1f("timeValX", ofGetElapsedTimef() * 10 );
        shader.setUniform1f("timeValY", -ofGetElapsedTimef() * 10.1 );
        shader.setUniform1f("distortAmount1", distortAmt );
        shader.setUniform1f("distortAmount2", distortAmt2 );
        shader.setUniform1f("distortAmount3", distortAmt3 );
        shader.setUniform1f("distortAmount4", distortAmt4 );
        
        //we also pass in the mouse position
        //we have to transform the coords to what the shader is expecting which is 0,0 in the center and y axis flipped.
        shader.setUniform2f("mouse", mouseX - ofGetWidth()/2, ofGetHeight()/2-mouseY );
        
	}
	
    //finally draw our text
    font.setLetterSpacing(.9);
    
    // TODO: get this working for stringasshapes...
    font.setSpaceSize(.6);
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
   
    ofTranslate(700, centerPoint-(renderedPhraseLineCount*144/2));

    //font.drawStringAsShapes("go! go!\ngo! go! go!", 0,0);
    string phraseString;
    if(currentPhraseWords.size()>0){
        int lineLength = 0;
            renderedPhraseLineCount =0;
        for (int i =0; i<=phraseWordIndex; i++) {
            // translate by the size of the last shapes drawn???
            //font.drawStringAsShapes(currentPhraseWords[i], 0, i*144);
            
            lineLength +=currentPhraseWords[i].length()+1;
            if (lineLength>10) {
                phraseString.append("\n");
                lineLength = currentPhraseWords[i].length();
                renderedPhraseLineCount++;
            }else{
                if(i>0)phraseString.append(" ");
            }
            phraseString.append(currentPhraseWords[i]);
        
//            if(currentPhraseWords[i].length()>10)

            
        }
        font.drawStringAsShapes(phraseString, 0, 0);
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
    }else if(key == '4'){
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

