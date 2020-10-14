/****************************************************************************
Copyright (c) 2009-2010 Ricardo Quesada
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
#include "2d/CCTMXTiledMap.h"
#include "2d/CCTMXXMLParser.h"
#include "2d/CCTMXLayer.h"
#include "2d/CCSprite.h"
#include "deprecated/CCString.h" // For StringUtils::format

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#define CC_ENABLE_LAYER_VM       1
#endif

#ifdef CC_ENABLE_LAYER_VM
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "platform/CCFileUtils.h"
#endif


#ifndef S_IRUSR
#define	S_IRUSR		0000400
#endif

#ifndef S_IWUSR
#define	S_IWUSR		0000200
#endif

NS_CC_BEGIN

// implementation TMXTiledMap
// Modified by ChenFei 2014-12-26 V3.2 support
//TMXTiledMap * TMXTiledMap::create(const std::string& tmxFile)
//{
//    TMXTiledMap *ret = new TMXTiledMap();
//    if (ret->initWithTMXFile(tmxFile))
//    {
//        ret->autorelease();
//        return ret;
//    }
//    CC_SAFE_DELETE(ret);
//    return nullptr;
//}
TMXTiledMap * TMXTiledMap::create(const std::string& tmxFile, const Point& pos)
{
    TMXTiledMap *ret = new TMXTiledMap();
    if (ret->initWithTMXFile(tmxFile.c_str(), pos, Point(0, 0)))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}


TMXTiledMap* TMXTiledMap::createWithXML(const std::string& tmxString, const std::string& resourcePath)
{
    TMXTiledMap *ret = new (std::nothrow) TMXTiledMap();
    if (ret->initWithXML(tmxString, resourcePath))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

//bool TMXTiledMap::initWithTMXFile(const std::string& tmxFile)
//{
//    CCASSERT(tmxFile.size()>0, "TMXTiledMap: tmx file should not be empty");
//    
//    setContentSize(Size::ZERO);
//
//    TMXMapInfo *mapInfo = TMXMapInfo::create(tmxFile);
//
//    if (! mapInfo)
//    {
//        return false;
//    }
//    CCASSERT( !mapInfo->getTilesets().empty(), "TMXTiledMap: Map not found. Please check the filename.");
//    buildWithMapInfo(mapInfo);
//
//    return true;
//}
//
//typedef std::unordered_map<std::string, TMXMapInfo*> TMXValueMap;


std::unordered_map<std::string, TMXMapInfo*> TMXTiledMap::mCaches;

void TMXTiledMap::setCacheMapInfo( const char *tmxFile, TMXMapInfo* tMapinfo )
{
#ifdef CC_ENABLE_LAYER_VM
    std::string path = FileUtils::getInstance()->getWritablePath();
    int index = 0;
    CCLOG("fileCached:%s",path.c_str());
    for ( auto& layinfo: tMapinfo->getLayers())
    {
        if( layinfo->_tilesBufLen > 0)
        {
            errno = 0;
            char filePath[512] = {0};
            sprintf(filePath, "%s%s_%d.mfl", path.c_str(),tmxFile,index++);
            CCLOG("fileCahced genpath: %s", filePath);
            int fd = open(filePath, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
            if(fd == -1)
            {
                CCLOG("fileCached open failed: %d",errno);
                return;
            }
            errno = 0;
            ssize_t len = layinfo->_tilesBufLen;
            ssize_t result = lseek(fd, len - 1, SEEK_SET);
            if (result == -1) {
                CCLOG("fileCached lseek failed: %d",errno);
                close(fd);
                return;
            }
            errno = 0;
            result = write(fd, "", 1);
            if (result != 1) {
                CCLOG("fileCached write failed: %d",errno);
                close(fd);
                return;
            }
            
            uint32_t* pUInt32 = (uint32_t*)mmap(NULL, len, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0 );
            if (pUInt32 == (uint32_t*)(-1)) {
                CCLOG("fileCached mmap failed: %d",errno);
                close(fd);
                return;
            }
            memcpy((void*)pUInt32, (const void*)layinfo->_tiles, len);
            free(layinfo->_tiles);
            layinfo->_tiles = pUInt32;
            //munmap(pInt32, len);
            close(fd);
        }
    }
    mCaches[tmxFile] = tMapinfo;
    tMapinfo->retain();
#endif
}

TMXMapInfo* TMXTiledMap::getCacheMapInfo( const char *tmxFile )
{
#ifdef CC_ENABLE_LAYER_VM
    clock_t start = clock();
    TMXMapInfo* tMapinfo = NULL;
    if( mCaches.find(tmxFile) != mCaches.end() )
    {
        tMapinfo = mCaches[tmxFile];
        std::string path = FileUtils::getInstance()->getWritablePath();
        int index = 0;
        for ( auto& layinfo: tMapinfo->getLayers())
        {
            if(layinfo->_tilesBufLen > 0 && layinfo->_tiles == NULL)
            {
                char filePath[512] = {0};
                sprintf(filePath, "%s%s_%d.mfl", path.c_str(),tmxFile,index++);
                int fd = open(filePath, O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
                if(fd == -1)
                {
                    return tMapinfo;
                }
                ssize_t len = layinfo->_tilesBufLen;
                uint32_t* pInt32 = (uint32_t*)mmap(NULL, len, PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0 );
                close(fd);
                layinfo->_tiles = pInt32;
            }
        }
    }
    if( tMapinfo != NULL )
    {
        CCLOG("read from cache time: %ld", (clock() - start));
    }
    return tMapinfo;
#else
    return NULL;
#endif
}

void TMXTiledMap::freeLayerInfo( void* pTiles )
{
#ifdef CC_ENABLE_LAYER_VM
    for (auto &item : mCaches) {
        TMXMapInfo* tMapInfo = item.second;
        for ( auto& layinfo: tMapInfo->getLayers())
        {
            if( layinfo->_tiles == pTiles)
            {
                ssize_t len = layinfo->_tilesBufLen;
                // donothing
                //munmap(pTiles, len);
                //layinfo->_tiles = NULL;
            }
        }
    }
#endif
}



bool TMXTiledMap::initWithTMXFile(const char *tmxFile, const cocos2d::Point &pos, const cocos2d::Point &startPt, int loopSize) {
    CCAssert(tmxFile != NULL && strlen(tmxFile)>0, "TMXTiledMap: tmx file should not bi NULL");
    
    setContentSize(Size::ZERO);
    
    TMXMapInfo *mapInfo = getCacheMapInfo(tmxFile);
    
    if( !mapInfo )
    {
        clock_t start = clock();
        mapInfo = TMXMapInfo::formatWithTMXFile(tmxFile);
        CCLOG("read from cache time(formatWithTMXFile): %ld", (clock() - start));
        if (! mapInfo)
        {
            return false;
        }
        setCacheMapInfo(tmxFile, mapInfo);
    }
    
   
    CCAssert( mapInfo->getTilesets().size() != 0, "TMXTiledMap: Map not found. Please check the filename.");
    
    _mapSize = mapInfo->getMapSize();
    _tileSize = mapInfo->getTileSize();
    _mapOrientation = mapInfo->getOrientation();
    
    _objectGroups = mapInfo->getObjectGroups();
    
    _properties = mapInfo->getProperties();
    
    _tileProperties = mapInfo->getTileProperties();
    
    int idx=0;
    
    Vector<TMXLayerInfo*>& layers = mapInfo->getLayers();
    if (layers.size()>0)
    {
        TMXLayerInfo* layerInfo = NULL;
        for(auto iter = layers.begin(); iter != layers.end(); iter++)
        {
            layerInfo = (TMXLayerInfo*)(*iter);
            if (layerInfo && layerInfo->_visible)
            {
                TMXTilesetInfo *tileset = tilesetForLayer(layerInfo, mapInfo);
                TMXLayer *child = TMXLayer::create(tileset, layerInfo, mapInfo, 448,loopSize);
                
                // tell the layerinfo to release the ownership of the tiles map.
                layerInfo->_ownTiles = false;
                child->getLayerName();
//                CCLOG("Dump layer.begin:%s", child->getLayerName().c_str());
                child->initSetupTiles();
                child->setupTilesByCoordinate(pos,startPt);
                
                addChild((Node*)child, idx, idx);
                
//                CCLOG("Dump layer.end:%s", child->getLayerName().c_str());
                // update content size with the max size
                const Size& childSize = child->getContentSize();
                Size currentSize = this->getContentSize();
                currentSize.width = MAX( currentSize.width, childSize.width );
                currentSize.height = MAX( currentSize.height, childSize.height );
                this->setContentSize(currentSize);
                
                idx++;
            }
        }
    }
    
    return true;
}

// Added by ChenFei 2014-12-26 V3.2 support
void TMXTiledMap::updateMap(const cocos2d::Point &pos, const cocos2d::Point &mapStartPt) {
    
    for(auto iter = _children.begin(); iter != _children.end(); ++iter)
    {
        auto layer = dynamic_cast<TMXLayer*>(*iter);
        if (!layer) {
            continue;
        }
        layer->setupTilesByCoordinate(pos, mapStartPt);
    }
}

bool TMXTiledMap::initWithXML(const std::string& tmxString, const std::string& resourcePath)
{
    setContentSize(Size::ZERO);

    TMXMapInfo *mapInfo = TMXMapInfo::createWithXML(tmxString, resourcePath);

    CCASSERT( !mapInfo->getTilesets().empty(), "TMXTiledMap: Map not found. Please check the filename.");
    buildWithMapInfo(mapInfo);

    return true;
}

TMXTiledMap::TMXTiledMap()
    :_mapSize(Size::ZERO)
    ,_tileSize(Size::ZERO)        
{
}

TMXTiledMap::~TMXTiledMap()
{
    for(auto& child : this->getChildren())
    {
        if( dynamic_cast<TMXLayer*>(child) != 0) {
            TMXLayer* pLayer = dynamic_cast<TMXLayer*>(child);
            freeLayerInfo(pLayer->getTiles());
            pLayer->setTiles(NULL);
        }
    }
}

// private
TMXLayer * TMXTiledMap::parseLayer(TMXLayerInfo *layerInfo, TMXMapInfo *mapInfo)
{
    TMXTilesetInfo *tileset = tilesetForLayer(layerInfo, mapInfo);
    if (tileset == nullptr)
        return nullptr;
    
    TMXLayer *layer = TMXLayer::create(tileset, layerInfo, mapInfo);

    // tell the layerinfo to release the ownership of the tiles map.
    layerInfo->_ownTiles = false;
    layer->setupTiles();

    return layer;
}

TMXTilesetInfo * TMXTiledMap::tilesetForLayer(TMXLayerInfo *layerInfo, TMXMapInfo *mapInfo)
{
    Size size = layerInfo->_layerSize;
    auto& tilesets = mapInfo->getTilesets();
    if (tilesets.size()>0)
    {
        TMXTilesetInfo* tileset = nullptr;
        for (auto iter = tilesets.crbegin(); iter != tilesets.crend(); ++iter)
        {
            tileset = *iter;
            if (tileset)
            {
                for( int y=0; y < size.height; y++ )
                {
                    for( int x=0; x < size.width; x++ )
                    {
                        int pos = static_cast<int>(x + size.width * y);
                        int gid = layerInfo->_tiles[ pos ];

                        // gid are stored in little endian.
                        // if host is big endian, then swap
                        //if( o == CFByteOrderBigEndian )
                        //    gid = CFSwapInt32( gid );
                        /* We support little endian.*/

                        // FIXME:: gid == 0 --> empty tile
                        if( gid != 0 ) 
                        {
                            // Optimization: quick return
                            // if the layer is invalid (more than 1 tileset per layer) an CCAssert will be thrown later
                            if( (gid & kTMXFlippedMask) >= tileset->_firstGid )
                                return tileset;
                        }
                    }
                }        
            }
        }
    }

    // If all the tiles are 0, return empty tileset
    CCLOG("cocos2d: Warning: TMX Layer '%s' has no tiles", layerInfo->_name.c_str());
    return nullptr;
}

