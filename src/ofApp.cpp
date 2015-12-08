#include "ofApp.h"

/*
 These functions are for adding quads and triangles to an ofMesh -- either
 vertices, or texture coordinates.
 */
//--------------------------------------------------------------
void ofApp::addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c) {
	mesh.addVertex(a);
	mesh.addVertex(b);
	mesh.addVertex(c);
}

//--------------------------------------------------------------
void ofApp::addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d) {
	addFace(mesh, a, b, c);
	addFace(mesh, a, c, d);
}

//--------------------------------------------------------------
void ofApp::addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c) {
	mesh.addTexCoord(a);
	mesh.addTexCoord(b);
	mesh.addTexCoord(c);
}

//--------------------------------------------------------------
void ofApp::addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d) {
	addTexCoords(mesh, a, b, c);
	addTexCoords(mesh, a, c, d);
}

/*
 The 3d data is stored in an image where alpha represents depth. Here we create
 a 3d point from the current x,y image position.
 */
//--------------------------------------------------------------
ofVec3f ofApp::getVertexFromImg(ofImage& img, int x, int y) {
	ofColor color = img.getColor(x, y);
	if(color.a > 0) {
		float z = ofMap(color.a, 0, 255, -480, 480);
		return ofVec3f(x - img.getWidth() / 2, y - img.getHeight() / 2, z);
	} else {
		return ofVec3f(0, 0, 0);
	}
}



//--------------------------------------------------------------
void ofApp::setup(){
    //-------
        #ifdef TARGET_OPENGLES
        // While this will will work on normal OpenGL as well, it is
        // required for OpenGL ES because ARB textures are not supported.
        // If this IS set, then we conditionally normalize our
        // texture coordinates below.
        ofEnableNormalizedTexCoords();
        #endif
    
    	img.loadImage("linzer.png");
    
    	// OF_PRIMITIVE_TRIANGLES means every three vertices create a triangle
    	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    	int skip = 10;	// this controls the resolution of the mesh
    
    	int width = img.getWidth();
    	int height = img.getHeight();
    
    	ofVec2f imageSize(width,height);
    
    	ofVec3f zero(0, 0, 0);
    	for(int y = 0; y < height - skip; y += skip) {
    		for(int x = 0; x < width - skip; x += skip) {
    			/*
    			 To construct a mesh, we have to build a collection of quads made up of
    			 the current pixel, the one to the right, to the bottom right, and
    			 beneath. These are called nw, ne, se and sw. To get the texture coords
    			 we need to use the actual image indices.
    			 */
    			ofVec3f nw = getVertexFromImg(img, x, y);
    			ofVec3f ne = getVertexFromImg(img, x + skip, y);
    			ofVec3f sw = getVertexFromImg(img, x, y + skip);
    			ofVec3f se = getVertexFromImg(img, x + skip, y + skip);
    			ofVec2f nwi(x, y);
    			ofVec2f nei(x + skip, y);
    			ofVec2f swi(x, y + skip);
    			ofVec2f sei(x + skip, y + skip);
    
    			// ignore any zero-data (where there is no depth info)
    			if(nw != zero && ne != zero && sw != zero && se != zero) {
    				addFace(mesh, nw, ne, se, sw);
    
    				// Normalize our texture coordinates if normalized
    				// texture coordinates are currently enabled.
    				if(ofGetUsingNormalizedTexCoords()) {
    					nwi /= imageSize;
    					nei /= imageSize;
    					sei /= imageSize;
    					swi /= imageSize;
    				}
    
    				addTexCoords(mesh, nwi, nei, sei, swi);
    			}
    		}
    	}
    	
    	vboMesh = mesh;

    //----------
    
	ofEnableSmoothing();
	ofSetVerticalSync(true);
	
	video.initGrabber(320, 240);
	finder.setup("haarcascade_frontalface_default.xml");
	usePreview = false;
	
	previewCamera.setDistance(3.0f);
	previewCamera.setNearClip(0.01f);
	previewCamera.setFarClip(500.0f);
	previewCamera.setPosition(0.4f, 0.2f, 0.8f);
	previewCamera.lookAt(ofVec3f(0.0f, 0.0f, 0.0f));
	
	headTrackedCamera.setNearClip(0.01f);
	headTrackedCamera.setFarClip(1000.0f);
	
	//defining the real world coordinates of the window which is being headtracked is important for visual accuracy
	windowWidth = 0.3f;
	windowHeight = 0.2f;
	
	windowTopLeft = ofVec3f(-windowWidth / 2.0f,
							+windowHeight / 2.0f,
							0.0f);
	windowBottomLeft = ofVec3f(-windowWidth / 2.0f,
							   - windowHeight / 2.0f,
							   0.0f);
	windowBottomRight = ofVec3f(+windowWidth / 2.0f,
								-windowHeight / 2.0f,
								0.0f);
	
	//we use this constant since we're using a really hacky headtracking in this example
	//if you use something that can properly locate the head in 3d (like a kinect), you don't need this fudge factor
	viewerDistance = 0.4f;
    
    //--------
    snapCounter = 0;
    bSnapshot = false;
    cooper.loadFont("cooperBlack.ttf", 50);
    phase = 0;
    memset(snapString, 0, 255);		// clear the string by setting all chars to 0
    
    //--------
    
}

