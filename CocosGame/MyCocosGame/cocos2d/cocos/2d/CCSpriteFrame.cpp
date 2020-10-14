/****************************************************************************
Copyright (c) 2008-2011 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
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

#include "renderer/CCTextureCache.h"
#include "base/CCDirector.h"
#include "2d/CCSpriteFrame.h"
#include "2d/CCSpriteFrameCache.h"

NS_CC_BEGIN

#define CC_2x2_WHITE_IMAGE_KEY  "/cc_2x2_white_image"

// implementation of SpriteFrame

SpriteFrame* SpriteFrame::create(const std::string& filename, const Rect& rect)
{
    SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
    spriteFrame->initWithTextureFilename(filename, rect);
    spriteFrame->autorelease();

    return spriteFrame;
}

SpriteFrame* SpriteFrame::createWithTexture(Texture2D *texture, const Rect& rect)
{
    SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
    spriteFrame->initWithTexture(texture, rect);
    spriteFrame->autorelease();

    return spriteFrame;
}

SpriteFrame* SpriteFrame::createWithTexture(Texture2D* texture, const Rect& rect, bool rotated, const Vec2& offset, const Size& originalSize)
{
    SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
    spriteFrame->initWithTexture(texture, rect, rotated, offset, originalSize);
    spriteFrame->autorelease();

    return spriteFrame;
}

SpriteFrame* SpriteFrame::create(const std::string& filename, const Rect& rect, bool rotated, const Vec2& offset, const Size& originalSize)
{
    SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
    spriteFrame->initWithTextureFilename(filename, rect, rotated, offset, originalSize);
    spriteFrame->autorelease();

    return spriteFrame;
}

SpriteFrame* SpriteFrame::create_no_autorelease(const std::string& filename, const Rect& rect, bool rotated, const Vec2& offset, const Size& originalSize)
{
    SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
    spriteFrame->initWithTextureFilename(filename, rect, rotated, offset, originalSize);
//    spriteFrame->autorelease();
    
    return spriteFrame;
}

SpriteFrame::SpriteFrame()
: _rotated(false)
//, _texture(nullptr)
, m_bGrey(false)
, _textPathIndex(-1)
{
    
}

bool SpriteFrame::initWithTexture(Texture2D* texture, const Rect& rect)
{
    Rect rectInPixels = CC_RECT_POINTS_TO_PIXELS(rect);
    return initWithTexture(texture, rectInPixels, false, Vec2::ZERO, rectInPixels.size);
}

bool SpriteFrame::initWithTextureFilename(const std::string& filename, const Rect& rect)
{
    Rect rectInPixels = CC_RECT_POINTS_TO_PIXELS( rect );
    return initWithTextureFilename(filename, rectInPixels, false, Vec2::ZERO, rectInPixels.size);
}

bool SpriteFrame::initWithTexture(Texture2D* texture, const Rect& rect, bool rotated, const Vec2& offset, const Size& originalSize)
{
//    setTexture(texture);
    
    std::string s = texture->getFileName();
    if (s.empty()) {
        int a = 0;
    }
    setTextureFileName(s);

    _rectInPixels = rect;
    _rect = CC_RECT_PIXELS_TO_POINTS(rect);
    _offsetInPixels = offset;
    _offset = CC_POINT_PIXELS_TO_POINTS( _offsetInPixels );
    _originalSizeInPixels = originalSize;
    _originalSize = CC_SIZE_PIXELS_TO_POINTS( _originalSizeInPixels );
    
    _rotated = rotated;

    return true;
}

bool SpriteFrame::initWithTextureFilename(const std::string& filename, const Rect& rect, bool rotated, const Vec2& offset, const Size& originalSize)
{
//    CC_SAFE_RELEASE_NULL(_texture);
    setTextureFileName(filename);
    
    _rectInPixels = rect;
    _rect = CC_RECT_PIXELS_TO_POINTS( rect );
    _offsetInPixels = offset;
    _offset = CC_POINT_PIXELS_TO_POINTS( _offsetInPixels );
    _originalSizeInPixels = originalSize;
    _originalSize = CC_SIZE_PIXELS_TO_POINTS( _originalSizeInPixels );
    _rotated = rotated;

    return true;
}

SpriteFrame::~SpriteFrame()
{
//    CCLOGINFO("deallocing SpriteFrame: %p", this);
//    CC_SAFE_RELEASE_NULL(_texture);
}

SpriteFrame* SpriteFrame::clone() const
{
    std::string s = getTextureFileName();
    
	// no copy constructor	
    SpriteFrame *copy = new (std::nothrow) SpriteFrame();
    copy->initWithTextureFilename(s.c_str(), _rectInPixels, _rotated, _offsetInPixels, _originalSizeInPixels);
//    copy->setTexture(_texture);
    copy->autorelease();
    return copy;
}

void SpriteFrame::setRect(const Rect& rect)
{
    _rect = rect;
    _rectInPixels = CC_RECT_POINTS_TO_PIXELS(_rect);
}

void SpriteFrame::setRectInPixels(const Rect& rectInPixels)
{
    _rectInPixels = rectInPixels;
    _rect = CC_RECT_PIXELS_TO_POINTS(rectInPixels);
}

const Vec2& SpriteFrame::getOffset() const
{
    return _offset;
}

void SpriteFrame::setOffset(const Vec2& offsets)
{
    _offset = offsets;
    _offsetInPixels = CC_POINT_POINTS_TO_PIXELS( _offset );
}

const Vec2& SpriteFrame::getOffsetInPixels() const
{
    return _offsetInPixels;
}

void SpriteFrame::setOffsetInPixels(const Vec2& offsetInPixels)
{
    _offsetInPixels = offsetInPixels;
    _offset = CC_POINT_PIXELS_TO_POINTS( _offsetInPixels );
}

//void SpriteFrame::setTexture(Texture2D * texture)
//{
//    if( _texture != texture ) {
//        CC_SAFE_RELEASE(_texture);
//        CC_SAFE_RETAIN(texture);
//        _texture = texture;
//    }
//}

Texture2D* SpriteFrame::getTexture()
{
//    if( _texture ) {
//        return _texture;
//    }

    std::string const& s = getTextureFileName();
    
    if( s.length() > 0 )
    {
        Texture2D *texture;
        texture = TextureCache::sharedTextureCache()->getTextureForKey(s);
        if (texture) {
            return texture;
        }
        
//        CCLOG("SpriteFrame::getTexture -- %s", s.c_str());
        TextureCache::sharedTextureCache()->EnableDefaultTexture(false);
        texture = Director::getInstance()->getTextureCache()->addImage(s);
        TextureCache::sharedTextureCache()->EnableDefaultTexture(true);
        
        if (texture) {
            return texture;
        }
    }
    
    // no texture or texture filename
    CCLOG("SpriteFrame::getTexture no texture??!!! name: %s", s.c_str());
    Texture2D* texture = Director::getInstance()->getTextureCache()->getTextureForKey(CC_2x2_WHITE_IMAGE_KEY);
    assert(0);
    
    return texture;
}

void SpriteFrame::setTextureFileName(std::string const& fn)
{
    _textPathIndex = SpriteFrameCache::getInstance()->addTextureNameIndex(fn);
    return;
}

std::string const& SpriteFrame::getTextureFileName() const
{
    return SpriteFrameCache::getInstance()->getTextureNameFromIndex(_textPathIndex);
}

NS_CC_END