void TMXTiledMap::buildWithMapInfo(TMXMapInfo* mapInfo)
{
    _mapSize = mapInfo->getMapSize();
    _tileSize = mapInfo->getTileSize();
    _mapOrientation = mapInfo->getOrientation();

    _objectGroups = mapInfo->getObjectGroups();

    _properties = mapInfo->getProperties();

    _tileProperties = mapInfo->getTileProperties();

    int idx=0;

    auto& layers = mapInfo->getLayers();
    for(const auto &layerInfo : layers) {
        if (layerInfo->_visible) {
            TMXLayer *child = parseLayer(layerInfo, mapInfo);
            if (child == nullptr) {
                idx++;
                continue;
            }
            addChild(child, idx, idx);
            
            // update content size with the max size
            const Size& childSize = child->getContentSize();
            Size currentSize = this->getContentSize();
            currentSize.width = std::max( currentSize.width, childSize.width );
            currentSize.height = std::max( currentSize.height, childSize.height );
            this->setContentSize(currentSize);
            
            idx++;
        }
    }
}

// public
TMXLayer * TMXTiledMap::getLayer(const std::string& layerName) const
{
    CCASSERT(layerName.size() > 0, "Invalid layer name!");
    
    for (auto& child : _children)
    {
        TMXLayer* layer = dynamic_cast<TMXLayer*>(child);
        if(layer)
        {
            if(layerName.compare( layer->getLayerName()) == 0)
            {
                return layer;
            }
        }
    }

    // layer not found
    return nullptr;
}

