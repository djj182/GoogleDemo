LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../../cocos2d/cocos)

LOCAL_MODULE := game_shared
LOCAL_MODULE_FILENAME := libgame

LOCAL_LDLIBS += -llog

include $(LOCAL_PATH)/Sources.mk
include $(LOCAL_PATH)/Includes.mk 

LOCAL_STATIC_LIBRARIES := cocos2dx_ui_static
LOCAL_STATIC_LIBRARIES += cocos2dx_static
LOCAL_STATIC_LIBRARIES += cocos2d_lua_static
LOCAL_STATIC_LIBRARIES += cpufeatures
            
include $(BUILD_SHARED_LIBRARY)

# $(call import-module,prebuilt/android)
$(call import-module,scripting/lua-bindings/proj.android)
$(call import-module,android/cpufeatures)
