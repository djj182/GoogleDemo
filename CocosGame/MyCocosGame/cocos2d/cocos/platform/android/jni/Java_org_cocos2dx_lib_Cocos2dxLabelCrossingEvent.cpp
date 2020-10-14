/****************************************************************************
Copyright (c) 2012 cocos2d-x.org
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

#include "JniHelper.h"
#include <string.h>

#include "2d/CCInternalEvent.h"
#include <assert.h>

#include <jni.h>

using namespace cocos2d;

extern "C" 
{

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_CCLabelCrossingEvent_nativeLabelBitmapEvent(JNIEnv * env, jobject thiz, 
    	jstring text, jstring fontName, jint fontSize,
        jint cw, jint ch, jint bw, jint bh, jint aw, jint ah) 
    {
        std::string _text =  JniHelper::jstring2string(text);
        std::string _fontName =  JniHelper::jstring2string(fontName);
        
        struct ccEventLabelCrossingCallback* _cb = ccGetEventCallbackLabelCrossing();
        assert(NULL != _cb->callback && "_cb->callback is a null.");

		ccEventLabelCrossingElem _args;
		_args.text = _text;
		_args.fontName = _fontName;
		_args.fontSize = fontSize;
		_args.cw = cw;
		_args.ch = ch;
		_args.bw = bw;
		_args.bh = bh;
		_args.aw = aw;
		_args.ah = ah;

        _cb->callback(_cb->obj, &_args);
    }
}
