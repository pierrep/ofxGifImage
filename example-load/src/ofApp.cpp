#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    bLoaded = gif.load("cowboy.gif");
    imgNum = 0;
}

//--------------------------------------------------------------
void ofApp::update()
{
}

//--------------------------------------------------------------
void ofApp::draw()
{
    gif.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if (key != OF_KEY_ESC) {
        ofLogNotice() << "Load new Gif!";

        imgNum++;
        if (imgNum > 5)
            imgNum = 0;
        switch (imgNum) {
        case 0:
            gif.load("skate.gif");
            break;
        case 1:
            gif.load("rabbit.gif");
            break;
        case 2:
            gif.load("eyes.gif");
            break;
        case 3:
            gif.load("boy.gif");
            break;
        case 4:
            gif.load("cry.gif");
            break;
        case 5:
            gif.load("cowboy.gif");
            break;
        default:
            break;
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
