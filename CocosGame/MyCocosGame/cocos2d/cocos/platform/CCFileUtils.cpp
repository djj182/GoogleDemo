/****************************************************************************
Copyright (c) 2010-2013 cocos2d-x.org
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

#include "CCFileUtils.h"

#include <stack>

#include "base/CCData.h"
#include "base/ccMacros.h"
#include "base/CCDirector.h"
#include "platform/CCSAXParser.h"
#include "base/ccUtils.h"
#include "base/CCGlobalLock.h"
#include "tinyxml2.h"
#ifdef MINIZIP_FROM_SYSTEM
#include <minizip/unzip.h>
#else // from our embedded sources
#include "unzip.h"
#endif
#include <sys/stat.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
#include <regex>
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include <ftw.h>
#endif

#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32) && (CC_TARGET_PLATFORM != CC_PLATFORM_WINRT)
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#endif
#include "tinydir/tinydir.h"
#include <fts.h>
extern bool IsQuitGame();
extern void addFabricLog(const char* fmt, ...);
extern bool Common_IsObbOk();
extern std::string Common_getSDCardObbPath();
extern std::string Common_getOBBDownPath();


int recursive_delete(const char *dir)
{
    int ret = 0;
    FTS *ftsp = NULL;
    FTSENT *curr;
    
    // Cast needed (in C) because fts_open() takes a "char * const *", instead
    // of a "const char * const *", which is only allowed in C++. fts_open()
    // does not modify the argument.
    char *files[] = { (char *) dir, NULL };
    
    // FTS_NOCHDIR  - Avoid changing cwd, which could cause unexpected behavior
    //                in multithreaded programs
    // FTS_PHYSICAL - Don't follow symlinks. Prevents deletion of files outside
    //                of the specified directory
    // FTS_XDEV     - Don't cross filesystem boundaries
    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!ftsp) {
        fprintf(stderr, "%s: fts_open failed: %s\n", dir, strerror(errno));
        ret = -1;
        goto finish;
    }
    
    while ((curr = fts_read(ftsp))) {
        switch (curr->fts_info) {
            case FTS_NS:
            case FTS_DNR:
            case FTS_ERR:
                //                fprintf(stderr, "%s: fts_read error: %s\n",
                //                        curr->fts_accpath, strerror(curr->fts_errno));
                break;
                
            case FTS_DC:
            case FTS_DOT:
            case FTS_NSOK:
                // Not reached unless FTS_LOGICAL, FTS_SEEDOT, or FTS_NOSTAT were
                // passed to fts_open()
                break;
                
            case FTS_D:
                // Do nothing. Need depth-first search, so directories are deleted
                // in FTS_DP
                break;
                
            case FTS_DP:
            case FTS_F:
            case FTS_SL:
            case FTS_SLNONE:
            case FTS_DEFAULT:
                if (remove(curr->fts_accpath) < 0) {
                    //                    fprintf(stderr, "%s: Failed to remove: %s\n",
                    //                            curr->fts_path, strerror(errno));
                    ret = -1;
                }
                break;
        }
    }
    
finish:
    if (ftsp) {
        fts_close(ftsp);
    }
    
    return ret;
}

bool rmtree(const char path[])
{
    char* p = strdup(path);
    bool ret = recursive_delete(path);
    free(p);
    return ret;
}

int mkdir_p(const char *path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[4096];
    char *p;
    
    errno = 0;
    
    /* Copy string so its mutable */
    if (len > sizeof(_path)-1) {
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(_path, path);
    
    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';
            
            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return -1;
            }
            
            *p = '/';
        }
    }
    
    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return -1;
    }
    
    return 0;
}


////////////////////////////////////////////////////////

#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS) && (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)
//#if 1

#include <plist/plistHelper.hpp>
// 使用plist_get_data速度会快丝毫！
extern "C" {
    #include <../src/plist.h>
}

using cocos2d::ValueMap;
using cocos2d::Value;
using cocos2d::ValueVector;

//
// 仿照CCFileUtils-apple.mm中做的同样的处理
// 放到这里主要是方便mac调试用，等打包的时候别忘了将上方if 1注释还原！！ -- lsy
//
static void addValueToDict(std::string const& key, plist_t node, ValueMap& dict);
static void addItemToArray(plist_t node, ValueVector& array)
{
    plist_type type = plist_get_node_type(node);
    
    switch (type)
    {
        case PLIST_BOOLEAN:
        {
            uint8_t val = false;
            plist_get_bool_val(node, &val);
            array.push_back(Value(val));
            break;
        }
            
        case PLIST_UINT:
        {
            uint64_t val = 0;
            plist_get_uint_val(node, &val);
            array.push_back(Value((int)val));
            break;
        }
            
        case PLIST_REAL:
        {
            double val = 0.f;
            plist_get_real_val(node, &val);
            array.push_back(Value(val));
            break;
        }
            
        case PLIST_KEY:
        {
            char* val = nullptr;
            plist_get_key_val(node, &val);
            array.push_back(Value(val));
            free(val);
            break;
        }
            
        case PLIST_STRING:
        {
//            plist_data_t data = plist_get_data(node);
//            array.push_back(Value(data->strval));
            char* val = nullptr;
            plist_get_string_val(node, &val);
            array.push_back(Value(val));
            free(val);
            break;
        }
            
        case PLIST_ARRAY:
        {
            array.push_back(Value(ValueVector()));
            ValueVector& subArray = array.back().asValueVector();
            
            uint32_t count = plist_array_get_size(node);
            for (int i=0; i<count; ++i)
            {
                plist_t subNode = plist_array_get_item(node, i);
                addItemToArray(subNode, subArray);
            }
            
            break;
        }
            
        case PLIST_DICT:
        {
            array.push_back(Value(ValueMap()));
            ValueMap& subDict = array.back().asValueMap();
            
            plist_dict_iter it = NULL;
            plist_t subnode = NULL;
            plist_dict_new_iter(node, &it);
            
            do
            {
                char* itkey = NULL;
                plist_dict_next_item(node, it, &itkey, &subnode);
                if (!itkey)
                    break;
                
                addValueToDict(itkey, subnode, subDict);
                
                free(itkey);
            }
            while (1);
            free(it);
            
            break;
        }
            
        default:
            assert(0);
            break;
    }
    
    return;
}

static void addValueToDict(std::string const& key, plist_t node, ValueMap& dict)
{
    plist_type type = plist_get_node_type(node);
    
    switch (type)
    {
        case PLIST_BOOLEAN:
        {
            uint8_t val = false;
            plist_get_bool_val(node, &val);
            dict[key] = Value(val);
            break;
        }
            
        case PLIST_UINT:
        {
            uint64_t val = 0;
            plist_get_uint_val(node, &val);
            dict[key] = Value((int)val);
            break;
        }
            
        case PLIST_REAL:
        {
            double val = 0.f;
            plist_get_real_val(node, &val);
            dict[key] = Value(val);
            break;
        }
            
        case PLIST_KEY:
        {
            char* val = nullptr;
            plist_get_key_val(node, &val);
            dict[key] = Value(val);
            free(val);
            break;
        }
            
        case PLIST_STRING:
        {
//            plist_data_t data = plist_get_data(node);
//            dict[key] = Value(data->strval);
            
            char* val = nullptr;
            plist_get_string_val(node, &val);
            dict[key] = Value(val);
            free(val);
            break;
        }
            
        case PLIST_ARRAY:
        {
            auto ret = dict.insert(std::make_pair(key, Value(ValueVector())));
            ValueVector& valueArray = ret.first->second.asValueVector();
            
            uint32_t count = plist_array_get_size(node);
            for (int i=0; i<count; ++i)
            {
                plist_t subNode = plist_array_get_item(node, i);
                addItemToArray(subNode, valueArray);
            }
            
            break;
        }
            
        case PLIST_DICT:
        {
            auto ret = dict.insert(std::make_pair(key, Value(ValueMap())));
            ValueMap& subDict = ret.first->second.asValueMap();
            
            plist_dict_iter it = NULL;
            plist_t subnode = NULL;
            plist_dict_new_iter(node, &it);
            
            do
            {
                char* itkey = NULL;
                plist_dict_next_item(node, it, &itkey, &subnode);
                if (!itkey)
                    break;
                
                addValueToDict(itkey, subnode, subDict);
                
                free(itkey);
            }
            while (1);
            free(it);
            
            break;
        }
            
        default:
            assert(0);
            break;
    }
    
    return;
}


