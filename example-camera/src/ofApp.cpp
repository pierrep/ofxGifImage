#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    videoGrabber.setup(640, 480);
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

//--------------------------------------------------------------
void ofApp::draw()
{
    videoGrabber.draw(0, 0);
    if (gif.getNumFrames() > 0) {
        gif.draw(640, 0);
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
            gif.save("animation.gif");
            gif.load("animation.gif");
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
