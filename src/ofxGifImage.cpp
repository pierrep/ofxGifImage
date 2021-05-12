#include "ofxGifImage.h"

#define OFX_GIF_DEFAULT_FRAME_DELAY 0.1f
#define SAVE_TO_CUSTOM_FOLDER 1

//-----------------------------------------------------------------------
ofxGifImage::ofxGifImage()
{
    globalPalette = nullptr;

    clear();

    numColours = 256;
    ditherMode = OFX_GIF_DITHER_NONE;
    defaultFrameDuration = OFX_GIF_DEFAULT_FRAME_DELAY;
    bSetTransparencyOptimisation = false;
#ifdef SAVE_TO_CUSTOM_FOLDER
    customFolder = "~/Pictures/";
#endif    
    bUseTexture = true;
}

ofxGifImage::~ofxGifImage()
{
    ofLogVerbose() << "Destructed ofxGifImage";
    clear();
}

//-----------------------------------------------------------------------
//ofxGifImage::ofxGifImage(const ofxGifImage& parent)
//{
//    globalPalette = nullptr;

//    clear();

//    cout << "copy constructor called" << endl;
//    numColours = parent.numColours;
//    ditherMode = parent.ditherMode;
//    defaultFrameDuration = parent.defaultFrameDuration;
//    bSetTransparencyOptimisation = parent.bSetTransparencyOptimisation;
//#ifdef SAVE_TO_CUSTOM_FOLDER
//    customFolder = parent.customFolder;
//#endif
//   bUseTexture = parent.bUseTexture;

//   globalPalette = parent.globalPalette;

//}

//-----------------------------------------------------------------------
bool ofxGifImage::load(string filename)
{
    bool bLoaded = false;
    FIMULTIBITMAP* multiBmp = nullptr;
    FREE_IMAGE_FORMAT filetype = FIF_UNKNOWN;

    filename = ofToDataPath(filename);
    filetype = FreeImage_GetFileType(filename.c_str(), 0);
    if (filetype != FIF_GIF) {
        ofLog(OF_LOG_ERROR, "Load failed, this is not a Gif file.");
        return bLoaded;
    }

    clear();

    multiBmp = FreeImage_OpenMultiBitmap(filetype, filename.c_str(), false, false, true, GIF_LOAD256);

    if (multiBmp) {

        // num frames
        int frameCount = FreeImage_GetPageCount(multiBmp);
        ofLogVerbose() << "Gif loaded: " << frameCount << " frames.";

        // here we process the first frame
        for (int i = 0; i < frameCount; i++) {
            FIBITMAP* bmp = FreeImage_LockPage(multiBmp, i);
            if (bmp) {
                if (i == 0) {
                    getMetadata(bmp);
                    bLoaded = true;
                }
                ofLogVerbose() << "Decoding frame: " << i << " -------------";
                decodeFrame(bmp);
                FreeImage_UnlockPage(multiBmp, bmp, false);
            } else {
                ofLog(OF_LOG_ERROR, "Problem locking page while opening Gif.");
            }
        }
        FreeImage_CloseMultiBitmap(multiBmp, 0);
    } else {
        ofLog(OF_LOG_ERROR, "Couldn't open Gif as multi-bitmap.");
    }

    return bLoaded;
}

