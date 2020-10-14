/****************************************************************************
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
#include "base/CCUserDefault.h"
#include "platform/CCCommon.h"
#include "platform/CCFileUtils.h"
#include "tinyxml2.h"
#include "base/base64.h"
#include "base/ccUtils.h"
#include "base/CCDirector.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS && CC_TARGET_PLATFORM != CC_PLATFORM_MAC && CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)

// root name of xml
#define USERDEFAULT_ROOT_NAME    "userDefaultRoot"

#define XML_FILE_NAME "UserDefault.xml"

using namespace std;

NS_CC_BEGIN

/**
 * define the functions here because we don't want to
 * export xmlNodePtr and other types in "CCUserDefault.h"
 */

static tinyxml2::XMLElement* getXMLNodeForKey(const char* pKey, tinyxml2::XMLElement** rootNode, tinyxml2::XMLDocument **doc)
{
    tinyxml2::XMLElement* curNode = nullptr;

    // check the key value
    if (! pKey)
    {
        return nullptr;
    }

    do 
    {
 		tinyxml2::XMLDocument* xmlDoc = new tinyxml2::XMLDocument();
		*doc = xmlDoc;

        std::string xmlBuffer = FileUtils::getInstance()->getStringFromFile(UserDefault::getInstance()->getXMLFilePath());

		if (xmlBuffer.empty())
		{
			CCLOG("can not read xml file");
			break;
		}
		xmlDoc->Parse(xmlBuffer.c_str(), xmlBuffer.size());

		// get root node
		*rootNode = xmlDoc->RootElement();
		if (nullptr == *rootNode)
		{
			CCLOG("read root node error");
			break;
		}
		// find the node
		curNode = (*rootNode)->FirstChildElement();
		while (nullptr != curNode)
		{
			const char* nodeName = curNode->Value();
			if (!strcmp(nodeName, pKey))
			{
				break;
			}

			curNode = curNode->NextSiblingElement();
		}
	} while (0);

	return curNode;
}

void setValueForKey(const char* pKey, const char* pValue)
{
    std::lock_guard<LockType> _(m_dataMutex);
    if (strcmp(pKey, "") == 0){
        assert(0);
    }
 	tinyxml2::XMLElement* rootNode;
	tinyxml2::XMLDocument* doc;
	tinyxml2::XMLElement* node;
	// check the params
	if (! pKey || ! pValue)
	{
		return;
	}
	// find the node
	node = getXMLNodeForKey(pKey, &rootNode, &doc);
	// if node exist, change the content
	if (node)
	{
        if (node->FirstChild())
        {
            node->FirstChild()->SetValue(pValue);
        }
        else
        {
            tinyxml2::XMLText* content = doc->NewText(pValue);
            node->LinkEndChild(content);
        }
	}
	else
	{
		if (rootNode)
		{
			tinyxml2::XMLElement* tmpNode = doc->NewElement(pKey);//new tinyxml2::XMLElement(pKey);
			rootNode->LinkEndChild(tmpNode);
			tinyxml2::XMLText* content = doc->NewText(pValue);//new tinyxml2::XMLText(pValue);
			tmpNode->LinkEndChild(content);
		}	
	}

    // save file and free doc
	if (doc)
	{
        doc->SaveFile(FileUtils::getInstance()->getSuitableFOpen(UserDefault::getInstance()->getXMLFilePath()).c_str());
		delete doc;
	}
}

/**
 * implements of UserDefault
 */

UserDefault* UserDefault::_userDefault = nullptr;
string UserDefault::_filePath = string("");
bool UserDefault::_isFilePathInitialized = false;

UserDefault::~UserDefault()
{
}

UserDefault::UserDefault()
{
}

bool UserDefault::getBoolForKey(const char* pKey)
{
    return getBoolForKey(pKey, false);
}

bool UserDefault::getBoolForKey(const char* pKey, bool defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    const char* value = nullptr;
	tinyxml2::XMLElement* rootNode;
	tinyxml2::XMLDocument* doc;
	tinyxml2::XMLElement* node;
	node =  getXMLNodeForKey(pKey, &rootNode, &doc);
	// find the node
	if (node && node->FirstChild())
	{
        value = (const char*)(node->FirstChild()->Value());
	}

	bool ret = defaultValue;

	if (value)
	{
		ret = (! strcmp(value, "true"));
	}

    if (doc) delete doc;

	return ret;
}

