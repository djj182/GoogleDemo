/****************************************************************************
Copyright (c) 2008      Apple Inc. All Rights Reserved.
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies Inc.

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



/*
* Support for RGBA_4_4_4_4 and RGBA_5_5_5_1 was copied from:
* https://devforums.apple.com/message/37855#37855 by a1studmuffin
*/

#include "renderer/CCTexture2D.h"

#include "platform/CCGL.h"
#include "platform/CCImage.h"
#include "base/ccUtils.h"
#include "platform/CCDevice.h"
#include "base/ccConfig.h"
#include "base/ccMacros.h"
#include "base/CCConfiguration.h"
#include "platform/CCPlatformMacros.h"
#include "base/CCDirector.h"
#include "renderer/CCGLProgram.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCGLProgramCache.h"
#include "base/CCNinePatchImageParser.h"
#include "deprecated/CCString.h"
#include "CCFileUtils.h"

//#include "base/unchecked.h"
//#include "vector"
#if CC_ENABLE_CACHE_TEXTURE_DATA
    #include "renderer/CCTextureCache.h"
#endif

#define ENABLE_SCALE_TEXT_RENDER 1

//修复安卓上RTL语言方向对齐错误
extern bool g_fix_ar_direction;

NS_CC_BEGIN



namespace {
    typedef Texture2D::PixelFormatInfoMap::value_type PixelFormatInfoMapValue;
    static const PixelFormatInfoMapValue TexturePixelFormatInfoTablesValue[] =
    {
        PixelFormatInfoMapValue(Texture2D::PixelFormat::BGRA8888, Texture2D::PixelFormatInfo(GL_BGRA, GL_BGRA, GL_UNSIGNED_BYTE, 32, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGBA8888, Texture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 32, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGBA4444, Texture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 16, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGB5A1, Texture2D::PixelFormatInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 16, false, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGB565, Texture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 16, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::RGB888, Texture2D::PixelFormatInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 24, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::A8, Texture2D::PixelFormatInfo(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 8, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::I8, Texture2D::PixelFormatInfo(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 8, false, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::AI88, Texture2D::PixelFormatInfo(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 16, false, true)),
        
#ifdef GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC2, Texture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC2A, Texture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 2, true, true)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC4, Texture2D::PixelFormatInfo(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
        PixelFormatInfoMapValue(Texture2D::PixelFormat::PVRTC4A, Texture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, true)),
#endif
        
#ifdef GL_ETC1_RGB8_OES
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ETC, Texture2D::PixelFormatInfo(GL_ETC1_RGB8_OES, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif
        
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
        PixelFormatInfoMapValue(Texture2D::PixelFormat::S3TC_DXT1, Texture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif
        
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
        PixelFormatInfoMapValue(Texture2D::PixelFormat::S3TC_DXT3, Texture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
        
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
        PixelFormatInfoMapValue(Texture2D::PixelFormat::S3TC_DXT5, Texture2D::PixelFormatInfo(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
        
#ifdef GL_ATC_RGB_AMD
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ATC_RGB, Texture2D::PixelFormatInfo(GL_ATC_RGB_AMD,
            0xFFFFFFFF, 0xFFFFFFFF, 4, true, false)),
#endif
        
#ifdef GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ATC_EXPLICIT_ALPHA, Texture2D::PixelFormatInfo(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD,
            0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
        
#ifdef GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD
        PixelFormatInfoMapValue(Texture2D::PixelFormat::ATC_INTERPOLATED_ALPHA, Texture2D::PixelFormatInfo(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD,
            0xFFFFFFFF, 0xFFFFFFFF, 8, true, false)),
#endif
    };
}

//CLASS IMPLEMENTATIONS:

//The PixpelFormat corresponding information
const Texture2D::PixelFormatInfoMap Texture2D::_pixelFormatInfoTables(TexturePixelFormatInfoTablesValue,
                                                                     TexturePixelFormatInfoTablesValue + sizeof(TexturePixelFormatInfoTablesValue) / sizeof(TexturePixelFormatInfoTablesValue[0]));

// If the image has alpha, you can create RGBA8 (32-bit) or RGBA4 (16-bit) or RGB5A1 (16-bit)
// Default is: RGBA8888 (32-bit textures)
static Texture2D::PixelFormat g_defaultAlphaPixelFormat = Texture2D::PixelFormat::DEFAULT;

//////////////////////////////////////////////////////////////////////////
//convertor function

// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBB
void Texture2D::convertI8ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i=0; i < dataLen; ++i)
    {
        *outData++ = data[i];     //R
        *outData++ = data[i];     //G
        *outData++ = data[i];     //B
    }
}

// IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
void Texture2D::convertAI88ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
    {
        *outData++ = data[i];     //R
        *outData++ = data[i];     //G
        *outData++ = data[i];     //B
    }
}

// IIIIIIII -> RRRRRRRRGGGGGGGGGBBBBBBBBAAAAAAAA
void Texture2D::convertI8ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0; i < dataLen; ++i)
    {
        *outData++ = data[i];     //R
        *outData++ = data[i];     //G
        *outData++ = data[i];     //B
        *outData++ = 0xFF;        //A
    }
}

// IIIIIIIIAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
void Texture2D::convertAI88ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
    {
        *outData++ = data[i];     //R
        *outData++ = data[i];     //G
        *outData++ = data[i];     //B
        *outData++ = data[i + 1]; //A
    }
}

// IIIIIIII -> RRRRRGGGGGGBBBBB
void Texture2D::convertI8ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (int i = 0; i < dataLen; ++i)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00FC) << 3         //G
            | (data[i] & 0x00F8) >> 3;        //B
    }
}

// IIIIIIIIAAAAAAAA -> RRRRRGGGGGGBBBBB
void Texture2D::convertAI88ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00FC) << 3         //G
            | (data[i] & 0x00F8) >> 3;        //B
    }
}

// IIIIIIII -> RRRRGGGGBBBBAAAA
void Texture2D::convertI8ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0; i < dataLen; ++i)
    {
        *out16++ = (data[i] & 0x00F0) << 8    //R
        | (data[i] & 0x00F0) << 4             //G
        | (data[i] & 0x00F0)                  //B
        | 0x000F;                             //A
    }
}

// IIIIIIIIAAAAAAAA -> RRRRGGGGBBBBAAAA
void Texture2D::convertAI88ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
    {
        *out16++ = (data[i] & 0x00F0) << 8    //R
        | (data[i] & 0x00F0) << 4             //G
        | (data[i] & 0x00F0)                  //B
        | (data[i+1] & 0x00F0) >> 4;          //A
    }
}

// IIIIIIII -> RRRRRGGGGGBBBBBA
void Texture2D::convertI8ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (int i = 0; i < dataLen; ++i)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00F8) << 3         //G
            | (data[i] & 0x00F8) >> 2         //B
            | 0x0001;                         //A
    }
}

// IIIIIIIIAAAAAAAA -> RRRRRGGGGGBBBBBA
void Texture2D::convertAI88ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i] & 0x00F8) << 3         //G
            | (data[i] & 0x00F8) >> 2         //B
            | (data[i + 1] & 0x0080) >> 7;    //A
    }
}

// IIIIIIII -> IIIIIIIIAAAAAAAA
void Texture2D::convertI8ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0; i < dataLen; ++i)
    {
        *out16++ = 0xFF00     //A
        | data[i];            //I
    }
}