//-----------------------------------------------------------------------
bool ofxGifImage::load(ofBuffer buffer)
{
    bool bLoaded = false;
    FIMULTIBITMAP* multiBmp = nullptr;
    FREE_IMAGE_FORMAT filetype = FIF_UNKNOWN;
    FIMEMORY* hmem = nullptr;

    hmem = FreeImage_OpenMemory((unsigned char*) buffer.getData(), buffer.size());
    if (hmem == nullptr){
        ofLogError("ofxGifImage") << "load(): couldn't load image from ofBuffer, opening FreeImage memory failed";
        return false;
    }

    filetype= FreeImage_GetFileTypeFromMemory(hmem);
    if (filetype != FIF_GIF) {
        ofLogError("ofImage") << "loadImage(): couldn't load image from ofBuffer, this is not a Gif file";
        FreeImage_CloseMemory(hmem);
        return bLoaded;
    }

    clear();

    multiBmp = FreeImage_LoadMultiBitmapFromMemory(filetype, hmem, GIF_LOAD256);

    if (multiBmp) {

        // num frames
        int frameCount = FreeImage_GetPageCount(multiBmp);
        ofLogVerbose() << "Gif loaded: " << frameCount << " frames.";

        // here we process the first frame
        for (int i = 0; i < frameCount; i++) {
            FIBITMAP* bmp = FreeImage_LockPage(multiBmp, i);
            if (bmp) {
                if (i == 0) {
                    getMetadata(bmp);
                    bLoaded = true;
                }
                ofLogVerbose() << "Decoding frame: " << i << " -------------";
                decodeFrame(bmp);
                FreeImage_UnlockPage(multiBmp, bmp, false);
            } else {
                ofLog(OF_LOG_ERROR, "Problem locking page while opening Gif.");
            }
        }
        FreeImage_CloseMultiBitmap(multiBmp, 0);
    } else {
        ofLog(OF_LOG_ERROR, "Couldn't open Gif as multi-bitmap.");
    }

    return bLoaded;
}

//-----------------------------------------------------------------------
bool ofxGifImage::save(string filename, bool bAbsolutePath)
{
    if(frames.size() == 0) {
        ofLogError("ofxGifImage") << "Can't save an empty GIF, need to add some frames first...";
        return false;
    }

    string savefilename;
    if (bAbsolutePath) {
        savefilename = filename;
    } else {
        savefilename = ofToDataPath(filename);
    }

    // create a multipage bitmap
    FIMULTIBITMAP* multi = FreeImage_OpenMultiBitmap(FIF_GIF, savefilename.c_str(), TRUE, FALSE);
    if(multi == nullptr) {
        ofLogError("ofxGifImage") << "Failed to save GIF";
        return false;
    }
    // set width and height
    width = frames[0].width;
    height = frames[0].height;
    for (unsigned int i = 0; i < frames.size(); i++) {
        GifFrame currentFrame = frames[i];
        if((width != frames[i].width) && (height != frames[0].height)) {
            ofLogWarning("ofxGifImage") << "GIF frames aren't all the same size, GIF file width and height may vary";
        }
        encodeFrame(currentFrame, multi, i);
    }
    if (previousBmp) {
        FreeImage_Unload(previousBmp);
        previousBmp = nullptr;
    }

    FreeImage_CloseMultiBitmap(multi);
    ofLogNotice("ofxGifImage") << "Gif saved. Width: " << width << " Height: " << height << " Frames: " << frames.size();
    return true;
}

//-----------------------------------------------------------------------
void ofxGifImage::append(string filename)
{
    ofImage img;
    img.load(filename);
    GifFrame frame;
    frame.pixels = img.getPixels();
    frame.width = img.getWidth();
    frame.height = img.getHeight();
    frame.bpp = img.getPixels().getBitsPerPixel();
    frame.duration = defaultFrameDuration;
    frame.top = 0;
    frame.left = 0;
    frame.disposal = GIF_DISPOSAL_LEAVE;
    if (bUseTexture) {
        frame.tex.loadData(img.getPixels());
    }

    frames.push_back(frame);
}

//-----------------------------------------------------------------------
void ofxGifImage::append(ofPixels& pixels)
{
    GifFrame frame;
    frame.pixels = pixels;
    frame.width = pixels.getWidth();
    frame.height = pixels.getHeight();
    frame.bpp = pixels.getBitsPerPixel();
    frame.duration = defaultFrameDuration;
    frame.top = 0;
    frame.left = 0;
    frame.disposal = GIF_DISPOSAL_LEAVE;
    if (bUseTexture) {
        frame.tex.loadData(pixels);
    }

    frames.push_back(frame);
}

//-----------------------------------------------------------------------
void ofxGifImage::setDefaultFrameDuration(float duration)
{
    defaultFrameDuration = duration;
}

