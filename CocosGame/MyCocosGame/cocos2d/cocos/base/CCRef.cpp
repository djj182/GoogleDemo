/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies

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

#include "base/CCRef.h"
#include "base/CCAutoreleasePool.h"
#include "base/ccMacros.h"
#include "base/CCScriptSupport.h"
#include "base/CCDirector.h"
#if CC_REF_LEAK_DETECTION
#include <algorithm>    // std::find
#endif

#include <set>

extern pthread_t BACK_PROCESS_THREAD_ID;
extern bool g_isApplicationExit;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

//#define CC_ENABLE_BACKTRACE
#ifdef CC_ENABLE_BACKTRACE

#include <iostream>
#include <iomanip>
#include <unwind.h>
#include <dlfcn.h>
#include <stdio.h>

namespace test_debug{
    
    struct BacktraceState
    {
        void** current;
        void** end;
    };
    
    static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
    {
        BacktraceState* state = static_cast<BacktraceState*>(arg);
        uintptr_t pc = _Unwind_GetIP(context);
        if (pc) {
            if (state->current == state->end) {
                return _URC_END_OF_STACK;
            } else {
                *state->current++ = reinterpret_cast<void*>(pc);
            }
        }
        return _URC_NO_REASON;
    }
    
}

size_t captureBacktrace(void** buffer, size_t max)
{
    test_debug::BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(test_debug::unwindCallback, &state);
    
    return state.current - buffer;
}

void dumpBacktrace(void** buffer, size_t count)
{
    std::string filecontent;
    for (size_t idx = 0; idx < count; ++idx) {
        const void* addr = buffer[idx];
        const char* symbol = "";
        
        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) {
            symbol = info.dli_sname;
        }
        
        char buff[200] = {0};
        snprintf(buff, sizeof(buff), "backtrace #%zu 0x%llu %s", idx, (uint64_t)addr, symbol);
        
        filecontent += buff;
        filecontent += "\n";
        cocos2d::log("%s",buff);
    }
    filecontent += "=============================\n";
    FILE* file = fopen("/sdcard/backtrace.log","a+");
    
    if (file != NULL)
    {
        fputs(filecontent.c_str(), file);
        fflush(file);
        fclose(file);
    }
    
}

#endif
#endif

#include "2d/CCSprite.h"
#include "2d/CCLabel.h"
#include "ui/UIScale9Sprite.h"
#include "deprecated/CCString.h"
using namespace cocos2d::ui;

extern bool _main_thread_init;
extern decltype(std::this_thread::get_id()) _main_thread_id;

#if 0
class AllReleaseCheck
{
public:
    AllReleaseCheck() : curReleaseIdx(0)
    {
        
    }
    
    bool check(cocos2d::Ref* r)
    {
        return false;
        
        if (curReleaseIdx > 0) {
            int idx = (curReleaseIdx - 1) % 5;
            checkSlot(idx, r);
        }
        int idx = (curReleaseIdx) % 5;
        checkSlot(curReleaseIdx, r);
        allRelease[idx].insert(std::make_pair(r, r->getTypeName()));
        return false;
    }
    
    bool checkSlot(int idx, cocos2d::Ref* r)
    {
        auto ite = allRelease[idx].find(r);
        if (ite == allRelease[idx].end())
        {
            return false;
        }
        
        CCLOG("checkSlot error!!! [%p] %s", ite->first, ite->second.c_str());
//        assert(0);
        return true;
    }
    
    void remove(cocos2d::Ref* r)
    {
        return;
        
        std::string pp = r->getTypeName();
        for (int i=0;i<5;++i) {
            auto ite = allRelease[i].find(r);
            if (ite == allRelease[i].end())
            {
                continue;
            }
            
            std::string& tt = ite->second;
            if (tt == "13LoadingScene2")
            {
                int a = 0;
            }
        }
    }
    
private:
    std::map<cocos2d::Ref*, std::string> allRelease[5];
    int curReleaseIdx;
}
gAllReleaseCheck;


std::set<cocos2d::Ref*> allRefs;


USING_NS_CC;
void printRefAll()
{
    CCLOG("printAll count(%d)", allRefs.size());
    
    for (auto ite = allRefs.begin(); ite != allRefs.end(); ++ite)
    {
//        CCLOG("    %p, %s", (*ite), (*ite)->getTypeName().c_str());
        
        
        
        Ref* p = (*ite);
        Label* ll = dynamic_cast<Label*>(p);
        Sprite* spr = dynamic_cast<Sprite*>(p);
        Scale9Sprite* spr9 = dynamic_cast<Scale9Sprite*>(p);
        __String* str = dynamic_cast<__String*>(p);

        std::string s;
        if (ll)
        {
            s = ll->getString();
        }
        if (spr)
        {
            s = spr->getSpriteFrameName();
        }
        if (spr9 && spr9->getSprite())
        {
            s = spr9->getSprite()->getSpriteFrameName();
        }
        if (str)
        {
            s = str->getCString();
        }
        
        CCLOG("[++]    %p(%d), auto(%d), %s, %s", p, p->getReferenceCount(),
              p->isAutoReleased(),
              p->getTypeName().c_str(), s.c_str());
        
    }
    
    return;
}
#endif

