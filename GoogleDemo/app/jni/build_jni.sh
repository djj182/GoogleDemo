#!/usr/bin/env bash
echo ">>>>>>>>>>>>> start build_jni.sh"

echo "=============================="

# 设定环境变量
export NDK_ROOT=/opt/android-ndk-r9c
export ANDROID_SDK_ROOT=/opt/android-sdk-macosx
export ANDROID_HOME=/opt/android-sdk-macosx
export ANDROID_NDK_ROOT=/opt/android-ndk-r9c
export PATH=$PATH:$ANDROID_NDK_ROOT
export PATH=$PATH:$ANDROID_SDK_ROOT
export ANT_HOME=/opt/apache-ant-1.9.4
export PATH=$PATH:$ANT_HOME/bin
export PATH=$PATH:$ANDROID_SDK_ROOT/platform-tools
export PATH=$PATH:/usr/local/bin/
export PATH=$PATH:$PWD/CCB/IF/bin/
export GRADLE_HOME=gradle-2.14
export PATH=$PATH:$GRADLE_HOME/bin

function buildjni() {
    # compile apk
    cd $2
#    if [ ! -d "$2/app/assets" ]; then
#	    mkdir "$2/app/assets"
#    fi
#    if [ ! -d "$2/../../../android_generate" ]; then
#	    mkdir "$2/../../../android_generate"
#    fi
#    if [ ! -d "$2/../../../android_generate_desert" ]; then
#	    mkdir "$2/../../../android_generate_desert"
#    fi

#    $COCOS_CONSOLE/cocos compile -s . -j 8 -p android -t android-26 --no-apk --android-studio --ndk-mode none -m release --daemon --parallel --offline NDK_INCLUDE_DAYZ_CPP=0

    cd $2/app/jni
#    sh mksrc.sh
    cd ..


    /opt/android-ndk-r21b/ndk-build NDK_DEBUG=0 -j 4 NDK_APPLICATION_MK=jni/Application_ndk_r10.mk

    if [ ! -f "./obj/local/armeabi-v7a/libgame.so" ]; then
	    set -e  #注意，这句最重要，一定要先设置这个
        exit 1  #然后再退出，jenkins就会报红显示构建失败
    fi

    cd ..

    if [ -d "$2/app/obj/local/armeabi" ]; then
        rm -rf "$2/app/obj/local/armeabi"
    fi

    sed -i '' "s/\"armeabi\"/\"armeabi-v7a\"/g" app/build.gradle
    ./gradlew --version

    #./gradlew --parallel --quiet assembleRelease
}

buildjni ../../.. .

