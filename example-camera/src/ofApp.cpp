#include "ofApp.h"

#ifdef TARGET_LINUX
#include <pwd.h>
#endif

//--------------------------------------------------------------
void ofApp::setup()
{
    bHasCamera = videoGrabber.setup(640, 480);
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

                    ofPixels pixels = videoGrabber.getPixels();
                    ofPixels p;
                    p.allocate(pixels.getWidth(), pixels.getHeight(), OF_PIXELS_RGBA);
                    for (int y = 0; y < pixels.getHeight(); y++) {
                        for (int x = 0; x < pixels.getWidth(); x++) {
                            unsigned int index = pixels.getPixelIndex(x, y);
                            ofColor c1 = prevPixels.getColor(x, y);
                            ofColor c2 = pixels.getColor(x, y);
                            ofColor c;
                            if (c1 == c2) {
                                c = ofColor(0);
                                c.a = 0;
                            } else {
                                c = c2;
                                c.a = 255;
                            }
                            p.setColor(index, c);
                        }
                    }
                    prevPixels = pixels;
                    gif.setTransparentColour(ofColor(0,0,0));
                    gif.setTransparency(true);

                    //gif.append(videoGrabber.getPixels());
                    gif.append(p);
                }
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofDrawLine(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight());

    if (bHasCamera) {
        videoGrabber.draw(0, 0);
    } else {
        ofDrawBitmapString("NO CAMERA", ofGetWidth() / 4 - 30, ofGetHeight() / 2);
    }

    if (bDoCapture) {
        ofPushStyle();
        ofSetColor(255, 0, 0);
        ofDrawCircle(30, 30, 10);
        ofPopStyle();
    } else {
        if (gif.getNumFrames() > 0) {
            gif.draw(640, 0);
        }
        else {
          ofDrawBitmapString("SPACEBAR TO RECORD", ofGetWidth() *0.75f - 60, ofGetHeight() / 2);
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
            ofFilePath path;
            filename = path.getUserHomeDir() + "/Pictures/" + filename;
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
