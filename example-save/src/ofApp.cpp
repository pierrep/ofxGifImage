#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    prevPixels.allocate(ofGetWidth(), ofGetHeight(), OF_PIXELS_RGBA);
//   bLoaded = gif.load("test.gif");
}

//--------------------------------------------------------------
void ofApp::update()
{
}

//--------------------------------------------------------------
void ofApp::draw()
{
//    gif.draw(0, 0);

    ofBackground(255, 255, 0);
    ofSetColor(0, 0, 255);
    ofDrawCircle(ofGetMouseX(), ofGetMouseY(), 50);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if (key == ' ')
        gif.save("test.gif");
    if (key == 'a') {
        ofImage image;
        image.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        ofPixels pixels = image.getPixels();
        ofPixels p;
        p.allocate(pixels.getWidth(), pixels.getHeight(), OF_PIXELS_RGBA);
        for (int y = 0; y < image.getHeight(); y++) {
            for (int x = 0; x < image.getWidth(); x++) {
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
        //gif.append(image.getPixels());
        gif.setTransparentColour(ofColor(0,0,0));
        gif.setTransparency(true);
        gif.append(p);
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