ValueMap getValueMapFromData2(const char* filedata, int filesize)
{
    ValueMap ret;
    
    // 在做读取的时候，做一个try..catch保护。防止文件出现意外导致读取狂崩溃。
    try
    {
        if (filedata == nullptr || filesize <= 0) {
            return ret;
        }
        
        //
        // android 使用第三方库来读取plist，因为cocos引擎不支持读取binary plist;
        // 关于为何使用这个库，可以找我讨论。
        //
        
        plist_t root_node = NULL;
        plist_from_memory(filedata, filesize, &root_node);
        if (root_node == NULL) {
            return ret;
        }
        
        plist_dict_iter it = NULL;
        plist_dict_new_iter(root_node, &it);
        
        do
        {
            char* key = NULL;
            plist_t subnode = NULL;
            
            plist_dict_next_item(root_node, it, &key, &subnode);
            if (!key)
                break;
            
            addValueToDict(key, subnode, ret);
            
            free(key);
        }
        while (1);
        
        free(it);
        plist_free(root_node);
        
    }
    catch (...)
    {
        CCLOG("getValueMapFromData2 exception!!!");
    }
    
    return ret;
}

NS_CC_BEGIN


typedef enum
{
    SAX_NONE = 0,
    SAX_KEY,
    SAX_DICT,
    SAX_INT,
    SAX_REAL,
    SAX_STRING,
    SAX_ARRAY
}SAXState;

typedef enum
{
    SAX_RESULT_NONE = 0,
    SAX_RESULT_DICT,
    SAX_RESULT_ARRAY
}SAXResult;

class DictMaker : public SAXDelegator
{
public:
    SAXResult _resultType;
    ValueMap _rootDict;
    ValueVector _rootArray;
    
    std::string _curKey;   ///< parsed key
    std::string _curValue; // parsed value
    SAXState _state;
    
    ValueMap*  _curDict;
    ValueVector* _curArray;
    
    std::stack<ValueMap*> _dictStack;
    std::stack<ValueVector*> _arrayStack;
    std::stack<SAXState>  _stateStack;
    
public:
    DictMaker()
    : _resultType(SAX_RESULT_NONE)
    , _state(SAX_NONE)
    {
    }
    
    ~DictMaker()
    {
    }
    
    ValueMap dictionaryWithContentsOfFile(const std::string& fileName)
    {
        _resultType = SAX_RESULT_DICT;
        SAXParser parser;
        
        CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
        parser.setDelegator(this);
        
        parser.parse(fileName);
        return _rootDict;
    }
    
    ValueMap dictionaryWithDataOfFile(const char* filedata, int filesize)
    {
        _resultType = SAX_RESULT_DICT;
        SAXParser parser;
        
        CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
        parser.setDelegator(this);
        
        parser.parse(filedata, filesize);
        return _rootDict;
    }
    
    ValueVector arrayWithContentsOfFile(const std::string& fileName)
    {
        _resultType = SAX_RESULT_ARRAY;
        SAXParser parser;
        
        CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
        parser.setDelegator(this);
        
        parser.parse(fileName);
        return _rootArray;
    }
    
    void startElement(void *ctx, const char *name, const char **atts) override
    {
        const std::string sName(name);
        if( sName == "dict" )
        {
            if(_resultType == SAX_RESULT_DICT && _rootDict.empty())
            {
                _curDict = &_rootDict;
            }
            
            _state = SAX_DICT;
            
            SAXState preState = SAX_NONE;
            if (! _stateStack.empty())
            {
                preState = _stateStack.top();
            }
            
            if (SAX_ARRAY == preState)
            {
                // add a new dictionary into the array
                _curArray->push_back(Value(ValueMap()));
                _curDict = &(_curArray->rbegin())->asValueMap();
            }
            else if (SAX_DICT == preState)
            {
                // add a new dictionary into the pre dictionary
                CCASSERT(! _dictStack.empty(), "The state is wrong!");
                ValueMap* preDict = _dictStack.top();
                (*preDict)[_curKey] = Value(ValueMap());
                _curDict = &(*preDict)[_curKey].asValueMap();
            }
            
            // record the dict state
            _stateStack.push(_state);
            _dictStack.push(_curDict);
        }
        else if(sName == "key")
        {
            _state = SAX_KEY;
        }
        else if(sName == "integer")
        {
            _state = SAX_INT;
        }
        else if(sName == "real")
        {
            _state = SAX_REAL;
        }
        else if(sName == "string")
        {
            _state = SAX_STRING;
        }
        else if (sName == "array")
        {
            _state = SAX_ARRAY;
            
            if (_resultType == SAX_RESULT_ARRAY && _rootArray.empty())
            {
                _curArray = &_rootArray;
            }
            SAXState preState = SAX_NONE;
            if (! _stateStack.empty())
            {
                preState = _stateStack.top();
            }
            
            if (preState == SAX_DICT)
            {
                (*_curDict)[_curKey] = Value(ValueVector());
                _curArray = &(*_curDict)[_curKey].asValueVector();
            }
            else if (preState == SAX_ARRAY)
            {
                CCASSERT(! _arrayStack.empty(), "The state is wrong!");
                ValueVector* preArray = _arrayStack.top();
                preArray->push_back(Value(ValueVector()));
                _curArray = &(_curArray->rbegin())->asValueVector();
            }
            // record the array state
            _stateStack.push(_state);
            _arrayStack.push(_curArray);
        }
        else
        {
            _state = SAX_NONE;
        }
    }
    
    void endElement(void *ctx, const char *name) override
    {
        SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
        const std::string sName((char*)name);
        if( sName == "dict" )
        {
            _stateStack.pop();
            _dictStack.pop();
            if ( !_dictStack.empty())
            {
                _curDict = _dictStack.top();
            }
        }
        else if (sName == "array")
        {
            _stateStack.pop();
            _arrayStack.pop();
            if (! _arrayStack.empty())
            {
                _curArray = _arrayStack.top();
            }
        }
        else if (sName == "true")
        {
            if (SAX_ARRAY == curState)
            {
                _curArray->push_back(Value(true));
            }
            else if (SAX_DICT == curState)
            {
                (*_curDict)[_curKey] = Value(true);
            }
        }
        else if (sName == "false")
        {
            if (SAX_ARRAY == curState)
            {
                _curArray->push_back(Value(false));
            }
            else if (SAX_DICT == curState)
            {
                (*_curDict)[_curKey] = Value(false);
            }
        }
        else if (sName == "string" || sName == "integer" || sName == "real")
        {
            if (SAX_ARRAY == curState)
            {
                if (sName == "string")
                    _curArray->push_back(Value(_curValue));
                else if (sName == "integer")
                    _curArray->push_back(Value(atoi(_curValue.c_str())));
                else
                    _curArray->push_back(Value(std::atof(_curValue.c_str())));
            }
            else if (SAX_DICT == curState)
            {
                if (sName == "string")
                    (*_curDict)[_curKey] = Value(_curValue);
                else if (sName == "integer")
                    (*_curDict)[_curKey] = Value(atoi(_curValue.c_str()));
                else
                    (*_curDict)[_curKey] = Value(std::atof(_curValue.c_str()));
            }
            
            _curValue.clear();
        }
        
        _state = SAX_NONE;
    }
    
