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
#include "JniHelper.h"
#include <android/log.h>
#include <string.h>
#include <algorithm>
#include <pthread.h>

#include "base/ccUTF8.h"

#define  LOG_TAG    "JniHelper"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

static pthread_key_t g_key;

jclass _getClassID(const char *className) {
    if (nullptr == className) {
        return nullptr;
    }
    // modfy by liudi, fit old string write
    // LOGE("Class to Find %s", className);
    std::string str(className);
    std::replace( str.begin(), str.end(), '/', '.'); // replace all '/' to '.'
    const char *newName = str.c_str();
    // LOGE("Class do Find %s", newName);

    JNIEnv* env = cocos2d::JniHelper::getEnv();
    if( !env )
    {
         LOGE("Failed to get JNIEnv");
        return nullptr;
    
    }
    jstring _jstrClassName = env->NewStringUTF(newName);

    //modefy end by liudi

    jclass _clazz = (jclass) env->CallObjectMethod(cocos2d::JniHelper::classloader,
                                                   cocos2d::JniHelper::loadclassMethod_methodID,
                                                   _jstrClassName);

    if (nullptr == _clazz) {
        LOGE("Classloader failed to find class of %s", className);
        env->ExceptionClear();
    }

    env->DeleteLocalRef(_jstrClassName);
        
    return _clazz;
}

void _detachCurrentThread(void* a) {
    cocos2d::JniHelper::getJavaVM()->DetachCurrentThread();
}

namespace cocos2d {

    JavaVM* JniHelper::_psJavaVM = nullptr;
    jmethodID JniHelper::loadclassMethod_methodID = nullptr;
    jobject JniHelper::classloader = nullptr;

    JavaVM* JniHelper::getJavaVM() {
        // pthread_t thisthread = pthread_self();
        // LOGD("JniHelper::getJavaVM(), pthread_self() = %ld", thisthread);
        return _psJavaVM;
    }

    void JniHelper::setJavaVM(JavaVM *javaVM) {
        // pthread_t thisthread = pthread_self();
        // LOGD("JniHelper::setJavaVM(%p), pthread_self() = %ld", javaVM, thisthread);
        _psJavaVM = javaVM;

        pthread_key_create(&g_key, _detachCurrentThread);
    }

    JNIEnv* JniHelper::cacheEnv(JavaVM* jvm) {
        JNIEnv* _env = nullptr;
        // get jni environment
        jint ret = jvm->GetEnv((void**)&_env, JNI_VERSION_1_4);
        
        switch (ret) {
        case JNI_OK :
            // Success!
            //pthread_setspecific(g_key, _env);
            return _env;
                
        case JNI_EDETACHED :
            // Thread not attached
            pthread_key_t __threadKey;
            pthread_key_create (&__threadKey, _detachCurrentThread);
            
            if (jvm->AttachCurrentThread(&_env, nullptr) < 0)
            {
                LOGE("Failed to get the environment using AttachCurrentThread()");

                return nullptr;
            }
           LOGE("cacheEnv  AttachCurrentThread()%d", __threadKey);
            if (pthread_getspecific(__threadKey) == NULL) {
                pthread_setspecific(__threadKey, _env);
            }
                
            return _env;
            
                
        case JNI_EVERSION :
            // Cannot recover from this error
            LOGE("JNI interface version 1.4 not supported");
        default :
            LOGE("Failed to get the environment using GetEnv()");
            return nullptr;
        }
        
    }

    JNIEnv* JniHelper::getEnv() {
        JNIEnv* env = nullptr;
        switch(JniHelper::getJavaVM()->GetEnv((void**)&env, JNI_VERSION_1_4))
        {
            case JNI_OK:
                break;
            case JNI_EDETACHED:
                pthread_key_t __threadKey;
                pthread_key_create (&__threadKey, _detachCurrentThread);
                
                if (JniHelper::getJavaVM()->AttachCurrentThread(&env, 0) < 0)
                {
                    LOGD("%s", "Failed to get the environment using AttachCurrentThread()");
                    break;
                }
                LOGE("getEnv AttachCurrentThread%d",  __threadKey);
                if (pthread_getspecific(__threadKey) == NULL) {
                    pthread_setspecific(__threadKey, env);
                }
                
                break;
            default:
                LOGE("%s", "Failed to get the environment using GetEnv()");
                break;
        }
        return env;
    }

