/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009      Jason Booth
Copyright (c) 2009      Robert J Payne
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

#include "2d/CCSpriteFrameCache.h"

#include <vector>


#include "2d/CCSprite.h"
#include "platform/CCFileUtils.h"
#include "base/CCNS.h"
#include "base/ccMacros.h"
#include "base/CCDirector.h"
#include "renderer/CCTexture2D.h"
#include "renderer/CCTextureCache.h"
#include "base/CCNinePatchImageParser.h"

#include "deprecated/CCString.h"
#include "base/ZipUtils.h"
#include "base/CCGlobalLock.h"
#include "CCSpriteManager.hpp"

#include <plist/plistHelper.hpp>
using namespace std;
extern pthread_t MAIN_THREAD_ID;

NS_CC_BEGIN

static std::string gEmptyStr;
static SpriteFrameCache *_sharedSpriteFrameCache = nullptr;


SpriteFrameCache* SpriteFrameCache::getInstance()
{
    if (! _sharedSpriteFrameCache)
    {
        _sharedSpriteFrameCache = new (std::nothrow) SpriteFrameCache();
        _sharedSpriteFrameCache->init();
    }

    return _sharedSpriteFrameCache;
}

void SpriteFrameCache::destroyInstance()
{
    CC_SAFE_RELEASE_NULL(_sharedSpriteFrameCache);
}

bool SpriteFrameCache::init()
{
    _spriteFrames.reserve(20);
//    _spriteFramesAliases.reserve(20);
    _loadedFileNames = new std::set<std::string>();
    return true;
}

SpriteFrameCache::~SpriteFrameCache()
{
    CC_SAFE_DELETE(_loadedFileNames);
}

