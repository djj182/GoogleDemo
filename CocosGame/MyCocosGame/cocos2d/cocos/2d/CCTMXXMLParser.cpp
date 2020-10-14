/****************************************************************************
Copyright (c) 2011      Максим Аксенов 
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

#include "2d/CCTMXXMLParser.h"
#include <unordered_map>
#include <sstream>
#include "2d/CCTMXTiledMap.h"
#include "base/ZipUtils.h"
#include "base/base64.h"
#include "base/CCDirector.h"
#include "platform/CCFileUtils.h"
// Added by ChenFei 2014-12-26 V3.2 support
#include "deprecated/CCString.h"

using namespace std;

NS_CC_BEGIN

// implementation TMXLayerInfo
TMXLayerInfo::TMXLayerInfo()
: _name("")
, _tiles(nullptr)
, _ownTiles(true)
, _tilesBufLen(0)
{
}

TMXLayerInfo::~TMXLayerInfo()
{
    CCLOGINFO("deallocing TMXLayerInfo: %p", this);
    if (_ownTiles && _tiles)
    {
        free(_tiles);
        _tiles = nullptr;
    }
}

ValueMap& TMXLayerInfo::getProperties()
{
    return _properties;
}

void TMXLayerInfo::setProperties(ValueMap var)
{
    _properties = var;
}

// implementation TMXTilesetInfo
TMXTilesetInfo::TMXTilesetInfo()
    :_firstGid(0)
    ,_tileSize(Size::ZERO)
    ,_spacing(0)
    ,_margin(0)
    ,_imageSize(Size::ZERO)
// Added by ChenFei 2014-12-26 V3.2 support
, m_terrainInfoDict(NULL)
{
}

TMXTilesetInfo::~TMXTilesetInfo()
{
    // Added by ChenFei 2014-12-26 V3.2 support
    CC_SAFE_RELEASE(m_terrainInfoDict);
    CCLOGINFO("deallocing TMXTilesetInfo: %p", this);
}

Rect TMXTilesetInfo::getRectForGID(uint32_t gid)
{
    Rect rect;
    rect.size = _tileSize;
    gid &= kTMXFlippedMask;
    gid = gid - _firstGid;
    int max_x = (int)((_imageSize.width - _margin*2 + _spacing) / (_tileSize.width + _spacing));
    // 以防除零
    if( max_x == 0 )
        max_x = 1;
    
    //    int max_y = (imageSize.height - margin*2 + spacing) / (tileSize.height + spacing);
    rect.origin.x = (gid % max_x) * (_tileSize.width + _spacing) + _margin;
    rect.origin.y = (gid / max_x) * (_tileSize.height + _spacing) + _margin;
    return rect;
}

// implementation TMXMapInfo

TMXMapInfo * TMXMapInfo::create(const std::string& tmxFile)
{
    TMXMapInfo *ret = new (std::nothrow) TMXMapInfo();
    if (ret->initWithTMXFile(tmxFile))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

TMXMapInfo * TMXMapInfo::createWithXML(const std::string& tmxString, const std::string& resourcePath)
{
    TMXMapInfo *ret = new (std::nothrow) TMXMapInfo();
    if (ret->initWithXML(tmxString, resourcePath))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void TMXMapInfo::internalInit(const std::string& tmxFileName, const std::string& resourcePath)
{
    if (!tmxFileName.empty())
    {
        _TMXFileName = FileUtils::getInstance()->fullPathForFilename(tmxFileName);
    }
    
    if (!resourcePath.empty())
    {
        _resources = resourcePath;
    }
    
    _objectGroups.reserve(4);

    // tmp vars
    _currentString = "";
    _storingCharacters = false;
    _layerAttribs = TMXLayerAttribNone;
    _parentElement = TMXPropertyNone;
    _currentFirstGID = -1;
}

bool TMXMapInfo::initWithXML(const std::string& tmxString, const std::string& resourcePath)
{
    internalInit("", resourcePath);
    return parseXMLString(tmxString);
}

bool TMXMapInfo::initWithTMXFile(const std::string& tmxFile)
{
    internalInit(tmxFile, "");
    return parseXMLFile(_TMXFileName.c_str());
}

TMXMapInfo::TMXMapInfo()
: _mapSize(Size::ZERO)    
, _tileSize(Size::ZERO)
, _layerAttribs(0)
, _storingCharacters(false)
, _xmlTileIndex(0)
, _currentFirstGID(-1)
, _recordFirstGID(true)
{
}

TMXMapInfo::~TMXMapInfo()
{
    CCLOGINFO("deallocing TMXMapInfo: %p", this);
}

bool TMXMapInfo::parseXMLString(const std::string& xmlString)
{
    size_t len = xmlString.size();
    if (len <= 0)
        return false;

    SAXParser parser;

    if (false == parser.init("UTF-8") )
    {
        return false;
    }

    parser.setDelegator(this);

    return parser.parse(xmlString.c_str(), len);
}

bool TMXMapInfo::parseXMLFile(const std::string& xmlFilename)
{
    SAXParser parser;
    
    if (false == parser.init("UTF-8") )
    {
        return false;
    }
    
    parser.setDelegator(this);

    return parser.parse(FileUtils::getInstance()->fullPathForFilename(xmlFilename).c_str());
}

// the XML parser calls here with all the elements
void TMXMapInfo::startElement(void *ctx, const char *name, const char **atts)
{    
    CC_UNUSED_PARAM(ctx);
    TMXMapInfo *tmxMapInfo = this;
    std::string elementName = name;
    ValueMap attributeDict;
    if (atts && atts[0])
    {
        for (int i = 0; atts[i]; i += 2)
        {
            std::string key = atts[i];
            std::string value = atts[i+1];
            attributeDict.insert(std::make_pair(key, Value(value)));
        }
    }
    if (elementName == "map")
    {
        std::string version = attributeDict["version"].asString();
        if ( version != "1.0")
        {
            CCLOG("cocos2d: TMXFormat: Unsupported TMX version: %s", version.c_str());
        }
        std::string orientationStr = attributeDict["orientation"].asString();
        if (orientationStr == "orthogonal") {
            tmxMapInfo->setOrientation(TMXOrientationOrtho);
        }
        else if (orientationStr  == "isometric") {
            tmxMapInfo->setOrientation(TMXOrientationIso);
        }
        else if (orientationStr == "hexagonal") {
            tmxMapInfo->setOrientation(TMXOrientationHex);
        }
        else if (orientationStr == "staggered") {
            tmxMapInfo->setOrientation(TMXOrientationStaggered);
        }
        else {
            CCLOG("cocos2d: TMXFomat: Unsupported orientation: %d", tmxMapInfo->getOrientation());
        }

        Size s;
        s.width = attributeDict["width"].asFloat();
        s.height = attributeDict["height"].asFloat();
        tmxMapInfo->setMapSize(s);

        s.width = attributeDict["tilewidth"].asFloat();
        s.height = attributeDict["tileheight"].asFloat();
        tmxMapInfo->setTileSize(s);

        // The parent element is now "map"
        tmxMapInfo->setParentElement(TMXPropertyMap);
    } 
    else if (elementName == "tileset") 
    {
        // If this is an external tileset then start parsing that
        std::string externalTilesetFilename = attributeDict["source"].asString();
        if (externalTilesetFilename != "")
        {
            // Tileset file will be relative to the map file. So we need to convert it to an absolute path
            if (_TMXFileName.find_last_of("/") != string::npos)
            {
                string dir = _TMXFileName.substr(0, _TMXFileName.find_last_of("/") + 1);
                externalTilesetFilename = dir + externalTilesetFilename;
            }
            else 
            {
                externalTilesetFilename = _resources + "/" + externalTilesetFilename;
            }
            externalTilesetFilename = FileUtils::getInstance()->fullPathForFilename(externalTilesetFilename.c_str());
            
            _currentFirstGID = attributeDict["firstgid"].asInt();
            if (_currentFirstGID < 0)
            {
                _currentFirstGID = 0;
            }
            _recordFirstGID = false;
            
            tmxMapInfo->parseXMLFile(externalTilesetFilename.c_str());
        }
        else
        {
            TMXTilesetInfo *tileset = new (std::nothrow) TMXTilesetInfo();
            tileset->_name = attributeDict["name"].asString();
            
            if (_recordFirstGID)
            {
                // unset before, so this is tmx file.
                tileset->_firstGid = attributeDict["firstgid"].asInt();
                
                if (tileset->_firstGid < 0)
                {
                    tileset->_firstGid = 0;
                }
            }
            else
            {
                tileset->_firstGid = _currentFirstGID;
                _currentFirstGID = 0;
            }
            
            tileset->_spacing = attributeDict["spacing"].asInt();
            tileset->_margin = attributeDict["margin"].asInt();
            Size s;
            s.width = attributeDict["tilewidth"].asFloat();
            s.height = attributeDict["tileheight"].asFloat();
            tileset->_tileSize = s;

            tmxMapInfo->getTilesets().pushBack(tileset);
            tileset->release();
        }
    }
    else if (elementName == "tile")
    {
        if (tmxMapInfo->getParentElement() == TMXPropertyLayer)
        {
            TMXLayerInfo* layer = tmxMapInfo->getLayers().back();
            Size layerSize = layer->_layerSize;
            uint32_t gid = static_cast<uint32_t>(attributeDict["gid"].asInt());
            int tilesAmount = layerSize.width*layerSize.height;
            
            if (_xmlTileIndex < tilesAmount)
            {
                layer->_tiles[_xmlTileIndex++] = gid;
            }
        }
        else
        {
            TMXTilesetInfo* info = tmxMapInfo->getTilesets().back();
            tmxMapInfo->setParentGID(info->_firstGid + attributeDict["id"].asInt());
            
            //add by liudi from chenfei
             ValueMap dict;
            // parse terrain info
            // format : left-up,right-up,left-bottom,right-bottom (param is TID or blank)
            string strterrainInfo = attributeDict["terrain"].asString();
            auto terrainInfo = strterrainInfo.c_str();
            if (strcmp(terrainInfo,"") != 0) {
                ValueVector terrain;
                dict["terrain"] = terrain;
                char *tmpStore = new char[512];
                char *tmp = tmpStore;
                strcpy(tmp, terrainInfo);
                while (tmp != NULL) {
                    char* del = strstr(tmp, ",");
                    if (del == NULL || strcmp(del, tmp) != 0) {
                        int tid = atoi(tmp);
                        auto tname = info->m_terrainNames[tid];
                        terrain.push_back(Value(tname));
                    } else {
                        terrain.push_back(Value(""));
                        ++del;
                    }
                    tmp = del;
                }
                delete [] tmpStore;
                dict["terrain"] = terrain;
            }
            
            tmxMapInfo->getTileProperties()[tmxMapInfo->getParentGID()] = dict;
            tmxMapInfo->setParentElement(TMXPropertyTile);
        }
    }
    else if (elementName == "layer")
    {
        TMXLayerInfo *layer = new (std::nothrow) TMXLayerInfo();
        layer->_name = attributeDict["name"].asString();

        Size s;
        s.width = attributeDict["width"].asFloat();
        s.height = attributeDict["height"].asFloat();
        layer->_layerSize = s;

        Value& visibleValue = attributeDict["visible"];
        layer->_visible = visibleValue.isNull() ? true : visibleValue.asBool();

        Value& opacityValue = attributeDict["opacity"];
        layer->_opacity = opacityValue.isNull() ? 255 : (unsigned char)(255.0f * opacityValue.asFloat());

        float x = attributeDict["x"].asFloat();
        float y = attributeDict["y"].asFloat();
        layer->_offset.set(x, y);

        tmxMapInfo->getLayers().pushBack(layer);
        layer->release();

        // The parent element is now "layer"
        tmxMapInfo->setParentElement(TMXPropertyLayer);
    } 
    else if (elementName == "objectgroup")
    {
        TMXObjectGroup *objectGroup = new (std::nothrow) TMXObjectGroup();
        objectGroup->setGroupName(attributeDict["name"].asString());
        Vec2 positionOffset;
        positionOffset.x = attributeDict["x"].asFloat() * tmxMapInfo->getTileSize().width;
        positionOffset.y = attributeDict["y"].asFloat() * tmxMapInfo->getTileSize().height;
        objectGroup->setPositionOffset(positionOffset);

        tmxMapInfo->getObjectGroups().pushBack(objectGroup);
        objectGroup->release();

        // The parent element is now "objectgroup"
        tmxMapInfo->setParentElement(TMXPropertyObjectGroup);
    }
    else if (elementName == "image")
    {
        TMXTilesetInfo* tileset = tmxMapInfo->getTilesets().back();

        // build full path
        std::string imagename = attributeDict["source"].asString();

        if (_TMXFileName.find_last_of("/") != string::npos)
        {
            string dir = _TMXFileName.substr(0, _TMXFileName.find_last_of("/") + 1);
            tileset->_sourceImage = dir + imagename;
        }
        else 
        {
            tileset->_sourceImage = _resources + (_resources.size() ? "/" : "") + imagename;
        }
        // Added by ChenFei 2014-12-26 V3.2 support
        tileset->_imageName = imagename;
    }
    else if (elementName == "data")
    {
        std::string encoding = attributeDict["encoding"].asString();
        std::string compression = attributeDict["compression"].asString();

        if (encoding == "")
        {
            tmxMapInfo->setLayerAttribs(tmxMapInfo->getLayerAttribs() | TMXLayerAttribNone);
            
            TMXLayerInfo* layer = tmxMapInfo->getLayers().back();
            Size layerSize = layer->_layerSize;
            int tilesAmount = layerSize.width*layerSize.height;

            uint32_t *tiles = (uint32_t*) malloc(tilesAmount*sizeof(uint32_t));
            // set all value to 0
            memset(tiles, 0, tilesAmount*sizeof(int));

            layer->_tiles = tiles;
            layer->_tilesBufLen = tilesAmount*sizeof(uint32_t);
        }
        else if (encoding == "base64")
        {
            int layerAttribs = tmxMapInfo->getLayerAttribs();
            tmxMapInfo->setLayerAttribs(layerAttribs | TMXLayerAttribBase64);
            tmxMapInfo->setStoringCharacters(true);

            if (compression == "gzip")
            {
                layerAttribs = tmxMapInfo->getLayerAttribs();
                tmxMapInfo->setLayerAttribs(layerAttribs | TMXLayerAttribGzip);
            } else
            if (compression == "zlib")
            {
                layerAttribs = tmxMapInfo->getLayerAttribs();
                tmxMapInfo->setLayerAttribs(layerAttribs | TMXLayerAttribZlib);
            }
            CCASSERT( compression == "" || compression == "gzip" || compression == "zlib", "TMX: unsupported compression method" );
        }
    } 
    else if (elementName == "object")
    {
        TMXObjectGroup* objectGroup = tmxMapInfo->getObjectGroups().back();

        // The value for "type" was blank or not a valid class name
        // Create an instance of TMXObjectInfo to store the object and its properties
        ValueMap dict;
        // Parse everything automatically
        const char* keys[] = {"name", "type", "width", "height", "gid"};
        
        for (const auto& key : keys)
        {
            Value value = attributeDict[key];
            dict[key] = value;
            
            // Added by ChenFei 2014-12-26 V3.2 support
            // --------------  added by jimin START -------------- //
            if (strcmp("gid", key) == 0) {
                int gid = value.asInt();
                Vector<TMXTilesetInfo*>& tileSets = tmxMapInfo->getTilesets();
                
                auto tilesetCount = tileSets.size();
                for (auto j = 0; j < tilesetCount; j ++)
                {
                    TMXTilesetInfo* tileSet = (TMXTilesetInfo*)tileSets.at(j);
                    if (gid == tileSet->_firstGid) {
                        
                        dict["file"] = Value(tileSet->_imageName.c_str());
                        break;
                    }
                    
                    if (gid < tileSet->_firstGid && j > 0) {
                        // 要取上一个guid
                        TMXTilesetInfo* lastTileSet = (TMXTilesetInfo*)tileSets.at(j - 1);
                        dict["file"] = Value(lastTileSet->_imageName.c_str());
                        break;
                    }
                }
            }
        }

        // But X and Y since they need special treatment
        // X
        int x = attributeDict["x"].asInt();
        // Y
        int y = attributeDict["y"].asInt();
        
        Vec2 p(x + objectGroup->getPositionOffset().x, _mapSize.height * _tileSize.height - y  - objectGroup->getPositionOffset().y - attributeDict["height"].asInt());
        p = CC_POINT_PIXELS_TO_POINTS(p);
        dict["x"] = Value(p.x);
        dict["y"] = Value(p.y);
        
        int width = attributeDict["width"].asInt();
        int height = attributeDict["height"].asInt();
        Size s(width, height);
        s = CC_SIZE_PIXELS_TO_POINTS(s);
        dict["width"] = Value(s.width);
        dict["height"] = Value(s.height);

        // Add the object to the objectGroup
        objectGroup->getObjects().push_back(Value(dict));

        // The parent element is now "object"
        tmxMapInfo->setParentElement(TMXPropertyObject);
    } 
    else if (elementName == "property")
    {
        if ( tmxMapInfo->getParentElement() == TMXPropertyNone ) 
        {
            CCLOG( "TMX tile map: Parent element is unsupported. Cannot add property named '%s' with value '%s'",
                  attributeDict["name"].asString().c_str(), attributeDict["value"].asString().c_str() );
        } 
        else if ( tmxMapInfo->getParentElement() == TMXPropertyMap )
        {
            // The parent element is the map
            Value value = attributeDict["value"];
            std::string key = attributeDict["name"].asString();
            tmxMapInfo->getProperties().insert(std::make_pair(key, value));
        }
        else if ( tmxMapInfo->getParentElement() == TMXPropertyLayer )
        {
            // The parent element is the last layer
            TMXLayerInfo* layer = tmxMapInfo->getLayers().back();
            Value value = attributeDict["value"];
            std::string key = attributeDict["name"].asString();
            // Add the property to the layer
            layer->getProperties().insert(std::make_pair(key, value));
        }
        else if ( tmxMapInfo->getParentElement() == TMXPropertyObjectGroup ) 
        {
            // The parent element is the last object group
            TMXObjectGroup* objectGroup = tmxMapInfo->getObjectGroups().back();
            Value value = attributeDict["value"];
            std::string key = attributeDict["name"].asString();
            objectGroup->getProperties().insert(std::make_pair(key, value));
        }
        else if ( tmxMapInfo->getParentElement() == TMXPropertyObject )
        {
            // The parent element is the last object
            TMXObjectGroup* objectGroup = tmxMapInfo->getObjectGroups().back();
            ValueMap& dict = objectGroup->getObjects().rbegin()->asValueMap();

            std::string propertyName = attributeDict["name"].asString();
            dict[propertyName] = attributeDict["value"];
        }
        else if ( tmxMapInfo->getParentElement() == TMXPropertyTile ) 
        {
            ValueMap& dict = tmxMapInfo->getTileProperties().at(tmxMapInfo->getParentGID()).asValueMap();

            std::string propertyName = attributeDict["name"].asString();
            dict[propertyName] = attributeDict["value"];
        }
        
        // Added by ChenFei 2014-12-26 V3.2 support
        else if ( tmxMapInfo->getParentElement() == TMXPropertyTerrain )
        {
            TMXTilesetInfo* tileset = (TMXTilesetInfo*)tmxMapInfo->getTilesets().at(tmxMapInfo->getTilesets().size() - 1);
            int tid = tmxMapInfo->getParentTID();
            auto tName = tileset->m_terrainNames[tid];
            if (!tName.empty()) {
                auto dict = dynamic_cast<__Dictionary*>(tileset->m_terrainInfoDict->objectForKey(tName));
                if (dict) {
//                    const char* propertyName = attributeDict["name"].asString().c_str();
                    __String *propertyValue = new __String(attributeDict["value"].asString().c_str());
                    dict->setObject(propertyValue,  attributeDict["name"].asString());
                    propertyValue->release();
                }
            }
            
        }
    }
    else if (elementName == "polygon") 
    {
        // find parent object's dict and add polygon-points to it
        TMXObjectGroup* objectGroup = _objectGroups.back();
        ValueMap& dict = objectGroup->getObjects().rbegin()->asValueMap();

        // get points value string
        std::string value = attributeDict["points"].asString();
        if (!value.empty())
        {
            
            // Added by ChenFei 2014-12-26 V3.2 support
            auto position = Point();
            position.x = atof(dict["x"].asString().c_str());
            position.y = atof(dict["y"].asString().c_str());
            
            ValueVector pointsArray;
            pointsArray.reserve(10);

            // parse points string into a space-separated set of points
            stringstream pointsStream(value);
            string pointPair;
            while (std::getline(pointsStream, pointPair, ' '))
            {
                // parse each point combo into a comma-separated x,y point
                stringstream pointStream(pointPair);
                string xStr, yStr;
                
                ValueMap pointDict;

                // set x
                if (std::getline(pointStream, xStr, ','))
                {
                    // Modified by ChenFei 2014-12-26 V3.2 support
                    //                    int x = atoi(xStr.c_str()) + (int)objectGroup->getPositionOffset().x;
                    int x = atoi(xStr.c_str()) + position.x;
                    pointDict["x"] = Value(x);
                }

                // set y
                if (std::getline(pointStream, yStr, ','))
                {
                    // Modified by ChenFei 2014-12-26 V3.2 support
                    //                    int y = atoi(yStr.c_str()) + (int)objectGroup->getPositionOffset().y;
                    int y = -atoi(yStr.c_str()) + position.y;
                    pointDict["y"] = Value(y);
                }
                
                // add to points array
                pointsArray.push_back(Value(pointDict));
            }
            
            dict["points"] = Value(pointsArray);
        }
    } 
    else if (elementName == "polyline")
    {
        // find parent object's dict and add polyline-points to it
        TMXObjectGroup* objectGroup = _objectGroups.back();
        ValueMap& dict = objectGroup->getObjects().rbegin()->asValueMap();
        
        // get points value string
        std::string value = attributeDict["points"].asString();
        if (!value.empty())
        {
            ValueVector pointsArray;
            pointsArray.reserve(10);
            
            // parse points string into a space-separated set of points
            stringstream pointsStream(value);
            string pointPair;
            while (std::getline(pointsStream, pointPair, ' '))
            {
                // parse each point combo into a comma-separated x,y point
                stringstream pointStream(pointPair);
                string xStr, yStr;
                
                ValueMap pointDict;
                
                // set x
                if (std::getline(pointStream, xStr, ','))
                {
                    int x = atoi(xStr.c_str()) + (int)objectGroup->getPositionOffset().x;
                    pointDict["x"] = Value(x);
                }
                
                // set y
                if (std::getline(pointStream, yStr, ','))
                {
                    int y = atoi(yStr.c_str()) + (int)objectGroup->getPositionOffset().y;
                    pointDict["y"] = Value(y);
                }
                
                // add to points array
                pointsArray.push_back(Value(pointDict));
            }
            
            dict["polylinePoints"] = Value(pointsArray);
        }
    }
    
    // Added by ChenFei 2014-12-26 V3.2 support
    else if (elementName == "terraintypes")
    {
        TMXTilesetInfo* tileset = (TMXTilesetInfo*)tmxMapInfo->getTilesets().back();
        tileset->m_terrainInfoDict = new __Dictionary();
        tileset->m_terrainNames.clear();
        
        tmxMapInfo->setParentElement(TMXPropertyTerrainTypes);
        tmxMapInfo->setParentTID(-1);
    }
    // Added by ChenFei 2014-12-26 V3.2 support
    else if (elementName == "terrain")
    {
        TMXTilesetInfo* tileset = (TMXTilesetInfo*)tmxMapInfo->getTilesets().back();
        if (tileset->m_terrainInfoDict) {
            auto dict = new __Dictionary();
            std::string terrainName = attributeDict["name"].asString();
            unsigned int terrainId = tmxMapInfo->getParentTID()+1;
            tileset->m_terrainInfoDict->setObject(dict, terrainName);
            tileset->m_terrainNames.push_back(terrainName);
            dict->release();
            
            tmxMapInfo->setParentElement(TMXPropertyTerrain);
            tmxMapInfo->setParentTID(terrainId);
        }
    }
    
}

void TMXMapInfo::endElement(void *ctx, const char *name)
{
    CC_UNUSED_PARAM(ctx);
    TMXMapInfo *tmxMapInfo = this;
    std::string elementName = name;

    if (elementName == "data")
    {
        if (tmxMapInfo->getLayerAttribs() & TMXLayerAttribBase64)
        {
            tmxMapInfo->setStoringCharacters(false);
            
            TMXLayerInfo* layer = tmxMapInfo->getLayers().back();
            
            std::string currentString = tmxMapInfo->getCurrentString();
            unsigned char *buffer;
            auto len = base64Decode((unsigned char*)currentString.c_str(), (unsigned int)currentString.length(), &buffer);
            if (!buffer)
            {
                CCLOG("cocos2d: TiledMap: decode data error");
                return;
            }
            
            if (tmxMapInfo->getLayerAttribs() & (TMXLayerAttribGzip | TMXLayerAttribZlib))
            {
                unsigned char *deflated = nullptr;
                Size s = layer->_layerSize;
                // int sizeHint = s.width * s.height * sizeof(uint32_t);
                ssize_t sizeHint = s.width * s.height * sizeof(unsigned int);
                
                ssize_t CC_UNUSED inflatedLen = ZipUtils::inflateMemoryWithHint(buffer, len, &deflated, sizeHint);
                CCASSERT(inflatedLen == sizeHint, "");
                
                free(buffer);
                buffer = nullptr;
                
                if (!deflated)
                {
                    CCLOG("cocos2d: TiledMap: inflate data error");
                    return;
                }
                
                layer->_tiles = reinterpret_cast<uint32_t*>(deflated);
                layer->_tilesBufLen = inflatedLen;
            }
            else
            {
                layer->_tiles = reinterpret_cast<uint32_t*>(buffer);
                layer->_tilesBufLen = len;
            }
            
            tmxMapInfo->setCurrentString("");
        }
        else if (tmxMapInfo->getLayerAttribs() & TMXLayerAttribNone)
        {
            _xmlTileIndex = 0;
        }
    }
    else if (elementName == "map")
    {
        // The map element has ended
        tmxMapInfo->setParentElement(TMXPropertyNone);
    }    
    else if (elementName == "layer")
    {
        // The layer element has ended
        tmxMapInfo->setParentElement(TMXPropertyNone);
    }
    else if (elementName == "objectgroup")
    {
        // The objectgroup element has ended
        tmxMapInfo->setParentElement(TMXPropertyNone);
    } 
    else if (elementName == "object") 
    {
        // The object element has ended
        tmxMapInfo->setParentElement(TMXPropertyNone);
    }
    else if (elementName == "tileset")
    {
        _recordFirstGID = true;
    }
    
    // Added by ChenFei 2014-12-26 V3.2 support
    else if (elementName == "terrainTypes")
    {
        tmxMapInfo->setParentElement(TMXPropertyNone);
    }
    // Added by ChenFei 2014-12-26 V3.2 support
    else if (elementName == "terrain")
    {
        tmxMapInfo->setParentElement(TMXPropertyNone);
    }
    
}

void TMXMapInfo::textHandler(void *ctx, const char *ch, size_t len)
{
    CC_UNUSED_PARAM(ctx);
    TMXMapInfo *tmxMapInfo = this;
    std::string text(ch, 0, len);

    if (tmxMapInfo->isStoringCharacters())
    {
        std::string currentString = tmxMapInfo->getCurrentString();
        currentString += text;
        tmxMapInfo->setCurrentString(currentString.c_str());
    }
}

NS_CC_END