    void textHandler(void *ctx, const char *ch, size_t len) override
    {
        if (_state == SAX_NONE)
        {
            return;
        }
        
        SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
        const std::string text = std::string((char*)ch,len);
        
        switch(_state)
        {
            case SAX_KEY:
                _curKey = text;
                break;
            case SAX_INT:
            case SAX_REAL:
            case SAX_STRING:
            {
                if (curState == SAX_DICT)
                {
                    CCASSERT(!_curKey.empty(), "key not found : <integer/real>");
                }
                
                _curValue.append(text);
            }
                break;
            default:
                break;
        }
    }
};

ValueMap FileUtils::getValueMapFromFile(const std::string& filename)
{
//    CCLOG("getValueMapFromFile - %s", filename.c_str());
    
    char temp[1024];
    sprintf(temp, "getValueMapFromFile - %s", filename.c_str());
    LOG_HELPER_USE_TIME(temp, 3);
    
    // 修改这这样，使其能从包文件读取
    std::string fullPath = fullPathForFilename(filename);
    auto d = getDataFromFile(fullPath);
    return getValueMapFromData((const char*)d.getBytes(), d.getSize());
}

ValueMap FileUtils::getValueMapFromData(const char* filedata, int filesize)
{
//    CCLOG("FileUtils::getValueMapFromData begin");
    return getValueMapFromData2(filedata, filesize);
    
//    CCLOG("FileUtils::getValueMapFromData dictMaker!");
//    DictMaker tMaker;
//    return tMaker.dictionaryWithDataOfFile(filedata, filesize);
}

ValueVector FileUtils::getValueVectorFromFile(const std::string& filename)
{
    // 这个先不支持，如果遇见了，请大喊一声BUG！
    assert(0);
    return ValueVector();
//    const std::string fullPath = fullPathForFilename(filename.c_str());
//    DictMaker tMaker;
//    return tMaker.arrayWithContentsOfFile(fullPath.c_str());
}


/*
 * forward statement
 */
static tinyxml2::XMLElement* generateElementForArray(const ValueVector& array, tinyxml2::XMLDocument *doc);
static tinyxml2::XMLElement* generateElementForDict(const ValueMap& dict, tinyxml2::XMLDocument *doc);

/*
 * Use tinyxml2 to write plist files
 */
bool FileUtils::writeToFile(ValueMap& dict, const std::string &fullPath)
{
    return writeValueMapToFile(dict, fullPath);
}