int UserDefault::getIntegerForKey(const char* pKey)
{
    return getIntegerForKey(pKey, 0);
}

int UserDefault::getIntegerForKey(const char* pKey, int defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    const char* value = nullptr;
	tinyxml2::XMLElement* rootNode;
	tinyxml2::XMLDocument* doc;
	tinyxml2::XMLElement* node;
	node =  getXMLNodeForKey(pKey, &rootNode, &doc);
	// find the node
	if (node && node->FirstChild())
	{
        value = (const char*)(node->FirstChild()->Value());
	}

	int ret = defaultValue;

	if (value)
	{
		ret = atoi(value);
	}

	if(doc)
	{
		delete doc;
	}


	return ret;
}

float UserDefault::getFloatForKey(const char* pKey)
{
    return getFloatForKey(pKey, 0.0f);
}

float UserDefault::getFloatForKey(const char* pKey, float defaultValue)
{
    float ret = (float)getDoubleForKey(pKey, (double)defaultValue);
 
    return ret;
}

double  UserDefault::getDoubleForKey(const char* pKey)
{
    return getDoubleForKey(pKey, 0.0);
}

double UserDefault::getDoubleForKey(const char* pKey, double defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);
    
	const char* value = nullptr;
	tinyxml2::XMLElement* rootNode;
	tinyxml2::XMLDocument* doc;
	tinyxml2::XMLElement* node;
	node =  getXMLNodeForKey(pKey, &rootNode, &doc);
	// find the node
	if (node && node->FirstChild())
	{
        value = (const char*)(node->FirstChild()->Value());
	}

	double ret = defaultValue;

	if (value)
	{
		ret = utils::atof(value);
	}

    if (doc) delete doc;

	return ret;
}

std::string UserDefault::getStringForKey(const char* pKey)
{
    return getStringForKey(pKey, "");
}
/**
 *  @author 徐壮   xz change_sym, 16-04-05 18:04:43
 *
 *  取出ccuserdefault使用warz加过密的字符串
 *
 *  @param key          key
 *  @param defaultValue value
 *
 *  @return 解密之后的东西
 */

std::string UserDefault::getStringForKeyWithEnc(const char* pKey)
{
	std::lock_guard<LockType> _(m_dataMutex);
    
    std::string tempKey = pKey;
    tempKey.append("AgainstZ").append("X");
    Data value = getDataForKey(tempKey.c_str());
    std::string tempStr2 = userDDec(value);
    return tempStr2;
}

string UserDefault::getStringForKey(const char* pKey, const std::string & defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);
    
    const char* value = nullptr;
	tinyxml2::XMLElement* rootNode;
	tinyxml2::XMLDocument* doc;
	tinyxml2::XMLElement* node;
	node =  getXMLNodeForKey(pKey, &rootNode, &doc);
	// find the node
	if (node && node->FirstChild())
	{
        value = (const char*)(node->FirstChild()->Value());
	}

	string ret = defaultValue;

	if (value)
	{
		ret = string(value);
	}

    if (doc) delete doc;

	return ret;
}

Data UserDefault::getDataForKey(const char* pKey)
{
    return getDataForKey(pKey, Data::Null);
}

Data UserDefault::getDataForKey(const char* pKey, const Data& defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);
    
    const char* encodedData = nullptr;
	tinyxml2::XMLElement* rootNode;
	tinyxml2::XMLDocument* doc;
	tinyxml2::XMLElement* node;
	node =  getXMLNodeForKey(pKey, &rootNode, &doc);
	// find the node
	if (node && node->FirstChild())
	{
        encodedData = (const char*)(node->FirstChild()->Value());
	}
    
	Data ret = defaultValue;
    
	if (encodedData)
	{
        unsigned char * decodedData = nullptr;
        int decodedDataLen = base64Decode((unsigned char*)encodedData, (unsigned int)strlen(encodedData), &decodedData);
        
        if (decodedData) {
            ret.fastSet(decodedData, decodedDataLen);
        }
	}
    
    if (doc) delete doc;
    
	return ret;    
}


void UserDefault::setBoolForKey(const char* pKey, bool value)
{
    // save bool value as string

    if (true == value)
    {
        setStringForKey(pKey, "true");
    }
    else
    {
        setStringForKey(pKey, "false");
    }
}

