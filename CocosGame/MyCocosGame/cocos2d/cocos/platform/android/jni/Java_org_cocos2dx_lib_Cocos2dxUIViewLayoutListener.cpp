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

#include "cocos2d.h"
#include "2d/CCInternalEvent.h"
#include "base/CCIMEDispatcher.h"
#include "base/CCConsole.h"

#include <assert.h>

#include <jni.h>

using namespace cocos2d;

extern "C" 
{
    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxUIViewLayoutListener_OnDispatchKeyboardWillShow(JNIEnv * env, jobject obj, 
        jobject hRectB, jobject hRectE, jfloat hAnimationDuration) 
    {
        GLView* pGLView = cocos2d::Director::getInstance()->getOpenGLView();
        if(NULL != pGLView)
        {
            cocos2d::Vec2 pos;
            cocos2d::CCSize dim;
            cocos2d::IMEKeyboardNotificationInfo hNotifyInfo;

            jfloatArray j_hRectB = (jfloatArray)(hRectB);
            jfloatArray j_hRectE = (jfloatArray)(hRectE);
            jfloat* jarr_hRectB = env->GetFloatArrayElements(j_hRectB, NULL);
            jfloat* jarr_hRectE = env->GetFloatArrayElements(j_hRectE, NULL);

            float hScaleX = pGLView->getScaleX();
            float hScaleY = pGLView->getScaleY();

            const cocos2d::CCRect& hViewportRect = pGLView->getViewPortRect();
            float hOffestY = hViewportRect.origin.y;
            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillShow hOffestY = %f", hOffestY);
            if (hOffestY < 0.0f)
            {
                jarr_hRectB[1] += hOffestY;
                jarr_hRectB[3] -= hOffestY;
                jarr_hRectE[3] -= hOffestY;
            }

            pos.x = jarr_hRectB[0] / hScaleX;
            pos.y = jarr_hRectB[1] / hScaleY;
            dim.width  = jarr_hRectB[2] / hScaleX;
            dim.height = jarr_hRectB[3] / hScaleY;
            hNotifyInfo.begin = cocos2d::CCRect(pos, dim);

            pos.x = jarr_hRectE[0] / hScaleX;
            pos.y = jarr_hRectE[1] / hScaleY;
            dim.width  = jarr_hRectE[2] / hScaleX;
            dim.height = jarr_hRectE[3] / hScaleY;
            hNotifyInfo.end   = cocos2d::CCRect(pos, dim);

            hNotifyInfo.duration = hAnimationDuration;

            cocos2d::IMEDispatcher* pDispatcher = cocos2d::IMEDispatcher::sharedDispatcher();
            pDispatcher->dispatchKeyboardWillShow(hNotifyInfo);

            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillShow (%f %f %f %f) (%f %f %f %f) %f %f %f", 
            //     jarr_hRectB[0], jarr_hRectB[1], jarr_hRectB[2], jarr_hRectB[3], 
            //     jarr_hRectE[0], jarr_hRectE[1], jarr_hRectE[2], jarr_hRectE[3], hAnimationDuration, hScaleX, hScaleY);
            // 
            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillShow (%f %f %f %f) (%f %f %f %f) %f %f %f", 
            //     hNotifyInfo.begin.origin.x, hNotifyInfo.begin.origin.y, hNotifyInfo.begin.size.width, hNotifyInfo.begin.size.height, 
            //     hNotifyInfo.end.origin.x, hNotifyInfo.end.origin.y, hNotifyInfo.end.size.width, hNotifyInfo.end.size.height, hAnimationDuration, hScaleX, hScaleY);
            // 
            // float adjustHeight = hNotifyInfo.end.origin.y + hNotifyInfo.end.size.height;
            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillShow adjustHeight: %f", adjustHeight);

            env->ReleaseFloatArrayElements(j_hRectB, jarr_hRectB, 0);
            env->ReleaseFloatArrayElements(j_hRectE, jarr_hRectE, 0);
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxUIViewLayoutListener_OnDispatchKeyboardWillHide(JNIEnv * env, jobject obj, 
    	jobject hRectB, jobject hRectE, jfloat hAnimationDuration) 
    {
        GLView* pGLView = cocos2d::Director::getInstance()->getOpenGLView();
        if(NULL != pGLView)
        {
            cocos2d::Vec2 pos;
            cocos2d::CCSize dim;
            cocos2d::IMEKeyboardNotificationInfo hNotifyInfo;

            jfloatArray j_hRectB = (jfloatArray)(hRectB);
            jfloatArray j_hRectE = (jfloatArray)(hRectE);
            jfloat* jarr_hRectB = env->GetFloatArrayElements(j_hRectB, NULL);
            jfloat* jarr_hRectE = env->GetFloatArrayElements(j_hRectE, NULL);

            float hScaleX = pGLView->getScaleX();
            float hScaleY = pGLView->getScaleY();

            const cocos2d::CCRect& hViewportRect = pGLView->getViewPortRect();
            float hOffestY = hViewportRect.origin.y;
            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillHide hOffestY = %f", hOffestY);
            if (hOffestY < 0.0f)
            {
                jarr_hRectB[1] += hOffestY;
                jarr_hRectB[3] -= hOffestY;
                jarr_hRectE[3] -= hOffestY;
            }

            pos.x = jarr_hRectB[0] / hScaleX;
            pos.y = jarr_hRectB[1] / hScaleY;
            dim.width  = jarr_hRectB[2] / hScaleX;
            dim.height = jarr_hRectB[3] / hScaleY;
            hNotifyInfo.begin = cocos2d::CCRect(pos, dim);

            pos.x = jarr_hRectE[0] / hScaleX;
            pos.y = jarr_hRectE[1] / hScaleY;
            dim.width  = jarr_hRectE[2] / hScaleX;
            dim.height = jarr_hRectE[3] / hScaleY;
            hNotifyInfo.end   = cocos2d::CCRect(pos, dim);

            hNotifyInfo.duration = hAnimationDuration;

            cocos2d::IMEDispatcher* pDispatcher = cocos2d::IMEDispatcher::sharedDispatcher();
            pDispatcher->dispatchKeyboardWillHide(hNotifyInfo);

            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillHide (%f %f %f %f) (%f %f %f %f) %f %f %f", 
            //     jarr_hRectB[0], jarr_hRectB[1], jarr_hRectB[2], jarr_hRectB[3], 
            //     jarr_hRectE[0], jarr_hRectE[1], jarr_hRectE[2], jarr_hRectE[3], hAnimationDuration, hScaleX, hScaleY);
            // 
            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillHide (%f %f %f %f) (%f %f %f %f) %f %f %f", 
            //     hNotifyInfo.begin.origin.x, hNotifyInfo.begin.origin.y, hNotifyInfo.begin.size.width, hNotifyInfo.begin.size.height, 
            //     hNotifyInfo.end.origin.x, hNotifyInfo.end.origin.y, hNotifyInfo.end.size.width, hNotifyInfo.end.size.height, hAnimationDuration, hScaleX, hScaleY);
            // 
            // float adjustHeight = hNotifyInfo.end.origin.y + hNotifyInfo.end.size.height;
            // cocos2d::CCLog("UIViewLayoutListener.OnDispatchKeyboardWillHide adjustHeight: %f", adjustHeight);

            env->ReleaseFloatArrayElements(j_hRectB, jarr_hRectB, 0);
            env->ReleaseFloatArrayElements(j_hRectE, jarr_hRectE, 0);
        }
    }
}