bool SpriteFrameCache::addSpriteFramesWithDictionary(ValueMap& dictionary, std::string const& textureName)
{
    if( dictionary["frames"].getType() != Value::Type::MAP )
        return false;
    
    ValueMap& framesDict = dictionary["frames"].asValueMap();
    
    int format = 0;
    
    // get the format
    if (dictionary.find("metadata") != dictionary.end())
    {
        ValueMap& metadataDict = dictionary["metadata"].asValueMap();
        format = metadataDict["format"].asInt();
    }
    
    // check the format
    CCASSERT(format >=0 && format <= 3, "format is not supported for SpriteFrameCache addSpriteFramesWithDictionary:textureFilename:");
    
    bool isGood = true;
    
    for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
    {
        if (iter->second.getType() != Value::Type::MAP) {
            assert(0);
            continue;
        }
        
        ValueMap& frameDict = iter->second.asValueMap();
        std::string const& spriteFrameName = iter->first;
        SpriteFrame* spriteFrame = _spriteFrames.at(spriteFrameName);
//        if (spriteFrame)
//        {
//            isGood = false;
//            continue;
//        }
        
        if(format == 0)
        {
            float x = frameDict["x"].asFloat();
            float y = frameDict["y"].asFloat();
            float w = frameDict["width"].asFloat();
            float h = frameDict["height"].asFloat();
            float ox = frameDict["offsetX"].asFloat();
            float oy = frameDict["offsetY"].asFloat();
            int ow = frameDict["originalWidth"].asInt();
            int oh = frameDict["originalHeight"].asInt();
            // check ow/oh
            if(!ow || !oh)
            {
                CCLOGWARN("cocos2d: WARNING: originalWidth/Height not found on the SpriteFrame. AnchorPoint won't work as expected. Regenrate the .plist");
            }
            // abs ow/oh
            ow = abs(ow);
            oh = abs(oh);
            
            // create frame
            if (spriteFrame == nullptr)
            {
                spriteFrame = SpriteFrame::create_no_autorelease(textureName,
                                                         Rect(x, y, w, h),
                                                         false,
                                                         Vec2(ox, oy),
                                                         Size((float)ow, (float)oh)
                                                         );
            }
            else
            {
                spriteFrame->retain();
                spriteFrame->initWithTextureFilename(textureName,
                                                     Rect(x, y, w, h),
                                                     false,
                                                     Vec2(ox, oy),
                                                     Size((float)ow, (float)oh)
                                                     );
            }
        }
        else if(format == 1 || format == 2)
        {
            Rect frame = RectFromString(frameDict["frame"].asString());
            bool rotated = false;
            
            // rotation
            if (format == 2)
            {
                rotated = frameDict["rotated"].asBool();
            }
            
            Vec2 offset = PointFromString(frameDict["offset"].asString());
            Size sourceSize = SizeFromString(frameDict["sourceSize"].asString());
            
            // create frame
            if (spriteFrame == nullptr)
            {
                spriteFrame = SpriteFrame::create_no_autorelease(textureName,
                                                         frame,
                                                         rotated,
                                                         offset,
                                                         sourceSize
                                                         );
            }
            else
            {
                spriteFrame->retain();
                spriteFrame->initWithTextureFilename(textureName,
                                                                 frame,
                                                                 rotated,
                                                                 offset,
                                                                 sourceSize
                                                                 );
            }
        }
        else if (format == 3)
        {
            // get values
            Size spriteSize = SizeFromString(frameDict["spriteSize"].asString());
            Vec2 spriteOffset = PointFromString(frameDict["spriteOffset"].asString());
            Size spriteSourceSize = SizeFromString(frameDict["spriteSourceSize"].asString());
            Rect textureRect = RectFromString(frameDict["textureRect"].asString());
            bool textureRotated = frameDict["textureRotated"].asBool();
            
            // get aliases
            ValueVector& aliases = frameDict["aliases"].asValueVector();
            
            for(const auto &value : aliases) {
                std::string oneAlias = value.asString();
                if (_spriteFramesAliases.find(oneAlias) != _spriteFramesAliases.end())
                {
                    CCLOGWARN("cocos2d: WARNING: an alias with name %s already exists", oneAlias.c_str());
                }
                
                _spriteFramesAliases[oneAlias] = Value(spriteFrameName);
            }
            
            // create frame
            if (spriteFrame == nullptr)
            {
                spriteFrame = SpriteFrame::create_no_autorelease(textureName,
                                                         Rect(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
                                                         textureRotated,
                                                         spriteOffset,
                                                         spriteSourceSize);
            }
            else
            {
                spriteFrame->retain();
                spriteFrame->initWithTextureFilename(textureName,
                                                         Rect(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
                                                         textureRotated,
                                                         spriteOffset,
                                                         spriteSourceSize);
            }
        }
        
        // add sprite frame
        _spriteFrames.insert(spriteFrameName, spriteFrame);
        spriteFrame->release();
//        spriteFrame->setTextureFileName(spriteFrameName);
    }
    
    dictionary["isGood"] = Value(isGood);
    return true;
}

bool SpriteFrameCache::addSpriteFramesWithDictionary(ValueMap& dictionary, Texture2D* texture)
{

#if 1
    /*
    Supported Zwoptex Formats:

    ZWTCoordinatesFormatOptionXMLLegacy = 0, // Flash Version
    ZWTCoordinatesFormatOptionXML1_0 = 1, // Desktop Version 0.0 - 0.4b
    ZWTCoordinatesFormatOptionXML1_1 = 2, // Desktop Version 1.0.0 - 1.0.1
    ZWTCoordinatesFormatOptionXML1_2 = 3, // Desktop Version 1.0.2+
    */

    if( dictionary["frames"].getType() != Value::Type::MAP )
        return false;
    
    ValueMap& framesDict = dictionary["frames"].asValueMap();
    
 
    int format = 0;

    // get the format
    if (dictionary.find("metadata") != dictionary.end())
    {
        ValueMap& metadataDict = dictionary["metadata"].asValueMap();
        format = metadataDict["format"].asInt();
    }

    // check the format
    CCASSERT(format >=0 && format <= 3, "format is not supported for SpriteFrameCache addSpriteFramesWithDictionary:textureFilename:");

    auto textureFileName = Director::getInstance()->getTextureCache()->getTextureFilePath(texture);
    bool isGood = true;

    for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
    {
        ValueMap& frameDict = iter->second.asValueMap();
        std::string spriteFrameName = iter->first;
        SpriteFrame* spriteFrame = _spriteFrames.at(spriteFrameName);
        if (spriteFrame)
        {
            isGood = false;
            continue;
        }
        
        if(format == 0) 
        {
            float x = frameDict["x"].asFloat();
            float y = frameDict["y"].asFloat();
            float w = frameDict["width"].asFloat();
            float h = frameDict["height"].asFloat();
            float ox = frameDict["offsetX"].asFloat();
            float oy = frameDict["offsetY"].asFloat();
            int ow = frameDict["originalWidth"].asInt();
            int oh = frameDict["originalHeight"].asInt();
            // check ow/oh
            if(!ow || !oh)
            {
                CCLOGWARN("cocos2d: WARNING: originalWidth/Height not found on the SpriteFrame. AnchorPoint won't work as expected. Regenrate the .plist");
            }
            // abs ow/oh
            ow = abs(ow);
            oh = abs(oh);
            // create frame
            spriteFrame = SpriteFrame::createWithTexture(texture,
                                                         Rect(x, y, w, h),
                                                         false,
                                                         Vec2(ox, oy),
                                                         Size((float)ow, (float)oh)
                                                         );
        } 
        else if(format == 1 || format == 2) 
        {
            Rect frame = RectFromString(frameDict["frame"].asString());
            bool rotated = false;

            // rotation
            if (format == 2)
            {
                rotated = frameDict["rotated"].asBool();
            }

            Vec2 offset = PointFromString(frameDict["offset"].asString());
            Size sourceSize = SizeFromString(frameDict["sourceSize"].asString());

            // create frame
            spriteFrame = SpriteFrame::createWithTexture(texture,
                                                         frame,
                                                         rotated,
                                                         offset,
                                                         sourceSize
                                                         );
        } 
        else if (format == 3)
        {
            // get values
            Size spriteSize = SizeFromString(frameDict["spriteSize"].asString());
            Vec2 spriteOffset = PointFromString(frameDict["spriteOffset"].asString());
            Size spriteSourceSize = SizeFromString(frameDict["spriteSourceSize"].asString());
            Rect textureRect = RectFromString(frameDict["textureRect"].asString());
            bool textureRotated = frameDict["textureRotated"].asBool();

            // get aliases
            ValueVector& aliases = frameDict["aliases"].asValueVector();

            for(const auto &value : aliases) {
                std::string oneAlias = value.asString();
                if (_spriteFramesAliases.find(oneAlias) != _spriteFramesAliases.end())
                {
                    CCLOGWARN("cocos2d: WARNING: an alias with name %s already exists", oneAlias.c_str());
                }

                _spriteFramesAliases[oneAlias] = Value(spriteFrameName);
            }

            // create frame
            spriteFrame = SpriteFrame::createWithTexture(texture,
                                                         Rect(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
                                                         textureRotated,
                                                         spriteOffset,
                                                         spriteSourceSize);
        }

        // add sprite frame
        _spriteFrames.insert(spriteFrameName, spriteFrame);
        spriteFrame->setTextureFileName(spriteFrameName);
    }

    dictionary["isGood"] = Value(isGood);
#endif
    return true;
}

void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist, Texture2D *texture)
{
    std::string s = "addSpriteFramesWithFile" + plist;
    LOG_HELPER(s.c_str());
    
    if (_loadedFileNames->find(plist) != _loadedFileNames->end())
    {
        return; // We already added it
    }

    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(plist);
    
    // hope to RVO!!!
    ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(fullPath);

    addSpriteFramesWithDictionary(dict, texture);
    _loadedFileNames->insert(plist);
}

void SpriteFrameCache::addSpriteFramesWithFileContent(const std::string& plist_content, Texture2D *texture)
{
    ValueMap dict = FileUtils::getInstance()->getValueMapFromData(plist_content.c_str(), static_cast<int>(plist_content.size()));
    addSpriteFramesWithDictionary(dict, texture);
}

void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist, const std::string& textureFileName)
{
    CCASSERT(textureFileName.size()>0, "texture name should not be null");
    
    Director::getInstance()->getTextureCache()->EnableDefaultTexture(false);
    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(textureFileName);

    if (texture)
    {
        addSpriteFramesWithFile(plist, texture);
    }
    else
    {
        CCLOG("cocos2d: SpriteFrameCache: couldn't load texture file. File not found %s", textureFileName.c_str());
    }
    
     Director::getInstance()->getTextureCache()->EnableDefaultTexture(true);
}

void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist)
{
    CCASSERT(plist.size()>0, "plist filename should not be nullptr");
    
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(plist);
    if (fullPath.size() == 0)
    {
        // return if plist file doesn't exist
        CCLOG("cocos2d: SpriteFrameCache: can not find %s", plist.c_str());
        return;
    }

    // force 表示是否强制加载图片，譬如热更之类的文件，要强制刷一下现在的表
    if (_loadedFileNames->find(plist) != _loadedFileNames->end())
    {
        return;
    }
    
//    std::string s = "addSpriteFramesWithFile.." + plist;
//    LOG_HELPER(s.c_str());
    
    ValueMap dict;
    FileUtils::getInstance()->getValueMapFromFile(fullPath).swap(dict);
    
    string texturePath;
    Size textureSize;
    
    if (getSpriteFrameTexture(dict, texturePath, textureSize))
    {
        // build texture path relative to plist file
        texturePath = FileUtils::getInstance()->fullPathFromRelativeFile(texturePath.c_str(), plist);
    }
    else
    {
        if (getTextureFromPlist(plist, texturePath, textureSize) == false)
        {
            CCLOG("cocos2d: SpriteFrameCache: get texture error %s!!!", plist.c_str());
            return;
        }
    }
    
    std::string textFullPath = FileUtils::getInstance()->fullPathForFilename(texturePath);
    addSpriteFramesWithDictionary(dict, textFullPath);
    _loadedFileNames->insert(plist);

    //
    // 读取plist的时候不加载纹理，纹理在第一次加载的时候才缓式调用。
    // 如果加载plist的时候同时加载纹理，那么请打开下面代码
    // 个人见解：理论上frame和texture就不应该有太大的耦合，frame只是保存使用的哪张纹理的什么位置
    //         他的加载和使用，应该和纹理分开。惰性初始化理论上也适合我们项目
    //
#if 0
    ///////////////////////////
    bool isGood = false;

    TextureCache::sharedTextureCache()->EnableDefaultTexture(false);
    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(texturePath.c_str(), textureSize);
    TextureCache::sharedTextureCache()->EnableDefaultTexture(true);
    
    if (texture)
    {
        addSpriteFramesWithDictionary(dict, texture);
        _loadedFileNames->insert(plist);
        
        Value& resGood = dict["isGood"];
        if(!resGood.isNull()) {
            isGood = resGood.asBool();
        }
    }
    else
    {
        CCLOG("cocos2d: SpriteFrameCache: Couldn't load texture");
    }
#endif
    
    return;
}

