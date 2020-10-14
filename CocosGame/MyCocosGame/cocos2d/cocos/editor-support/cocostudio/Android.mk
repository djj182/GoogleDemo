LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := cocostudio_static

LOCAL_MODULE_FILENAME := libcocostudio

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := CocoStudio.cpp


LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. \
                    $(LOCAL_PATH)/WidgetReader

LOCAL_CFLAGS += -fexceptions

LOCAL_STATIC_LIBRARIES := cocos_ui_static
LOCAL_STATIC_LIBRARIES += cocosdenshion_static
LOCAL_STATIC_LIBRARIES += cocos_flatbuffers_static

include $(BUILD_STATIC_LIBRARY)