bool FileUtils::writeValueMapToFile(const ValueMap& dict, const std::string& fullPath)
{
    tinyxml2::XMLDocument *doc = new (std::nothrow)tinyxml2::XMLDocument();
    if (nullptr == doc)
        return false;

    tinyxml2::XMLDeclaration *declaration = doc->NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
    if (nullptr == declaration)
    {
        delete doc;
        return false;
    }

    doc->LinkEndChild(declaration);
    tinyxml2::XMLElement *docType = doc->NewElement("!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"");
    doc->LinkEndChild(docType);

    tinyxml2::XMLElement *rootEle = doc->NewElement("plist");
    if (nullptr == rootEle)
    {
        delete doc;
        return false;
    }
    rootEle->SetAttribute("version", "1.0");
    doc->LinkEndChild(rootEle);

    tinyxml2::XMLElement *innerDict = generateElementForDict(dict, doc);
    if (nullptr == innerDict)
    {
        delete doc;
        return false;
    }
    rootEle->LinkEndChild(innerDict);

    bool ret = tinyxml2::XML_SUCCESS == doc->SaveFile(getSuitableFOpen(fullPath).c_str());

    delete doc;
    return ret;
}

bool FileUtils::writeValueVectorToFile(const ValueVector& vecData, const std::string& fullPath)
{
    tinyxml2::XMLDocument *doc = new (std::nothrow)tinyxml2::XMLDocument();
    if (nullptr == doc)
        return false;

    tinyxml2::XMLDeclaration *declaration = doc->NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
    if (nullptr == declaration)
    {
        delete doc;
        return false;
    }

    doc->LinkEndChild(declaration);
    tinyxml2::XMLElement *docType = doc->NewElement("!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"");
    doc->LinkEndChild(docType);

    tinyxml2::XMLElement *rootEle = doc->NewElement("plist");
    if (nullptr == rootEle)
    {
        delete doc;
        return false;
    }
    rootEle->SetAttribute("version", "1.0");
    doc->LinkEndChild(rootEle);

    tinyxml2::XMLElement *innerDict = generateElementForArray(vecData, doc);
    if (nullptr == innerDict)
    {
        delete doc;
        return false;
    }
    rootEle->LinkEndChild(innerDict);

    bool ret = tinyxml2::XML_SUCCESS == doc->SaveFile(getSuitableFOpen(fullPath).c_str());

    delete doc;
    return ret;
}

/*
 * Generate tinyxml2::XMLElement for Object through a tinyxml2::XMLDocument
 */
static tinyxml2::XMLElement* generateElementForObject(const Value& value, tinyxml2::XMLDocument *doc)
{
    // object is String
    if (value.getType() == Value::Type::STRING)
    {
        tinyxml2::XMLElement* node = doc->NewElement("string");
        tinyxml2::XMLText* content = doc->NewText(value.asString().c_str());
        node->LinkEndChild(content);
        return node;
    }
    
    // object is integer
    if (value.getType() == Value::Type::INTEGER)
    {
        tinyxml2::XMLElement* node = doc->NewElement("integer");
        tinyxml2::XMLText* content = doc->NewText(value.asString().c_str());
        node->LinkEndChild(content);
        return node;
    }

    // object is real
    if (value.getType() == Value::Type::FLOAT || value.getType() == Value::Type::DOUBLE)
    {
        tinyxml2::XMLElement* node = doc->NewElement("real");
        tinyxml2::XMLText* content = doc->NewText(value.asString().c_str());
        node->LinkEndChild(content);
        return node;
    }
    
    //object is bool
    if (value.getType() == Value::Type::BOOLEAN) {
		tinyxml2::XMLElement* node = doc->NewElement(value.asString().c_str());
		return node;
    }

    // object is Array
    if (value.getType() == Value::Type::VECTOR)
        return generateElementForArray(value.asValueVector(), doc);
    
    // object is Dictionary
    if (value.getType() == Value::Type::MAP)
        return generateElementForDict(value.asValueMap(), doc);
    
    CCLOG("This type cannot appear in property list");
    return nullptr;
}

/*
 * Generate tinyxml2::XMLElement for Dictionary through a tinyxml2::XMLDocument
 */
static tinyxml2::XMLElement* generateElementForDict(const ValueMap& dict, tinyxml2::XMLDocument *doc)
{
    tinyxml2::XMLElement* rootNode = doc->NewElement("dict");
    
    for (const auto &iter : dict)
    {
        tinyxml2::XMLElement* tmpNode = doc->NewElement("key");
        rootNode->LinkEndChild(tmpNode);
        tinyxml2::XMLText* content = doc->NewText(iter.first.c_str());
        tmpNode->LinkEndChild(content);
        
        tinyxml2::XMLElement *element = generateElementForObject(iter.second, doc);
        if (element)
            rootNode->LinkEndChild(element);
    }
    return rootNode;
}

/*
 * Generate tinyxml2::XMLElement for Array through a tinyxml2::XMLDocument
 */
static tinyxml2::XMLElement* generateElementForArray(const ValueVector& array, tinyxml2::XMLDocument *pDoc)
{
    tinyxml2::XMLElement* rootNode = pDoc->NewElement("array");

    for(const auto &value : array) {
        tinyxml2::XMLElement *element = generateElementForObject(value, pDoc);
        if (element)
            rootNode->LinkEndChild(element);
    }
    return rootNode;
}

#else
NS_CC_BEGIN



/* The subclass FileUtilsApple should override these two method. */
ValueMap FileUtils::getValueMapFromFile(const std::string& filename) {return ValueMap();}
ValueMap FileUtils::getValueMapFromData(const char* filedata, int filesize) {return ValueMap();}
ValueVector FileUtils::getValueVectorFromFile(const std::string& filename) {return ValueVector();}
bool FileUtils::writeToFile(ValueMap& dict, const std::string &fullPath) {return false;}

#endif /* (CC_TARGET_PLATFORM != CC_PLATFORM_IOS) && (CC_TARGET_PLATFORM != CC_PLATFORM_MAC) */

bool SingleFile::init()
{
    cocos2d::CCLog(">>> obb>> init singlefile");
    IFLOGCRITICAL("1", ">>> obb>> init singlefile");
    // 只打开本地目录的压缩包，其他目录的压缩包不考虑。
    std::string zipRes = "DayZResources.zip";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    //优先查找sdcard中从gp下载的
    zipRes = Common_getSDCardObbPath();
    if (!cocos2d::FileUtils::getInstance()->isFileExist(zipRes)){
        zipRes = Common_getOBBDownPath();
    }
#endif
    if (cocos2d::FileUtils::getInstance()->isFileExist(zipRes) == false) {
        CCLOG("SingleFile not found DayZResources.zip");
        return false;
    }
    //        return false;
    
    // 进入Resource目录，然后执行，zip -r ../DayZResources.zip ./
    m_packMgr = createLightPackage(0);
    //    packMgr = createLightPackage(1);
    
    std::string fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(zipRes);
    cocos2d::log(">>> obb>> SingleFile fullpath: %s ", fullPath.c_str());
    if (m_packMgr->openPackFile(fullPath)) {
        cocos2d::log(">>> obb>> SingleFile open ok");
        CCLOG("SingleFile open ok!!!");
        return true;
    }
    cocos2d::log(">>> obb>> SingleFile open error");
    CCLOG("SingleFile open error!!!");
    destroyLightPackage(m_packMgr);
    m_packMgr = nullptr;
    
    return false;
}

bool SingleFile::hasFile(const std::string& filename)
{
    if (m_packMgr) {
        return m_packMgr->hasFile(filename);
    }
    
    return false;
}

unsigned char* SingleFile::getPackData(const std::string& filename, ssize_t *size)
{
    bool log = false;
    if (filename.find("AllianceTerritory.png") != std::string::npos){
        log = true;
    }
    unsigned char* buffer = nullptr;
    
    if (m_packMgr == nullptr) {
        return nullptr;
    }
    
    // FIMXE: 防止多线程同时访问packdata，因为底层是不是线程安全的。
    std::lock_guard<std::recursive_mutex> _(m_fileMutex);
    
    ILightReadFile* reader = m_packMgr->createReadFile(filename);
    if (reader)
    {
        *size = reader->size();
        buffer = (unsigned char*)malloc(*size + 1);
        
        int readsize = reader->read(buffer, *size);
        reader->close();
        
        buffer[*size] = '\0';
        
        if (readsize >= 0) {
            if (log == true){
                CCLog(">>> obb>> getPackData filename: %s ok!!", filename.c_str());
            }
        }
        else {
            CCLOG("getData read pack error - %s", filename.c_str());
            if (log == true){
                CCLog(">>> obb>> getPackData filename: %s fail 1 !!", filename.c_str());
            }
        }
    }
    else {
        CCLOG("create reader error.!");
        if (log == true){
            CCLog(">>> obb>> getPackData filename: %s fail 2 !!", filename.c_str());
        }
    }
    
    return buffer;
}

int SingleFile::getPackSize(const std::string& filename)
{
    int size = -1;
    if (m_packMgr == nullptr) {
        return -1;
    }
    
    // FIMXE: 防止多线程同时访问packdata，因为底层是不是线程安全的。
    std::lock_guard<std::recursive_mutex> _(m_fileMutex);
    
    ILightReadFile* reader = m_packMgr->createReadFile(filename);
    if (reader)
    {
        size = reader->size();
        reader->close();
    }
    
    return size;
}

#define SAFE_FUN_LOCK GlobalLock::AutoLock autolock_tmp_by_type(GlobalLock::Class_Mutex::Class_FileUtilsCache)


SingleFile gSingleFile;
FileUtils* FileUtils::s_sharedFileUtils = nullptr;

void FileUtils::destroyInstance()
{
    CC_SAFE_DELETE(s_sharedFileUtils);
}

void FileUtils::setDelegate(FileUtils *delegate)
{
    if (s_sharedFileUtils)
        delete s_sharedFileUtils;
        
    s_sharedFileUtils = delegate;
}

FileUtils::FileUtils()
    : _writablePath("")
{
}

FileUtils::~FileUtils()
{
}


bool FileUtils::writeStringToFile(const std::string& dataStr, const std::string& fullPath)
{
    Data data;
    data.fastSet((unsigned char*)dataStr.c_str(), dataStr.size());

    bool rv = writeDataToFile(data, fullPath);

    data.fastSet(nullptr, 0);
    return rv;
}

void FileUtils::writeStringToFile(std::string dataStr, const std::string& fullPath, std::function<void(bool)> callback)
{
    performOperationOffthread([fullPath](const std::string& dataStrIn) -> bool {
        return FileUtils::getInstance()->writeStringToFile(dataStrIn, fullPath);
    }, std::move(callback),std::move(dataStr));
}

bool FileUtils::writeDataToFile(const Data& data, const std::string& fullPath)
{
    size_t size = 0;
    const char* mode = "wb";

    CCASSERT(!fullPath.empty() && data.getSize() != 0, "Invalid parameters.");

    auto fileutils = FileUtils::getInstance();
    do
    {
        // Read the file from hardware
        FILE *fp = fopen(fileutils->getSuitableFOpen(fullPath).c_str(), mode);
        CC_BREAK_IF(!fp);
        size = data.getSize();

        fwrite(data.getBytes(), size, 1, fp);

        fclose(fp);

        return true;
    } while (0);

    return false;
}

void FileUtils::writeDataToFile(Data data, const std::string& fullPath, std::function<void(bool)> callback)
{
    performOperationOffthread([fullPath](const Data& dataIn) -> bool {
        return FileUtils::getInstance()->writeDataToFile(dataIn, fullPath);
    }, std::move(callback), std::move(data));
}

void FileUtils::writeValueMapToFile(ValueMap dict, const std::string& fullPath, std::function<void(bool)> callback)
{
    
    performOperationOffthread([fullPath](const ValueMap& dictIn) -> bool {
        return FileUtils::getInstance()->writeValueMapToFile(dictIn, fullPath);
    }, std::move(callback), std::move(dict));
}

void FileUtils::writeValueVectorToFile(ValueVector vecData, const std::string& fullPath, std::function<void(bool)> callback)
{
    performOperationOffthread([fullPath] (const ValueVector& vecDataIn) -> bool {
        return FileUtils::getInstance()->writeValueVectorToFile(vecDataIn, fullPath);
    }, std::move(callback), std::move(vecData));
}

bool FileUtils::init()
{
    _searchPathArray.push_back(_defaultResRootPath);
    _searchPathLocalArray.push_back(_defaultResRootPath);
    _searchResolutionsOrderArray.push_back("");
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    initSingleFile();
#endif
    
    return true;
}

void FileUtils::initSingleFile(){
    gSingleFile.init();
}

void FileUtils::purgeCachedEntries()
{
    SAFE_FUN_LOCK;
    _fullPathCache.clear();
    _canFindPaths.clear();
}

static Data getData(const std::string& filename, bool forString)
{
    if (IsQuitGame()) {
        addFabricLog("QuitGame getData %s", filename.c_str());
    }
    
    if (filename.empty())
    {
        return Data::Null;
    }
    
    Data ret;
    unsigned char* buffer = nullptr;
    size_t size = 0;
    size_t readsize;
    const char* mode = nullptr;
    
    if (forString)
        mode = "rt";
    else
        mode = "rb";

    auto fileutils = FileUtils::getInstance();
    do
    {
        // Read the file from hardware
        std::string fullPath = fileutils->fullPathForFilename(filename);
        
        // 如果是不是绝对目录的话，表示在本地没有找到，就要去打包中去找
        if (fullPath.find("pack://") == 0)
        {
            ssize_t rs;
            std::string packname = fullPath.substr(strlen("pack://"));
            buffer = gSingleFile.getPackData(packname, &rs);
            if (buffer) {
                readsize = rs;
                break;
            }
        }
        
        FILE *fp = fopen(fileutils->getSuitableFOpen(fullPath).c_str(), mode);
        CC_BREAK_IF(!fp);
        fseek(fp,0,SEEK_END);
        size = ftell(fp);
        fseek(fp,0,SEEK_SET);
        
        if (forString)
        {
            buffer = (unsigned char*)malloc(sizeof(unsigned char) * (size + 1));
            buffer[size] = '\0';
        }
        else
        {
            buffer = (unsigned char*)malloc(sizeof(unsigned char) * (size + 1));
            buffer[size] = '\0';
        }
        
        int c = fread(buffer, size, 1, fp);
        readsize = c * size;
        fclose(fp);
        
        if (readsize < size)
        {
            buffer[readsize] = '\0';
        }
    } while (0);
    
    if (nullptr == buffer || 0 == readsize)
    {
        CCLOG("Get data from file (4 %s failed", filename.c_str());
    }
    else
    {
        ret.fastSet(buffer, readsize);
    }
    
    return ret;
}

std::string FileUtils::getStringFromFile(const std::string& filename)
{
    Data data = getData(filename, true);
    if (data.isNull())
    	return "";
    
    std::string ret((const char*)data.getBytes());
    return ret;
}

Data FileUtils::getDataFromFile(const std::string& filename)
{
    return getData(filename, false);
}

unsigned char* FileUtils::getFileData(const std::string& filename, const char* mode, ssize_t *size)
{
    unsigned char * buffer = nullptr;
    CCASSERT(!filename.empty() && size != nullptr && mode != nullptr, "Invalid parameters.");
    *size = 0;
    std::string tmpPath = "";
    std::string tmpFullPath = "";
    do
    {
        // read the file from hardware
        const std::string fullPath = fullPathForFilename(filename);
        
        // 如果是不是绝对目录的话，表示在本地没有找到，就要去打包中去找
        if (fullPath.find("pack://") == 0)
        {
            std::string packname = fullPath.substr(strlen("pack://"));
            buffer = gSingleFile.getPackData(packname, size);
            if (buffer) {
                // 因为底层为了兼容，多分配了一个字节。。。
                *size -= 1;
                break;
            }
        }
        
        tmpFullPath = fullPath;
        tmpPath = getSuitableFOpen(fullPath);
        FILE *fp = fopen(tmpPath.c_str(), mode);
        CC_BREAK_IF(!fp);
        
        fseek(fp,0,SEEK_END);
        *size = ftell(fp);
        fseek(fp,0,SEEK_SET);
        buffer = (unsigned char*)malloc(*size);
        if (buffer == nullptr) {
            break;
        }
        
        *size = fread(buffer,sizeof(unsigned char), *size,fp);
        fclose(fp);
    } while (0);
    
    if (!buffer)
    {
        std::string msg = "Get data from file(5";
        msg.append(filename).append(") failed!\n").append(tmpPath.c_str()).append(tmpFullPath.c_str());
        
        CCLOG("%s", msg.c_str());
    }
    return buffer;
}

unsigned char* FileUtils::getFileDataFromZip(const std::string& zipFilePath, const std::string& filename, ssize_t *size)
{
    unsigned char * buffer = nullptr;
    unzFile file = nullptr;
    *size = 0;

    do 
    {
        CC_BREAK_IF(zipFilePath.empty());

        file = unzOpen(zipFilePath.c_str());
        CC_BREAK_IF(!file);

        // FIXME: Other platforms should use upstream minizip like mingw-w64  
        #ifdef MINIZIP_FROM_SYSTEM
        int ret = unzLocateFile(file, filename.c_str(), NULL);
        #else
        int ret = unzLocateFile(file, filename.c_str(), 1);
        #endif
        CC_BREAK_IF(UNZ_OK != ret);

        char filePathA[260];
        unz_file_info fileInfo;
        ret = unzGetCurrentFileInfo(file, &fileInfo, filePathA, sizeof(filePathA), nullptr, 0, nullptr, 0);
        CC_BREAK_IF(UNZ_OK != ret);

        ret = unzOpenCurrentFile(file);
        CC_BREAK_IF(UNZ_OK != ret);

        buffer = (unsigned char*)malloc(fileInfo.uncompressed_size);
        int CC_UNUSED readedSize = unzReadCurrentFile(file, buffer, static_cast<unsigned>(fileInfo.uncompressed_size));
        CCASSERT(readedSize == 0 || readedSize == (int)fileInfo.uncompressed_size, "the file size is wrong");

        *size = fileInfo.uncompressed_size;
        unzCloseCurrentFile(file);
    } while (0);

    if (file)
    {
        unzClose(file);
    }

    return buffer;
}

std::string FileUtils::getNewFilename(const std::string &filename) const
{
    std::string newFileName;
    
    // in Lookup Filename dictionary ?
    auto iter = _filenameLookupDict.find(filename);

    if (iter == _filenameLookupDict.end())
    {
        newFileName = filename;
    }
    else
    {
        newFileName = iter->second.asString();
    }
    return newFileName;
}

std::string FileUtils::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
    std::string file = filename;
    std::string file_path = "";
    size_t pos = filename.find_last_of("/");
    if (pos != std::string::npos)
    {
        file_path = filename.substr(0, pos+1);
        file = filename.substr(pos+1);
    }
    
    // searchPath + file_path + resourceDirectory
    std::string path = searchPath;
    path += file_path;
    path += resolutionDirectory;
    
    path = getFullPathForDirectoryAndFilename(path, file);
    
    //CCLOG("getPathForFilename, fullPath = %s", path.c_str());
    return path;
}

std::string FileUtils::fullPathForFilename(const std::string &filename) const
{
    SAFE_FUN_LOCK;
    if (filename.empty())
    {
        return "";
    }
    
    if (isAbsolutePath(filename))
    {
        return filename;
    }
    
    if (filename.find("pack://") == 0)
    {
        return filename;
    }
    
    std::string fullpath;

    // 如果打包系统中有此文件的话，就直接返回
    std::string tmpFileName = filename;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    tmpFileName = "android_expack/"+tmpFileName;
#endif
    if (gSingleFile.hasFile(tmpFileName) && Common_IsObbOk() == false)
    {
        fullpath = "pack://" + tmpFileName;
        return fullpath;
    }

    // 这里暂时先不缓存，否则会有多线程同步的BUG。而且就从这里的代码看，也没有必要缓存。
#if 1
    // Already Cached ?
    auto cacheIter = _fullPathCache.find(filename);
    if(cacheIter != _fullPathCache.end())
    {
        return cacheIter->second;
    }
#endif
    if(_canFindPaths.find(filename) != _canFindPaths.end() && _canFindPaths[filename] == false)
    {
        return "";
    }
    // Get the new file name.
    const std::string newFilename( getNewFilename(filename) );
    
    for (const auto& searchIt : _searchPathArray)
    {
        for (const auto& resolutionIt : _searchResolutionsOrderArray)
        {
            fullpath = this->getPathForFilename(newFilename, resolutionIt, searchIt);
            
            if (fullpath.length() > 0)
            {
                // Using the filename passed in as key.
                _fullPathCache.insert(std::make_pair(filename, fullpath));
                _canFindPaths[filename] = true;
                return fullpath;
            }
            
        }
    }
    _canFindPaths[filename] = false;
//    for (const auto& searchIt : _searchPathLocalArray)
//    {
//        for (const auto& resolutionIt : _searchResolutionsOrderArray)
//        {
//            fullpath = this->getPathForFilename(newFilename, resolutionIt, searchIt);
//
//            if (fullpath.length() > 0)
//            {
//                // Using the filename passed in as key.
////                _fullPathCache.insert(std::make_pair(filename, fullpath));
//                return fullpath;
//            }
//
//        }
//    }
    
//    // NOTICE: 这个暂时不能放在下面，因为ios更新的时候老文件没有删除，所以会先读取原始文件。
//    //         而原始文件可能是老文件，所以会导致崩溃。。。
//    // 如果打包系统中有此文件的话，就直接返回
//    if (gSingleFile.hasFile(filename))
//    {
//        fullpath = "pack://" + filename;
//        return fullpath;
//    }
    
    if(isPopupNotify()){
        CCLOG("cocos2d: fullPathForFilename: No file found at %s. Possible missing file.", filename.c_str());
    }

    // The file wasn't found, return empty string.
    return "";
}
//该文件能被找到
void FileUtils::setCanFindPath(const std::string &filename)
{
    std::size_t pos = filename.find_last_of("/");
    std::string _fileName = filename.substr(pos+1);
    for (auto iter = _canFindPaths.begin(); iter != _canFindPaths.end(); iter++) {
        const std::string &key = iter->first;
        if(key.find(_fileName) != std::string::npos)
        {
            iter->second = true;
        }
    }
}

std::string FileUtils::fullPathFromRelativeFile(const std::string &filename, const std::string &relativeFile)
{
    return relativeFile.substr(0, relativeFile.rfind('/')+1) + getNewFilename(filename);
}

void FileUtils::setSearchResolutionsOrder(const std::vector<std::string>& searchResolutionsOrder)
{
    SAFE_FUN_LOCK;
    bool existDefault = false;
    _fullPathCache.clear();
    _canFindPaths.clear();
    _searchResolutionsOrderArray.clear();
    for(const auto& iter : searchResolutionsOrder)
    {
        std::string resolutionDirectory = iter;
        if (!existDefault && resolutionDirectory == "")
        {
            existDefault = true;
        }
        
        if (resolutionDirectory.length() > 0 && resolutionDirectory[resolutionDirectory.length()-1] != '/')
        {
            resolutionDirectory += "/";
        }
        
        _searchResolutionsOrderArray.push_back(resolutionDirectory);
    }

    if (!existDefault)
    {
        _searchResolutionsOrderArray.push_back("");
    }
}

void FileUtils::addSearchResolutionsOrder(const std::string &order,const bool front)
{
    SAFE_FUN_LOCK;
    std::string resOrder = order;
    if (!resOrder.empty() && resOrder[resOrder.length()-1] != '/')
        resOrder.append("/");
    
    if (front) {
        _searchResolutionsOrderArray.insert(_searchResolutionsOrderArray.begin(), resOrder);
    } else {
        _searchResolutionsOrderArray.push_back(resOrder);
    }
}

const std::vector<std::string>& FileUtils::getSearchResolutionsOrder() const
{
    return _searchResolutionsOrderArray;
}

const std::vector<std::string>& FileUtils::getSearchPaths() const
{
    return _searchPathArray;
}

void FileUtils::setWritablePath(const std::string& writablePath)
{
    _writablePath = writablePath;
}

void FileUtils::setDefaultResourceRootPath(const std::string& path)
{
    _defaultResRootPath = path;
}

void FileUtils::setSearchPaths(const std::vector<std::string>& searchPaths)
{
    SAFE_FUN_LOCK;
    bool existDefaultRootPath = false;
    
    _fullPathCache.clear();
    _canFindPaths.clear();
    _searchPathArray.clear();
    for (const auto& iter : searchPaths)
    {
        std::string prefix;
        std::string path;
        
        if (!isAbsolutePath(iter))
        { // Not an absolute path
            prefix = _defaultResRootPath;
        }
        path = prefix + (iter);
        if (path.length() > 0 && path[path.length()-1] != '/')
        {
            path += "/";
        }
        if (!existDefaultRootPath && path == _defaultResRootPath)
        {
            existDefaultRootPath = true;
        }
        _searchPathArray.push_back(path);
    }
    
    if (!existDefaultRootPath)
    {
        //CCLOG("Default root path doesn't exist, adding it.");
        _searchPathArray.push_back(_defaultResRootPath);
    }
}

void FileUtils::setSearchLocalPaths(const std::vector<std::string>& searchLocalPaths)
{
    SAFE_FUN_LOCK;
    bool existDefaultRootPath = false;
    _searchPathLocalArray.clear();
    for (const auto& iter : searchLocalPaths)
    {
        std::string prefix;
        std::string path;
        
        if (!isAbsolutePath(iter))
        { // Not an absolute path
            prefix = _defaultResRootPath;
        }
        path = prefix + (iter);
        if (path.length() > 0 && path[path.length()-1] != '/')
        {
            path += "/";
        }
        if (!existDefaultRootPath && path == _defaultResRootPath)
        {
            existDefaultRootPath = true;
        }
        _searchPathLocalArray.push_back(path);
    }
    
    if (!existDefaultRootPath)
    {
        _searchPathLocalArray.push_back(_defaultResRootPath);
    }
}

void FileUtils::addSearchPath(const std::string &searchpath,const bool front)
{
    SAFE_FUN_LOCK;
    std::string prefix;
    if (!isAbsolutePath(searchpath))
        prefix = _defaultResRootPath;

    std::string path = prefix + searchpath;
    if (path.length() > 0 && path[path.length()-1] != '/')
    {
        path += "/";
    }
    if (front) {
        _searchPathArray.insert(_searchPathArray.begin(), path);
    } else {
        _searchPathArray.push_back(path);
    }
}

void FileUtils::setFilenameLookupDictionary(const ValueMap& filenameLookupDict)
{
    SAFE_FUN_LOCK;
    _fullPathCache.clear();
    _canFindPaths.clear();
    _filenameLookupDict = filenameLookupDict;
}

void FileUtils::loadFilenameLookupDictionaryFromFile(const std::string &filename)
{
    const std::string fullPath = fullPathForFilename(filename);
    if (fullPath.length() > 0)
    {
        ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(fullPath);
        if (!dict.empty())
        {
            ValueMap& metadata =  dict["metadata"].asValueMap();
            int version = metadata["version"].asInt();
            if (version != 1)
            {
                CCLOG("cocos2d: ERROR: Invalid filenameLookup dictionary version: %d. Filename: %s", version, filename.c_str());
                return;
            }
            setFilenameLookupDictionary( dict["filenames"].asValueMap());
        }
    }
}

std::string FileUtils::getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const
{
    // get directory+filename, safely adding '/' as necessary 
    std::string ret = directory;
    if (directory.size() && directory[directory.size()-1] != '/'){
        ret += '/';
    }
    ret += filename;
    
    // if the file doesn't exist, return an empty string
    if (!isFileExistInternal(ret)) {
        ret = "";
    }
    return ret;
}

bool FileUtils::isFileExist(const std::string& filename) const
{
    if (isAbsolutePath(filename))
    {
        return isFileExistInternal(filename);
    }
    else
    {
        std::string fullpath = fullPathForFilename(filename);
        if (fullpath.empty()) {
            return false;
        }
        else
            return true;
    }
}

bool FileUtils::isAbsolutePath(const std::string& path) const
{
    return (!path.empty() && path[0] == '/');
}

bool FileUtils::isDirectoryExistInternal(const std::string& dirPath) const
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    std::wstring wdirPath(dirPath.begin(), dirPath.end());
    if (GetFileAttributesEx(wdirPath.c_str(), GetFileExInfoStandard, &wfad))
	{
		return true;
	}
	return false;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	unsigned long fAttrib = GetFileAttributesA(dirPath.c_str());
    if (fAttrib != INVALID_FILE_ATTRIBUTES &&
        (fAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
		return true;
    }
    return false;
#else
    struct stat st;
    if (stat(dirPath.c_str(), &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }
    return false;
#endif


}

bool FileUtils::isDirectoryExist(const std::string& dirPath) const
{
    SAFE_FUN_LOCK;
    CCASSERT(!dirPath.empty(), "Invalid path");
    
    if (isAbsolutePath(dirPath))
    {
        return isDirectoryExistInternal(dirPath);
    }
    
    // Already Cached ?
    auto cacheIter = _fullPathCache.find(dirPath);
    if( cacheIter != _fullPathCache.end() )
    {
        return isDirectoryExistInternal(cacheIter->second);
    }
    if(_canFindPaths.find(dirPath) != _canFindPaths.end() && _canFindPaths[dirPath] == false)
    {
        return false;
    }
	std::string fullpath;
    for (const auto& searchIt : _searchPathArray)
    {
        for (const auto& resolutionIt : _searchResolutionsOrderArray)
        {
            // searchPath + file_path + resourceDirectory
            fullpath = searchIt + dirPath + resolutionIt;
            if (isDirectoryExistInternal(fullpath))
            {
                _fullPathCache.insert(std::make_pair(dirPath, fullpath));
                _canFindPaths[dirPath] = true;
                return true;
            }
        }
    }
    
    _canFindPaths[dirPath] = false;
    
//    for (const auto& searchIt : _searchPathLocalArray)
//    {
//        for (const auto& resolutionIt : _searchResolutionsOrderArray)
//        {
//            // searchPath + file_path + resourceDirectory
//            fullpath = searchIt + dirPath + resolutionIt;
//            if (isDirectoryExistInternal(fullpath))
//            {
//                _fullPathCache.insert(std::make_pair(dirPath, fullpath));
//                return true;
//            }
//        }
//    }
    
    return false;
}

bool FileUtils::createDirectory(const std::string& path)
{
    CCASSERT(!path.empty(), "Invalid path");
    
    if (isDirectoryExist(path))
        return true;
    
    // Split the path
    size_t start = 0;
    size_t found = path.find_first_of("/\\", start);
    std::string subpath;
    std::vector<std::string> dirs;
    
    if (found != std::string::npos)
    {
        while (true)
        {
            subpath = path.substr(start, found - start + 1);
            if (!subpath.empty())
                dirs.push_back(subpath);
            start = found+1;
            found = path.find_first_of("/\\", start);
            if (found == std::string::npos)
            {
                if (start < path.length())
                {
                    dirs.push_back(path.substr(start));
                }
                break;
            }
        }
    }


#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	WIN32_FILE_ATTRIBUTE_DATA wfad;
    std::wstring wpath(path.begin(), path.end());
    if (!(GetFileAttributesEx(wpath.c_str(), GetFileExInfoStandard, &wfad)))
	{
		subpath = "";
		for(unsigned int i = 0 ; i < dirs.size() ; ++i)
		{
			subpath += dirs[i];
			if (i > 0 && !isDirectoryExist(subpath))
			{
                std::wstring wsubpath(subpath.begin(), subpath.end());
                BOOL ret = CreateDirectory(wsubpath.c_str(), NULL);
				if (!ret && ERROR_ALREADY_EXISTS != GetLastError())
				{
					return false;
				}
			}
		}
	}
	return true;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    if ((GetFileAttributesA(path.c_str())) == INVALID_FILE_ATTRIBUTES)
    {
		subpath = "";
		for (unsigned int i = 0; i < dirs.size(); ++i)
		{
			subpath += dirs[i];
			if (!isDirectoryExist(subpath))
			{
				BOOL ret = CreateDirectoryA(subpath.c_str(), NULL);
				if (!ret && ERROR_ALREADY_EXISTS != GetLastError())
				{
					return false;
				}
			}
		}
    }
    return true;
#else
    DIR *dir = NULL;

    // Create path recursively
    subpath = "";
    for (int i = 0; i < dirs.size(); ++i)
    {
        subpath += dirs[i];
        dir = opendir(subpath.c_str());
        
        if (!dir)
        {
            // directory doesn't exist, should create a new one
            
            int ret = mkdir(subpath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
            if (ret != 0 && (errno != EEXIST))
            {
                // current directory can not be created, sub directories can not be created too
                // should return
                return false;
            }
        }
        else
        {
            // directory exists, should close opened dir
            closedir(dir);
        }
    }
    return true;
#endif
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    if( strstr(fpath, "usr.dat") ) {
        return 0;
    }
    auto ret = remove(fpath);
    if (ret)
    {
        log("Fail to remove: %s ",fpath);
    }
    
    return ret;
}
#endif

bool FileUtils::removeDirectory(const std::string& path)
{
    if (path.size() > 0 && path[path.size() - 1] != '/')
    {
        CCLOGERROR("Fail to remove directory, path must termniate with '/': %s", path.c_str());
        return false;
    }
    
    // Remove !!!!!ed files

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    std::wstring wpath = std::wstring(path.begin(), path.end());
    std::wstring files = wpath +  L"*.*";
	WIN32_FIND_DATA wfd;
	HANDLE  search = FindFirstFileEx(files.c_str(), FindExInfoStandard, &wfd, FindExSearchNameMatch, NULL, 0);
	bool ret=true;   
	if (search!=INVALID_HANDLE_VALUE)   
	{   
		BOOL find=true;   
		while (find)
		{ 
			//. ..
			if(wfd.cFileName[0]!='.')  
			{   
                std::wstring temp = wpath + wfd.cFileName;
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
                    temp += '/';
                    ret = ret && this->removeDirectory(std::string(temp.begin(), temp.end()));
				}
				else
				{   
                    SetFileAttributes(temp.c_str(), FILE_ATTRIBUTE_NORMAL);
                    ret = ret && DeleteFile(temp.c_str());
				}
			}
			find = FindNextFile(search, &wfd);
		}
		FindClose(search);
	}
    if (ret && RemoveDirectory(wpath.c_str()))
    {
        return true;
    }
	return false;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	std::string command = "cmd /c rd /s /q ";
	// Path may include space.
	command += "\"" + path + "\"";

	if (WinExec(command.c_str(), SW_HIDE) > 31)
		return true;
	else
		return false;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    if (nftw(path.c_str(),unlink_cb, 64, FTW_DEPTH | FTW_PHYS))
        return false;
    else
        return true;
#else
    std::string command = "rm -r ";
    // Path may include space.
    command += "\"" + path + "\"";
    if (system(command.c_str()) >= 0)
        return true;
    else
        return false;
#endif
}

bool FileUtils::removeFile(const std::string &path)
{
    // Remove !!!!!ed file

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    std::wstring wpath(path.begin(), path.end());
    if (DeleteFile(wpath.c_str()))
	{
		return true;
	}
	return false;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	std::string command = "cmd /c del /q ";
	std::string win32path = path;
	int len = win32path.length();
	for (int i = 0; i < len; ++i)
	{
		if (win32path[i] == '/')
		{
			win32path[i] = '\\';
		}
	}
	command += win32path;

	if (WinExec(command.c_str(), SW_HIDE) > 31)
		return true;
	else
		return false;
#else
    if (remove(path.c_str())) {
        return false;
    } else {
        return true;
    }
#endif
}

bool FileUtils::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
    CCASSERT(!path.empty(), "Invalid path");
    std::string oldPath = path + oldname;
    std::string newPath = path + name;
 
    // Rename a file
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
    std::regex pat("\\/");
    std::string _old = std::regex_replace(oldPath, pat, "\\");
    std::string _new = std::regex_replace(newPath, pat, "\\");
    if (MoveFileEx(std::wstring(_old.begin(), _old.end()).c_str(), 
        std::wstring(_new.begin(), _new.end()).c_str(),
        MOVEFILE_REPLACE_EXISTING & MOVEFILE_WRITE_THROUGH))
    {
        return true;
    }
    return false;
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) 
    std::regex pat("\\/");
    std::string _old = std::regex_replace(oldPath, pat, "\\");
    std::string _new = std::regex_replace(newPath, pat, "\\");

    if(FileUtils::getInstance()->isFileExist(_new))
    {
        if (!DeleteFileA(_new.c_str()))
        {
            CCLOGERROR("Fail to delete file %s !Error code is 0x%x", newPath.c_str(), GetLastError());
        }
    }

    if (MoveFileA(_old.c_str(), _new.c_str()))
    {
        return true;
    }
    else
    {
        CCLOGERROR("Fail to rename file %s to %s !Error code is 0x%x", oldPath.c_str(), newPath.c_str(), GetLastError());
        return false;
    }
#else
    int errorCode = rename(oldPath.c_str(), newPath.c_str());

    if (0 != errorCode)
    {
        CCLOGERROR("Fail to rename file %s to %s !Error code is %d", oldPath.c_str(), newPath.c_str(), errorCode);
        return false;
    }
    return true;
#endif
}