// IIIIIIIIAAAAAAAA -> AAAAAAAA
void Texture2D::convertAI88ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 1; i < dataLen; i += 2)
    {
        *outData++ = data[i]; //A
    }
}

// IIIIIIIIAAAAAAAA -> IIIIIIII
void Texture2D::convertAI88ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 1; i < l; i += 2)
    {
        *outData++ = data[i]; //R
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA
void Texture2D::convertRGB888ToRGBA8888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
    {
        *outData++ = data[i];         //R
        *outData++ = data[i + 1];     //G
        *outData++ = data[i + 2];     //B
        *outData++ = 0xFF;            //A
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRRRRGGGGGGGGBBBBBBBB
void Texture2D::convertRGBA8888ToRGB888(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
    {
        *outData++ = data[i];         //R
        *outData++ = data[i + 1];     //G
        *outData++ = data[i + 2];     //B
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGGBBBBB
void Texture2D::convertRGB888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00FC) << 3     //G
            | (data[i + 2] & 0x00F8) >> 3;    //B
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRRGGGGGGBBBBB
void Texture2D::convertRGBA8888ToRGB565(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00FC) << 3     //G
            | (data[i + 2] & 0x00F8) >> 3;    //B
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> AAAAAAAA
void Texture2D::convertRGB888ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
    {
        *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //A =  (R*299 + G*587 + B*114 + 500) / 1000
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIII
void Texture2D::convertRGB888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
    {
        *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIII
void Texture2D::convertRGBA8888ToI8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
    {
        *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> AAAAAAAA
void Texture2D::convertRGBA8888ToA8(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen -3; i < l; i += 4)
    {
        *outData++ = data[i + 3]; //A
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> IIIIIIIIAAAAAAAA
void Texture2D::convertRGB888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
    {
        *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        *outData++ = 0xFF;
    }
}


// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> IIIIIIIIAAAAAAAA
void Texture2D::convertRGBA8888ToAI88(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
    {
        *outData++ = (data[i] * 299 + data[i + 1] * 587 + data[i + 2] * 114 + 500) / 1000;  //I =  (R*299 + G*587 + B*114 + 500) / 1000
        *outData++ = data[i + 3];
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRGGGGBBBBAAAA
void Texture2D::convertRGB888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
    {
        *out16++ = ((data[i] & 0x00F0) << 8           //R
                    | (data[i + 1] & 0x00F0) << 4     //G
                    | (data[i + 2] & 0xF0)            //B
                    |  0x0F);                         //A
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA -> RRRRGGGGBBBBAAAA
void Texture2D::convertRGBA8888ToRGBA4444(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 3; i < l; i += 4)
    {
        *out16++ = (data[i] & 0x00F0) << 8    //R
        | (data[i + 1] & 0x00F0) << 4         //G
        | (data[i + 2] & 0xF0)                //B
        |  (data[i + 3] & 0xF0) >> 4;         //A
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
void Texture2D::convertRGB888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 3)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00F8) << 3     //G
            | (data[i + 2] & 0x00F8) >> 2     //B
            |  0x01;                          //A
    }
}

// RRRRRRRRGGGGGGGGBBBBBBBB -> RRRRRGGGGGBBBBBA
void Texture2D::convertRGBA8888ToRGB5A1(const unsigned char* data, ssize_t dataLen, unsigned char* outData)
{
    unsigned short* out16 = (unsigned short*)outData;
    for (ssize_t i = 0, l = dataLen - 2; i < l; i += 4)
    {
        *out16++ = (data[i] & 0x00F8) << 8    //R
            | (data[i + 1] & 0x00F8) << 3     //G
            | (data[i + 2] & 0x00F8) >> 2     //B
            |  (data[i + 3] & 0x0080) >> 7;   //A
    }
}
// conventer function end
//////////////////////////////////////////////////////////////////////////


Texture2D::Texture2D()
: _pixelFormat(Texture2D::PixelFormat::DEFAULT)
, _pixelsWide(0)
, _pixelsHigh(0)
, _name(0)
, m_alphaName(0) // Added by ChenFei 2014-12-15 V3.2 support mod by liudi as do not need
, _maxS(0.0)
, _maxT(0.0)
, _hasPremultipliedAlpha(false)
, _hasMipmaps(false)
, _shaderProgram(nullptr)
, _antialiasEnabled(true)
, m_fileName("")
, _ninePatchInfo(nullptr)
{
// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    increaseInstanceCount(true);
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
}

Texture2D::~Texture2D()
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
    VolatileTextureMgr::removeTexture(this);
#endif

    // CCLOG("Texture2D::~Texture2D: %p - id=%u, alpha=%u", this, _name, m_alphaName);
    CC_SAFE_RELEASE(_shaderProgram);

    CC_SAFE_DELETE(_ninePatchInfo);
    if(_name)
    {
        GL::deleteTexture(_name);
    }
    
    // Added by ChenFei 2014-12-15 V3.2 support
    if(m_alphaName)
    {
        GL::deleteTexture(m_alphaName);
    }
// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    increaseInstanceCount(false);
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
}

void Texture2D::releaseGLTexture()
{
    if(_name)
    {
        GL::deleteTexture(_name);
    }
    _name = 0;
    // Added by ChenFei 2014-12-15 V3.2 support
    if(m_alphaName)
    {
        GL::deleteTexture(m_alphaName);
    }
    m_alphaName = 0;
}


Texture2D::PixelFormat Texture2D::getPixelFormat() const
{
    return _pixelFormat;
}

int Texture2D::getPixelsWide() const
{
    return _pixelsWide;
}

int Texture2D::getPixelsHigh() const
{
    return _pixelsHigh;
}

GLuint Texture2D::getName() const
{
    return _name;
}

void Texture2D::clearName()
{
    // 防止纹理被删除
    _name = 0;
}
    //comment by liudi not need
// Added by ChenFei 2014-12-16 V3.2 support
GLuint Texture2D::getAlphaName() const
{
    return m_alphaName;
}

void Texture2D::setAlphaName(GLuint value) {
    m_alphaName = value;
}

void Texture2D::setContentSizeInPixel(const Size& contentSizePixel) {
    _contentSize = contentSizePixel;
    _pixelsWide = contentSizePixel.width;
    _pixelsHigh = contentSizePixel.height;
}

Size Texture2D::getContentSize() const
{
    Size ret;
    ret.width = _contentSize.width / CC_CONTENT_SCALE_FACTOR();
    ret.height = _contentSize.height / CC_CONTENT_SCALE_FACTOR();
    
    return ret;
}

const Size& Texture2D::getContentSizeInPixels()
{
    return _contentSize;
}

GLfloat Texture2D::getMaxS() const
{
    return _maxS;
}

void Texture2D::setMaxS(GLfloat maxS)
{
    _maxS = maxS;
}

GLfloat Texture2D::getMaxT() const
{
    return _maxT;
}

void Texture2D::setMaxT(GLfloat maxT)
{
    _maxT = maxT;
}

GLProgram* Texture2D::getGLProgram() const
{
    return _shaderProgram;
}

void Texture2D::setGLProgram(GLProgram* shaderProgram)
{
    CC_SAFE_RETAIN(shaderProgram);
    CC_SAFE_RELEASE(_shaderProgram);
    _shaderProgram = shaderProgram;
}

bool Texture2D::hasPremultipliedAlpha() const
{
    return _hasPremultipliedAlpha;
}

bool Texture2D::initWithData(const void *data, ssize_t dataLen, Texture2D::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const Size& contentSize)
{
    CCASSERT(dataLen>0 && pixelsWide>0 && pixelsHigh>0, "Invalid size");

    //if data has no mipmaps, we will consider it has only one mipmap
    MipmapInfo mipmap;
    mipmap.address = (unsigned char*)data;
    mipmap.len = static_cast<int>(dataLen);
    return initWithMipmaps(&mipmap, 1, pixelFormat, pixelsWide, pixelsHigh);
}

bool Texture2D::initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, PixelFormat pixelFormat, int pixelsWide, int pixelsHigh)
{


    //the pixelFormat must be a certain value 
    CCASSERT(pixelFormat != PixelFormat::NONE && pixelFormat != PixelFormat::AUTO, "the \"pixelFormat\" param must be a certain value!");
    CCASSERT(pixelsWide>0 && pixelsHigh>0, "Invalid size");

    if (mipmapsNum <= 0)
    {
        CCLOG("cocos2d: WARNING: mipmap number is less than 1");
        return false;
    }
    

    if(_pixelFormatInfoTables.find(pixelFormat) == _pixelFormatInfoTables.end())
    {
        CCLOG("cocos2d: WARNING: unsupported pixelformat: %lx", (unsigned long)pixelFormat );
        return false;
    }

    const PixelFormatInfo& info = _pixelFormatInfoTables.at(pixelFormat);

    if (info.compressed && !Configuration::getInstance()->supportsPVRTC()
                        && !Configuration::getInstance()->supportsETC()
                        && !Configuration::getInstance()->supportsS3TC()
                        && !Configuration::getInstance()->supportsATITC())
    {
        CCLOG("cocos2d: WARNING: PVRTC/ETC images are not supported");
        return false;
    }

    //Set the row align only when mipmapsNum == 1 and the data is uncompressed
    if (mipmapsNum == 1 && !info.compressed)
    {
        unsigned int bytesPerRow = pixelsWide * info.bpp / 8;

        if(bytesPerRow % 8 == 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
        }
        else if(bytesPerRow % 4 == 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
        else if(bytesPerRow % 2 == 0)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
        }
        else
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
    }else
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    if(_name != 0)
    {
        GL::deleteTexture(_name);
        _name = 0;
    }
    // Added by ChenFei 2014-12-26 V3.2 support
    if(m_alphaName != 0)
    {
        GL::deleteTexture(m_alphaName);
        m_alphaName = 0;
    }

    glGenTextures(1, &_name);
    GL::bindTexture2D(_name);

    if (mipmapsNum == 1)
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST);
    }else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _antialiasEnabled ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST);
    }
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _antialiasEnabled ? GL_LINEAR : GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

#if CC_ENABLE_CACHE_TEXTURE_DATA
    if (_antialiasEnabled)
    {
        TexParams texParams = {(GLuint)(_hasMipmaps?GL_LINEAR_MIPMAP_NEAREST:GL_LINEAR),GL_LINEAR,GL_NONE,GL_NONE};
        VolatileTextureMgr::setTexParameters(this, texParams);
    } 
    else
    {
        TexParams texParams = {(GLuint)(_hasMipmaps?GL_NEAREST_MIPMAP_NEAREST:GL_NEAREST),GL_NEAREST,GL_NONE,GL_NONE};
        VolatileTextureMgr::setTexParameters(this, texParams);
    }
#endif

    // clean possible GL error
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        CCLOG("OpenGL error 0x%04X in %s %s %d\n", err, __FILE__, __FUNCTION__, __LINE__);
        if( GL_OUT_OF_MEMORY == err || GL_INVALID_VALUE == err)
            return false;
    }
    
    // Specify OpenGL texture image
    int width = pixelsWide;
    int height = pixelsHigh;
    
    for (int i = 0; i < mipmapsNum; ++i)
    {
        unsigned char *data = mipmaps[i].address;
        GLsizei datalen = mipmaps[i].len;

        if (info.compressed)
        {
           
            glCompressedTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, datalen, data);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, i, info.internalFormat, (GLsizei)width, (GLsizei)height, 0, info.format, info.type, data);
        }

        if (i > 0 && (width != height || ccNextPOT(width) != width ))
        {
            CCLOG("cocos2d: Texture2D. WARNING. Mipmap level %u is not squared. Texture won't render correctly. width=%d != height=%d", i, width, height);
        }

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            CCLOG("cocos2d: Texture2D: Error uploading compressed texture level: %u . glError: 0x%04X", i, err);
            return false;
        }

        width = MAX(width >> 1, 1);
        height = MAX(height >> 1, 1);
    }

    _contentSize = Size((float)pixelsWide, (float)pixelsHigh);
    _pixelsWide = pixelsWide;
    _pixelsHigh = pixelsHigh;
    _pixelFormat = pixelFormat;
    _maxS = 1;
    _maxT = 1;

    //_hasPremultipliedAlpha = true;
    _hasPremultipliedAlpha = false;
    _hasMipmaps = mipmapsNum > 1;

    // shader
    setGLProgram(GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_TEXTURE));
    return true;
}

