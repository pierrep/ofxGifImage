#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    bool bHasCamera = videoGrabber.setup(640, 480);
    frameDelay = 0.15f;
    bDoCapture = false;
    frameTimer = 0;
    gif.setDefaultFrameDuration(0.06f);
    gif.setNumColours(64);
    //gif.setDither(OFX_GIF_DITHER_FS);
}

//--------------------------------------------------------------
void ofApp::update()
{
    if (bHasCamera) {
        videoGrabber.update();
        if (videoGrabber.isFrameNew()) {

            if (frameTimer == 0) {
                frameTimer = ofGetElapsedTimef();
            }
            if (bDoCapture) {
                if ((ofGetElapsedTimef() - frameTimer) >= frameDelay) {
                    float diff = (ofGetElapsedTimef() - frameTimer) - frameDelay;
                    frameTimer = ofGetElapsedTimef() - diff;

                    gif.append(videoGrabber.getPixels());
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofLine(ofGetWidth()/2,0,ofGetWidth()/2,ofGetHeight());

    if (bHasCamera) {
        videoGrabber.draw(0, 0);
    } else {
        ofDrawBitmapString("NO CAMERA", ofGetWidth()/4,ofGetHeight()/2);
    }


    if (bDoCapture) {
        ofPushStyle();
        ofSetColor(255, 0, 0);
        ofDrawCircle(30, 30, 10);
        ofPopStyle();
    }

    if (!bDoCapture) {
        if (gif.getNumFrames() > 0) {
            gif.draw(640, 0);
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if (key == ' ') {
        bDoCapture = !bDoCapture;
        if (bDoCapture) {
            gif.clear();
            frameTimer = 0;
        } else if (gif.getNumFrames() > 0) {
            string timestamp = ofGetTimestampString();
            string filename = "animation" + timestamp + ".gif";

#ifdef TARGET_LINUX
            filename = "~/Pictures/" + filename;
            gif.save(filename, true);

#else
            gif.save(filename);
#endif

            gif.load(filename);
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}
