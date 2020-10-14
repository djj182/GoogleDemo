/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2015 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __CC_IMAGE_H__
#define __CC_IMAGE_H__
/// @cond DO_NOT_SHOW

#include "base/CCRef.h"
#include "renderer/CCTexture2D.h"

#if defined(CC_USE_WIC)
#include "WICImageLoader-winrt.h"
#endif

// premultiply alpha, or the effect will wrong when want to use other pixel format in Texture2D,
// such as RGB888, RGB5A1
#define CC_RGB_PREMULTIPLY_ALPHA(vr, vg, vb, va) \
    (unsigned)(((unsigned)((unsigned char)(vr) * ((unsigned char)(va) + 1)) >> 8) | \
    ((unsigned)((unsigned char)(vg) * ((unsigned char)(va) + 1) >> 8) << 8) | \
    ((unsigned)((unsigned char)(vb) * ((unsigned char)(va) + 1) >> 8) << 16) | \
    ((unsigned)(unsigned char)(va) << 24))

NS_CC_BEGIN

/**
 * @addtogroup platform
 * @{
 */

/**
 @brief Structure which can tell where mipmap begins and how long is it
 */
typedef struct _MipmapInfo
{
    unsigned char* address;
    int len;
    _MipmapInfo():address(NULL),len(0){}
}MipmapInfo;

class CC_DLL Image : public Ref
{
public:
    friend class TextureCache;
    /**
     * @js ctor
     */
    Image();
    /**
     * @js NA
     * @lua NA
     */
    virtual ~Image();

    /** Supported formats for Image */
    enum class Format
    {
        //! JPEG
        JPG,
        //! PNG
        PNG,
        //! TIFF
        TIFF,
        //! WebP
        WEBP,
        //! PVR
        PVR,
        //! ETC
        ETC,
        //! S3TC
        S3TC,
        //! ATITC
        ATITC,
        //! TGA
        TGA,
        //! Raw Data
        RAW_DATA,
        //! Wrz
        WRZ,
        //! Cwrz
        CWRZ,
        //! Unknown format
        UNKNOWN
    };

    /**
    @brief Load the image from the specified path.
    @param path   the absolute file path.
    @return true if loaded correctly.
    */
    bool initWithImageFile(const std::string& path);
    bool initWithETCDataBySoftUncompressed(const std::string& path, const Rect& rect=Rect::ZERO,bool isRotate = false); // PKM文件中的图片可能是旋转过的
    bool initWithPVRv2Data(const std::string& path, const Rect& rect=Rect::ZERO,bool isRotate = false);


    /**
    @brief Load image from stream buffer.
    @param data  stream buffer which holds the image data.
    @param dataLen  data length expressed in (number of) bytes.
    @return true if loaded correctly.
    * @js NA
    * @lua NA
    */
    bool initWithImageData(const unsigned char * data, ssize_t dataLen);

    // @warning kFmtRawData only support RGBA8888
    bool initWithRawData(const unsigned char * data, ssize_t dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);

    // Getters
    inline unsigned char *   getData()               { return _data; }
    inline unsigned char *   getTotaldata()               { return _totaldata; }
    inline ssize_t           getDataLen()            { return _dataLen; }
    inline ssize_t           getTotalDataLen()            { return _totalDataLen; }
    inline Format            getFileType()           {return _fileType; }
    inline Texture2D::PixelFormat getRenderFormat()  { return _renderFormat; }
    inline int               getWidth()              { return _width; }
    inline int               getHeight()             { return _height; }
    inline int               getNumberOfMipmaps()    { return _numberOfMipmaps; }
    inline MipmapInfo*       getMipmaps()            { return _mipmaps; }
    inline bool              hasPremultipliedAlpha() { return _hasPremultipliedAlpha; }
    CC_DEPRECATED_ATTRIBUTE inline bool isPremultipliedAlpha()  { return _hasPremultipliedAlpha;   }


    // Added by ChenFei for supporting ETC,PKM with alpha texture
    inline std::string       getFilePath()           { return _filePath; }


    int                      getBitPerPixel();
    bool                     hasAlpha();
    bool                     isCompressed();