bool Texture2D::updateWithData(const void *data,int offsetX,int offsetY,int width,int height)
{
    if (_name)
    {
        GL::bindTexture2D(_name);
        const PixelFormatInfo& info = _pixelFormatInfoTables.at(_pixelFormat);
        glTexSubImage2D(GL_TEXTURE_2D,0,offsetX,offsetY,width,height,info.format, info.type,data);

        return true;
    }
    return false;
}

std::string Texture2D::getDescription() const
{
    return StringUtils::format("<Texture2D | Name = %u | Dimensions = %ld x %ld | Coordinates = (%.2f, %.2f)>", _name, (long)_pixelsWide, (long)_pixelsHigh, _maxS, _maxT);
}

// implementation Texture2D (Image)
bool Texture2D::initWithImage(Image *image,const std::string & path)
{
    bool ret = false;
    Director::getInstance()->runSyncOnGLThread([this,&ret, &image, &path] {
        ret = initWithImage(image, g_defaultAlphaPixelFormat, path);
    });
    return ret;
}

bool Texture2D::initWithImage(Image *image, PixelFormat format,const std::string & path)
{
    if (image == nullptr)
    {
        CCLOG("cocos2d: Texture2D. Can't create Texture. UIImage is nil");
        return false;
    }
    
    setFileName(path);
    
    int imageWidth = image->getWidth();
    int imageHeight = image->getHeight();
    this->_filePath = image->getFilePath();
    Configuration *conf = Configuration::getInstance();

    int maxTextureSize = conf->getMaxTextureSize();
    if (imageWidth > maxTextureSize || imageHeight > maxTextureSize) 
    {
        CCLOG("cocos2d: WARNING: Image (%u x %u) is bigger than the supported %u x %u", imageWidth, imageHeight, maxTextureSize, maxTextureSize);
        return false;
    }

    unsigned char*   tempData = image->getData();
    Size             imageSize = Size((float)imageWidth, (float)imageHeight);
    PixelFormat      pixelFormat = ((PixelFormat::NONE == format) || (PixelFormat::AUTO == format)) ? image->getRenderFormat() : format;
    PixelFormat      renderFormat = image->getRenderFormat();
    size_t	         tempDataLen = image->getDataLen();


    if (image->getNumberOfMipmaps() > 1)
    {
        if (pixelFormat != image->getRenderFormat())
        {
            CCLOG("cocos2d: WARNING: This image has more than 1 mipmaps and we will not convert the data format");
        }

        initWithMipmaps(image->getMipmaps(), image->getNumberOfMipmaps(), image->getRenderFormat(), imageWidth, imageHeight);
        
        // set the premultiplied tag
        _hasPremultipliedAlpha = image->hasPremultipliedAlpha();
        
        return true;
    }
    else if (image->isCompressed())
    {
        if (pixelFormat != image->getRenderFormat())
        {
//            CCLOG("cocos2d: WARNING: This image is compressed and we cann't convert it for now");
        }

        initWithData(tempData, tempDataLen, image->getRenderFormat(), imageWidth, imageHeight, imageSize);
        
        // set the premultiplied tag
        _hasPremultipliedAlpha = image->hasPremultipliedAlpha();
        
//        CCLOG("Texture2D::initWithImage(:%s", image->getFilePath().c_str());
        // set the premultiplied tag
        _hasPremultipliedAlpha = image->hasPremultipliedAlpha();
        // Added by Chenfei for support ETC, PKM compressed texture
        switch(image->getFileType())
        {
            case Image::Format::ETC:
            case Image::Format::PVR:
            {
                initAlphaTexture(image);
            }
                break;
            default:
                break;
        }
        return true;
    }
    else
    {
        unsigned char* outTempData = nullptr;
        ssize_t outTempDataLen = 0;

        pixelFormat = convertDataToFormat(tempData, tempDataLen, renderFormat, pixelFormat, &outTempData, &outTempDataLen);

        initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);

        // 检查是否要加载通道
        switch(image->getFileType())
        {
            case Image::Format::ETC:
            case Image::Format::PVR:
            {
                initAlphaTexture(image);
                break;
            }
        }
        
        if (outTempData != nullptr && outTempData != tempData)
        {

            free(outTempData);
        }

        // set the premultiplied tag
        _hasPremultipliedAlpha = image->hasPremultipliedAlpha();
        
        return true;
    }
}

