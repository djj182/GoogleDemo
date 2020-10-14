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

#include "platform/CCPlatformConfig.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)

#import <Foundation/Foundation.h>

#include <string>

#import "CCUserDefault.h"
#import "tinyxml2.h"
#import "platform/CCPlatformConfig.h"
#import "platform/CCPlatformMacros.h"
#import "base64.h"
#import "platform/CCFileUtils.h"
#include "base/CCDirector.h"
#define XML_FILE_NAME "UserDefault.xml"

// root name of xml
#define USERDEFAULT_ROOT_NAME    "userDefaultRoot"

//
// UserDefaults 理论上不用加锁，文档说：The UserDefaults class is thread-safe.
// https://developer.apple.com/documentation/foundation/nsuserdefaults
//

using namespace std;

NS_CC_BEGIN

/**
 * implements of UserDefault
 */
UserDefault* UserDefault::_userDefault = nullptr;

// 没使用，但头文件定义了
string UserDefault::_filePath;
bool UserDefault::_isFilePathInitialized = false;

#define USE_CACHE 1


// 这个全局变量的生命周期不好控制，这里直接做成初始分配和结束释放了，生命周期相当于一个成员变量。
static std::unordered_map<std::string, Value>* memberVar_CachesValue = nullptr;

void setCacheKV(const char* key, Value const& v)
{
#ifdef USE_CACHE
    if (memberVar_CachesValue) {
        memberVar_CachesValue->operator[](key) = v;
    }
#endif
}

bool getCacheKV(const char* key, Value& v)
{
#ifdef USE_CACHE
    if (memberVar_CachesValue) {
        auto valIndex = memberVar_CachesValue->find(key);
        if( valIndex != memberVar_CachesValue->end())
        {
            v = valIndex->second;
            return true;
        }
    }
#endif
    
    return false;
}

UserDefault::~UserDefault()
{
    CC_SAFE_DELETE(memberVar_CachesValue);
}

UserDefault::UserDefault()
{
    memberVar_CachesValue = new std::unordered_map<std::string, Value>;
}

bool UserDefault::getBoolForKey(const char* pKey)
{
    return getBoolForKey(pKey, false);
}

bool UserDefault::getBoolForKey(const char* pKey, bool defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    Value v;
    if (getCacheKV(pKey, v)) {
        return v.asBool();
    }
    
    bool ret = defaultValue;
    NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
    if (value)
    {
        ret = [value boolValue];
    }
    
    setCacheKV(pKey, Value(ret));
    return ret;
}

int UserDefault::getIntegerForKey(const char* pKey)
{
    return getIntegerForKey(pKey, 0);
}

int UserDefault::getIntegerForKey(const char* pKey, int defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    Value v;
    if (getCacheKV(pKey, v)) {
        return v.asInt();
    }
    
    int ret = defaultValue;
    
    NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
    if (value)
    {
        ret = [value intValue];
    }

    setCacheKV(pKey, Value(ret));
    return ret;
}

float UserDefault::getFloatForKey(const char* pKey)
{
    return getFloatForKey(pKey, 0);
}

float UserDefault::getFloatForKey(const char* pKey, float defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    Value v;
    if (getCacheKV(pKey, v)) {
        return v.asFloat();
    }
    
    float ret = defaultValue;
    
    NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
    if (value)
    {
        ret = [value floatValue];
    }

    setCacheKV(pKey, Value(ret));
    return ret;
}

double  UserDefault::getDoubleForKey(const char* pKey)
{
    return getDoubleForKey(pKey, 0);
}

double UserDefault::getDoubleForKey(const char* pKey, double defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    Value v;
    if (getCacheKV(pKey, v)) {
        return v.asDouble();
    }

	double ret = defaultValue;
    
    NSNumber *value = [[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:pKey]];
    if (value)
    {
        ret = [value doubleValue];
    }

    setCacheKV(pKey, Value(ret));
    return ret;
}

std::string UserDefault::getStringForKey(const char* pKey)
{
    return getStringForKey(pKey, "");
}
std::string UserDefault::getStringForKeyWithEnc(const char* pKey)
{
    std::string tempKey = pKey;
    tempKey.append("AgainstZ").append("X");
    
    std::lock_guard<LockType> _(m_dataMutex);

    Value v;
    if (getCacheKV(pKey, v)) {
        return v.asString();
    }

    Data value = getDataForKey(tempKey.c_str());
    std::string tempStr2 = userDDec(value);
    return tempStr2;
}
string UserDefault::getStringForKey(const char* pKey, const std::string & defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    Value v;
    if (getCacheKV(pKey, v)) {
        return v.asString();
    }
    
    NSString *str = [[NSUserDefaults standardUserDefaults] stringForKey:[NSString stringWithUTF8String:pKey]];
    if (! str)
    {
        setCacheKV(pKey, Value(defaultValue));
        return defaultValue;
    }
    else
    {
        string ret = [str UTF8String];
        setCacheKV(pKey, Value(ret));
        return ret;
    }
}

