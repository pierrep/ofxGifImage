#pragma once

#include "ofThread.h"
#include "ofxGifImage.h"
#include "ofURLFileLoader.h"
#include "ofTypes.h" 
#include "ofThreadChannel.h"


using namespace std;

class ofxThreadedGifSaver : public ofThread {
public:
    ofxThreadedGifSaver();
    ~ofxThreadedGifSaver();

    void saveToDisk(ofxGifImage& image, string file, bool absolutePath = false);
    float getProgress();
    void draw();

private:
	void update(ofEventArgs & a);
    virtual void threadedFunction();
    
    // Entry to load.
    struct ofGifImageSaverEntry {
        ofGifImageSaverEntry() {
            image = nullptr;
        }
        
        ofGifImageSaverEntry(ofxGifImage & pImage) {
            image = &pImage;
        }
        ofxGifImage* image;
        string filename;
        string name;
        bool absolute;
    };


    typedef map<string, ofGifImageSaverEntry>::iterator entry_iterator;

	int                 nextID;
    int                 numEntries;
    int                 totalEntries;

    ofThreadChannel<ofGifImageSaverEntry> images_to_save_to_disk;
    ofThreadChannel<ofGifImageSaverEntry> images_to_update;
};


