//
//  CCGlobalLock.hpp
//  cocos2d_libs
//
//  Created by Kai Guo on 15/12/3.
//
//

#ifndef CCGlobalLock_hpp
#define CCGlobalLock_hpp

#include <stdio.h>
#include <mutex>
#include "cocos2d.h"

NS_CC_BEGIN

#define         GBLK_GLOBAL 0
#define         GBLK_GLOBAL_LOCK_UNIQUE


class GlobalLock {
    

private:

    GlobalLock();
    ~GlobalLock();
    
public:
    std::recursive_mutex* _mtx;
    static GlobalLock _lock;
    
//    static bool tryLock( int type);
//    static void rollbackLock();
//    static void unlock( int type );
//    static void lock( int type );
    
    static bool trylockMainThread( int type = GBLK_GLOBAL );
    static void lockMainThread( int type = GBLK_GLOBAL );
    static void unlockMainThread( int type = GBLK_GLOBAL );
    
    static void lockMain2Thread();
    static void unlockMain2Thread();
    
    static bool trylockThread( int type );
    static void lockThread( int type );
    static void unlockThread( int type );
    
    enum Class_Mutex{
        Class_SpriteFrameCache = 0,
        Class_TextureCache,
        Class_ResourceUsedCache,
        Class_VolatileMgr,
        Class_FileUtilsCache,
        Class_MAX
    };
    
    static void unlockAllClassTypesMutex();
    static void lockAllClassTypesMutex();
    
    class AutoLock {
    private:
        Class_Mutex _type;
    public:
        AutoLock(Class_Mutex type);
        ~AutoLock();
    };
    
};


NS_CC_END

#endif /* CCGlobalLock_hpp */