int Texture2D::initAlphaTexture(Image* image)
{
    int ret = 0;
    Image* imageAlpha = nullptr;
    Texture2D* textureAlpha = nullptr;
    do
    {
        if ( image->getSubImage() ) {
            
            textureAlpha = new Texture2D();
            CC_BREAK_IF(!textureAlpha->initWithImage(image->getSubImage()));
            this->m_alphaName = textureAlpha->_name;
            textureAlpha->clearName();
        }
        else if (image->getAlphaImage()) {
//            CCLOG("Texture2D::initWithImage. getAlphaImage: %s begin...",
//                  image->getAlphaImage()->getFilePath().c_str());
            
            textureAlpha = new Texture2D();
            CC_BREAK_IF(!textureAlpha->initWithImage(image->getAlphaImage()));
            this->m_alphaName = textureAlpha->_name;
            
//            CCLOG("Texture2D::initWithImage. getAlphaImage: %s ok..name(%d)..",
//                  image->getAlphaImage()->getFilePath().c_str(), m_alphaName);
            
            textureAlpha->clearName();
            ret = 1;
        }
        else {
            CC_BREAK_IF(0 == image->getFilePath().size());
            std::string alphaExt = Image::Format::PVR == image->getFileType()? "_alpha.pvr": "_alpha.pkm";
            
            CC_BREAK_IF(std::string::npos != image->getFilePath().rfind(alphaExt));
            
            std::string alpha_filepath = image->getFilePath();
            alpha_filepath.replace(alpha_filepath.length() - 4, 4, alphaExt);
            if (FileUtils::getInstance()->isFileExist(alpha_filepath)) {
                imageAlpha = new Image();
                CC_BREAK_IF(nullptr == imageAlpha);
                CC_BREAK_IF(!imageAlpha->initWithImageFile(alpha_filepath));
                
//                CCLOG("Texture2D::initWithImage.alpha(:%s begin..", alpha_filepath.c_str());
                textureAlpha = new Texture2D();
                
                CC_BREAK_IF(!textureAlpha->initWithImage(imageAlpha));
                
                this->m_alphaName = textureAlpha->_name;
//                CCLOG("liudi Texture2D, %s, %d", alpha_filepath.c_str(),this->m_alphaName);
                textureAlpha->clearName();
                ret = 1;
            }
        }
        
    }
    while(0);
    CC_SAFE_DELETE(textureAlpha);
    CC_SAFE_DELETE(imageAlpha);
    
    if( this->m_alphaName ) {
        this->_hasAlphaChannel = true;
    }
    
    return ret;
}

Texture2D::PixelFormat Texture2D::convertI8ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
{
    switch (format)
    {
    case PixelFormat::RGBA8888:
        *outDataLen = dataLen*4;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertI8ToRGBA8888(data, dataLen, *outData);
        break;
    case PixelFormat::RGB888:
        *outDataLen = dataLen*3;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertI8ToRGB888(data, dataLen, *outData);
        break;
    case PixelFormat::RGB565:
        *outDataLen = dataLen*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertI8ToRGB565(data, dataLen, *outData);
        break;
    case PixelFormat::AI88:
        *outDataLen = dataLen*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertI8ToAI88(data, dataLen, *outData);
        break;
    case PixelFormat::RGBA4444:
        *outDataLen = dataLen*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertI8ToRGBA4444(data, dataLen, *outData);
        break;
    case PixelFormat::RGB5A1:
        *outDataLen = dataLen*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertI8ToRGB5A1(data, dataLen, *outData);
        break;
    default:
        // unsupported conversion or don't need to convert
        if (format != PixelFormat::AUTO && format != PixelFormat::I8)
        {
            CCLOG("Can not convert image format PixelFormat::I8 to format ID:%d, we will use it's origin format PixelFormat::I8", static_cast<int>(format));
        }

        *outData = (unsigned char*)data;
        *outDataLen = dataLen;
        return PixelFormat::I8;
    }

    return format;
}

Texture2D::PixelFormat Texture2D::convertAI88ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
{
    switch (format)
    {
    case PixelFormat::RGBA8888:
        *outDataLen = dataLen*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertAI88ToRGBA8888(data, dataLen, *outData);
        break;
    case PixelFormat::RGB888:
        *outDataLen = dataLen/2*3;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertAI88ToRGB888(data, dataLen, *outData);
        break;
    case PixelFormat::RGB565:
        *outDataLen = dataLen;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertAI88ToRGB565(data, dataLen, *outData);
        break;
    case PixelFormat::A8:
        *outDataLen = dataLen/2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertAI88ToA8(data, dataLen, *outData);
        break;
    case PixelFormat::I8:
        *outDataLen = dataLen/2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertAI88ToI8(data, dataLen, *outData);
        break;
    case PixelFormat::RGBA4444:
        *outDataLen = dataLen;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertAI88ToRGBA4444(data, dataLen, *outData);
        break;
    case PixelFormat::RGB5A1:
        *outDataLen = dataLen;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertAI88ToRGB5A1(data, dataLen, *outData);
        break;
    default:
        // unsupported conversion or don't need to convert
        if (format != PixelFormat::AUTO && format != PixelFormat::AI88)
        {
            CCLOG("Can not convert image format PixelFormat::AI88 to format ID:%d, we will use it's origin format PixelFormat::AI88", static_cast<int>(format));
        }

        *outData = (unsigned char*)data;
        *outDataLen = dataLen;
        return PixelFormat::AI88;
        break;
    }

    return format;
}

