#include "ofMain.h"
#include "ofxLiquidEvent.h"
#include "ofxLibwebsockets.h"
#include "ofxKinectBlobTracker.h"
#include "ofxGui.h"

namespace Emitters {
    struct Toggle {
        void bind( ofxToggle &toggle ) {
            toggle.addListener( this, &Toggle::mouseReleased );
        }
        void unbind( ofxToggle &toggle ) {
            toggle.removeListener( this, &Toggle::mouseReleased );
        }
        
        void mouseReleased(bool & value) { released.notifyListeners(value); }
        
        ofxLiquidEvent<bool> released;
    };
    
    struct Button {
        void bind( ofxButton &button ) {
            button.addListener( this, &Button::mouseReleased );
        }
        void unbind( ofxButton &button ) {
            button.removeListener( this, &Button::mouseReleased );
        }
        
        void mouseReleased() { released.notifyListeners(); }
        
        ofxLiquidEvent<void> released;
    };
    
    struct WebsocketServer {
        void bind( ofxLibwebsockets::Server &server) {
            server.addListener( this );
        }
        void unbind( ofxLibwebsockets::Server &server) {
            server.removeListener( this );
        }
        
        ofxLiquidEvent<ofxLibwebsockets::Event> connect;
        ofxLiquidEvent<ofxLibwebsockets::Event> open;
        ofxLiquidEvent<ofxLibwebsockets::Event> close;
        ofxLiquidEvent<ofxLibwebsockets::Event> idle;
        ofxLiquidEvent<ofxLibwebsockets::Event> message;
        ofxLiquidEvent<ofxLibwebsockets::Event> broadcast;
        
        // # Websocket Methods
        void onConnect ( ofxLibwebsockets::Event& args ) { connect.notifyListeners( args ); }
        void onOpen ( ofxLibwebsockets::Event& args ) { open.notifyListeners( args ); }
        void onClose ( ofxLibwebsockets::Event& args ) { close.notifyListeners( args ); }
        void onIdle ( ofxLibwebsockets::Event& args ) { idle.notifyListeners( args ); }
        void onMessage ( ofxLibwebsockets::Event& args ) { message.notifyListeners( args ); }
        void onBroadcast ( ofxLibwebsockets::Event& args ) { broadcast.notifyListeners( args ); }
    };
    
    struct BlobTracker : public ofxKinectBlobListener {
        void bind( ofxKinectBlobTracker &tracker ) {
            tracker.setListener( this );
        }
        void unbind( ofxKinectBlobTracker &tracker ) {
            tracker.setListener( NULL );
        }
        
        struct Args {
            Args( ofVec3f centroid, int id, int order )
            :centroid(centroid),
            id(id),
            order(order){};
            
            ofVec3f centroid;
            int id;
            int order;
        };
        
        ofxLiquidEvent<Args> on;
        ofxLiquidEvent<Args> off;
        ofxLiquidEvent<Args> moved;
        
        void blobOn( ofVec3f centroid, int id, int order ) { Args args( centroid, id, order ); on.notifyListeners( args ); };
        void blobMoved( ofVec3f centroid, int id, int order ) { Args args( centroid, id, order ); moved.notifyListeners( args ); };
        void blobOff( ofVec3f centroid, int id, int order ) { Args args( centroid, id, order ); off.notifyListeners( args ); };

    };
}
