#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofDirectory dir;
    dir.allowExt("gif");
    dir.listDir("");
    dir.sort();

    unsigned int num_files = dir.size();

    for(unsigned int i = 0; i < num_files; i++) {
        ofxGifImage gif;
        images.push_back(gif);
    }

    for(unsigned int i = 0; i < dir.size(); i++) {
        loader.loadFromDisk(images[i], dir.getPath(i));
    }

    webImage.setUseTexture(false);
    bLoadWebImage = false;

    bLoadingWebImage = false;
}

//--------------------------------------------------------------
void ofApp::update(){
    for(unsigned int i = 0; i < images.size(); i++) {
        if(images[i].isUsingTexture()) {
            images[i].update();
        }
    }

    if(webImage.isUsingTexture())
    {
        webImage.update();
    }

    if(bReloadImages) {
        ofDirectory dir;
        dir.allowExt("gif");
        dir.listDir("");
        dir.sort();

        for(unsigned int i = 0; i < dir.size(); i++) {
            loader.loadFromDisk(images[i], dir.getPath(i));
        }
        bReloadImages = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    // draw the local images.
    ofSetColor(255);
    for(unsigned int i = 0; i < images.size(); ++i) {
        int x = (i%8);
        int y = (i/8);
        if(images[i].isUsingTexture()) {
            images[i].draw(x*128,y*128, 128,128);
        }
    }

    if(loader.getProgress() < 1.0f) {
        // draw progress bar
        loader.draw();
    }

    if(bLoadWebImage){
        bLoadingWebImage = true;
        urlloader.loadFromURL(webImage,"https://blog.loomly.com/wp-content/uploads/2019/06/earth.gif");
        bLoadWebImage = false;
    }

    if(webImage.isUsingTexture())
    {
        //draw the web image
        ofSetColor(255);
        webImage.draw(0,256,128,128);
        bLoadingWebImage = false;
    }

    if(bLoadingWebImage) {
        ofDrawBitmapString("Loading image from web",10, 256);
    }

    // draw the FPS
    ofSetColor(200,200,255);
    ofDrawBitmapString("Hit spacebar to load a GIF from the web ",20,ofGetHeight()-60);
    ofDrawBitmapString("Hit 'r' to reload local images ",20,ofGetHeight()-40);
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate()),20,ofGetHeight()-20);
}

//--------------------------------------------------------------
void ofApp::exit(){
    loader.stopThread();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == ' ') {
        webImage.setUseTexture(false);
        bLoadWebImage = true;
    }

    if(key == 'r') {
        bReloadImages = true;
    }
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
