/****************************************************************************
Copyright (c) 2009      On-Core
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
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
#ifndef __EFFECTS_CCINTERNALEVENT_H__
#define __EFFECTS_CCINTERNALEVENT_H__

#include "base/CCRef.h"
#include "platform/CCGL.h"

NS_CC_BEGIN

/**
 * @addtogroup _2d
 * @{
 */

/** Most of time we need expand internal event, here assign global callback function */

// common event base.
class ccEventArgs
{
public:
    virtual ~ccEventArgs(){}
};

// Event for label or rich label crossing.
class ccEventLabelCrossingElem : public ccEventArgs
{
public:
    std::string text;
    std::string fontName;
    int fontSize;
    int cw;
    int ch;
    int bw;
    int bh;
    int aw;
    int ah;
};
typedef void (*EventLabelCrossing)(void* obj, const ccEventArgs* evt);
struct ccEventLabelCrossingCallback
{
    EventLabelCrossing callback;
    void* obj;
};
CC_DLL void ccEventLabelCrossingDefault(void* obj, const ccEventArgs* evt);
CC_DLL void ccSetEventCallbackLabelCrossing(void* obj, EventLabelCrossing callback);
CC_DLL struct ccEventLabelCrossingCallback* ccGetEventCallbackLabelCrossing();


// end of effects group
/// @}

NS_CC_END

#endif // __EFFECTS_CCINTERNALEVENT_H__
