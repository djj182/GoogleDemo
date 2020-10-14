//
//  CCGlobalLock.cpp
//  cocos2d_libs
//
//  Created by Kai Guo on 15/12/3.
//
//

#include "CCGlobalLock.h"

#define PERF_TEST 0
#if PERF_TEST
extern pthread_t MAIN_THREAD_ID;
extern bool g_isInLoadingScene;
extern std::unordered_map<std::string, long> methodCall_perfomance;
extern int* loading_progress;
extern long mainloop_dur[10];
static int cmp(const std::pair<std::string, long>& x, const std::pair<std::string, long>& y) {
    return x.second > y.second;
}


std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

std::chrono::high_resolution_clock::time_point start_run = std::chrono::high_resolution_clock::now();

int internalCall = 0;
#endif

extern pthread_t BACK_PROCESS_THREAD_ID;

NS_CC_BEGIN

GlobalLock GlobalLock::_lock;

static std::recursive_mutex* _mtxBackThread = new std::recursive_mutex();
static std::mutex* _mtxMain2Thread = new std::mutex();

static bool isFinalized = false;
GlobalLock::GlobalLock() {
    _mtx = new std::recursive_mutex();
}

GlobalLock::~GlobalLock() {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    //_mtx->unlock();
#endif
    //delete _mtx;
    isFinalized = true;
}

void GlobalLock::lockMain2Thread() {
    _mtxMain2Thread->lock();
}

void GlobalLock::unlockMain2Thread() {
    _mtxMain2Thread->unlock();
}
void GlobalLock::lockMainThread( int type ) {
    if(isFinalized) return;
    
#if PERF_TEST
    internalCall++;
    start = std::chrono::high_resolution_clock::now();
#endif
    
    GlobalLock::_lock._mtx->lock();
    
#if PERF_TEST
    start_run = std::chrono::high_resolution_clock::now();
#endif
}

void GlobalLock::unlockMainThread( int type ) {
    
    if(isFinalized) return;
    GlobalLock::_lock._mtx->unlock();
   
#if PERF_TEST
    internalCall--;
    auto end = std::chrono::high_resolution_clock::now();
    long dur = static_cast<long>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    
    bool isMainThread = false;
    if( pthread_self() == MAIN_THREAD_ID ) {
        isMainThread = true;
    }
    
    if( dur > 80 * 1000 && g_isInLoadingScene  ) {
        long dur_run = static_cast<long>(std::chrono::duration_cast<std::chrono::microseconds>(end - start_run).count());
        cocos2d::log("THDTest: %ld %d %ld %d %d %d", dur, isMainThread, dur_run, 1, internalCall, loading_progress?(*loading_progress):-123);
        
        cocos2d::log("THDTest: mainloop: %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
                     mainloop_dur[0],
                     mainloop_dur[1],
                     mainloop_dur[2],
                     mainloop_dur[3],
                     mainloop_dur[4],
                     mainloop_dur[5],
                     mainloop_dur[6],
                     mainloop_dur[7],
                     mainloop_dur[8],
                     mainloop_dur[9]);
        
        std::vector<std::pair<std::string,long> > tVector;
        tVector.clear();
        for (std::unordered_map<std::string, long>::iterator curr = methodCall_perfomance.begin(); curr != methodCall_perfomance.end(); curr++)
        {
            tVector.push_back(std::make_pair(curr->first, curr->second));
        }
        std::sort(tVector.begin(), tVector.end(), cmp);
        
        size_t len = tVector.size();
        len = len > 3 ? 3 : len;
        for (size_t i = 0; i < len ; ++i ) {
            char tempBuffer[100] = {0};
            sprintf(tempBuffer, "%04ld %s", tVector.at(i).second, tVector.at(i).first.c_str());
            if( i == 0 )cocos2d::log("THDTest: %s", tempBuffer);
            if( i == 1 )cocos2d::log("THDTest: %s", tempBuffer);
            if( i == 2 )cocos2d::log("THDTest: %s", tempBuffer);

        }
    }
    
#endif
    
}

bool GlobalLock::trylockMainThread( int type ) {
    if(isFinalized) return false;
    return GlobalLock::_lock._mtx->try_lock();
}

bool GlobalLock::trylockThread( int type ) {
    if( type == 100 ) {
        bool res = _mtxBackThread->try_lock();
        return res;
    }
    return false;
}

void GlobalLock::lockThread( int type ) {
    if( type == 100 ) {
        _mtxBackThread->lock();
        return;
    }
}
void GlobalLock::unlockThread( int type ) {
    if( type == 100 ) {
        _mtxBackThread->unlock();
        return;
    }
}

struct MutexStruct {
    std::recursive_mutex _mutex;
    int _lock_count{0};
};

static MutexStruct* classTypesMutex[GlobalLock::Class_Mutex::Class_MAX] = {0};

//std::recursive_mutex* GlobalLock::getMutexByClassType(Class_Mutex type) {
//    auto ret = classTypesMutex[type];
//    if (ret) {
//        return &ret->_mutex;
//    }
//    classTypesMutex[type] = new MutexStruct;
//    return &classTypesMutex[type%GlobalLock::Class_Mutex::Class_MAX]->_mutex;
//}
static std::mutex* classTypesMutex_SafeLock = new std::mutex;
void GlobalLock::unlockAllClassTypesMutex() {
//    std::lock_guard<std::mutex> temp(*classTypesMutex_SafeLock);
    for (int i = 0; i < GlobalLock::Class_Mutex::Class_MAX; ++i) {
        MutexStruct* ptr = nullptr;
        classTypesMutex_SafeLock->lock();
        ptr = classTypesMutex[i];
        int count = ptr ? ptr->_lock_count : 0;
        classTypesMutex_SafeLock->unlock();
        if (ptr) {
            for (int j = 0; j < count; ++j) {
                ptr->_mutex.unlock();
            }
        }
    }
}
void GlobalLock::lockAllClassTypesMutex() {
    for (int i = 0; i < GlobalLock::Class_Mutex::Class_MAX; ++i) {
        MutexStruct* ptr = nullptr;
        classTypesMutex_SafeLock->lock();
        ptr = classTypesMutex[i];
        int count = ptr ? ptr->_lock_count : 0;
        classTypesMutex_SafeLock->unlock();
        if (ptr) {
            for (int j = 0; j < count; ++j) {
                
                ptr->_mutex.lock();
            }
        }
    }
}

GlobalLock::AutoLock::AutoLock(GlobalLock::Class_Mutex type):_type(type) {
    
    MutexStruct* ret = nullptr;
    {
        classTypesMutex_SafeLock->lock();
        if( !classTypesMutex[_type]) {
            classTypesMutex[_type] = new MutexStruct;
        }
        ret = classTypesMutex[_type];
        classTypesMutex_SafeLock->unlock();
        
    }
    ret->_mutex.lock();
    classTypesMutex_SafeLock->lock();
    if (BACK_PROCESS_THREAD_ID == pthread_self()) {
        ret->_lock_count++;
    }
    classTypesMutex_SafeLock->unlock();
    
}

GlobalLock::AutoLock::~AutoLock() {
    
    MutexStruct* ret = nullptr;
    {
        classTypesMutex_SafeLock->lock();
        ret = classTypesMutex[_type];
        classTypesMutex_SafeLock->unlock();
    }
   
    
    if(ret) {
        classTypesMutex_SafeLock->lock();
        if (BACK_PROCESS_THREAD_ID == pthread_self()) {
            ret->_lock_count--;
        }
        classTypesMutex_SafeLock->unlock();
        ret->_mutex.unlock();
    }
}

NS_CC_END
