#include "base/CCIMEDispatcher.h"
#include "base/CCDirector.h"
#include "base/CCEventType.h"
#include "base/CCEventCustom.h"
#include "base/CCEventDispatcher.h"
#include "base/CCGlobalLock.h"
#include "../CCApplication.h"
#include "platform/CCFileUtils.h"
#include "JniHelper.h"
#include <jni.h>

extern pthread_t MAIN_THREAD_ID;

using namespace cocos2d;

extern bool g_isInLoadingScene;
extern bool g_isInDataParsing;

extern "C" {

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeRender(JNIEnv* env) {
        cocos2d::GlobalLock::lockMainThread();
        cocos2d::Director::getInstance()->mainLoop();
        cocos2d::GlobalLock::unlockMainThread();
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnPause() {
        if (Director::getInstance()->getOpenGLView()) {
                Application::getInstance()->applicationDidEnterBackground();
                cocos2d::EventCustom backgroundEvent(EVENT_COME_TO_BACKGROUND);
                cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&backgroundEvent);
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnResume() {
        if (Director::getInstance()->getOpenGLView()) {
            MAIN_THREAD_ID = pthread_self();
            Application::getInstance()->applicationWillEnterForeground();
            cocos2d::EventCustom foregroundEvent(EVENT_COME_TO_FOREGROUND);
            cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&foregroundEvent);
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeInsertText(JNIEnv* env, jobject thiz, jstring text) {
        const char* pszText = env->GetStringUTFChars(text, NULL);
        cocos2d::IMEDispatcher::sharedDispatcher()->dispatchInsertText(pszText, strlen(pszText));
        env->ReleaseStringUTFChars(text, pszText);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeDeleteBackward(JNIEnv* env, jobject thiz) {
        cocos2d::IMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
    }

    JNIEXPORT jstring JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeGetContentText() {
        JNIEnv * env = 0;

        if (JniHelper::getJavaVM()->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK || ! env) {
            return 0;
        }
        std::string pszText = cocos2d::IMEDispatcher::sharedDispatcher()->getContentText();
        return env->NewStringUTF(pszText.c_str());
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeLockMainThread(JNIEnv* env) {
        cocos2d::GlobalLock::lockMainThread();
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeUnlockMainThread(JNIEnv* env) {
        cocos2d::GlobalLock::unlockMainThread();
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeLockThread(JNIEnv* env, jclass, jint type) {
        cocos2d::GlobalLock::lockThread((int)type);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeUnlockThread(JNIEnv* env, jclass, jint type) {
        cocos2d::GlobalLock::unlockThread((int)type);
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeDoBackWorkProc(JNIEnv *, jclass, jlong handle) {
        std::function<void()>* pfunc = (std::function<void()>*)handle;
        if ( *pfunc ) {
            (*pfunc)();
            delete pfunc;
        }
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeDoGLWorkProc(JNIEnv *, jclass, jlong handle) {
        std::function<void()>* pfunc = (std::function<void()>*)handle;
        if ( *pfunc ) {
            (*pfunc)();
            delete pfunc;
        }
    }

    JNIEXPORT jboolean JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeIsLoadingScene() {
        //bool res = Director::getInstance()->isHavePriorityWorking();
        if( g_isInLoadingScene ) {
            return JNI_TRUE;
        }
        return JNI_FALSE;
    }

    JNIEXPORT jboolean JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeIsInDataParsing() {
        //bool res = Director::getInstance()->isHavePriorityWorking();
        if( g_isInDataParsing ) {
            return JNI_TRUE;
        }
        return JNI_FALSE;
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnPauseInApp() {
        CCNotificationCenter::sharedNotificationCenter()->postNotification("msg_native_on_pause_in_app");
    }

    JNIEXPORT void JNICALL Java_org_cocos2dx_lib_Cocos2dxRenderer_nativeOnResumeInApp() {
        CCNotificationCenter::sharedNotificationCenter()->postNotification("msg_native_on_resume_in_app");
    }

}
