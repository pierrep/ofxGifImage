#pragma once

#include "ofThread.h"
#include "ofxGifImage.h"
#include "ofURLFileLoader.h"
#include "ofTypes.h" 
#include "ofThreadChannel.h"


using namespace std;

class ofxThreadedGifLoader : public ofThread {
public:
    ofxThreadedGifLoader();
    ~ofxThreadedGifLoader();

    void loadFromDisk(ofxGifImage& image, string file);
    void loadFromURL(ofxGifImage& image, string url);
    float getProgress();
    void draw();

private:
	void update(ofEventArgs & a);
    virtual void threadedFunction();
	void urlResponse(ofHttpResponse & response);
    
    // Entry to load.
    struct ofGifImageLoaderEntry {
        ofGifImageLoaderEntry() {
            image = NULL;
        }
        
        ofGifImageLoaderEntry(ofxGifImage & pImage) {
            image = &pImage;
        }
        ofxGifImage* image;
        string filename;
        string url;
        string name;
    };


    typedef map<string, ofGifImageLoaderEntry>::iterator entry_iterator;

	int                 nextID;
    int                 numEntries;
    int                 totalEntries;

    map<string,ofGifImageLoaderEntry> images_async_loading; // keeps track of images which are loading async
    ofThreadChannel<ofGifImageLoaderEntry> images_to_load_from_disk;
    ofThreadChannel<ofGifImageLoaderEntry> images_to_update;
};