//-----------------------------------------------------------------------
unsigned int ofxGifImage::getNumFrames()
{
    return frames.size();
}

//-----------------------------------------------------------------------
void ofxGifImage::draw(float x, float y)
{
    if (frames.size() == 0) {
        ofLogVerbose("ofxGifImage") << "No frames to draw!";
        return;
    }

    updateFrameIndex();
    drawFrame(x, y, width, height, frameIndex);
}

//-----------------------------------------------------------------------
void ofxGifImage::draw(float x, float y, float w, float h)
{
    if (frames.size() == 0) {
        ofLogVerbose("ofxGifImage") << "No frames to draw!";
        return;
    }

    updateFrameIndex();
    drawFrame(x, y, w, h, frameIndex);
}

//-----------------------------------------------------------------------
void ofxGifImage::drawFrame(float x, float y, int frameNum)
{
    if (frameNum < 0 || frameNum >= frames.size()) {
        ofLogWarning("ofxGifImage") << "drawFrame() frame index out of bounds!";
        return;
    }
    drawFrame(x, y, width, height, frameNum);
}

//-----------------------------------------------------------------------
void ofxGifImage::drawFrame(float x, float y, int w, int h, int frameNum)
{
    if (frameNum < 0 || frameNum >= frames.size()) {
        ofLogWarning("ofxGifImage") << "drawFrame() frame index out of bounds!";
        return;
    }
    if (bUseTexture) {
        frames[frameNum].tex.draw(x, y, w, h);
    }
}

//-----------------------------------------------------------------------
void ofxGifImage::updateFrameIndex()
{
    if (lastDrawn == 0) {
        lastDrawn = ofGetElapsedTimef();
    }
    if ((ofGetElapsedTimef() - lastDrawn) >= frames[frameIndex].duration) {
        float diff = (ofGetElapsedTimef() - lastDrawn) - frames[frameIndex].duration;
        lastDrawn = ofGetElapsedTimef() - diff;
        frameIndex++;
        frameIndex %= frames.size();
    }
    return;
}

//-----------------------------------------------------------------------
void ofxGifImage::update()
{
//    width = pixels.getWidth();
//    height = pixels.getHeight();
//    bpp = pixels.getBitsPerPixel();
//    type = pixels.getImageType();
//    if (pixels.isAllocated() && bUseTexture){
//        int glInternalFormat = ofGetGLInternalFormat(pixels);
//        if(!tex.isAllocated() || tex.getWidth() != width || tex.getHeight() != height || tex.getTextureData().glInternalFormat != glInternalFormat){
//            tex.allocate(pixels);
//        }else{
//            tex.loadData(pixels);
//        }
//    }

    updateFrameIndex();

//    for(int i = 0; i < frames.size(); i++ ) {
//        frames[i].tex.loadData(frames[i].pixels);
//    }

}

//-----------------------------------------------------------------------
void ofxGifImage::clear()
{
    accumPx.clear();
    frames.clear();
    palette.clear();
    frameIndex = 0;
    lastDrawn = 0;
    width = 0;
    height = 0;
    previousBmp = nullptr;
    if (globalPalette != nullptr) {
        ofLogVerbose() << "Deleting globalPalette: " << globalPalette;
        delete[] globalPalette;
        globalPalette = nullptr;
    }
}

//-----------------------------------------------------------------------
void ofxGifImage::setNumColours(int colours)
{
    if (colours < 2) {
        ofLogError("ofxGifImage") << "setNumColours() : minimum colours is 2";
        return;
    }
    numColours = colours;
}

//-----------------------------------------------------------------------
void ofxGifImage::setDither(GifDitherType type)
{
    ditherMode = type;
}

//-----------------------------------------------------------------------
void ofxGifImage::setTransparencyOptimisation(bool value)
{
    bSetTransparencyOptimisation = value;
}