bool FileUtils::renameFile(const std::string &oldfullpath, const std::string &newfullpath)
{
    CCASSERT(!oldfullpath.empty(), "Invalid path");
    CCASSERT(!newfullpath.empty(), "Invalid path");

    int errorCode = rename(oldfullpath.c_str(), newfullpath.c_str());

    if (0 != errorCode)
    {
        CCLOGERROR("Fail to rename file %s to %s !Error code is %d", oldfullpath.c_str(), newfullpath.c_str(), errorCode);
        return false;
    }
    return true;
}

long FileUtils::getFileSize(const std::string &filepath)
{
    CCASSERT(!filepath.empty(), "Invalid path");
    
    std::string fullpath = filepath;
    if (!isAbsolutePath(filepath))
    {
        fullpath = fullPathForFilename(filepath);
        if (fullpath.empty())
            return 0;
    }
    
    // 如果是不是绝对目录的话，表示在本地没有找到，就要去打包中去找
    if (fullpath.find("pack://") == 0)
    {
        std::string packname = fullpath.substr(strlen("pack://"));
        return gSingleFile.getPackSize(packname);
    }
    
    struct stat info;
    // Get data associated with "crt_stat.c":
    int result = stat( fullpath.c_str(), &info );
    
    // Check if statistics are valid:
    if( result != 0 )
    {
        // Failed
        return -1;
    }
    else
    {
        return (long)(info.st_size);
    }
}

