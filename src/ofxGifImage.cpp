#include "ofxGifImage.h"

#define OFX_GIF_DEFAULT_FRAME_DELAY 0.1f
#define SAVE_TO_CUSTOM_FOLDER 1

//-----------------------------------------------------------------------
ofxGifImage::ofxGifImage()
{
    numColours = 256;
    ditherMode = OFX_GIF_DITHER_NONE;
    defaultFrameDuration = OFX_GIF_DEFAULT_FRAME_DELAY;
    bSetTransparencyOptimisation = false;
#ifdef SAVE_TO_CUSTOM_FOLDER
    customFolder = "~/Pictures/";
#endif
    clear();
    previousBmp = nullptr;
}

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
        ofLogNotice() << "Gif loaded: " << frameCount << " frames.";

        // here we process the first frame
        for (int i = 0; i < frameCount; i++) {
            FIBITMAP* bmp = FreeImage_LockPage(multiBmp, i);
            if (bmp) {
                if (i == 0) {
                    getMetadata(bmp);
                    bLoaded = true;
                }
                ofLogVerbose() << "Decoding frame: " << i << " -------------------------------------------";
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
void ofxGifImage::save(string filename, bool bAbsolutePath)
{
    string savefilename;
    if (bAbsolutePath) {
        savefilename = filename;
    } else {
        savefilename = ofToDataPath(filename);
    }

    // create a multipage bitmap
    FIMULTIBITMAP* multi = FreeImage_OpenMultiBitmap(FIF_GIF, savefilename.c_str(), TRUE, FALSE);
    for (unsigned int i = 0; i < frames.size(); i++) {
        GifFrame currentFrame = frames[i];
        encodeFrame(currentFrame, multi, i);
    }
    if (previousBmp) {
        FreeImage_Unload(previousBmp);
        previousBmp = nullptr;
    }

    FreeImage_CloseMultiBitmap(multi);
    ofLogNotice() << "Gif saved: " << frames.size() << " frames.";
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
    frame.tex.loadData(img.getPixels());

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
    frame.tex.loadData(pixels);

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
        ofLogWarning() << "ofxGifFile::No frames to draw!";
        return;
    }

    updateFrameIndex();
    drawFrame(frameIndex, x, y, width, height);
}

//-----------------------------------------------------------------------
void ofxGifImage::draw(float x, float y, float w, float h)
{
    if (frames.size() == 0) {
        ofLogWarning() << "ofxGifFile::No frames to draw!";
        return;
    }

    updateFrameIndex();
    drawFrame(frameIndex, x, y, w, h);
}

//-----------------------------------------------------------------------
void ofxGifImage::drawFrame(int frameNum, float x, float y)
{
    if (frameNum < 0 || frameNum >= frames.size()) {
        ofLog(OF_LOG_WARNING, "ofxGifFile::drawFrame frame out of bounds. not drawing");
        return;
    }
    drawFrame(frameNum, x, y, width, height);
}

//-----------------------------------------------------------------------
void ofxGifImage::drawFrame(int frameNum, float x, float y, int w, int h)
{
    if (frameNum < 0 || frameNum >= frames.size()) {
        ofLog(OF_LOG_WARNING, "ofxGifFile::drawFrame frame out of bounds. not drawing");
        return;
    }
    frames[frameNum].tex.draw(x, y, w, h);
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
void ofxGifImage::clear()
{
    accumPx.clear();
    frames.clear();
    palette.clear();
    frameIndex = 0;
    lastDrawn = 0;
}

//-----------------------------------------------------------------------
void ofxGifImage::setNumColours(int colours)
{
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
            RGBQUAD* globalPalette = (RGBQUAD*)FreeImage_GetTagValue(tag);
            for (int i = 0; i < paletteSize; i++) {
                ofColor c;
                c.set(globalPalette[i].rgbRed, globalPalette[i].rgbGreen, globalPalette[i].rgbBlue);
                palette.push_back(c);
            }
            ofLogVerbose() << "Using Global Palette.";
        }
    } else {
        ofLogVerbose() << "No Global Palette.";
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
    FIBITMAP* bmpConverted = NULL;
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
            frame.tex.loadData(pix);
            accumPx = pix; // we assume 1st frame is fully drawn`
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
            frame.tex.loadData(accumPx);
        }

        frames.push_back(frame);

    } else {
        ofLogError() << "ofImage::putBmpIntoPixels() unable to set ofPixels from FIBITMAP";
    }
}