Data UserDefault::getDataForKey(const char* pKey)
{
    return getDataForKey(pKey, Data::Null);
}

Data UserDefault::getDataForKey(const char* pKey, const Data& defaultValue)
{
    std::lock_guard<LockType> _(m_dataMutex);

    
    NSData *data = [[NSUserDefaults standardUserDefaults] dataForKey:[NSString stringWithUTF8String:pKey]];
    if (! data)
    {
        return defaultValue;
    }
    else
    {
        Data ret;
        ret.copy((unsigned char*)data.bytes, data.length);
        return ret;
    }
}

void UserDefault::setBoolForKey(const char* pKey, bool value)
{
    std::lock_guard<LockType> _(m_dataMutex);
    setCacheKV(pKey, Value(value));
    
    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:value] forKey:[NSString stringWithUTF8String:pKey]];
}

void UserDefault::setIntegerForKey(const char* pKey, int value)
{
    std::lock_guard<LockType> _(m_dataMutex);
    setCacheKV(pKey, Value(value));
    
    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithInt:value] forKey:[NSString stringWithUTF8String:pKey]];
}

void UserDefault::setFloatForKey(const char* pKey, float value)
{
    std::lock_guard<LockType> _(m_dataMutex);
    setCacheKV(pKey, Value(value));
    
    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithFloat:value] forKey:[NSString stringWithUTF8String:pKey]];
}

void UserDefault::setDoubleForKey(const char* pKey, double value)
{
    std::lock_guard<LockType> _(m_dataMutex);
    setCacheKV(pKey, Value(value));
    
    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithDouble:value] forKey:[NSString stringWithUTF8String:pKey]];
}

void UserDefault::setStringForKey(const char* pKey, const std::string & value)
{
    if (strcmp(pKey, "") == 0){
        assert(0);
    }
    std::lock_guard<LockType> _(m_dataMutex);
    setCacheKV(pKey, Value(value));
    
    [[NSUserDefaults standardUserDefaults] setObject:[NSString stringWithUTF8String:value.c_str()] forKey:[NSString stringWithUTF8String:pKey]];
}

void UserDefault::setStringForKeyWithEnc(const char* pKey, const std::string & value)
{
    // check key
    if (!pKey || 0 == strcmp(pKey, ""))
    {
        return;
    }
    
    std::string tempKey = pKey;
    tempKey.append("AgainstZ").append("X");
    
    std::lock_guard<LockType> _(m_dataMutex);

    Data dataValue = userDEnc(value.c_str());

    setCacheKV(pKey, Value(value));
    setDataForKey(tempKey.c_str(), dataValue);
}

void UserDefault::setDataForKey(const char* pKey, const Data& value)
{
    std::lock_guard<LockType> _(m_dataMutex);
  
    [[NSUserDefaults standardUserDefaults] setObject:[NSData dataWithBytes: value.getBytes() length: value.getSize()] forKey:[NSString stringWithUTF8String:pKey]];
}

UserDefault* UserDefault::getInstance()
{    
    if (! _userDefault)
    {
        _userDefault = new (std::nothrow) UserDefault();
    }
    
    return _userDefault;
}

void UserDefault::destroyInstance()
{
    CC_SAFE_DELETE(_userDefault);
}

// FIXME:: deprecated
UserDefault* UserDefault::sharedUserDefault()
{
    return UserDefault::getInstance();
}

// FIXME:: deprecated
void UserDefault::purgeSharedUserDefault()
{
    UserDefault::destroyInstance();
}

bool UserDefault::isXMLFileExist()
{
    return false;
}

void UserDefault::initXMLFilePath()
{
    // 没必要支持了
}

// create new xml file
bool UserDefault::createXMLFile()
{
    return false;
}

const string& UserDefault::getXMLFilePath()
{
    return "";
}

void UserDefault::flush()
{
    std::lock_guard<LockType> _(m_dataMutex);
    [[NSUserDefaults standardUserDefaults] synchronize];
}

void UserDefault::resetUserDefault()
{
    std::lock_guard<LockType> _(m_dataMutex);
    memberVar_CachesValue->clear();
    
    NSString *domainName = [[NSBundle mainBundle] bundleIdentifier];
    [[NSUserDefaults standardUserDefaults] removePersistentDomainForName:domainName];
}

void UserDefault::setValueForKey(const char* pKey, const char* pValue)
{
    
}


NS_CC_END

#endif // (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