//-----------------------------------------------------------------------
void ofxGifImage::quantize()
{
    for (unsigned int i = 0; i < frames.size(); i++) {
        GifFrame currentFrame = frames[i];
        quantize(currentFrame.pixels);
        if (bUseTexture) {
            currentFrame.tex.loadData(currentFrame.pixels);
        }
    }
}

//-----------------------------------------------------------------------
void ofxGifImage::quantize(ofPixels &pix)
{
    FIBITMAP* bmpConverted = NULL;

    pix.swapRgb();			// should add this

    unsigned char* pixels = pix.getData();
    unsigned int width = pix.getWidth();
    unsigned int height = pix.getHeight();
    unsigned int bpp = pix.getBitsPerPixel();

    FREE_IMAGE_TYPE freeImageType = FIT_BITMAP;		// not need to check, just use unsigned char
    FIBITMAP* bmp = FreeImage_AllocateT(freeImageType, width, height, bpp);
    unsigned char* bmpBits = FreeImage_GetBits(bmp);
    if (bmpBits != NULL) {
        int srcStride = width * pix.getBytesPerPixel();
        int dstStride = FreeImage_GetPitch(bmp);
        unsigned char* src = (unsigned char*)pixels;
        unsigned char* dst = bmpBits;
        for (int i = 0; i < (int)height; i++) {
            memcpy(dst, src, srcStride);
            src += srcStride;
            dst += dstStride;
        }
    }
    else {
        ofLogError("ofImage") << "getBmpFromPixels(): unable to get FIBITMAP from ofPixels";
    }

    // ofPixels are top left, FIBITMAP is bottom left
    //FreeImage_FlipVertical(bmp);
    bpp = FreeImage_GetBPP(bmp);

    // this will create a 256-color palette from the image for gif
    FIBITMAP* convertedBmp;
    convertedBmp = FreeImage_ColorQuantize(bmp, FIQ_NNQUANT);
    //convertedBmp = FreeImage_ColorQuantizeEx(bmp, FIQ_NNQUANT, numColours, 0, nullptr);

    //FreeImage_FlipVertical(convertedBmp);

    bpp = FreeImage_GetBPP(convertedBmp);
    unsigned int channels = (bpp / sizeof(unsigned char)) / 8;
    unsigned int pitch = FreeImage_GetPitch(convertedBmp);

    bmpBits = FreeImage_GetBits(convertedBmp);
    if (bmpBits != NULL) {
        pix.setFromAlignedPixels((unsigned char*)bmpBits, width, height, channels, pitch);
    }
    else {
        ofLogError("ofImage") << "putBmpIntoPixels(): unable to set ofPixels from FIBITMAP";
    }

    FreeImage_Unload(bmp);

    if (bmpConverted != NULL) {
        FreeImage_Unload(bmpConverted);
    }

}

//-----------------------------------------------------------------------
void ofxGifImage::getMetadata(FIBITMAP* bmp)
{
    FITAG* tag;

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "LogicalWidth", &tag)) {
        width = *(unsigned short*)FreeImage_GetTagValue(tag);
    }

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "LogicalHeight", &tag)) {
        height = *(unsigned short*)FreeImage_GetTagValue(tag);
    }

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "GlobalPalette", &tag)) {
        unsigned int paletteSize = FreeImage_GetTagCount(tag);
        if (paletteSize >= 2) {
            RGBQUAD* filePalette = (RGBQUAD*)FreeImage_GetTagValue(tag);
            globalPalette = new RGBQUAD[paletteSize];
            memcpy(globalPalette, filePalette, paletteSize * sizeof(RGBQUAD));
            ofLogVerbose() << "Using Global Palette.";
        }
    } else {
        ofLogVerbose("ofxGifImage") << "No Global Palette.";
    }

    if (FreeImage_HasBackgroundColor(bmp)) {
        RGBQUAD bgColor;
        if (FreeImage_GetBackgroundColor(bmp, &bgColor)) {
            backgroundColour = ofColor(bgColor.rgbRed, bgColor.rgbGreen, bgColor.rgbBlue);
            ofLogVerbose() << "Background colour: " << backgroundColour;
        }
    } else {
        ofLogVerbose() << "No background colour set.";
    }

    ofLogVerbose() << "Is transparent? " << FreeImage_IsTransparent(bmp);
    ofLogVerbose() << "Transparency count: " << FreeImage_GetTransparencyCount(bmp);
    ofLogVerbose() << "Transparent index: " << FreeImage_GetTransparentIndex(bmp);
}