//--------------------------------------------------------------
void ofApp::update(){
	video.update();
	finder.findHaarObjects(video.getPixelsRef());
	
	ofVec3f headPosition(0,0,viewerDistance);
	
	if (finder.blobs.size() > 0) {
		//get the head position in camera pixel coordinates
		const ofxCvBlob & blob = finder.blobs.front();
		float cameraHeadX = blob.centroid.x;
		float cameraHeadY = blob.centroid.y;
		
		//do a really hacky interpretation of this, really you should be using something better to find the head (e.g. kinect skeleton tracking)
		
		//since camera isn't mirrored, high x in camera means -ve x in world
		float worldHeadX = ofMap(cameraHeadX, 0, video.getWidth(), windowBottomRight.x, windowBottomLeft.x);
		
		//low y in camera is +ve y in world
		float worldHeadY = ofMap(cameraHeadY, 0, video.getHeight(), windowTopLeft.y, windowBottomLeft.y);
		
		//set position in a pretty arbitrary way
		headPosition = ofVec3f(worldHeadX, worldHeadY, viewerDistance);
	} else {
		if (!video.isInitialized()) {
			//if video isn't working, just make something up
			headPosition = ofVec3f(0.5f * windowWidth * sin(ofGetElapsedTimef()), 0.5f * windowHeight * cos(ofGetElapsedTimef()), viewerDistance);
		}
	}
	
	headPositionHistory.push_back(headPosition);
	while (headPositionHistory.size() > 50.0f){
		headPositionHistory.pop_front();
	}

	//these 2 lines of code must be called every time the head position changes
	headTrackedCamera.setPosition(headPosition);
	headTrackedCamera.setupOffAxisViewPortal(windowTopLeft, windowBottomLeft, windowBottomRight);
}

//--------------------------------------------------------------
void ofApp::drawScene(bool isPreview){
	
	ofEnableDepthTest();
    
	ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if (bSnapshot == true){
        // grab a rectangle at 200,200, width and height of 300,180
        image.grabScreen(0,0,500,300);
        
        string fileName = "snapshot_"+ofToString(10000+snapCounter)+".png";
        image.saveImage(fileName);
        sprintf(snapString, "saved %s", fileName.c_str());
        snapCounter++;
        bSnapshot = false;
    }
    
    ofDrawBitmapString(snapString, 600,460);
    
    
    ofSetHexColor(0xFFFFFF);
    if(snapCounter > 0) {
        img.draw(600,200,300,180);
    }
    
//----------

	//ofBackgroundGradient(ofColor(50), ofColor(0));
	//------
	//draw the scene
	//
	if (usePreview){
		previewCamera.begin();
	}
	else{
		headTrackedCamera.begin();
	}
	
	drawScene(usePreview);
	
	if (usePreview){
		previewCamera.end();
	}
	else{
		headTrackedCamera.end();
	}
	//
	//------
	
	
	//-----
	//
    video.draw(0, 0);
    
        ofBackgroundGradient(ofColor(50), ofColor(0));

    	cam.begin();
    	ofEnableDepthTest();
    
    for (unsigned int i=0; i<headPositionHistory.size(); i++) {
        float degreeX = ofMap(headPositionHistory[i].x, 0, windowWidth, 0, 8);
        float degreeY = ofMap(headPositionHistory[i].y, 0, windowWidth, 2,8);
        ofRotateY(degreeX/10);
        ofRotateX(0.3-degreeY/10);
        printf("%.6f",degreeX);
    };
    	//ofRotateY(ofGetElapsedTimef() * 30); // slowly rotate the model
    
    	ofScale(1, -1, 1); // make y point down
    	ofScale(1.5, 1.5, 1.5); // make everything a bit smaller
    
    	img.bind(); // bind the image to begin texture mapping
    	int n = 1; // make a 5x5 grid
    	ofVec2f spacing(img.getWidth(), img.getHeight()); // spacing between meshes
    	ofTranslate(-spacing.x * n / 2, -spacing.y * n / 2, 0); // center the grid
    	for(int i = 0; i < n; i++) { // loop through the rows
    		for(int j = 0; j < n; j++) { // loop through the columns
    			ofPushMatrix();
    			ofTranslate(i * spacing.x, j * spacing.y+40); // position the current mesh
    			ofTranslate(spacing.x / 2, spacing.y / 2); // center the mesh
    			if(ofGetKeyPressed()) {
    				vboMesh.draw(); // draw a vboMesh (faster) when a key is pressed
    			} else {
    				mesh.draw(); // draw an ofMesh (slower) when no key is pressed
    			}
    			ofPopMatrix();
    		}
        }
    
        
    	img.unbind();
    
    	ofDisableDepthTest();
    	cam.end();
    

}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    if (key == 'x'){
        bSnapshot = true;
    }
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
