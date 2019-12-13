#pragma once

#include "FreeImage.h"
#include "ofMain.h"

enum GifFrameDisposal {
    GIF_DISPOSAL_UNSPECIFIED,
    GIF_DISPOSAL_LEAVE,
    GIF_DISPOSAL_BACKGROUND,
    GIF_DISPOSAL_PREVIOUS
};

enum GifDitherType {
    OFX_GIF_DITHER_NONE         = -1,
    OFX_GIF_DITHER_FS           = FID_FS,           // Floyd & Steinberg error diffusion
    OFX_GIF_DITHER_BAYER4x4     = FID_BAYER4x4,		// Bayer ordered dispersed dot dithering (order 2 dithering matrix)
    OFX_GIF_DITHER_BAYER8x8     = FID_BAYER8x8, 	// Bayer ordered dispersed dot dithering (order 3 dithering matrix)
    OFX_GIF_DITHER_CLUSTER6x6   = FID_CLUSTER6x6,	// Ordered clustered dot dithering (order 3 - 6x6 matrix)
    OFX_GIF_DITHER_CLUSTER8x8   = FID_CLUSTER8x8,	// Ordered clustered dot dithering (order 4 - 8x8 matrix)
    OFX_GIF_DITHER_CLUSTER16x16 = FID_CLUSTER16x16, // Ordered clustered dot dithering (order 8 - 16x16 matrix)
    OFX_GIF_DITHER_BAYER16x16   = FID_BAYER16x16,	// Bayer ordered dispersed dot dithering (order 4 dithering matrix)
};

struct GifFrame {
    ofPixels pixels;
    ofTexture tex;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    int top;
    int left;
    float duration;
    GifFrameDisposal disposal;
};

class ofxGifImage {
public:
    ofxGifImage();

    bool load(string filename);
    void save(string filename);
    void append(string filename);
    void append(ofPixels& pixels);
    void draw(float x, float y);
    void draw(float x, float y, float w, float h);
    void drawFrame(int frameNum, float x, float y);
    void drawFrame(int frameNum, float x, float y, int w, int h);
    void setDefaultFrameDuration(float duration);
    unsigned int getNumFrames();
    void clear();
    void setNumColours(int numColours);
    void setDither(GifDitherType dither);

protected:

    void getMetadata(FIBITMAP* bmp);
    void decodeFrame(FIBITMAP* bmp);
    void encodeFrame(GifFrame& frame, FIMULTIBITMAP* multi);
    void updateFrameIndex();
    string getPixelFormatString(ofPixels p);

    unsigned int width;
    unsigned int height;
    vector<GifFrame> frames;
    vector <ofColor> palette;
    ofPixels accumPx;
    float defaultFrameDuration;
    ofColor backgroundColour;
    float lastDrawn;
    int frameIndex;
    int numColours;
    int ditherMode;

private:

};