Texture2D::PixelFormat Texture2D::convertRGB888ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
{
    switch (format)
    {
    case PixelFormat::RGBA8888:
        *outDataLen = dataLen/3*4;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGB888ToRGBA8888(data, dataLen, *outData);
        break;
    case PixelFormat::RGB565:
        *outDataLen = dataLen/3*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGB888ToRGB565(data, dataLen, *outData);
        break;
    case PixelFormat::A8:
        *outDataLen = dataLen/3;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGB888ToA8(data, dataLen, *outData);
        break;
    case PixelFormat::I8:
        *outDataLen = dataLen/3;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGB888ToI8(data, dataLen, *outData);
        break;
    case PixelFormat::AI88:
        *outDataLen = dataLen/3*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGB888ToAI88(data, dataLen, *outData);
        break;
    case PixelFormat::RGBA4444:
        *outDataLen = dataLen/3*2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGB888ToRGBA4444(data, dataLen, *outData);
        break;
    case PixelFormat::RGB5A1:
        *outDataLen = dataLen;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGB888ToRGB5A1(data, dataLen, *outData);
        break;
    default:
        // unsupported conversion or don't need to convert
        if (format != PixelFormat::AUTO && format != PixelFormat::RGB888)
        {
            CCLOG("Can not convert image format PixelFormat::RGB888 to format ID:%d, we will use it's origin format PixelFormat::RGB888", static_cast<int>(format));
        }

        *outData = (unsigned char*)data;
        *outDataLen = dataLen;
        return PixelFormat::RGB888;
    }
    return format;
}

Texture2D::PixelFormat Texture2D::convertRGBA8888ToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
{

    switch (format)
    {
    case PixelFormat::RGB888:
        *outDataLen = dataLen/4*3;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGBA8888ToRGB888(data, dataLen, *outData);
        break;
    case PixelFormat::RGB565:
        *outDataLen = dataLen/2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGBA8888ToRGB565(data, dataLen, *outData);
        break;
    case PixelFormat::A8:
        *outDataLen = dataLen/4;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGBA8888ToA8(data, dataLen, *outData);
        break;
    case PixelFormat::I8:
        *outDataLen = dataLen/4;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGBA8888ToI8(data, dataLen, *outData);
        break;
    case PixelFormat::AI88:
        *outDataLen = dataLen/2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGBA8888ToAI88(data, dataLen, *outData);
        break;
    case PixelFormat::RGBA4444:
        *outDataLen = dataLen/2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGBA8888ToRGBA4444(data, dataLen, *outData);
        break;
    case PixelFormat::RGB5A1:
        *outDataLen = dataLen/2;
        *outData = (unsigned char*)malloc(sizeof(unsigned char) * (*outDataLen));
        convertRGBA8888ToRGB5A1(data, dataLen, *outData);
        break;
    default:
        // unsupported conversion or don't need to convert
        if (format != PixelFormat::AUTO && format != PixelFormat::RGBA8888)
        {
            CCLOG("Can not convert image format PixelFormat::RGBA8888 to format ID:%d, we will use it's origin format PixelFormat::RGBA8888", static_cast<int>(format));
        }

        *outData = (unsigned char*)data;
        *outDataLen = dataLen;
        return PixelFormat::RGBA8888;
    }

    return format;
}

/*
convert map:
1.PixelFormat::RGBA8888
2.PixelFormat::RGB888
3.PixelFormat::RGB565
4.PixelFormat::A8
5.PixelFormat::I8
6.PixelFormat::AI88
7.PixelFormat::RGBA4444
8.PixelFormat::RGB5A1

gray(5) -> 1235678
gray alpha(6) -> 12345678
rgb(2) -> 1235678
rgba(1) -> 12345678

*/
Texture2D::PixelFormat Texture2D::convertDataToFormat(const unsigned char* data, ssize_t dataLen, PixelFormat originFormat, PixelFormat format, unsigned char** outData, ssize_t* outDataLen)
{
    // don't need to convert
    if (format == originFormat || format == PixelFormat::AUTO)
    {
        *outData = (unsigned char*)data;
        *outDataLen = dataLen;
        return originFormat;
    }
    
    switch (originFormat)
    {
    case PixelFormat::I8:
        return convertI8ToFormat(data, dataLen, format, outData, outDataLen);
    case PixelFormat::AI88:
        return convertAI88ToFormat(data, dataLen, format, outData, outDataLen);
    case PixelFormat::RGB888:
        return convertRGB888ToFormat(data, dataLen, format, outData, outDataLen);
    case PixelFormat::RGBA8888:
        return convertRGBA8888ToFormat(data, dataLen, format, outData, outDataLen);
    default:
        CCLOG("unsupported conversion from format %d to format %d", static_cast<int>(originFormat), static_cast<int>(format));
        *outData = (unsigned char*)data;
        *outDataLen = dataLen;
        return originFormat;
    }
}

// implementation Texture2D (Text)
bool Texture2D::initWithString(const char *text, const std::string& fontName, float fontSize, const Size& dimensions/* = Size(0, 0)*/, TextHAlignment hAlignment/* =  TextHAlignment::CENTER */, TextVAlignment vAlignment/* =  TextVAlignment::TOP */)
{
    FontDefinition tempDef;
    
    tempDef._shadow._shadowEnabled = false;
    tempDef._stroke._strokeEnabled = false;
   
    
    tempDef._fontName      = fontName;
    tempDef._fontSize      = fontSize;
    tempDef._dimensions    = dimensions;
    tempDef._alignment     = hAlignment;
    tempDef._vertAlignment = vAlignment;
    tempDef._fontFillColor = Color3B::WHITE;

    return initWithString(text, tempDef);
}

