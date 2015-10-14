#include "ofApp.h"

#pragma mark - Utils

#define KINECT_MAX_RANGE_MM 10000.f // 10 m
#define KINECT_MIN_RANGE_MM 400.f // 0.4 m
#define KINECT_HORIZONTAL_FOV 58.f
#define KINECT_VERTICAL_FOV 45.f
#define KINECT_CUBE_X 11086.181f
#define KINECT_CUBE_X_HALF 5543.0905f
#define KINECT_CUBE_Z KINECT_MAX_RANGE_MM
#define KINECT_CUBE_Y 8904.5737061707f
#define KINECT_CUBE_Y_HALF 4452.286853085f

ofColor grey( 76, 76, 76 );
ofColor darkGrey( 23, 23, 23 );
ofColor yellow( 231, 245, 0 );

struct ColorStack {
    int idx;
    ColorStack():idx(-1){}
    ofColor at( int position ) {
        static vector<ofColor> colorList = { ofColor( 0, 255, 248 ), ofColor( 255, 0, 89 ), ofColor( 255, 128, 0 ), ofColor( 0, 204, 185 ), ofColor( 0, 213, 69 ), ofColor( 255, 241, 0 ) };
        return colorList[ position % colorList.size() ];
    }
};

void drawPointCloudGL ( ofxKinect &kinect ) {
    ofPushStyle();
    glEnable( GL_POINT_SMOOTH );
    glPointSize( 2 );
    glBegin( GL_POINTS );
    ofVec3f position;
    ofColor color;
    for(int y = 0; y < ofxKinect::height; y += 1) {
        for(int x = 0; x < ofxKinect::width; x += 1) {
            position = kinect.getWorldCoordinateAt( x, y );
            color = kinect.getColorAt( x, y );
            ofSetColor( color );
            glVertex3f( position.x, position.y, position.z );
        }
    }
    glEnd();
    ofPopStyle();
}

struct KinectBounds {
    ofPoint kinectPoint;
    ofPoint topLeftPoint;
    ofPoint topRightPoint;
    ofPoint bottomRightPoint;
    ofPoint bottomLeftPoint;
    ofPolyline frame;
    
    KinectBounds () {
        {
            ofMatrix4x4 mat;
            mat.rotate( KINECT_VERTICAL_FOV/2.0f, 1, 0, 0 );
            mat.rotate( -KINECT_HORIZONTAL_FOV/2.0f, 0, 1, 0 );
            topLeftPoint = ofPoint( 0, 0, KINECT_MAX_RANGE_MM ) * mat;
        }
        {
            ofMatrix4x4 mat;
            mat.rotate( KINECT_VERTICAL_FOV/2.0f, 1, 0, 0 );
            mat.rotate( KINECT_HORIZONTAL_FOV/2.0f, 0, 1, 0 );
            topRightPoint = ofPoint( 0, 0, KINECT_MAX_RANGE_MM ) * mat;
        }
        {
            ofMatrix4x4 mat;
            mat.rotate( -KINECT_VERTICAL_FOV/2.0f, 1, 0, 0 );
            mat.rotate( KINECT_HORIZONTAL_FOV/2.0f, 0, 1, 0 );
            bottomRightPoint = ofPoint( 0, 0, KINECT_MAX_RANGE_MM ) * mat;
        }
        {
            ofMatrix4x4 mat;
            mat.rotate( -KINECT_VERTICAL_FOV/2.0f, 1, 0, 0 );
            mat.rotate( -KINECT_HORIZONTAL_FOV/2.0f, 0, 1, 0 );
            bottomLeftPoint = ofPoint( 0, 0, KINECT_MAX_RANGE_MM ) * mat;
        }
        
        vector<ofPoint> points = { topLeftPoint, kinectPoint,
            topRightPoint, topLeftPoint,
            bottomLeftPoint, kinectPoint,
            bottomRightPoint, bottomLeftPoint,
            bottomRightPoint, topRightPoint };
        
        frame.addVertices( points );
    }
};


struct KinectViewGrid {
    KinectViewGrid(){
        rows = 10;
        columns = 10;
 
    }
    KinectBounds kinectBounds;
   
    int rows;
    int columns;
    