NS_CC_BEGIN

bool Ref::ms_bEnableAutoRelease = true;
void Ref::enableAutorelease(bool bIsEnable)
{
    ms_bEnableAutoRelease = bIsEnable;
}

Ref::Ref()
: _referenceCount(1) // when the Ref is created, the reference count of it is 1
,_isInAutoPool(false)
,_isObjPaused(false),_isObjAnimation(false)
{
#if CC_ENABLE_SCRIPT_BINDING
    static unsigned int uObjectCount = 0;
    _luaID = 0;
    _ID = ++uObjectCount;
    _scriptObject = nullptr;
#endif
    
// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    increaseInstanceCount(true);
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    _lifeFg = std::make_shared<bool>(true);
    
    if (BACK_PROCESS_THREAD_ID == pthread_self()) {
        _isObjPaused = true;
        Director::getInstance()->addPausedList(this);
    }

//    gAllReleaseCheck.remove(this);
//    allRefs.insert(this);
}

//Ref::Ref(const Ref &rRef)
//{
//    *this = rRef;
//}

Ref::Ref(const Ref &rRef):_isInAutoPool(false)
{
    *this = rRef;
// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    increaseInstanceCount(true);
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS

    _lifeFg = std::make_shared<bool>(true);

    if (BACK_PROCESS_THREAD_ID == pthread_self()) {
        _isObjPaused = true;
        Director::getInstance()->addPausedList(this);
    }
}


Ref::~Ref()
{
#if CC_ENABLE_SCRIPT_BINDING
    // if the object is referenced by Lua engine, remove it
    if (_luaID)
    {
        ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptObjectByObject(this);
    }
    else
    {
        // we use lua engine so we don't need these source!!! guok
//        ScriptEngineProtocol* pEngine = ScriptEngineManager::getInstance()->getScriptEngine();
//        if (pEngine != nullptr && pEngine->getScriptType() == kScriptTypeJavascript)
//        {
//            pEngine->removeScriptObjectByObject(this);
//        }
    }
#endif

//    allRefs.erase(this);

// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    increaseInstanceCount(false);
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    
    if (_isObjPaused) {
        Director::getInstance()->removePausedList(this);
    }
}

void Ref::retain()
{
    CCASSERT(_referenceCount > 0, "reference count should be greater than 0");
    if(_referenceCount <= 0){
        return;
    }
    
    ++_referenceCount;
    return;
}

void Ref::release()
{
    if (g_isApplicationExit) {
        return;
    }
    
    CCASSERT(_referenceCount > 0, "reference count should be greater than 0");
    if(_referenceCount <= 0){
        CCLOG("[bugbug] _referenceCount <= 0");
//        gAllReleaseCheck.check(this);
        return;
    }
    
    --_referenceCount;
    
    bool destory = (_referenceCount <= 0);
    // FIMXE: 看起来这段代码没啥意义。。。
    if(_isInAutoPool)
    {
//        auto poolManager = PoolManager::getInstance();
//        if (poolManager->getCurrentPool() && poolManager->getCurrentPool()->isClearing()) {
//            _isInAutoPool = false;
//        }
    }

    if (destory)
    {
//        gAllReleaseCheck.check(this);
        delete this;
    }
}

Ref* Ref::autorelease()
{
    // zym 2015.12.15 如果游戏退出时就不让进入了
    if(ms_bEnableAutoRelease == false)
        return this;
    
    PoolManager::getInstance()->getCurrentPool()->addObject(this);
    _isInAutoPool = true;
    return this;
}

unsigned int Ref::getReferenceCount() const
{
    return _referenceCount;
}

bool Ref::isObjPaused()
{
    return _isObjPaused;
}

bool Ref::isObjAnimation()
{
    return _isObjAnimation;
}

void Ref::setObjPaused(bool isPaused)
{
    _isObjPaused = isPaused;
}

void Ref::setObjAnimation(bool isAnimation)
{
    _isObjAnimation = isAnimation;
}

bool Ref::isAutoReleased()
{
    return _isInAutoPool;
}

// 返回类型信息，调试使用
std::string Ref::getTypeName()
{
    try {
        return typeid(*this).name();
    }
    catch (const std::bad_typeid& e) {
        std::string ret = "[]";
        ret.insert(1, e.what());
        return ret;
    }
    
    return "[?]";
}

// get how many Ref objects is running, added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
std::atomic_ulong Ref::instanceCount(0);
unsigned long Ref::getInstanceCount()
{
    return instanceCount;
}

void Ref::increaseInstanceCount(bool increased)
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

#endif //#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS

NS_CC_END