//-----------------------------------------------------------------------
void ofxGifImage::encodeFrame(GifFrame& frame, FIMULTIBITMAP* multi, unsigned int pageNum)
{
    FIBITMAP* bmp = NULL;

    ofLogVerbose() << "Encoding frame: " << pageNum << "-----------------------------------------------------";

    // get the pixel format
    ofLogVerbose() << "Pixel format: " << getPixelFormatString(frame.pixels);

#ifdef TARGET_LITTLE_ENDIAN
    if ((frame.pixels.getPixelFormat() == OF_PIXELS_RGBA) || (frame.pixels.getPixelFormat() == OF_PIXELS_RGB)) {
        frame.pixels.swapRgb();
    }
#endif

    // force RGB
    frame.pixels.setNumChannels(3);
    frame.bpp = frame.pixels.getBitsPerPixel();

    // get the pixel data
    bmp = FreeImage_ConvertFromRawBits(frame.pixels.getData(), frame.width, frame.height, frame.width * (frame.bpp / 8), frame.bpp, 0, 0, 0, true);
    FIBITMAP* quantizedBmp = nullptr;
    FIBITMAP* ditheredBmp = nullptr;
    FIBITMAP* processedBmp = nullptr;

    int paletteSize = numColours;
    if (paletteSize == 256) {
        paletteSize = 255; //reserve last index for transparency
    }

    if (previousBmp != nullptr) {
        RGBQUAD* Palette = FreeImage_GetPalette(previousBmp);
        quantizedBmp = FreeImage_ColorQuantizeEx(bmp, FIQ_NNQUANT, paletteSize, paletteSize, Palette);
    } else {
        quantizedBmp = FreeImage_ColorQuantizeEx(bmp, FIQ_NNQUANT, paletteSize, 0, nullptr);
    }

    processedBmp = quantizedBmp;

    if (previousBmp != nullptr) {
        // make pixels identical to last frame transparent
        if (bSetTransparencyOptimisation) {
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
        }
    }

    if (previousBmp) {
        FreeImage_Unload(previousBmp);
        previousBmp = nullptr;
    }
    if (quantizedBmp) {
        previousBmp = FreeImage_Clone(quantizedBmp);
    }

    // enable transparency optimisation
    if (bSetTransparencyOptimisation && (pageNum > 0)) {
        RGBQUAD* Palette = FreeImage_GetPalette(processedBmp);

        Palette[255].rgbRed = 0;
        Palette[255].rgbGreen = 0;
        Palette[255].rgbBlue = 0;

        FreeImage_SetTransparentIndex(processedBmp, 255);
    }

    ofLogVerbose() << "Is transparent? " << FreeImage_IsTransparent(processedBmp);
    ofLogVerbose() << "Transparency count: " << FreeImage_GetTransparencyCount(processedBmp);
    ofLogVerbose() << "Transparent index: " << FreeImage_GetTransparentIndex(processedBmp);
    ofLogVerbose() << "Get Transparent index: " << FreeImage_GetTransparentIndex(processedBmp);

    if (ditherMode > OFX_GIF_DITHER_NONE) {
        ditheredBmp = FreeImage_Dither(processedBmp, (FREE_IMAGE_DITHER)ditherMode);
        processedBmp = ditheredBmp;
    }

    uint32_t frameDuration = (uint32_t)(frame.duration * 1000.f);
    ofLogVerbose() << "Set frame duratiom: " << frameDuration;

    // clear animation metadata
    FreeImage_SetMetadata(FIMD_ANIMATION, processedBmp, NULL, NULL);

    FITAG* tag = FreeImage_CreateTag();
    if (tag) {
        if (pageNum == 0) {
            // set the global palette
            RGBQUAD* Palette = FreeImage_GetPalette(processedBmp);
            FreeImage_SetTagKey(tag, "GlobalPalette");
            FreeImage_SetTagType(tag, FIDT_PALETTE);
            FreeImage_SetTagCount(tag, numColours);
            FreeImage_SetTagLength(tag, numColours * sizeof(RGBQUAD));
            FreeImage_SetTagValue(tag, Palette);
            FreeImage_SetMetadata(FIMD_ANIMATION, processedBmp, FreeImage_GetTagKey(tag), tag);
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