void SpriteFrameCache::updateSpriteFramesWithFile(const std::string& plist)
{
    CCASSERT(plist.size()>0, "plist filename should not be nullptr");
    
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(plist);
    if (fullPath.size() == 0)
    {
        // return if plist file doesn't exist
        CCLOG("cocos2d: SpriteFrameCache: can not find %s", plist.c_str());
        return;
    }

//    std::string s = "addSpriteFramesWithFile.." + plist;
//    LOG_HELPER(s.c_str());
    
    ValueMap dict;
    FileUtils::getInstance()->getValueMapFromFile(fullPath).swap(dict);
    
    string texturePath;
    Size textureSize;
    
    if (getSpriteFrameTexture(dict, texturePath, textureSize))
    {
        // build texture path relative to plist file
        texturePath = FileUtils::getInstance()->fullPathFromRelativeFile(texturePath.c_str(), plist);
    }
    else
    {
        if (getTextureFromPlist(plist, texturePath, textureSize) == false)
        {
            CCLOG("cocos2d: SpriteFrameCache: get texture error %s!!!", plist.c_str());
            return;
        }
    }
    
    std::string textFullPath = FileUtils::getInstance()->fullPathForFilename(texturePath);
    addSpriteFramesWithDictionary(dict, textFullPath);
    _loadedFileNames->insert(plist);

    TextureCache::getInstance()->reloadTexture(texturePath);
    
    /////////////////////////////////////////////////////////
    // 更新处理一下
    if( dict["frames"].getType() != Value::Type::MAP )
        return;
    
    CCLOG("updateSpriteFramesWithFile - %s", plist.c_str());
    
    ValueMap& framesDict = dict["frames"].asValueMap();
    for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
    {
        if (iter->second.getType() != Value::Type::MAP) {
            assert(0);
            continue;
        }
        
        ValueMap& frameDict = iter->second.asValueMap();
        std::string const& spriteFrameName = iter->first;
        SpriteFrame* spriteFrame = _spriteFrames.at(spriteFrameName);
        if (spriteFrame) {
            CCSpriteManager::getInstance()->updateSpriteFrame(spriteFrame);
        }
    }
    
    return;
}