    ofVec2f getCellPosition( ofVec3f position ) {
        float x = ofMap( position.x, kinectBounds.bottomRightPoint.x, kinectBounds.bottomLeftPoint.x, 0, 1 );
        float y = ofMap( position.y, 0, kinectBounds.bottomRightPoint.z, 0, 1 );
        ofVec2f cellPosition( 1 + floor( x / (1/float(columns) ) ), 1 + floor( y / (1/float(rows)) ) );
        return cellPosition;
    }
    void draw () {
        for ( int i = 0; i <= rows; ++i ) {
            float z = ofMap( i, 0, rows, 0, kinectBounds.bottomLeftPoint.z );
            ofDrawLine( kinectBounds.bottomLeftPoint.x, 0, z,
                        kinectBounds.bottomRightPoint.x, 0, z );
        }
        
        for ( int j = 0; j <= columns; ++j ) {
            float x = ofMap( j, 0, columns, kinectBounds.bottomLeftPoint.x, kinectBounds.bottomRightPoint.x );
            ofDrawLine( x, 0, 0,
                        x, 0, kinectBounds.bottomLeftPoint.z );
        }
    }
};

KinectViewGrid kinectViewGrid;

void drawKinectFrame () {
    static KinectBounds kinectBounds;
    ofPushStyle();
    ofSetColor( yellow );
    kinectBounds.frame.draw();
    ofPopStyle();
}

void doBillboardToBlob ( ofEasyCam &camera, ofxKinectTrackedBlob &blob ) {
    ofNode billboardedTextPosition;
    billboardedTextPosition.setGlobalPosition( blob.massCenter * 1000 );
    billboardedTextPosition.lookAt( camera.getGlobalPosition(), camera.getUpDir() );
    ofQuaternion billboardedTextQuaternion = billboardedTextPosition.getGlobalOrientation();
    
    float angle = 0;
    ofPoint vec;
    billboardedTextQuaternion.getRotate( angle, vec );
    
    ofTranslate( blob.massCenter * 1000 );
    ofRotate( angle, vec.x, vec.y, vec.z );
}

void send( ofxLibwebsockets::Server &ws, ofxKinect &kinect, string action, int blobId, ofVec2f cellPosition, ofVec3f position, int columns, int rows  ) {
    string message = action + ", " + ofToString( kinect.getDeviceId() ) + ", " + ofToString( blobId ) + ", " + ofToString( cellPosition ) + ", " + ofToString( position ) + ", " + ofToString( rows ) + ", " + ofToString( columns );
    ws.send( message );
}


//void send( ofxOscSender &sender, ofxKinect &kinect, string action, int blobId, ofVec2f cellPosition, ofVec3f position, int columns, int rows ) {
//    ofxOscMessage m;
//    m.setAddress( "/kinect/" + action );
//    m.addIntArg( kinect.getDeviceId() );
//    m.addIntArg( blobId );
//    m.addIntArg( cellPosition.x );
//    m.addIntArg( cellPosition.y );
//    m.addFloatArg( position.x );
//    m.addFloatArg( position.y );
//    m.addFloatArg( position.z );
//    m.addIntArg( columns );
//    m.addIntArg( rows );
//    try {
//        sender.sendMessage(m, false);
//    } catch ( int e ) {
//        ofLogError() << "Failed to send OSC message";
//    }
//}

//void setupOsc( ofxOscSender &sender, string host, int port ) {
//    try {
//        sender.setup( host, port );
//    } catch ( int e ) {
//        ofSystemAlertDialog( "Failed to open OSC when opening " + host + ":" + ofToString( port, 0 ) );
//    }
//}

#pragma mark - Life Cycle