//-----------------------------------------------------------------------
void ofxGifImage::decodeFrame(FIBITMAP* bmp)
{
    FITAG* tag = nullptr;
    ofPixels pix;
    GifFrame frame;

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameLeft", &tag)) {
        if (tag != nullptr) {
            frame.left = *(unsigned short*)FreeImage_GetTagValue(tag);
            ofLogVerbose() << "Frame Left:" << frame.left;
        }
    }

    tag = nullptr;
    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTop", &tag)) {
        if (tag != nullptr) {
            frame.top = *(unsigned short*)FreeImage_GetTagValue(tag);
            ofLogVerbose() << "Frame Top:" << frame.top;
        }
    }

    tag = nullptr;
    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTime", &tag)) {
        if (tag != nullptr) {
            uint32_t* frameTime = (uint32_t*)FreeImage_GetTagValue(tag);
            ofLogVerbose() << "Tag type: " << FreeImage_GetTagType(tag);
            ofLogVerbose() << "Tag count: " << FreeImage_GetTagCount(tag);
            ofLogVerbose() << "Frame Time: " << *frameTime;

            if (*frameTime > 0) {
                frame.duration = (float)(*frameTime) / 1000.0f; // convert to milliseconds
            } else {
                frame.duration = defaultFrameDuration;
            }
            ofLogVerbose() << "Frame duration: " << frame.duration;
        } else {
            ofLogVerbose() << "GIF frame duration tag not found ";
        }
    }

    tag = nullptr;
    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "NoLocalPalette", &tag)) {
        if (tag != nullptr) {
            uint8_t noLocalPalette = *(uint8_t*)FreeImage_GetTagValue(tag);
            if (noLocalPalette == 0x00) {
                ofLogVerbose() << "Local Palette exists.";
            } else {
                ofLogVerbose() << "No Local Palette.";
            }
        }
    }

    if (FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "DisposalMethod", &tag)) {
        if (tag != nullptr) {
            frame.disposal = (GifFrameDisposal) * (unsigned char*)FreeImage_GetTagValue(tag);
            ofLogVerbose() << "Frame Disposal: " << frame.disposal;
        }
    }

    // Force all incoming frames to be RGBA
    FIBITMAP* bmpConverted = nullptr;
    if (FreeImage_GetColorType(bmp) == FIC_PALETTE || FreeImage_GetBPP(bmp) < 8) {
        bmpConverted = FreeImage_ConvertTo32Bits(bmp);
        bmp = bmpConverted;
        frame.bpp = 32;
    }

    frame.width = FreeImage_GetWidth(bmp);
    frame.height = FreeImage_GetHeight(bmp);
    unsigned int bpp = FreeImage_GetBPP(bmp);
    unsigned int channels = (bpp / sizeof(unsigned char)) / 8;
    unsigned int pitch = FreeImage_GetPitch(bmp);
    ofLogVerbose() << "Frame width: " << frame.width << " height: " << frame.height << " bpp: " << bpp << " channels: " << channels;

    FreeImage_FlipVertical(bmp); // ofPixels are top left, FIBITMAP is bottom left

    unsigned char* bmpBits = FreeImage_GetBits(bmp);

    // FreeImage uses a BGR[A] pixel layout under a Little Endian processor (Windows, Linux)
    // and uses a RGB[A] pixel layout under a Big Endian processor (Mac OS X or any Big Endian Linux / Unix).
    ofPixelFormat pixFormat;
#ifdef TARGET_LITTLE_ENDIAN
    if (channels == 3)
        pixFormat = OF_PIXELS_BGR;
    if (channels == 4)
        pixFormat = OF_PIXELS_BGRA;