bool correctUtfBytes(const char* bytes, int & index) {

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    if (Device::getSdkVersion() >= 23){
        int c,i,ix,n,j;
        int len = strlen(bytes);
        for (i=0, ix=len; i < ix; i++)
        {
            c = (unsigned char) bytes[i];
            //if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
            if (0x00 <= c && c <= 0x7f) n=0; // 0bbbbbbb
            else if ((c & 0xE0) == 0xC0) n=1; // 110bbbbb
            else if ( c==0xed && i<(ix-1) && ((unsigned char)bytes[i+1] & 0xa0)==0xa0) return false; //U+d800 to U+dfff
            else if ((c & 0xF0) == 0xE0) n=2; // 1110bbbb
            else if ((c & 0xF8) == 0xF0) n=3; // 11110bbb
            //else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
            //else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
            else return false;
            for (j=0; j<n && i<ix; j++) { // n bytes matching 10bbbbbb follow ?
                if ((++i == ix) || (( (unsigned char)bytes[i] & 0xC0) != 0x80))
                    return false;
            }
        }
    }else{
        index = 0;
        while (*bytes != '\0') {
            unsigned char utf8 = *(bytes++);
            ++index;
            // Switch on the high four bits.
            switch (utf8 >> 4) {
                case 0x00:
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07:
                    // Bit pattern 0xxx. No need for any extra bytes.
                    break;
                case 0x08:
                case 0x09:
                case 0x0a:
                case 0x0b:
                case 0x0f:
                    /*
                     * Bit pattern 10xx or 1111, which are illegal start bytes.
                     * Note: 1111 is valid for normal UTF-8, but not the
                     * modified UTF-8 used here.
                     */
                    // *(bytes-1) = '?';
                    break;
                case 0x0e:
                    // Bit pattern 1110, so there are two additional bytes.
                    utf8 = *(bytes++);
                    if ((utf8 & 0xc0) != 0x80) {
                        // *(bytes-1) = 0;
                        //--bytes;
                        //*(bytes-2) = 0;
                        return false;
                        break;
                    }
                    
                    // Fall through to take care of the final byte.
                case 0x0c:
                case 0x0d:
                    // Bit pattern 110x, so there is one additional byte.
                    utf8 = *(bytes++);
                    if ((utf8 & 0xc0) != 0x80) {
                        
                        //--bytes;
                        //if(three)--bytes;
                        // *(bytes-2) = 0;
                        // *(bytes-1)=0;
                        return false;
                    }
                    break;
            }
        }
    }
#endif
    return true;
}

bool Texture2D::initWithString(const char *text, const FontDefinition& textDefinition)
{
    if (!text || 0 == strlen(text))
    {
        return false;
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    int errIndex = 0;
    if( !correctUtfBytes( text, errIndex ) )
    {
        int len = strlen(text);
        std::string tempVal;
        for(int i = 0; i < len; i++) {
            char tmpbuf[10] =  {0};
            sprintf(tmpbuf, "%02x", (unsigned int)(text[i] & 0xFF));
            tempVal.append(tmpbuf);
        }
        cocos2d::log("gkTestStr:%s(%s),%d",text,tempVal.c_str(), errIndex);
        return false;
    }
#endif
    
    Director::getInstance()->runSyncOnGLThread([this, &text, textDefinition]
    {
        auto alignment = textDefinition._alignment;   
        //安卓的staticlayout由于默认支持RTL字体反向排列，这里倒转过来保证和ios行为一致，由我们自己处理文字对齐方向
#if(CC_TARGET_PLATFORM==CC_PLATFORM_ANDROID)
        if (g_fix_ar_direction)
        {
            if (alignment != TextHAlignment::CENTER)
            {
                if (isContainRTLChar(text))
                {
                    alignment = (textDefinition._alignment == TextHAlignment::LEFT) ? TextHAlignment::RIGHT:TextHAlignment::LEFT;
                }
            }
        }
#endif
        
    #if CC_ENABLE_CACHE_TEXTURE_DATA
        // cache the texture data
        VolatileTextureMgr::addStringTexture(this, text, textDefinition);
    #endif

        bool ret = false;
        Device::TextAlign align;
        
        if (TextVAlignment::TOP == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == alignment) ? Device::TextAlign::TOP
            : (TextHAlignment::LEFT == alignment) ? Device::TextAlign::TOP_LEFT : Device::TextAlign::TOP_RIGHT;
        }
        else if (TextVAlignment::CENTER == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == alignment) ? Device::TextAlign::CENTER
            : (TextHAlignment::LEFT == alignment) ? Device::TextAlign::LEFT : Device::TextAlign::RIGHT;
        }
        else if (TextVAlignment::BOTTOM == textDefinition._vertAlignment)
        {
            align = (TextHAlignment::CENTER == alignment) ? Device::TextAlign::BOTTOM
            : (TextHAlignment::LEFT == alignment) ? Device::TextAlign::BOTTOM_LEFT : Device::TextAlign::BOTTOM_RIGHT;
        }
        else
        {
            CCASSERT(false, "Not supported alignment format!");
            return false;
        }
        
    #if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID) && (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
//        CCASSERT(textDefinition._stroke._strokeEnabled == false, "Currently stroke only supported on iOS and Android!");
    #endif

        PixelFormat      pixelFormat = g_defaultAlphaPixelFormat;
        unsigned char* outTempData = nullptr;
        ssize_t outTempDataLen = 0;
        float scaleRate = 1.4f;
        
        if( textDefinition._fontSize * scaleRate < 23.0f ) {
            scaleRate = 23.0f / textDefinition._fontSize;
        }

        
#if ENABLE_SCALE_TEXT_RENDER
        auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR()*scaleRate;
#else
        auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();
#endif
        int imageWidth;
        int imageHeight;
        bool hasPremultipliedAlpha;
        
#if ENABLE_SCALE_TEXT_RENDER
        do {
#endif
        auto textDef = textDefinition;
        textDef._fontSize *= contentScaleFactor;
        textDef._dimensions.width *= contentScaleFactor;
        textDef._dimensions.height *= contentScaleFactor;
        textDef._stroke._strokeSize *= contentScaleFactor;
        textDef._shadow._shadowEnabled = false;
        
        
        Data outData = Device::getTextureDataForText(text, textDef, align, imageWidth, imageHeight, hasPremultipliedAlpha);
        if(outData.isNull())
        {
            return false;
        }
            
#if ENABLE_SCALE_TEXT_RENDER
        if(  scaleRate != 1.0f ) {
            if( imageWidth <= 0 || imageWidth > Configuration::getInstance()->getMaxTextureSize() ||
               imageHeight <= 0 || imageHeight > Configuration::getInstance()->getMaxTextureSize() )
            {
                scaleRate = 1.0f;
                contentScaleFactor = CC_CONTENT_SCALE_FACTOR()*scaleRate;
                continue;
            }
        }
#endif

        Size  imageSize = Size((float)imageWidth, (float)imageHeight);
        pixelFormat = convertDataToFormat(outData.getBytes(), imageWidth*imageHeight*4, PixelFormat::RGBA8888, pixelFormat, &outTempData, &outTempDataLen);

        ret = initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight, imageSize);

        if (outTempData != nullptr && outTempData != outData.getBytes())
        {
            free(outTempData);
        }
#if ENABLE_SCALE_TEXT_RENDER
        break;
        } while(1);
#endif

#if ENABLE_SCALE_TEXT_RENDER
        if( ret ) {
            _contentSize = Size((float)(imageWidth/scaleRate), (float)(imageHeight/scaleRate));
            _pixelsWide = (int)(imageWidth/scaleRate);
            _pixelsHigh = (int)(imageHeight/scaleRate);
        }

#endif
        
        _hasPremultipliedAlpha = hasPremultipliedAlpha;
        
            GLenum err = glGetError();
            if( err != GL_NO_ERROR )
            {
                CCLOG("cocos2d: initWithString zym Texture2D error:%d", err );
            }

        return ret;
    });
    
    return true;
}


// implementation Texture2D (Drawing)