void ofApp::setup () {
    
    // # Logging
    ofSetLogLevel( OF_LOG_NOTICE );
    
    // Ui
    appGui.setup( "App", "settings.xml", 20, 20 );
    appGui.add( backgroundColorSlider.setup( "Background Color", grey, ofColor::black, ofColor::white ) );
    
    viewGui.setup( "View", "settings.xml", 20, 160 );
    viewGui.add( drawRGB.setup( "Draw RGB", false ) );
    viewGui.add( drawBlobPoints.setup( "Draw blob points", true ) );
    viewGui.add( gridRowsSlider.setup( "Number of rows", 5, 1, 20 ) );
    viewGui.add( gridColumnsSlider.setup( "Number of columns", 5, 1, 20 ) );

    oscGui.setup( "OSC", "settings.xml", 20, 280 );
    oscGui.add( hostLabel.setup( "Host", "192.168.0.255" ) );
    oscGui.add( updateHostButton.setup( "Change host" ) );
    oscGui.add( portLabel.setup( "Port", "110001" ) );
    oscGui.add( updatePortButton.setup( "Change port" ) );
    
    kinectGui.setup( "Kinect", "settings.xml", 20, 400 );
    kinectGui.add( kinectId0Toggle.setup( "Kinect 0", true ) );
    kinectGui.add( kinectId1Toggle.setup( "Kinect 1", false ) );
    
    detectionGui.setup( "Detection", "settings.xml", 240, 20 );
    detectionGui.add( minVol.setup( "Min vol", 0.1f, 0, 5 ) );
    detectionGui.add( maxVol.setup( "Man vol", 10.0f, 0, 5 ) );
    detectionGui.add( cropBoxMin.setup( "Crop box min", ofVec3f(-10, -10, -10), ofVec3f(-10, -10, -10), ofVec3f(10, 10, 10) ) );
    detectionGui.add( cropBoxMax.setup( "Crop box max", ofVec3f(10, 10, 10), ofVec3f(-10, -10, -10), ofVec3f(10, 10, 10) ) );
    detectionGui.add( thresh3D.setup( "Threshold 3d", ofVec3f(0.2,0.2,0.3), ofVec3f(0,0,0), ofVec3f(1,1,1) ) );
    thresh2D.setup( "Threshold 2d", 1, 1.0f, 1.0f );
    //detectionGui.add( thresh2D.setup( "Threshold 2d", 0.98, 1f, 1.0f ) );
    detectionGui.add( maxBlobs.setup( "Max blobs", 10, 0, 30 ) );
    detectionGui.add( learnBackgroundToggle.setup( "Learn Background", false ) );
    
    // # Rendering
    camera.setAutoDistance( false );
    camera.setFarClip( 100000 );
    camera.roll( -3.7052 );
    camera.setTarget( ofVec3f( 0.754584, -0.482611, 0.444624 ) );
    camera.setPosition( -4410.55, 2115.81, 30.8963  );
    
    // # Websockets
    // ## Events
    websocketEmitter.bind( websocketServer );
    websocketEmitter.open += []( ofxLibwebsockets::Event& args ){ ofLogNotice() << "Websocket connection opened"; };
    websocketEmitter.broadcast += []( ofxLibwebsockets::Event& args ){ ofLogNotice() << "Websocket broadcast"; };
    websocketEmitter.connect += []( ofxLibwebsockets::Event& args ){ ofLogNotice() << "Websocket connect"; };
    websocketEmitter.message += []( ofxLibwebsockets::Event& args ){ ofLogNotice() << "Websocket message"; };
    websocketEmitter.idle += []( ofxLibwebsockets::Event& args ){ ofLogNotice() << "Websocket idle"; };
    websocketEmitter.close += []( ofxLibwebsockets::Event& args ){ ofLogNotice() << "Websocket close"; };
    // ## Config
    websocketServerOptions = ofxLibwebsockets::defaultServerOptions();
    websocketServerOptions.port = 11001;
    // ## Staring
    websocketServer.setup( websocketServerOptions );
    
    // # Kinect
    
    kinect.init();
    kinect.setRegistration( true );
    kinect.open( kinectId0Toggle ? 0 : 1 );
    
    kinectId0ToggleEmitter.bind( kinectId0Toggle );
    kinectId0ToggleEmitter.released += [this](bool value){
        kinectId1Toggle = !value;
        kinect.close();
        kinect.clear();
        kinect.init();
        kinect.setRegistration( true );
        kinect.open( value ? 0 : 1 );
    };
    
    kinectId1ToggleEmitter.bind( kinectId1Toggle );
    kinectId1ToggleEmitter.released += [this](bool value){
        kinectId0Toggle = !value;
        kinect.open( value ? 1 : 0 );
    };
    
    blobFinder.init( &kinect, true );
    blobFinder.setResolution( BF_LOW_RES );
    
    float  angle = kinect.getCurrentCameraTiltAngle();
    blobFinder.setRotation( ofVec3f( angle, 0, 0) );
    blobFinder.setTranslation( ofVec3f( 0, 0, 0 ) );
    blobFinder.setScale( ofVec3f( 0.001, 0.001, 0.001 ) ); // mm to meters
    
    float sqrResolution = blobFinder.getResolution();
    sqrResolution *= sqrResolution;
    int numPixels = kinect.width * kinect.height;
    minPoints = (int)( 0.001 * (float)numPixels/sqrResolution);
    
    // Set up the cv images
    colorImg.allocate(kinect.width, kinect.height);
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);
    bgImage.allocate(kinect.width, kinect.height);
    grayDiff.allocate(kinect.width, kinect.height);
    grayDiffOfImage.allocate(kinect.width, kinect.height, OF_IMAGE_GRAYSCALE);
    tracker.setListener( &trackerEmitter );
    
    // Events
    
    trackerEmitter.on += [this]( Emitters::BlobTracker::Args args ){
        send( websocketServer, kinect, "on", args.id, kinectViewGrid.getCellPosition( args.centroid ), args.centroid, gridColumnsSlider, gridRowsSlider );
        //send( oscSender, kinect, "on", args.id, kinectViewGrid.getCellPosition( args.centroid ), args.centroid, gridColumnsSlider, gridRowsSlider );
    };
   
    trackerEmitter.off += [this]( Emitters::BlobTracker::Args args ){
        send( websocketServer, kinect, "off", args.id, kinectViewGrid.getCellPosition( args.centroid ), args.centroid, gridColumnsSlider, gridRowsSlider );
        //send( oscSender, kinect, "off", args.id, kinectViewGrid.getCellPosition( args.centroid ), args.centroid, gridColumnsSlider, gridRowsSlider );
    };

    trackerEmitter.moved += [this]( Emitters::BlobTracker::Args args ){
        send( websocketServer, kinect, "moved", args.id, kinectViewGrid.getCellPosition( args.centroid ), args.centroid, gridColumnsSlider, gridRowsSlider );
        //send( oscSender, kinect, "moved", args.id, kinectViewGrid.getCellPosition( args.centroid ), args.centroid, gridColumnsSlider, gridRowsSlider );
    };
    
    // OSC
    hostButtonEmitter.bind( updateHostButton );
    hostButtonEmitter.released += [this]() {
        string host = ofSystemTextBoxDialog( "Enter host" );
        if ( host.length() > 0 ) {
            hostLabel = host;
        }
        //setupOsc( oscSender, hostLabel, ofToInt( portLabel ) );
    };
    
    portButtonEmitter.bind( updatePortButton );
    portButtonEmitter.released += [this]() {
        string host = ofSystemTextBoxDialog( "Enter port" );
        if ( host.length() > 0 ) {
            portLabel = host;
        }
        //setupOsc( oscSender, hostLabel, ofToInt( portLabel ) );
    };
    
    //setupOsc( oscSender, hostLabel, ofToInt( portLabel ) );

}

