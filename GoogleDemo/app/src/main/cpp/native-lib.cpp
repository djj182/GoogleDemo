#include <jni.h>
#include <string>
#include <stdio.h>

#define J2CSTRING(_cstring, _jstring)    \
string _cstring; \
if(env!=NULL && _jstring != NULL){ \
    _cstring = env->GetStringUTFChars(_jstring, NULL); \
}else{ \
    _cstring = ""; \
}

extern "C" {
JNIEXPORT jstring JNICALL
Java_com_im30_googledemo_ui_activitys_CppDemoActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    FILE *fp = fopen("/storage", "rb");
    // 测试BUG 注释掉
//    FILE *fp2= nullptr;
//    std::fgetc(fp2);
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT void JNICALL
Java_com_im30_googledemo_ui_activitys_CppDemoActivity_openFile(
        JNIEnv *env,
        jobject /* this */,
        jstring path) {
    const char *pathStr = env->GetStringUTFChars(path, NULL);
    FILE *fp = fopen(pathStr, "rt");
    int a = 0;
}
}