#elif
    if (channels == 3)
        pixFormat = OF_PIXELS_RGB;
    if (channels == 4)
        pixFormat = OF_PIXELS_RGBA;
#endif

    if (bmpBits != NULL) {
        pix.setFromAlignedPixels(bmpBits, frame.width, frame.height, pixFormat, pitch);
        ofLogVerbose() << "Decoding frame of pixel format: " << getPixelFormatString(pix);

#ifdef TARGET_LITTLE_ENDIAN
        if (channels >= 3) {
            pix.swapRgb();
        }
#endif

        if (frames.size() == 0) {
            frame.pixels = pix;
            if (bUseTexture) {
                frame.tex.loadData(pix);
            }
            accumPx = pix; // 1st frame is fully drawn
        } else {
            // add new pixels to accumPx
            unsigned int cropOriginX = frame.left;
            unsigned int cropOriginY = frame.top;

            for (unsigned int i = 0; i < accumPx.getWidth() * accumPx.getHeight(); i++) {
                unsigned int x = i % accumPx.getWidth();
                unsigned int y = i / accumPx.getWidth();

                if ((x >= frame.left) && (x < frame.left + pix.getWidth()) && (y >= frame.top) && (y < frame.top + pix.getHeight())) {
                    unsigned int cropX = x - cropOriginX;
                    unsigned int cropY = y - cropOriginY;

                    unsigned int alpha = pix.getColor(cropX, cropY).a;
                    if (alpha < 255) {
                        switch (frame.disposal) {
                        case GIF_DISPOSAL_BACKGROUND:
                            accumPx.setColor(x, y, backgroundColour);
                            break;

                        case GIF_DISPOSAL_LEAVE: // leave pixels as per previous frame
                        case GIF_DISPOSAL_UNSPECIFIED:
                            break;

                        case GIF_DISPOSAL_PREVIOUS:
                            ofLogError() << "GIF_DISPOSAL_PREVIOUS not yet implemented";
                            break;
                        }
                    } else {
                        accumPx.setColor(x, y, pix.getColor(cropX, cropY));
                    }
                }
            }
            frame.pixels = accumPx;
            if (bUseTexture) {
                frame.tex.loadData(accumPx);
            }
        }

        frames.push_back(frame);

        // FreeImage_ConvertTo32Bits() calls FreeImage_Allocate()
        if (bmpConverted != nullptr) {
            FreeImage_Unload(bmp);
        }

    } else {
        ofLogError("ofxGifImage") << "decodeFrame() unable to get frame bits";
    }
}

