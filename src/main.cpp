#include "ofMain.h"
#include "ofApp.h"

int main () {
    // Set Data Path
    ofSetDataPathRoot( "../Resources/data/" );
    
    ofSetupOpenGL( 1024, 768, OF_FULLSCREEN );
    ofRunApp( new ofApp() );    
}