    /**
     @brief    Save Image data to the specified file, with specified format.
     @param    filePath        the file's absolute path, including file suffix.
     @param    isToRGB        whether the image is saved as RGB format.
     */
    bool saveToFile(const std::string &filename, bool isToRGB = true);
    
    
    /** treats (or not) PVR files as if they have alpha premultiplied.
     Since it is impossible to know at runtime if the PVR images have the alpha channel premultiplied, it is
     possible load them as if they have (or not) the alpha channel premultiplied.
     
     By default it is disabled.
     */
    static void setPVRImagesHavePremultipliedAlpha(bool haveAlphaPremultiplied);
    

    inline void setSubImage( Image* pImage ) { _pSubImage = pImage; }
    inline Image* getSubImage() { return _pSubImage; }
    
    inline void setAlphaImage(Image* img) {
        _alphaImage = img;
    }
    
    inline Image* getAlphaImage() {
        return _alphaImage;
    }
    
    /*
     @brief The same result as with initWithImageFile, but thread safe. It is caused by
     loadImage() in TextureCache.cpp.
     @param fullpath  full path of the file.
     @param imageType the type of image, currently only supporting two types.
     @return  true if loaded correctly.
     */
    bool initWithImageFileThreadSafe(const std::string& fullpath);
    
    bool saveImageToWrz(const std::string& filePath);
    void premultipliedAlpha();
protected:
#if defined(CC_USE_WIC)
	bool encodeWithWIC(const std::string& filePath, bool isToRGB, GUID containerFormat);
	bool decodeWithWIC(const unsigned char *data, ssize_t dataLen);
#endif
    bool initWithJpgData(const unsigned char *  data, ssize_t dataLen);
    bool initWithPngData(const unsigned char * data, ssize_t dataLen);
    bool initWithTiffData(const unsigned char * data, ssize_t dataLen);
    bool initWithWebpData(const unsigned char * data, ssize_t dataLen);
    bool initWithPVRData(const unsigned char * data, ssize_t dataLen);
    bool initWithPVRv2Data(const unsigned char * data, ssize_t dataLen);
    bool initWithPVRv3Data(const unsigned char * data, ssize_t dataLen);
    bool initWithETCData(const unsigned char * data, ssize_t dataLen);
    bool initWithS3TCData(const unsigned char * data, ssize_t dataLen);
    bool initWithATITCData(const unsigned char *data, ssize_t dataLen);
    bool initWithWrzData(const unsigned char * data, ssize_t dataLen);
    bool initWithCwrzData(const unsigned char * data, ssize_t dataLen);
    typedef struct sImageTGA tImageTGA;
    bool initWithTGAData(tImageTGA* tgaData);

    bool saveImageToPNG(const std::string& filePath, bool isToRGB = true);
    bool saveImageToPNGEx(const std::string& filePath, bool isToRGB = false);
    bool saveImageToJPG(const std::string& filePath);
    
    

protected:
    /**
     @brief Determine how many mipmaps can we have.
     Its same as define but it respects namespaces
     */
    static const int MIPMAP_MAX = 16;
    unsigned char *_data;
     unsigned char *_totaldata;
    unsigned char *_tmpBufferdata;
    ssize_t _tmpBufferdataLen;
    ssize_t _totalDataLen;
    ssize_t _dataLen;
    int _width;
    int _height;
    bool _unpack;
    Format _fileType;
    Texture2D::PixelFormat _renderFormat;
    MipmapInfo _mipmaps[MIPMAP_MAX];   // pointer to mipmap images
    int _numberOfMipmaps;
    // false if we cann't auto detect the image is premultiplied or not.
    bool _hasPremultipliedAlpha;
    std::string _filePath;

    Image* _pSubImage;
    Image* _alphaImage;

protected:
    // noncopyable
    Image(const Image&    rImg);
    Image & operator=(const Image&);
    
    Format detectFormat(const unsigned char * data, ssize_t dataLen);
    bool isPng(const unsigned char * data, ssize_t dataLen);
    bool isJpg(const unsigned char * data, ssize_t dataLen);
    bool isTiff(const unsigned char * data, ssize_t dataLen);
    bool isWebp(const unsigned char * data, ssize_t dataLen);
    bool isPvr(const unsigned char * data, ssize_t dataLen);
    bool isEtc(const unsigned char * data, ssize_t dataLen);
    bool isS3TC(const unsigned char * data,ssize_t dataLen);
    bool isATITC(const unsigned char *data, ssize_t dataLen);
    bool isWrz(const unsigned char * data, ssize_t dataLen);
    bool isCwrz(const unsigned char * data, ssize_t dataLen);
};

// end of platform group
/// @}

NS_CC_END

/// @endcond
#endif    // __CC_IMAGE_H__