void FileUtils::listFilesRecursively(const std::string& dirPath, std::vector<std::string> *files) const
{
    std::string fullpath = fullPathForFilename(dirPath);
    if (isDirectoryExist(fullpath))
    {
        tinydir_dir dir;
        std::string fullpathstr = fullpath;

        if (tinydir_open(&dir, &fullpathstr[0]) != -1)
        {
            while (dir.has_next)
            {
                tinydir_file file;
                if (tinydir_readfile(&dir, &file) == -1)
                {
                    // Error getting file
                    break;
                }
                std::string fileName = file.name;

                if (fileName != "." && fileName != "..")
                {
                    std::string filepath = file.path;
                    if (file.is_dir)
                    {
                        filepath.append("/");
                        files->push_back(filepath);
                        listFilesRecursively(filepath, files);
                    }
                    else
                    {
                        files->push_back(filepath);
                    }
                }

                if (tinydir_next(&dir) == -1)
                {
                    // Error getting next file
                    break;
                }
            }
        }
        tinydir_close(&dir);
    }
}
//////////////////////////////////////////////////////////////////////////
// Notification support when getFileData from invalid file path.
//////////////////////////////////////////////////////////////////////////
static bool s_popupNotify = false;

void FileUtils::setPopupNotify(bool notify)
{
    s_popupNotify = notify;
}