// 获取纹理
bool SpriteFrameCache::getSpriteFrameTexture(ValueMap& dict, std::string& texturePath, Size& textureSize)
{
    if (dict.find("metadata") != dict.end())
    {
        ValueMap& metadataDict = dict["metadata"].asValueMap();
        // try to read  texture file name from meta data
        texturePath = metadataDict["textureFileName"].asString();
        
        string strSize = metadataDict["size"].asString();
        textureSize = SizeFromString(strSize);
        
        return true;
    }

    return false;
}

bool SpriteFrameCache::getTextureFromPlist(std::string const& plist, std::string& texturePath, Size& textureSize)
{
    // remove .xxx
    size_t startPos = plist.find_last_of(".");
    if (startPos == std::string::npos) {
        return false;
    }
    
    texturePath = plist.substr(0, startPos);
    texturePath = texturePath.append(".png");
        
    CCLOG("cocos2d: SpriteFrameCache: Trying to use file %s as texture", texturePath.c_str());
    return true;
}

// 获取spriteFrame包含的纹理名称
bool SpriteFrameCache::getSpriteFrameTexture(const std::string& plist, std::string& textPath, Size& textSize)
{
    ValueMap dict;
    
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(plist);
    FileUtils::getInstance()->getValueMapFromFile(fullPath).swap(dict);
    if (dict.empty()) {
        CCLOG("getSpriteFrameTexture can not find %s", plist.c_str());
        return false;
    }
    
    string texturePath;
    Size textureSize;
    
    if (getSpriteFrameTexture(dict, texturePath, textureSize))
    {
        // build texture path relative to plist file
        texturePath = FileUtils::getInstance()->fullPathFromRelativeFile(texturePath.c_str(), plist);
    }
    else
    {
        if (getTextureFromPlist(plist, texturePath, textureSize) == false)
        {
            CCLOG("getSpriteFrameTexture get texture error %s!!!", plist.c_str());
            return false;
        }
    }
    
    textPath = texturePath;
    textSize = textureSize;
    
    return true;
}