//-----------------------------------------------------------------------
void ofxGifImage::encodeFrame(GifFrame& frame, FIMULTIBITMAP* multi, unsigned int pageNum)
{
    FIBITMAP* bmp = NULL;
    FITAG* tag;
    FIBITMAP* quantizedBmp = nullptr;
    FIBITMAP* ditheredBmp = nullptr;
    FIBITMAP* processedBmp = nullptr;

    ofLogVerbose() << "Encoding frame: " << pageNum << "-----------------------------------------------------";
    ofLogVerbose() << "Pixel format: " << getPixelFormatString(frame.pixels);

#ifdef TARGET_LITTLE_ENDIAN
    if ((frame.pixels.getPixelFormat() == OF_PIXELS_RGBA) || (frame.pixels.getPixelFormat() == OF_PIXELS_RGB)) {
        frame.pixels.swapRgb();
    }
#endif

    // force RGB - GIFs can be transparent but don't have a regular alpha channelz
    frame.pixels.setNumChannels(3);
    frame.bpp = frame.pixels.getBitsPerPixel();

    // get the pixel data
    bmp = FreeImage_ConvertFromRawBits(frame.pixels.getData(), frame.width, frame.height, frame.width * (frame.bpp / 8), frame.bpp, 0, 0, 0, true);

    int paletteSize = numColours;
    if (bSetTransparencyOptimisation) {
        if (paletteSize == 256) {
            paletteSize = 255; //reserve last index for transparency
        }
    }

    if (pageNum == 0) {
        quantizedBmp = FreeImage_ColorQuantizeEx(bmp, FIQ_NNQUANT, paletteSize, 0, nullptr);
    } else {
        quantizedBmp = FreeImage_ColorQuantizeEx(bmp, FIQ_NNQUANT, paletteSize, paletteSize, globalPalette);
    }

    if (bSetTransparencyOptimisation) {
        // make redundant pixels transparent - doesn't seem to create smaller file sizes unfortunately
        calculateTransparencyOptimisation(quantizedBmp, pageNum);
    }

    processedBmp = quantizedBmp;

    ofLogVerbose() << "Is transparent? " << FreeImage_IsTransparent(processedBmp);
    ofLogVerbose() << "Transparency count: " << FreeImage_GetTransparencyCount(processedBmp);
    ofLogVerbose() << "Transparent index: " << FreeImage_GetTransparentIndex(processedBmp);

    if (ditherMode > OFX_GIF_DITHER_NONE) {
        ditheredBmp = FreeImage_Dither(processedBmp, (FREE_IMAGE_DITHER)ditherMode);
        processedBmp = ditheredBmp;
    }

    uint32_t frameDuration = (uint32_t)(frame.duration * 1000.f);
    ofLogVerbose() << "Set frame duratiom: " << frameDuration;

    // clear animation metadata
    FreeImage_SetMetadata(FIMD_ANIMATION, processedBmp, NULL, NULL);

    tag = FreeImage_CreateTag();
    if (tag) {
        if (pageNum == 0) {
            // set the global palette
            RGBQUAD* palette = FreeImage_GetPalette(processedBmp);
            FreeImage_SetTagKey(tag, "GlobalPalette");
            FreeImage_SetTagType(tag, FIDT_PALETTE);
            FreeImage_SetTagCount(tag, numColours);
            FreeImage_SetTagLength(tag, numColours * sizeof(RGBQUAD));
            FreeImage_SetTagValue(tag, palette);
            FreeImage_SetMetadata(FIMD_ANIMATION, processedBmp, FreeImage_GetTagKey(tag), tag);

            globalPalette = new RGBQUAD[paletteSize];
            memcpy(globalPalette, palette, paletteSize * sizeof(RGBQUAD));
            ofLogVerbose() << "Global Palette: " << globalPalette << " allocated";
        }
        // add animation tags
        FreeImage_SetTagKey(tag, "FrameTime");
        FreeImage_SetTagType(tag, FIDT_LONG);
        FreeImage_SetTagCount(tag, 1);
        FreeImage_SetTagLength(tag, 4);
        FreeImage_SetTagValue(tag, &frameDuration);
        FreeImage_SetMetadata(FIMD_ANIMATION, processedBmp, FreeImage_GetTagKey(tag), tag);

        // set the frame disposal method
        uint8_t disposal = frame.disposal;
        FreeImage_SetTagKey(tag, "DisposalMethod");
        FreeImage_SetTagType(tag, FIDT_BYTE);
        FreeImage_SetTagCount(tag, 1);
        FreeImage_SetTagLength(tag, 1);
        FreeImage_SetTagValue(tag, &disposal);
        FreeImage_SetMetadata(FIMD_ANIMATION, processedBmp, FreeImage_GetTagKey(tag), tag);

        // anything other than 0 will disable the local palette and use the global palette instead
        uint8_t noLocalPalette = 1;
        FreeImage_SetTagKey(tag, "NoLocalPalette");
        FreeImage_SetTagType(tag, FIDT_BYTE);
        FreeImage_SetTagCount(tag, 1);
        FreeImage_SetTagLength(tag, 1);
        FreeImage_SetTagValue(tag, &noLocalPalette);
        FreeImage_SetMetadata(FIMD_ANIMATION, processedBmp, FreeImage_GetTagKey(tag), tag);

        FreeImage_DeleteTag(tag);
    }

    FreeImage_AppendPage(multi, processedBmp);

    // clear freeimage stuff
    if (bmp != nullptr)
        FreeImage_Unload(bmp);
    if (quantizedBmp != nullptr)
        FreeImage_Unload(quantizedBmp);
    if (ditheredBmp != nullptr)
        FreeImage_Unload(ditheredBmp);

    // no need to unload processedBmp, as it points to either of the above
}