TMXObjectGroup * TMXTiledMap::getObjectGroup(const std::string& groupName) const
{
    CCASSERT(groupName.size() > 0, "Invalid group name!");

    if (_objectGroups.size()>0)
    {
        TMXObjectGroup* objectGroup = nullptr;
        for (auto iter = _objectGroups.cbegin(); iter != _objectGroups.cend(); ++iter)
        {
            objectGroup = *iter;
            if (objectGroup && objectGroup->getGroupName() == groupName)
            {
                return objectGroup;
            }
        }
    }

    // objectGroup not found
    return nullptr;
}

Value TMXTiledMap::getProperty(const std::string& propertyName) const
{
    if (_properties.find(propertyName) != _properties.end())
        return _properties.at(propertyName);
    
    return Value();
}

Value TMXTiledMap::getPropertiesForGID(int GID) const
{
    if (_tileProperties.find(GID) != _tileProperties.end())
        return _tileProperties.at(GID);
    
    return Value();
}

bool TMXTiledMap::getPropertiesForGID(int GID, Value** value)
{
    if (_tileProperties.find(GID) != _tileProperties.end()) {
        *value = &_tileProperties.at(GID);
        return true;
    } else {
        return false;
    }
}

std::string TMXTiledMap::getDescription() const
{
    return StringUtils::format("<TMXTiledMap | Tag = %d, Layers = %d", _tag, static_cast<int>(_children.size()));
}


NS_CC_END
