APP_PLATFORM := android-10
APP_ABI := armeabi
APP_STL := gnustl_static
APP_CFLAGS += -Wno-psabi -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections
APP_CPPFLAGS := -frtti -fsigned-char -std=gnu++11 -D__STDC_INT64__ -Wno-error=format-security -fexceptions -fvisibility=hidden -fvisibility-inlines-hidden -ffunction-sections -fdata-sections
APP_LDFLAGS := -latomic -Wl,--gc-sections
NDK_TOOLCHAIN_VERSION := 4.9

ifeq ($(NDK_DEBUG_BETA),1)
    APP_CPPFLAGS += -DCOCOS2D_BETA=1
endif

ifeq ($(NDK_DEBUG_LOCAL),1)
    APP_CPPFLAGS += -DCOCOS2D_DEBUG_LOCAL=1
endif

ifeq ($(NDK_DEBUG_PAYPAL),1)
	APP_CPPFLAGS += -DCOCOS2D_PAYPAL=1
endif

ifeq ($(NDK_DEBUG),1)
    APP_OPTIM := release
    APP_CPPFLAGS += -DCOCOS2D_DEBUG=1
    APP_CPPFLAGS += -DNDEBUG 
else
    APP_CPPFLAGS += -DNDEBUG 
  # -fvisibility=hidden -ffunction-sections -fdata-sections -fvisibility-inlines-hidden
   APP_OPTIM := release
endif