void ofxGifImage::calculateTransparencyOptimisation(FIBITMAP* quantizedBmp, unsigned int pageNum)
{
    if (previousBmp == nullptr) {
        ofLogError("ofxGifImage") << "previousBmp invalid, aborting transparency optimisation";
        return;
    }

    // make identical pixels in last frame transparent
    if (FreeImage_GetBPP(quantizedBmp) == 8) {
        for (unsigned int y = 0; y < FreeImage_GetHeight(quantizedBmp); y++) {
            BYTE* bits = (BYTE*)FreeImage_GetScanLine(quantizedBmp, y);
            BYTE* bits2 = (BYTE*)FreeImage_GetScanLine(previousBmp, y);
            for (unsigned int x = 0; x < FreeImage_GetWidth(quantizedBmp); x++) {
                if (bits[x] == bits2[x]) {
                    bits[x] = 255;
                }
            }
        }
    }

    if (previousBmp) {
        FreeImage_Unload(previousBmp);
        previousBmp = nullptr;
    }
    if (quantizedBmp) {
        previousBmp = FreeImage_Clone(quantizedBmp);
    }

    // enable transparency in palette
    if (pageNum > 0)
    {
        RGBQUAD* Palette = FreeImage_GetPalette(quantizedBmp);

        Palette[255].rgbRed = 0;
        Palette[255].rgbGreen = 0;
        Palette[255].rgbBlue = 0;

        FreeImage_SetTransparentIndex(quantizedBmp, 255);
    }
}

//-----------------------------------------------------------------------
string ofxGifImage::getPixelFormatString(ofPixels p)
{
    ofPixelFormat pixformat = p.getPixelFormat();
    string format = "";
    switch (pixformat) {
    case 2:
        format = "OF_PIXELS_RGB";
        break;
    case 3:
        format = "OF_PIXELS_BGR";
        break;
    case 4:
        format = "OF_PIXELS_RGBA";
        break;
    case 5:
        format = "OF_PIXELS_BGRA";
        break;
    default:
        break;
    }
    return format;
}

//-----------------------------------------------------------------------
ofColor ofxGifImage::getGlobalPalette(unsigned int index)
{
    ofColor c;

    if (globalPalette) {
        c.set(globalPalette[index].rgbRed, globalPalette[index].rgbGreen, globalPalette[index].rgbBlue);
    } else {
        ofLogError() << "Global palette not allocated.";
    }

    return c;
}

//-----------------------------------------------------------------------
unsigned int ofxGifImage::getFrameIndex()
{
    return frameIndex;
}

//-----------------------------------------------------------------------
float ofxGifImage::getWidth() const
{
    return width;
}

//-----------------------------------------------------------------------
float ofxGifImage::getHeight() const
{
    return height;
}

//-----------------------------------------------------------------------
ofPixels& ofxGifImage::getPixels()
{
    return frames.at(frameIndex).pixels;
}

//-----------------------------------------------------------------------
const ofPixels& ofxGifImage::getPixels() const
{
    return frames.at(frameIndex).pixels;
}

//-----------------------------------------------------------------------
ofTexture& ofxGifImage::getTexture()
{
    return frames.at(frameIndex).tex;
}

//-----------------------------------------------------------------------
const ofTexture& ofxGifImage::getTexture() const
{
    return frames.at(frameIndex).tex;
}

//-----------------------------------------------------------------------
void ofxGifImage::setUseTexture(bool bUseTex)
{
    bUseTexture = bUseTex;
}

//-----------------------------------------------------------------------
bool ofxGifImage::isUsingTexture() const
{
    return bUseTexture;
}

//-----------------------------------------------------------------------
void ofxGifImage::draw(float x, float y, float w, float h) const
{
    draw(x, y, w, h);
}