bool SpriteFrameCache::isSpriteFramesWithFileLoaded(const std::string& plist) const
{
    bool result = false;

    if (_loadedFileNames->find(plist) != _loadedFileNames->end())
    {
        result = true;
    }

    return result;
}

void SpriteFrameCache::addSpriteFrame(SpriteFrame* frame, const std::string& frameName)
{
    _spriteFrames.insert(frameName, frame);
    frame->setTextureFileName(frameName);
}

void SpriteFrameCache::removeSpriteFrames()
{
    _spriteFrames.clear();
    _spriteFramesAliases.clear();
    _loadedFileNames->clear();
}

void SpriteFrameCache::dumpSpriteFrames()
{
    CCLOG("begin dumpSpriteFrames");
    
    for (auto iter = _spriteFrames.begin(); iter != _spriteFrames.end(); ++iter)
    {
        SpriteFrame* spriteFrame = iter->second;
        CCLOG("key: %s texture: %s", iter->first.c_str(), spriteFrame->getTexture()->getFileName().c_str());
    }
    
    CCLOG("end dumpSpriteFrames");
}

std::string SpriteFrameCache::getSpriteFramesInfo()
{
    std::string strInfos;
    
    for (auto iter = _spriteFrames.begin(); iter != _spriteFrames.end(); ++iter)
    {
        char temp[8192];
        SpriteFrame* spriteFrame = iter->second;
        sprintf(temp, "key: %s texture: %s\n", iter->first.c_str(), spriteFrame->getTexture()->getFileName().c_str());
        strInfos += temp;
    }
    
    return strInfos;
}

