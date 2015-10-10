# Canterbury Kinects

3D blob detection application for UCA's Interior Architecture department.

# Requirements

+ OpenFrameworks v0.9.0
+ ofxLibWebsockets
+ ofxKinectBlobFinder
+ ofxKinectBlobTracker
+ ofxLiquidEvent
+ ofxOpenCv
+ ofxOsc
+ ofxCv
+ ofxGui

# WebSocket & OSC

When a blob event happens a notification will be sent via the app's websocket server or via its OSC sender. The blob event can be one of three actions: on (when a blob appears), off (when a blob moves) or moved.

## WebSocket Server

The WebSocket server is listening on port 11001.

The websocket message contains a string in the following format:

```
<action>, <kinect-device-id>, <blob-id>, <cell-x-position>, <cell-y-position>, <blob-position-x>,  <blob-position-y>, <blob-position-z>, <number-of-rows>, <number-of-cells>
````

## OSC Messages

The OSC Message has the following signature:

+ Address: `/kinect/on`, `/kinect/off` or `/kinect/off`
+ Int: `kinect-device-id`
+ Int: `blob-id`
+ Int: `cell-x-position`
+ Int: `cell-y-position`
+ Float: `blob-position-x`
+ Float: `blob-position-y`
+ Float: `blob-position-z`
+ Int: `number-of-rows`
+ Int: `number-of-cells`

_Happy hacking..._
