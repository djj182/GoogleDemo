LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libnative-lib
LOCAL_MODULE_FILENAME := libnative-lib

LOCAL_LDLIBS += -llog
#LOCAL_CFLAGS +=-fvisibility=default

include $(LOCAL_PATH)/Sources.mk
include $(LOCAL_PATH)/Includes.mk

#LOCAL_SRC_FILES := ../src/main/cpp/native-lib.cpp

include $(BUILD_SHARED_LIBRARY)

# $(call import-module,prebuilt/android)