void SpriteFrameCache::removeUnusedSpriteFrames()
{
    // FIXME：因为退出的时候有时候崩溃，所以这里暂时不处理了。以后有时间再研究一下。
#if 0
    SAFE_FUN_LOCK;
    bool removed = false;
    std::vector<std::string> toRemoveFrames;
    
    for (auto iter = _spriteFrames.begin(); iter != _spriteFrames.end(); ++iter)
    {
        SpriteFrame* spriteFrame = iter->second;
        if( spriteFrame->getReferenceCount() == 1 )
        {
            toRemoveFrames.push_back(iter->first);
            
            std::string const& s = spriteFrame->getTextureFileName();
            Texture2D *texture = TextureCache::sharedTextureCache()->getTextureForKey(s);
            if (texture) {
                texture->removeSpriteFrameCapInset(spriteFrame);
            }
                
//            spriteFrame->getTexture()->removeSpriteFrameCapInset(spriteFrame);
//            CCLOG("cocos2d: SpriteFrameCache: removing unused frame: %s", iter->first.c_str());
            removed = true;
        }
    }

    _spriteFrames.erase(toRemoveFrames);
#endif
    
    // FIXME:. Since we don't know the .plist file that originated the frame, we must remove all .plist from the cache
//    if( removed )
    {
        _loadedFileNames->clear();
    }
}


void SpriteFrameCache::removeSpriteFrameByName(const std::string& name)
{
    // explicit nil handling
    if( !(name.size()>0) )
        return;

    // Is this an alias ?
    std::string key = _spriteFramesAliases[name].asString();

    if (!key.empty())
    {
        _spriteFrames.erase(key);
        _spriteFramesAliases.erase(key);
    }
    else
    {
        _spriteFrames.erase(name);
    }

    // FIXME:. Since we don't know the .plist file that originated the frame, we must remove all .plist from the cache
    _loadedFileNames->clear();
}

void SpriteFrameCache::removeSpriteFramesFromFile(const std::string& plist)
{
    //
    // 由于spriteFrame占用内存极小，所以这里暂时不对其进行删除了, lsy
    //
    
#if 0
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(plist);
    ValueMap dict;
    FileUtils::getInstance()->getValueMapFromFile(fullPath).swap(dict);
    
    if (dict.empty())
    {
        CCLOG("cocos2d:SpriteFrameCache:removeSpriteFramesFromFile: create dict by %s fail.",plist.c_str());
        return;
    }
    
    // get the removed the one to the cache
    removeSpriteFramesFromDictionary(dict);
    
    // remove it from the cache
    set<string>::iterator ret = _loadedFileNames->find(plist);
    if (ret != _loadedFileNames->end())
    {
        _loadedFileNames->erase(ret);
    }
#endif
}

void SpriteFrameCache::removeSpriteFramesFromFileContent(const std::string& plist_content)
{
    ValueMap dict = FileUtils::getInstance()->getValueMapFromData(plist_content.data(), static_cast<int>(plist_content.size()));
    if (dict.empty())
    {
        CCLOG("cocos2d:SpriteFrameCache:removeSpriteFramesFromFileContent: create dict by fail.");
        return;
    }
    removeSpriteFramesFromDictionary(dict);
}

void SpriteFrameCache::removeSpriteFramesFromDictionary(ValueMap& dictionary)
{
    if( dictionary["frames"].getType() != Value::Type::MAP )
        return;
    
    ValueMap& framesDict = dictionary["frames"].asValueMap();
    std::vector<std::string> keysToRemove;

    for (auto iter = framesDict.cbegin(); iter != framesDict.cend(); ++iter)
    {
        if (_spriteFrames.at(iter->first))
        {
            keysToRemove.push_back(iter->first);
        }
    }

    _spriteFrames.erase(keysToRemove);
}

