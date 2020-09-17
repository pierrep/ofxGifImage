#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofDirectory dir;
    dir.listDir("");
    dir.allowExt("gif");
    dir.sort();

    for(unsigned int i = 0; i < dir.size(); i++) {
        ofxGifImage gif;
        images.push_back(gif);
    }

    for(unsigned int i = 0; i < dir.size(); i++) {
        loader.loadFromDisk(images[i], dir.getPath(i));
    }

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    // draw the images.
    ofSetColor(255);
    for(int i = 0; i < (int)images.size(); ++i) {
        int x = (i%8);
        int y = (i/8);
        images[i].draw(x*128,y*128, 128,128);
    }

    if(loader.getProgress() < 1.0f) {
    loader.draw();
    }

    // draw the FPS
    ofDrawRectangle(0,ofGetHeight()-20,30,20);

    ofSetColor(0);
    ofDrawBitmapString(ofToString(ofGetFrameRate(),0),5,ofGetHeight()-5);
}

//--------------------------------------------------------------
void ofApp::exit(){
    loader.stopThread();
    ofLogNotice() << "Stop Thread";
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