void Texture2D::drawAtPoint(const Vec2& point)
{
    GLfloat    coordinates[] = {
        0.0f,    _maxT,
        _maxS,_maxT,
        0.0f,    0.0f,
        _maxS,0.0f };

    GLfloat    width = (GLfloat)_pixelsWide * _maxS,
        height = (GLfloat)_pixelsHigh * _maxT;

    GLfloat        vertices[] = {    
        point.x,            point.y,
        width + point.x,    point.y,
        point.x,            height  + point.y,
        width + point.x,    height  + point.y };

    GL::enableVertexAttribs( GL::VERTEX_ATTRIB_FLAG_POSITION | GL::VERTEX_ATTRIB_FLAG_TEX_COORD );
    _shaderProgram->use();
    _shaderProgram->setUniformsForBuiltins();

    GL::bindTexture2D( _name );


#ifdef EMSCRIPTEN
    setGLBufferData(vertices, 8 * sizeof(GLfloat), 0);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    setGLBufferData(coordinates, 8 * sizeof(GLfloat), 1);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
#else
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
#endif // EMSCRIPTEN

    GLenum err = glGetError();
    if( err != GL_NO_ERROR )
    {
        CCLOG("cocos2d: glDrawElements zym Texture2D error:%d", err );
        return;
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Texture2D::drawInRect(const Rect& rect)
{
    GLfloat    coordinates[] = {    
        0.0f,    _maxT,
        _maxS,_maxT,
        0.0f,    0.0f,
        _maxS,0.0f };

    GLfloat    vertices[] = {    rect.origin.x,        rect.origin.y,                            /*0.0f,*/
        rect.origin.x + rect.size.width,        rect.origin.y,                            /*0.0f,*/
        rect.origin.x,                            rect.origin.y + rect.size.height,        /*0.0f,*/
        rect.origin.x + rect.size.width,        rect.origin.y + rect.size.height,        /*0.0f*/ };

    GL::enableVertexAttribs( GL::VERTEX_ATTRIB_FLAG_POSITION | GL::VERTEX_ATTRIB_FLAG_TEX_COORD );
    _shaderProgram->use();
    _shaderProgram->setUniformsForBuiltins();

    GL::bindTexture2D( _name );

#ifdef EMSCRIPTEN
    setGLBufferData(vertices, 8 * sizeof(GLfloat), 0);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    setGLBufferData(coordinates, 8 * sizeof(GLfloat), 1);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
#else
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, coordinates);
#endif // EMSCRIPTEN
    
    GLenum err = glGetError();
    if( err != GL_NO_ERROR )
    {
        CCLOG("cocos2d: glDrawElements zym Texture2D error1:%d", err );
        return;
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Texture2D::PVRImagesHavePremultipliedAlpha(bool haveAlphaPremultiplied)
{
    Image::setPVRImagesHavePremultipliedAlpha(haveAlphaPremultiplied);
}


//
// Use to apply MIN/MAG filter
//
// implementation Texture2D (GLFilter)

void Texture2D::generateMipmap()
{
    CCASSERT(_pixelsWide == ccNextPOT(_pixelsWide) && _pixelsHigh == ccNextPOT(_pixelsHigh), "Mipmap texture only works in POT textures");
    GL::bindTexture2D( _name );
    glGenerateMipmap(GL_TEXTURE_2D);
    // Added by ChenFei 2014-12-26 V3.2 support
    if(m_alphaName)
    {
        GL::bindTexture2D( m_alphaName );
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    _hasMipmaps = true;
#if CC_ENABLE_CACHE_TEXTURE_DATA
    VolatileTextureMgr::setHasMipmaps(this, _hasMipmaps);
#endif
}

bool Texture2D::hasMipmaps() const
{
    return _hasMipmaps;
}

void Texture2D::setTexParameters(const TexParams &texParams)
{
    CCASSERT((_pixelsWide == ccNextPOT(_pixelsWide) || texParams.wrapS == GL_CLAMP_TO_EDGE) &&
        (_pixelsHigh == ccNextPOT(_pixelsHigh) || texParams.wrapT == GL_CLAMP_TO_EDGE),
        "GL_CLAMP_TO_EDGE should be used in NPOT dimensions");

    _texParams = texParams;
    
    if( _name != 0 ) {
        GL::bindTexture2D( _name );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT );
    }

    // Added by ChenFei 2014-12-26 V3.2 support
    if( m_alphaName )
    {
//        CCLOG("liudi etc enable");
        GL::bindTexture2DN(1,  m_alphaName );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT );
    }

#if CC_ENABLE_CACHE_TEXTURE_DATA
    VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}

void Texture2D::setAliasTexParameters()
{
    if (! _antialiasEnabled)
    {
        return;
    }

    _antialiasEnabled = false;

    if (_name == 0)
    {
        return;
    }

    GL::bindTexture2D( _name );

    if( ! _hasMipmaps )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    }
    else
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
    }

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
#if CC_ENABLE_CACHE_TEXTURE_DATA
    TexParams texParams = {(GLuint)(_hasMipmaps?GL_NEAREST_MIPMAP_NEAREST:GL_NEAREST),GL_NEAREST,GL_NONE,GL_NONE};
    VolatileTextureMgr::setTexParameters(this, texParams);
#endif
}

void Texture2D::setAntiAliasTexParameters()
{
    if ( _antialiasEnabled )
    {
        return;
    }

    _antialiasEnabled = true;

    if (_name == 0)
    {
        return;
    }

    GL::bindTexture2D( _name );

    if( ! _hasMipmaps )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    }
    else
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    }

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
#if CC_ENABLE_CACHE_TEXTURE_DATA
    TexParams texParams = {(GLuint)(_hasMipmaps?GL_LINEAR_MIPMAP_NEAREST:GL_LINEAR),GL_LINEAR,GL_NONE,GL_NONE};
    VolatileTextureMgr::setTexParameters(this, texParams);
#endif
    
    GLenum err = glGetError();
    if( err != GL_NO_ERROR )
    {
        CCLOG("cocos2d: setAntiAliasTexParameters zym Texture2D error:%d", err );
        return;
    }
}

void Texture2D::setHasPremultipliedAlpha( bool val )
{
    _hasPremultipliedAlpha = val;
#if CC_ENABLE_CACHE_TEXTURE_DATA
    VolatileTextureMgr::setPremultipliedAlpha(this, val);
#endif
}


const char* Texture2D::getStringForFormat() const
{
    switch (_pixelFormat) 
    {
        case Texture2D::PixelFormat::RGBA8888:
            return  "RGBA8888";

        case Texture2D::PixelFormat::RGB888:
            return  "RGB888";

        case Texture2D::PixelFormat::RGB565:
            return  "RGB565";

        case Texture2D::PixelFormat::RGBA4444:
            return  "RGBA4444";

        case Texture2D::PixelFormat::RGB5A1:
            return  "RGB5A1";

        case Texture2D::PixelFormat::AI88:
            return  "AI88";

        case Texture2D::PixelFormat::A8:
            return  "A8";

        case Texture2D::PixelFormat::I8:
            return  "I8";

        case Texture2D::PixelFormat::PVRTC4:
            return  "PVRTC4";

        case Texture2D::PixelFormat::PVRTC2:
            return  "PVRTC2";

        case Texture2D::PixelFormat::PVRTC2A:
            return "PVRTC2A";
        
        case Texture2D::PixelFormat::PVRTC4A:
            return "PVRTC4A";
            
        case Texture2D::PixelFormat::ETC:
            return "ETC";

        case Texture2D::PixelFormat::S3TC_DXT1:
            return "S3TC_DXT1";
            
        case Texture2D::PixelFormat::S3TC_DXT3:
            return "S3TC_DXT3";

        case Texture2D::PixelFormat::S3TC_DXT5:
            return "S3TC_DXT5";
            
        case Texture2D::PixelFormat::ATC_RGB:
            return "ATC_RGB";

        case Texture2D::PixelFormat::ATC_EXPLICIT_ALPHA:
            return "ATC_EXPLICIT_ALPHA";

        case Texture2D::PixelFormat::ATC_INTERPOLATED_ALPHA:
            return "ATC_INTERPOLATED_ALPHA";
            
        default:
            CCASSERT(false , "unrecognized pixel format");
            CCLOG("stringForFormat: %ld, cannot give useful result", (long)_pixelFormat);
            break;
    }

    return  nullptr;
}