void SpriteFrameCache::removeSpriteFramesFromTexture(Texture2D* texture)
{
    std::vector<std::string> keysToRemove;

    for (auto iter = _spriteFrames.cbegin(); iter != _spriteFrames.cend(); ++iter)
    {
        std::string key = iter->first;
        SpriteFrame* frame = _spriteFrames.at(key);
        if (frame && (frame->getTexture() == texture))
        {
            keysToRemove.push_back(key);
        }
    }

    _spriteFrames.erase(keysToRemove);
}

SpriteFrame* SpriteFrameCache::getSpriteFrameByName(const std::string& name)
{
    THREAD_PERF_NODE("SpriteFrameCache::getSpriteFrameByName");
    SpriteFrame* frame = _spriteFrames.at(name);
    if (!frame)
    {
        // try alias dictionary
        std::string key = _spriteFramesAliases[name].asString();
        if (!key.empty())
        {
            frame = _spriteFrames.at(key);
            if (!frame)
            {
                CCLOG("cocos2d: SpriteFrameCache: Frame '%s' not found", name.c_str());
            }
            else
            {
                refreshSpriteRecord(key);
            }
        }
    }
    else
    {
        refreshSpriteRecord(name);
    }
    return frame;
}

void SpriteFrameCache::removeSpriteRecord()
{
    _spriteRecord.clear();
    _tempSpriteRecord.clear();
    return;
}

void SpriteFrameCache::refreshSpriteRecord(const std::string& name)
{
    // 先屏蔽，等待code review!
#if 1
    auto iter = _spriteRecord.find(name);
    if(iter != _spriteRecord.end())
    {
        iter->second++;// 如果在之前记录中存在此纹理名称，计数加一
    }
    else
    {
        //_spriteRecord为所有加载过的纹理名称(全）
        // _tempSpriteRecord为每次向服务器发送的纹理名称集合(更新）
        _spriteRecord.insert(std::make_pair(name, 1));
        _tempSpriteRecord.insert(std::make_pair(name, 1));
        
    }
#endif
    return;
}

void SpriteFrameCache::getSpriteRecord(std::map<std::string, int> & m_tempSpriteRecord)
{
    //隔几分钟向服务器发送纹理集合名称，曾经发送过的名称不再发送，所以使用_tempSpriteRecord
    m_tempSpriteRecord = _tempSpriteRecord;
    
    _tempSpriteRecord.clear();//赋值后清零
    
    return;
}



// 通过纹理全路径获取index
int SpriteFrameCache::addTextureNameIndex(std::string const& strFullPath)
{
    for (int index=0; index<_allLoadedSFTextures.size(); ++index)
    {
        if (_allLoadedSFTextures[index] == strFullPath) {
            return index;
        }
    }
    
    _allLoadedSFTextures.push_back(strFullPath);
    return (int)_allLoadedSFTextures.size() - 1;
}

std::string const& SpriteFrameCache::getTextureNameFromIndex(int index)
{
    if (index >= 0 && index < _allLoadedSFTextures.size()) {
        return _allLoadedSFTextures[index];
    }
 
    CCLOG("SpriteFrameCache::getTextureNameFromIndex out of range(%d) total - %d.", index, (int)_allLoadedSFTextures.size());
    return gEmptyStr;
}

// 通过spriteframe的指针返回名称（仅调试用）
bool SpriteFrameCache::findSpriteFrameName(SpriteFrame* sf, std::string& name)
{
    if (sf == nullptr) {
        return false;
    }
    
    for (auto ite = _spriteFrames.begin(); ite != _spriteFrames.end(); ++ite)
    {
        if (ite->second == sf)
        {
            name = ite->first;
            return true;
        }
    }
    
    return false;
}

const std::set<std::string>* SpriteFrameCache::getLoadedFileNames(){
    return _loadedFileNames;
}

NS_CC_END
