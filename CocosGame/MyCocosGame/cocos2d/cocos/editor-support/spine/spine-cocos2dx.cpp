/******************************************************************************
 * Spine Runtimes Software License
 * Version 2.3
 * 
 * Copyright (c) 2013-2015, Esoteric Software
 * All rights reserved.
 * 
 * You are granted a perpetual, non-exclusive, non-sublicensable and
 * non-transferable license to use, install, execute and perform the Spine
 * Runtimes Software (the "Software") and derivative works solely for personal
 * or internal use. Without the written permission of Esoteric Software (see
 * Section 2 of the Spine Software License Agreement), you may not (a) modify,
 * translate, adapt or otherwise create derivative works, improvements of the
 * Software or develop new applications using the Software or (b) remove,
 * delete, alter or obscure any trademarks or any copyright, trademark, patent
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 * 
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <spine/spine-cocos2dx.h>
#include <spine/extension.h>

USING_NS_CC;

GLuint wrap (spAtlasWrap wrap) {
	return wrap == SP_ATLAS_CLAMPTOEDGE ? GL_CLAMP_TO_EDGE : GL_REPEAT;
}

GLuint filter (spAtlasFilter filter) {
	switch (filter) {
	case SP_ATLAS_UNKNOWN_FILTER:
		break;
	case SP_ATLAS_NEAREST:
		return GL_NEAREST;
	case SP_ATLAS_LINEAR:
		return GL_LINEAR;
	case SP_ATLAS_MIPMAP:
		return GL_LINEAR_MIPMAP_LINEAR;
	case SP_ATLAS_MIPMAP_NEAREST_NEAREST:
		return GL_NEAREST_MIPMAP_NEAREST;
	case SP_ATLAS_MIPMAP_LINEAR_NEAREST:
		return GL_LINEAR_MIPMAP_NEAREST;
	case SP_ATLAS_MIPMAP_NEAREST_LINEAR:
		return GL_NEAREST_MIPMAP_LINEAR;
	case SP_ATLAS_MIPMAP_LINEAR_LINEAR:
		return GL_LINEAR_MIPMAP_LINEAR;
	}
	return GL_LINEAR;
}

void _spAtlasPage_createTexture (spAtlasPage* self, const char* path) {
	Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(path);
    if( texture )
    {
        texture->retain();
        Texture2D::TexParams textureParams = {filter(self->minFilter), filter(self->magFilter), wrap(self->uWrap), wrap(self->vWrap)};
        texture->setTexParameters(textureParams);
        self->rendererObject = texture;
        self->width = texture->getPixelsWide();
        self->height = texture->getPixelsHigh();
        std::string s(path);
        if(s.find(".png") != std::string::npos)
            self->isAndroidPNG = 1;
        else
            self->isAndroidPNG = 0;
            
    }
    else
    {
        self->rendererObject = NULL;
    }
}

void _spAtlasPage_disposeTexture (spAtlasPage* self) {
	((Texture2D*)self->rendererObject)->release();
}

char* _spUtil_readFile (const char* path, int* length) {
    if( !path || strlen(path) == 0 ) {
        return NULL;
    }
	Data data = FileUtils::getInstance()->getDataFromFile(
			FileUtils::getInstance()->fullPathForFilename(path).c_str());
	*length = data.getSize();
	char* bytes = MALLOC(char, *length);
    memcpy(bytes, data.getBytes(), *length);
	return bytes;
}

spAtlasRegion* spAtlas_findRegionFromSpriteFrame( spAtlas* self, const char* name ) {

    if( self->pages && 0 == self->pages->customFlag) {
        return NULL;
    }
    SpriteFrameCache* pCache = SpriteFrameCache::getInstance();
    std::string key(name);
    key += ".png";
    SpriteFrame* frame = pCache->getSpriteFrameByName(key.c_str());
    
    if (!frame) {
        return NULL;
    }
    
    Texture2D* texture = frame->getTexture();
    
    spAtlasPage *page = 0;
    if( !self->pages ) {
        page = ::spAtlasPage_create(self, "");
        self->pages = page;
        page->width = texture->getPixelsWide();
        page->height = texture->getPixelsHigh();
        if( texture->getPixelFormat() == cocos2d::Texture2D::PixelFormat::RGBA8888 ) {
            page->format = SP_ATLAS_RGBA8888;
        }
        else if( texture->getPixelFormat() == cocos2d::Texture2D::PixelFormat::RGBA4444 ) {
            page->format = SP_ATLAS_RGBA4444;
        }
        else {
            cocos2d::log("WRONG FORMAT %d",texture->getPixelFormat() );
        }
        
//        page->minFilter = (spAtlasFilter)texture->getTexParameters().minFilter;
//        page->magFilter = (spAtlasFilter)texture->getTexParameters().magFilter;
//        page->uWrap = (spAtlasWrap)texture->getTexParameters().wrapS;
//        page->vWrap = (spAtlasWrap)texture->getTexParameters().wrapT;
        page->rendererObject = texture;
        texture->retain();
        page->customFlag = 1;
    }
    page = self->pages;
    
    spAtlasRegion *region = NULL;
    if (!self->regions) {
        self->regions = spAtlasRegion_create();
        region = self->regions;
    }
    else {
        spAtlasRegion *lastRegion = self->regions;
        if( strcmp(lastRegion->name, name) == 0) {
            return lastRegion;
        }
        while (lastRegion->next) {
            if( strcmp(lastRegion->next->name, name) == 0) {
                return lastRegion->next;
            }
            lastRegion = lastRegion->next;
        }
        lastRegion->next = spAtlasRegion_create();
        region = lastRegion->next;
    }
    
//    if (lastRegion)
//        lastRegion->next = region;
//    else
//        self->regions = region;
//    lastRegion = region;

    region->page = page;
    region->name = strdup(name);
    region->rotate = frame->isRotated();
    Rect rect = frame->getRectInPixels();
    region->x = rect.origin.x;
    region->y = rect.origin.y;
    region->width = rect.size.width;
    region->height = rect.size.height;
//#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
//    region->u = (2*region->x+1) / (2 *(float)page->width);
//    region->v = (2*region->y+1) / (2 *(float)page->height);
//#else
    region->u = region->x / (float)page->width;
    region->v = region->y / (float)page->height;
//#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL

    if (region->rotate) {
        
        region->u2 = (region->x + region->height) / (float)page->width;
        region->v2 = (region->y + region->width) / (float)page->height;
    
    } else {
        region->u2 = (region->x + region->width) / (float)page->width;
        region->v2 = (region->y + region->height) / (float)page->height;
    }
    
    region->originalWidth = frame->getOriginalSizeInPixels().width;
    region->originalHeight = frame->getOriginalSizeInPixels().height;
    region->offsetX = frame->getOffsetInPixels().x * 2 + 0.5f;
    region->offsetY = frame->getOffsetInPixels().y * 2 + 0.5f;
    
    //fix by liudi , change the offset to atlas coordinate
    region->offsetX = (region->originalWidth - region->width + region->offsetX)/2;
    region->offsetY = (region->originalHeight - region->height + region->offsetY)/2;
    
    region->index = -1;
    return region;
}
