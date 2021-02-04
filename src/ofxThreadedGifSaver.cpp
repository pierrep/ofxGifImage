#include "ofxThreadedGifSaver.h"
#include "ofMain.h"
#include <sstream>
ofxThreadedGifSaver::ofxThreadedGifSaver()
{
    nextID = 0;
    ofAddListener(ofEvents().update, this, &ofxThreadedGifSaver::update);

    startThread();
    numEntries = 0;
    totalEntries = 0;
}

ofxThreadedGifSaver::~ofxThreadedGifSaver()
{
    images_to_save_to_disk.close();
    images_to_update.close();
    ofRemoveListener(ofEvents().update, this, &ofxThreadedGifSaver::update);
    stopThread();
    waitForThread(true);
    ofLogVerbose() << "Destructed ofxThreadedGifSaver";
}

// Load an image from disk.
//--------------------------------------------------------------
void ofxThreadedGifSaver::saveToDisk(ofxGifImage& image, string filename, bool absolutePath)
{
    nextID++;
    ofGifImageSaverEntry entry(image);
    entry.filename = filename;
//    entry.image->setUseTexture(false);
    entry.name = filename;
    entry.absolute = absolutePath;
    numEntries++;
    totalEntries++;
    images_to_save_to_disk.send(entry);
}


// Reads from the queue and loads new images.
//--------------------------------------------------------------
void ofxThreadedGifSaver::threadedFunction()
{
    setThreadName("ofxThreadedGifSaver " + ofToString(thread.get_id()));
    ofGifImageSaverEntry entry;
    while (images_to_save_to_disk.receive(entry)) {
        if (entry.image->save(entry.filename,entry.absolute)) {
            images_to_update.send(entry);
        } else {
            ofLogError("ofxThreadedGifSaver") << "couldn't save file: \"" << entry.filename << "\"";
        }
    }
    ofLogVerbose("ofxThreadedGifSaver") << "finishing thread on closed queue";
}

// Check the update queue
//--------------------------------------------------------------
void ofxThreadedGifSaver::update(ofEventArgs& a)
{
    ofGifImageSaverEntry entry;
    if (images_to_update.tryReceive(entry)) {
//        entry.image->setUseTexture(true);
//        entry.image->update();
        numEntries--;
        if (numEntries == 0)
            totalEntries = 0;
    }
}

//--------------------------------------------------------------
float ofxThreadedGifSaver::getProgress()
{
    if ((numEntries == 0) || (totalEntries == 0))
        return 1;

    float progress = 1.0f - static_cast<float>(numEntries) / static_cast<float>(totalEntries);
    return progress;
}

void ofxThreadedGifSaver::draw()
{
    const int bar_height = 10;
    const int curve = 5;

    ofSetColor(238,238,238);
    ofDrawRectRounded(ofGetWidth() / 2 - ofGetWidth() / 4, ofGetHeight() / 2, 0, ofGetWidth() / 2, bar_height, curve);
    ofSetColor(91, 184, 93);
    ofDrawRectRounded(ofGetWidth() / 2 - ofGetWidth() / 4, ofGetHeight() / 2, 1, getProgress() * (ofGetWidth() / 2), bar_height, curve);
}