void ofApp::update () {
    //ofBackground( grey );
    kinectViewGrid.rows = gridRowsSlider;
    kinectViewGrid.columns = gridColumnsSlider;
    
    kinect.update();
    if ( kinect.isFrameNew() ) {
        // Mangle the CV image
        // load grayscale depth image from the kinect source
        grayImage.setFromPixels( kinect.getDepthPixels(), kinect.width, kinect.height);
        // background subtraction
        if (learnBackgroundToggle) {
            bgImage = grayImage;   // let this frame be the background image from now on
            learnBackgroundToggle = false;
        }
        cvAbsDiff(bgImage.getCvImage(), grayImage.getCvImage(), grayDiff.getCvImage());
        cvErode(grayDiff.getCvImage(), grayDiff.getCvImage(), NULL, 2);
        cvDilate(grayDiff.getCvImage(), grayDiff.getCvImage(), NULL, 1);
        // threshold ignoring little differences
        cvThreshold(grayDiff.getCvImage(), grayDiff.getCvImage(), 3, 255, CV_THRESH_BINARY);
        grayDiff.flagImageChanged();
        // update the ofImage to be used as background mask for the blob finder
        grayDiffOfImage.setFromPixels( grayDiff.getPixels(), kinect.width, kinect.height, OF_IMAGE_GRAYSCALE );
        
        // Find the blobs
        blobFinder.findBlobs( &grayDiffOfImage,
                             cropBoxMin, cropBoxMax,
                             thresh3D, thresh2D,
                             minVol, maxVol,
                             minPoints, maxBlobs );
        tracker.trackBlobs( blobFinder.blobs );
    }

}

