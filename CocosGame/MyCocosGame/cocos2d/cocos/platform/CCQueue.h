//
//  CCQueue.h
//  IF
//
//  Created by 邹 程 on 13-9-6.
//
//

#ifndef IF_CCQueue_h
#define IF_CCQueue_h

#include <memory.h>
#include "cocos2d.h"

template <class T>
class CCQueue {
public:
    CCQueue():
    m_size(0)
    ,m_maxSize(4)
    ,m_front(0)
    ,m_end(0)
    {
        m_queue = (T *)calloc(4, sizeof(T));
        CCAssert(m_queue != NULL, "Alloc CCQueue failed!");
    }
    
    ~CCQueue(){
        m_size = m_front = m_end = 0;
        free(m_queue);
    }
    
    const bool empty() {
        return m_size == 0;
    }
    
    typedef bool (*Traverse)(const T& target,const void *args);
    
    void traverse(Traverse doTraverse,const void *arges) {
        if (!doTraverse || m_size == 0) {
            return;
        }
        for (int i = 0;i < m_size;i++) {
            doTraverse(m_queue[(i+m_front)%m_maxSize],arges);
        }
    }

    int traverseBreakAtFirstProcess(Traverse doTraverse,const void *args) {
        if (!doTraverse || m_size == 0) {
            return -1;
        }
        for (int i = 0;i < m_size; i++) {
            if (doTraverse(m_queue[(i+m_front)%m_maxSize],args)) {
                return (i+m_front)%m_maxSize;
            }
        }
        return -1;
    }

    bool haveElement(const T& ele) {
        return traverseBreakAtFirstProcess(isEqual, &ele) != -1;
    }

    bool removeElement(const T& ele) {
        int index = traverseBreakAtFirstProcess(isEqual, &ele);
        if (index == -1) {
            return false;
        }
        while ((index+1)%m_maxSize != m_end) {
            memcpy(&m_queue[index], &m_queue[(index + 1)%m_maxSize],sizeof(T));
            index = (index + 1) % m_maxSize;
        }
        m_end = index;
        m_size--;
        return true;
    }

    void push(const T& ele) {
        // when hit max size , double it
        if (m_size == m_maxSize) {
            m_queue = (T *)realloc((void *)m_queue,sizeof(T)*m_maxSize*2);
            CCAssert(m_queue != NULL, "Realloc CCQueue failed!");
            if (m_end > 0) {
                memcpy(m_queue + m_maxSize, m_queue, sizeof(T)*m_end);
            }
            m_end += m_maxSize;
            m_maxSize *= 2;
        }
        m_queue[m_end] = ele;
        m_end = (m_end + 1) % m_maxSize;
        m_size++;
    }

    T pop() {
        CCAssert(!empty(), "CCQueue empty, can not pop!");
        int retFront = m_front;
        m_front = (m_front + 1) % m_maxSize;
        m_size--;
        return m_queue[retFront];
    }

private:

    static bool isEqual(const T& rhs,const void *args) {
        return rhs == *((T *)args);
    }
    
    int m_size;
    int m_maxSize;
    int m_front;
    int m_end;
    T *m_queue;
};


#endif