    bool JniHelper::setClassLoaderFrom(jobject activityinstance) {
        JniMethodInfo _getclassloaderMethod;
        if (!JniHelper::getMethodInfo_DefaultClassLoader(_getclassloaderMethod,
                                                         "android/content/Context",
                                                         "getClassLoader",
                                                         "()Ljava/lang/ClassLoader;")) {
            return false;
        }

        jobject _c = cocos2d::JniHelper::getEnv()->CallObjectMethod(activityinstance,
                                                                    _getclassloaderMethod.methodID);

        if (nullptr == _c) {
            return false;
        }

        JniMethodInfo _m;
        if (!JniHelper::getMethodInfo_DefaultClassLoader(_m,
                                                         "java/lang/ClassLoader",
                                                         "loadClass",
                                                         "(Ljava/lang/String;)Ljava/lang/Class;")) {
            return false;
        }

        JniHelper::classloader = cocos2d::JniHelper::getEnv()->NewGlobalRef(_c);
        JniHelper::loadclassMethod_methodID = _m.methodID;

        return true;
    }

    bool JniHelper::getStaticMethodInfo(JniMethodInfo &methodinfo,
                                        const char *className, 
                                        const char *methodName,
                                        const char *paramCode) {
        try {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            LOGE("Failed to get JNIEnv");
            return false;
        }
            
        jclass classID = _getClassID(className);
        if(env->ExceptionCheck())
        {
            LOGE("exst Failed to find class %s", className);
            env->ExceptionDescribe();
            env->ExceptionClear();
            return false;
        }
            
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionDescribe();
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetStaticMethodID(classID, methodName, paramCode);
        if(env->ExceptionCheck())
        {
             LOGE("exst Failed to find static method id of %s", methodName);
             env->ExceptionDescribe();
             env->ExceptionClear();
             return false;
        }
            
        if (! methodID) {
            LOGE("Failed to find static method id of %s", methodName);
            env->ExceptionDescribe();
            env->ExceptionClear();
            return false;
        }
        if( methodinfo.classID != NULL ) {
            if(methodinfo.env)methodinfo.env->DeleteLocalRef(methodinfo.classID);
        }    
        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;
        } catch (...) {
            return false;
        }

        return true;
    }

    bool JniHelper::getMethodInfo_DefaultClassLoader(JniMethodInfo &methodinfo,
                                                     const char *className,
                                                     const char *methodName,
                                                     const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return false;
        }

        jclass classID = env->FindClass(className);
        
        
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }

        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;

        return true;
    }

    bool JniHelper::getMethodInfo(JniMethodInfo &methodinfo,
                                  const char *className,
                                  const char *methodName,
                                  const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return false;
        }

        jclass classID = _getClassID(className);
        
        if(env->ExceptionCheck())
        {
            LOGE("ex Failed to find class %s", className);
            env->ExceptionClear();
            
            return false;
        }
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
        if(env->ExceptionCheck())
        {
            LOGE("ex Failed to find method id of %s", methodName);
            env->ExceptionClear();
            
            return false;
        }
        if (! methodID) {
            LOGE("Failed to find method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }

        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;

        return true;
    }

    std::string JniHelper::jstring2string(jstring jstr) {
        if (jstr == nullptr) {
            return "";
        }
        
        JNIEnv *env = JniHelper::getEnv();
        if (!env) {
            return "";
        }
        
        std::string strValue = cocos2d::StringUtils::getStringUTFCharsJNI(env, jstr);
        
        return strValue;
    }

    jstring JniHelper::convert(LocalRefMapType& localRefs, cocos2d::JniMethodInfo& t, const char* x) {
        jstring ret = cocos2d::StringUtils::newStringUTFJNI(t.env, x ? x : "");
        localRefs[t.env].push_back(ret);
        return ret;
    }

    jstring JniHelper::convert(LocalRefMapType& localRefs, cocos2d::JniMethodInfo& t, const std::string& x) {
        return convert(localRefs, t, x.c_str());
    }

    void JniHelper::deleteLocalRefs(JNIEnv* env, LocalRefMapType& localRefs) {
        if (!env) {
            return;
        }

        for (const auto& ref : localRefs[env]) {
            env->DeleteLocalRef(ref);
        }
        localRefs[env].clear();
    }

    void JniHelper::reportError(const std::string& className, const std::string& methodName, const std::string& signature) {
        LOGE("Failed to find static java method. Class name: %s, method name: %s, signature: %s ",  className.c_str(), methodName.c_str(), signature.c_str());
    }

} //namespace cocos2d