void UserDefault::setIntegerForKey(const char* pKey, int value)
{
    // check key
    if (! pKey)
    {
        return;
    }

    // format the value
    char tmp[50];
    memset(tmp, 0, 50);
    sprintf(tmp, "%d", value);

    setValueForKey(pKey, tmp);
}

void UserDefault::setFloatForKey(const char* pKey, float value)
{
    setDoubleForKey(pKey, value);
}

void UserDefault::setDoubleForKey(const char* pKey, double value)
{
    // check key
    if (! pKey)
    {
        return;
    }

    // format the value
    char tmp[50];
    memset(tmp, 0, 50);
    sprintf(tmp, "%f", value);

    setValueForKey(pKey, tmp);
}

void UserDefault::setStringForKey(const char* pKey, const std::string & value)
{
    // check key
    if (! pKey)
    {
        return;
    }

    setValueForKey(pKey, value.c_str());
}
/**
 *  @author 徐壮   xz change_sym, 16-04-05 18:04:55
 *
 *  将字符串加密之后写入ccuserdefault
 *
 *  @param key   key
 *  @param value value
 */
void UserDefault::setStringForKeyWithEnc(const char* pKey, const std::string & value)
{
    // check key
    if (!pKey || 0 == strcmp(pKey, ""))
    {
        return;
    }
    
    std::string tempKey = pKey;
    tempKey.append("AgainstZ").append("X");
    Data dataValue = userDEnc(value.c_str());
    setDataForKey(tempKey.c_str(), dataValue);
}

void UserDefault::setDataForKey(const char* pKey, const Data& value) {
    // check key
    if (! pKey)
    {
        return;
    }
    
    std::lock_guard<LockType> _(m_dataMutex);
    
    char *encodedData = 0;
    
    base64Encode(value.getBytes(), static_cast<unsigned int>(value.getSize()), &encodedData);
        
    setValueForKey(pKey, encodedData);
    
    if (encodedData)
        free(encodedData);
}

UserDefault* UserDefault::getInstance()
{
    if (!_userDefault)
    {
        initXMLFilePath();

        // only create xml file one time
        // the file exists after the program exit
        if ((!isXMLFileExist()) && (!createXMLFile()))
        {
            return nullptr;
        }

        _userDefault = new (std::nothrow) UserDefault();
    }

    return _userDefault;
}

void UserDefault::destroyInstance()
{
    CC_SAFE_DELETE(_userDefault);
}

void UserDefault::setDelegate(UserDefault *delegate)
{
    if (_userDefault)
        delete _userDefault;

    _userDefault = delegate;
}

bool UserDefault::isXMLFileExist()
{
    return FileUtils::getInstance()->isFileExist(_filePath);
}

void UserDefault::initXMLFilePath()
{
    if (! _isFilePathInitialized)
    {
        _filePath += FileUtils::getInstance()->getWritablePath() + XML_FILE_NAME;
        _isFilePathInitialized = true;
    }    
}

// create new xml file
bool UserDefault::createXMLFile()
{
	bool bRet = false;  
    tinyxml2::XMLDocument *pDoc = new tinyxml2::XMLDocument(); 
    if (nullptr==pDoc)  
    {  
        return false;  
    }  
	tinyxml2::XMLDeclaration *pDeclaration = pDoc->NewDeclaration(nullptr);  
	if (nullptr==pDeclaration)  
	{  
		return false;  
	}  
	pDoc->LinkEndChild(pDeclaration); 
	tinyxml2::XMLElement *pRootEle = pDoc->NewElement(USERDEFAULT_ROOT_NAME);  
	if (nullptr==pRootEle)  
	{  
		return false;  
	}  
	pDoc->LinkEndChild(pRootEle);  
    bRet = tinyxml2::XML_SUCCESS == pDoc->SaveFile(FileUtils::getInstance()->getSuitableFOpen(_filePath).c_str());

	if(pDoc)
	{
		delete pDoc;
	}

	return bRet;
}

const string& UserDefault::getXMLFilePath()
{
    return _filePath;
}

void UserDefault::flush()
{
}

void UserDefault::resetUserDefault()
{
    
}

NS_CC_END

#endif // (CC_TARGET_PLATFORM != CC_PLATFORM_IOS && CC_PLATFORM != CC_PLATFORM_ANDROID)