bool FileUtils::isPopupNotify() const
{
    return s_popupNotify;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
static std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
{
    std::wstring ret;
    if (!strUtf8.empty())
    {
        int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
        if (nNum)
        {
            WCHAR* wideCharString = new WCHAR[nNum + 1];
            wideCharString[0] = 0;

            nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);

            ret = wideCharString;
            delete[] wideCharString;
        }
        else
        {
            CCLOG("Wrong convert to WideChar code:0x%x", GetLastError());
        }
    }
    return ret;
}

static std::string UTF8StringToMultiByte(const std::string& strUtf8)
{
    std::string ret;
    if (!strUtf8.empty())
    {
        std::wstring strWideChar = StringUtf8ToWideChar(strUtf8);
        int nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
        if (nNum)
        {
            char* ansiString = new char[nNum + 1];
            ansiString[0] = 0;

            nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, ansiString, nNum + 1, nullptr, FALSE);

            ret = ansiString;
            delete[] ansiString;
        }
        else
        {
            CCLOG("Wrong convert to Ansi code:0x%x", GetLastError());
        }
    }

    return ret;
}

std::string FileUtils::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return UTF8StringToMultiByte(filenameUtf8);
}
#else
std::string FileUtils::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return filenameUtf8;
}
#endif

std::string FileUtils::getFileExtension(const std::string& filePath) const
{
    std::string fileExtension;
    size_t pos = filePath.find_last_of('.');
    if (pos != std::string::npos)
    {
        fileExtension = filePath.substr(pos, filePath.length());
        
        std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
    }
    
    return fileExtension;
}

void FileUtils::valueMapCompact(ValueMap& /*valueMap*/)
{
}

void FileUtils::valueVectorCompact(ValueVector& /*valueVector*/)
{
}

NS_CC_END

