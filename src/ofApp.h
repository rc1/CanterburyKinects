#pragma once

#include "ofMain.h"
// Addons
#include "ofxLibwebsockets.h"
#include "ofxKinect.h"
#include "ofxKinectBlobFinder.h"
#include "ofxKinectBlobTracker.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxOsc.h"
// Libs
#include "Emitters.h"

class ofApp : public ofBaseApp {
 public:

    // #ÊLifecycle
    void setup ();
    void update ();
    void draw ();
    void exit ();
    void keyPressed( int key );
    
    // #ÊRendering
    ofEasyCam camera;
    
    // # Communications
    // ##ÊWebsocket
    Emitters::WebsocketServer websocketEmitter;
    ofxLibwebsockets::ServerOptions websocketServerOptions;
    ofxLibwebsockets::Server websocketServer;
    
    // ##ÊOSC
    ofxOscSender oscSender;
    
    // #ÊKinect
    ofxKinect kinect;
    // ##ÊKinect Finder
    ofxKinectBlobTracker tracker;
    Emitters::BlobTracker trackerEmitter;
    ofxKinectBlobFinder blobFinder;
    
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    ofxCvGrayscaleImage bgImage;
    ofxCvGrayscaleImage grayDiff;
    // for KinectBlobTracker
    ofImage grayDiffOfImage;
    float minPoints;
    
    // Ui
    ofxPanel viewGui;
    ofxToggle drawRGB;
    ofxToggle drawBlobPoints;
    ofxIntSlider gridRowsSlider;
    ofxIntSlider gridColumnsSlider;

    ofxPanel detectionGui;
    ofxFloatSlider minVol;
    ofxFloatSlider maxVol;
    ofxVec3Slider cropBoxMin;
    ofxVec3Slider cropBoxMax;
    ofxVec3Slider thresh3D;
    ofxFloatSlider thresh2D;
    ofxIntSlider maxBlobs;
    ofxToggle learnBackgroundToggle;
    
    ofxPanel appGui;
    ofxColorSlider backgroundColorSlider;

    ofxPanel oscGui;
    ofxLabel hostLabel;
    ofxButton updateHostButton;
    Emitters::Button hostButtonEmitter;
    ofxLabel portLabel;
    ofxButton updatePortButton;
    Emitters::Button portButtonEmitter;
    
    ofxPanel kinectGui;
    ofxToggle kinectId0Toggle;
    Emitters::Toggle kinectId0ToggleEmitter;
    ofxToggle kinectId1Toggle;
    Emitters::Toggle kinectId1ToggleEmitter;
    
};