void ofApp::draw () {
    ofColor brighter = backgroundColorSlider;
    brighter.setBrightness(brighter.getBrightness() + 0.1);
    ofBackgroundGradient( backgroundColorSlider, darkGrey );
    
    ColorStack colorStack;
    static KinectBounds kinectBounds;
    
    //# Draw Camera
    
    camera.begin();
    {
        ofEnableDepthTest();
        
        if ( drawRGB && kinect.isConnected() ) {
            drawPointCloudGL( kinect );
        }
        
        ofPushMatrix();
        {
            drawKinectFrame();

            // Draw the grid
            ofPushStyle();
            {
                ofSetColor( 30, 30, 30 );
                kinectViewGrid.draw();
            }
            ofPopStyle();
            
            // Draw the video feed
            ofPushMatrix();
            {
                ofPushStyle();
                ofSetColor( 255, 255, 255, 20 );
                ofTranslate( 0, 0, kinectBounds.topLeftPoint.z);
                float scale = 14;
                ofScale( scale, -scale );
                ofTranslate( -kinect.width/2.0f, -kinect.height/2.0f );
                kinect.draw(0, 0);
                ofPopStyle();
            }
            ofPopMatrix();
            
            ofDisableDepthTest();
            ofPushMatrix();
            {
                ofRotate( 90, 1, 0, 0 );
                
                for ( auto blob : tracker.blobs ) {
                    ofColor blobColour = colorStack.at( blob.id );
                    ofPushMatrix();
                    ofScale( 1000, 1000, 1000 );
                    ofPushStyle();
                    ofSetColor( blobColour );
                    if ( drawBlobPoints ) {
                        blob.mesh.drawVertices();
                    }
                    ofPopStyle();
                    ofPopMatrix();
                    
                    ofPushMatrix();
                    {
                        doBillboardToBlob( camera, blob );
                        ofPushStyle();
                        {
                            ofSetColor( blobColour );
                            ofDrawSphere( 0, 0, 0, 40 );
                        }
                        ofPopStyle();
                        {
                            ofSetColor( ofColor::white );
                            ofDrawBitmapString( "ID: " + ofToString( blob.id ) + "\n"
                                                // + "Position:\n"
                                                // + ofToString( 1000 * blob.massCenter.x, 0 ) + " "
                                                // + ofToString( 1000 * blob.massCenter.y, 0 ) + " "
                                                // + ofToString( 1000 * blob.massCenter.z, 0 )
                                                // + "\nVolume: " + ofToString( blob.volume, 1 )
                                                + "\nCell: " + ofToString( kinectViewGrid.getCellPosition( blob.massCenter * 1000 ) ),
                                                ofPoint() );
                        }
                        ofPopStyle();
                    }
                    ofPopMatrix();
                }
            }
            ofPopMatrix();
        } 
        ofPopMatrix();
    }
    camera.end();
    
    //# Draw Plan View
    ofPushMatrix();
    {
        ofTranslate( 0, ofGetHeight() );
        
        
        ofSetColor( yellow );
        ofScale( 0.05, 0.05, 0 );
        ofTranslate( kinectBounds.bottomRightPoint.x, 0);
        
        ofPushStyle();
        ofFill();
        ofSetColor( ofColor::black );
        ofDrawRectangle( -KINECT_CUBE_Y_HALF, -8100, KINECT_CUBE_Y, 8200*2 );
        ofPopStyle();
        
        ofRotate( 90, 0, 0, 0 ); // yup.
        ofScale( -1, 1, 1  );
        
        // Draw the grid
        ofPushStyle();
        {
            ofSetColor( 30, 30, 30 );
            kinectViewGrid.draw();
        }
        ofPopStyle();

        drawKinectFrame();
        
        ofRotate( 90, 1, 0, 0 );
        
        for ( auto blob : tracker.blobs ) {
            ofColor blobColour = colorStack.at( blob.id );
            ofPoint massCenter = blob.massCenter * 1000;
            ofPushStyle();
            {
                ofSetColor( blobColour );
                ofDrawSphere( massCenter.x, massCenter.y, massCenter.z, 100 );
            }
            ofPopStyle();
            
            ofPushStyle();
            {
                ofSetColor( ofColor::white );
                ofDrawBitmapString( ofToString( kinectViewGrid.getCellPosition( massCenter ) ),
                                    ofPoint( massCenter.x, massCenter.y, massCenter.z ) );
            }
            ofPopStyle();
        }
    }
    ofPopMatrix();
    
    viewGui.draw();
    appGui.draw();
    oscGui.draw();
    kinectGui.draw();
    detectionGui.draw();
}

void ofApp::exit () {}

void ofApp::keyPressed( int key ) {
    
    switch ( key ) {
        case 'f':
            ofToggleFullscreen();
            break;
    }
    
}
