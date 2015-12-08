#pragma once

#include "ofMain.h"
#include "ofxCvHaarFinder.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void drawScene(bool isPreview);
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

		ofVideoGrabber video;
		ofxCvHaarFinder finder;
	
		//the view window is defined by 3 corners
		ofVec3f windowTopLeft;
		ofVec3f windowBottomLeft;
		ofVec3f windowBottomRight;
		ofCamera headTrackedCamera;
		ofEasyCam previewCamera;
	
		bool usePreview;
		float windowWidth;
		float windowHeight;
		float viewerDistance;
	
		deque<ofPoint> headPositionHistory;

		ofVboMesh window;
    
    		// helper functions
    		void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c);
    		void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d);
    		void addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c);
    		void addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d);
    		ofVec3f getVertexFromImg(ofImage& img, int x, int y);
    
    		ofEasyCam cam;
    		ofMesh mesh;
    		ofVboMesh vboMesh;
    		ofImage img;
    
    int 				snapCounter;
    char 				snapString[255];
    ofImage 			image;
    ofTrueTypeFont		cooper;
    bool 				bSnapshot;
    float 				phase;

};
