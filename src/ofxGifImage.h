#pragma once

#include "ofMain.h"
#include "FreeImage.h"

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

class ofxGifImage: public ofBaseImage {
public:
    ofxGifImage();
    //ofxGifImage(const ofxGifImage&);
    ~ofxGifImage();

    ofTexture & getTexture();
    const ofTexture & getTexture() const;
    void setUseTexture(bool bUseTex);
    bool isUsingTexture() const;
    ofPixels & getPixels();
    const ofPixels & getPixels() const;
    float getHeight() const;
    float getWidth() const;

    bool load(string filename);
    bool load(ofBuffer buffer);
    bool save(string filename, bool bAbsolutePath = false);
    void append(string filename);
    void append(ofPixels& pixels);
    void draw(float x, float y);
    void draw(float x, float y, float w, float h);
    void draw(float x, float y, float w, float h) const;
    void drawFrame(float x, float y, int frameNum);
    void drawFrame(float x, float y, int w, int h, int frameNum);
    void update();
    void updateTextures();
    void setDefaultFrameDuration(float duration);
    void setLoopState(ofLoopType state);
    unsigned int getNumFrames();
    unsigned int getFrameIndex();
    void clear();
    void setNumColours(int numColours);
    void setDither(GifDitherType dither);
    void setTransparencyOptimisation(bool value);
    ofColor getGlobalPalette(unsigned int index);
    void quantize(ofPixels &pix);
    void quantize();
    void play() {bIsPlaying = true;}
    void playFromStart() {bIsPlaying = true; frameIndex = 0;}
    void stop() {bIsPlaying = false; frameIndex = 0;}
    void pause() {bIsPlaying = false;}


protected:

    void getMetadata(FIBITMAP* bmp);
    void decodeFrame(FIBITMAP* bmp);
    void encodeFrame(GifFrame& frame, FIMULTIBITMAP* multi, unsigned int pageNum);
    void updateFrameIndex();
    string getPixelFormatString(ofPixels p);
    void calculateTransparencyOptimisation(FIBITMAP* quantizedBmp, unsigned int pageNum);

    unsigned int width;
    unsigned int height;
    bool bUseTexture;
    vector<GifFrame> frames;
    vector <ofColor> palette;
    ofPixels accumPx;
    float defaultFrameDuration;
    ofColor backgroundColour;
    float lastDrawn;
    unsigned int frameIndex;
    ofLoopType loopMode;
    bool bIsPlaying;
    int numColours;
    int ditherMode;
    bool bSetTransparencyOptimisation;
    string customFolder;
    FIBITMAP* previousBmp;
    RGBQUAD* globalPalette;

private:

};
