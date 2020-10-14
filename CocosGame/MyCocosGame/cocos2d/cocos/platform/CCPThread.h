//
//  CCPThread.h
//  IF
//
//  Created by 邹 程 on 13-9-6.
//
//

#ifndef __IF__CCPThread__
#define __IF__CCPThread__

#include <pthread.h>
#include "cocos2d.h"
#include "CCQueue.h"

#define PTHREAD_NAME_MAX 32

NS_CC_BEGIN
class CCMessage
{
public:
    int m_id;
    void* m_data;
};

typedef bool (cocos2d::Ref::*CCMessageHandler)(const CCMessage &msg);
#define message_handler_selector(_SELECTOR) (CCMessageHandler)(&_SELECTOR)

class CCPThread
{
public:
    
    static CCPThread* create(const char *name);
    
    // register a handler to current hander thread
    // when a message comes,
    // "processMessage" will dispatch msg to each registered handler
    void registerHandler(const Ref *pTarget, const CCMessageHandler handler);
    
    // unregister a handler
    void unregisterHandler(const Ref *pTarget, const CCMessageHandler handler);
    
    // post a message to current handler thread, it's async call
    void postMessage(const CCMessage &msg);
    
    // drop stub message
    int dropMessage();
    
    ~CCPThread();
    friend class Director;
    friend class DisplayLinkDirector;
private:
    CCPThread(const char* name);
    
    CCPThread(const CCPThread&) = default;
    CCPThread& operator=(const CCPThread&) = default;
    
    static void processMessage(CCPThread* pThread);
    static bool hasMessage(CCPThread* pThread);
    static void* loop(void* pArgs);
    
    class Invoker{
    public:
        Invoker(const Ref *pTarget,const CCMessageHandler handler):
        m_pTarget((Ref*)pTarget)
        ,m_handler(handler)
        {}
        
        bool operator==(const Invoker &rhs) const {
            return m_pTarget == rhs.m_pTarget && m_handler == rhs.m_handler;
        }
        
        Ref *m_pTarget;
        CCMessageHandler m_handler;
    };
    
    static bool processInternal(const Invoker &invoker,const void *args) {
        return (invoker.m_pTarget->*(invoker.m_handler))(*(CCMessage*)args);
    }
    
    char m_name[PTHREAD_NAME_MAX+1];
    CCQueue<Invoker> m_invokerStack;
    CCQueue<CCMessage> m_messageStack;
    
    pthread_mutex_t m_mutexInvoker;
    pthread_mutex_t m_mutexMessage;
    pthread_cond_t m_condMessage;
    
    pthread_t m_pThreadId;
    
};
NS_CC_END
#endif /* defined(__IF__CCPThread__) */