//
// Texture options for images that contains alpha
//
// implementation Texture2D (PixelFormat)

void Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat format)
{
    g_defaultAlphaPixelFormat = format;
}

Texture2D::PixelFormat Texture2D::getDefaultAlphaPixelFormat()
{
    return g_defaultAlphaPixelFormat;
}

unsigned int Texture2D::getBitsPerPixelForFormat(Texture2D::PixelFormat format) const
{
    if (format == PixelFormat::NONE || format == PixelFormat::DEFAULT)
    {
        return 0;
    }
    
	return _pixelFormatInfoTables.at(format).bpp;
}

unsigned int Texture2D::getBitsPerPixelForFormat() const
{
	return this->getBitsPerPixelForFormat(_pixelFormat);
}

const Texture2D::PixelFormatInfoMap& Texture2D::getPixelFormatInfoMap()
{
    return _pixelFormatInfoTables;
}

void Texture2D::addSpriteFrameCapInset(SpriteFrame* spritframe, const Rect& capInsets)
{
    if(nullptr == _ninePatchInfo)
    {
        _ninePatchInfo = new (std::nothrow) NinePatchInfo;
    }
    if(nullptr == spritframe)
    {
        _ninePatchInfo->capInsetSize = capInsets;
    }
    else
    {
        _ninePatchInfo->capInsetMap[spritframe] = capInsets;
    }
}

bool Texture2D::isContain9PatchInfo()const
{
    return nullptr != _ninePatchInfo;
}

bool Texture2D::isContainEnglishChar(const char *text)const
{
    size_t len = strlen(text);
    for (int i =0; i < len ; i++) {
        if (isalpha(text[i])) {
            return true;
        }
    }
    return false;
}

static const unsigned char utf8d[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    070,070,070,070,070,070,070,070,070,070,070,070,070,070,070,070,
    050,050,050,050,050,050,050,050,050,050,050,050,050,050,050,050,
    030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,
    030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,
    204,204,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
    188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
    174,158,158,158,158,158,158,158,158,158,158,158,158,142,126,126,
    111, 95, 95, 95, 79,207,207,207,207,207,207,207,207,207,207,207,
    0,1,1,1,8,7,6,4,5,4,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
    1,4,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4,4,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,4,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,8,7,6,4,5,4,3,2,1,1,1,1,
};

bool Texture2D::isContainRTLChar(const char *text)const
{
    int move_main = 0;
    int c = Texture2D::utf8c(text, &text, &move_main);
    while (c) {
        if ((c == 0x5BE) || (c == 0x5C0) || (c == 0x5C3) || (c == 0x5C6) || ((c >= 0x5D0) && (c <= 0x5F4)) || (c == 0x608) || (c == 0x60B) || (c == 0x60D) ||
            ((c >= 0x61B) && (c <= 0x64A)) || ((c >= 0x66D) && (c <= 0x66F)) || ((c >= 0x671) && (c <= 0x6D5)) || ((c >= 0x6E5) && (c <= 0x6E6)) ||
            ((c >= 0x6EE) && (c <= 0x6EF)) || ((c >= 0x6FA) && (c <= 0x710)) || ((c >= 0x712) && (c <= 0x72F)) || ((c >= 0x74D) && (c <= 0x7A5)) ||
            ((c >= 0x7B1) && (c <= 0x7EA)) || ((c >= 0x7F4) && (c <= 0x7F5)) || ((c >= 0x7FA) && (c <= 0x815)) || (c == 0x81A) || (c == 0x824) || (c == 0x828) ||
            ((c >= 0x830) && (c <= 0x858)) || ((c >= 0x85E) && (c <= 0x8AC)) || (c == 0x200F) || (c == 0xFB1D) || ((c >= 0xFB1F) && (c <= 0xFB28)) ||
            ((c >= 0xFB2A) && (c <= 0xFD3D)) || ((c >= 0xFD50) && (c <= 0xFDFC)) || ((c >= 0xFE70) && (c <= 0xFEFC)) || ((c >= 0x10800) && (c <= 0x1091B)) ||
            ((c >= 0x10920) && (c <= 0x10A00)) || ((c >= 0x10A10) && (c <= 0x10A33)) || ((c >= 0x10A40) && (c <= 0x10B35)) || ((c >= 0x10B40) && (c <= 0x10C48)) ||
            ((c >= 0x1EE00) && (c <= 0x1EEBB))
            )
        {
            return true;
        }
        move_main++;
        c = Texture2D::utf8c(text, &text, &move_main);
    }
    return false;
}

int Texture2D::utf8c(const char * src, const char ** ss,int * move)const
{
    unsigned char a, b, t = 9;
    int u         = 0;
    const char * s= src;
    int n         = 0;
    while ((b=*s++)){
        n++;
        a = utf8d[b];
        t = utf8d[ 256 + (t << 4) + (a >> 4) ];
        b = (b^(unsigned char)(a<<4));
        u = (u<<6)|b;
        if (!t){
            if (ss!=NULL)   *ss = s;
            if (move!=NULL) *move=n;
            return u;
        }
    }
    if (ss!=NULL)   *ss = s;
    if (move!=NULL) *move=n;
    return 0;
}


const Rect& Texture2D::getSpriteFrameCapInset( cocos2d::SpriteFrame *spriteFrame )const
{
    CCASSERT(_ninePatchInfo != nullptr,
             "Can't get the sprite frame capInset when the texture contains no 9-patch info.");
    if(nullptr == spriteFrame)
    {
        return this->_ninePatchInfo->capInsetSize;
    }
    else
    {
        auto &capInsetMap = this->_ninePatchInfo->capInsetMap;
        if(capInsetMap.find(spriteFrame) != capInsetMap.end())
        {
            return capInsetMap.at(spriteFrame);
        }
        else
        {
            return this->_ninePatchInfo->capInsetSize;
        }
    }
}


void Texture2D::removeSpriteFrameCapInset(SpriteFrame* spriteFrame)
{
    if(nullptr != this->_ninePatchInfo)
    {
        auto capInsetMap = this->_ninePatchInfo->capInsetMap;
        if(capInsetMap.find(spriteFrame) != capInsetMap.end())
        {
            capInsetMap.erase(spriteFrame);
        }
    }
}

// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
unsigned long Texture2D::instanceCount = 0;
unsigned long Texture2D::getInstanceCount()
{
    return instanceCount;
}
void Texture2D::increaseInstanceCount(bool increased)
{
    if(increased)
    {
        ++instanceCount;
    }
    else
    {
        --instanceCount;
    }
}
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS

NS_CC_END
