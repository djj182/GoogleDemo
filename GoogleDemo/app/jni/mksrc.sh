#!/bin/bash
CLIENT_ROOT_PATH='..';

#find $CLIENT_ROOT_PATH/IF/Classes -name "AllSource.cpp"|awk '{print $1,"\\"}' >> Sources.mk
#find $CLIENT_ROOT_PATH/ -name "*.c"|awk '{print $1,"\\"}' >> Sources.mk
find $CLIENT_ROOT_PATH/src/main/cpp -name "*.cpp"|awk '{print LOCAL_SRC_FILES := $1,"\\"}' >> Sources.mk
#find $CLIENT_ROOT_PATH/IF/DayZClasses -name "AllSource**.cpp"|awk '{print $1,"\\"}'| sort >> Sources.mk
#find $CLIENT_ROOT_PATH/IF/DayZClasses -name "*.c"|awk '{print $1,"\\"}' >> Sources.mk
#find $CLIENT_ROOT_PATH/IF/libs -name "*.cpp"|awk '{print $1,"\\"}' >> Sources.mk
#find $CLIENT_ROOT_PATH/IF/libs -name "*.c"|awk '{print $1,"\\"}' >> Sources.mk

echo >> Sources.mk

echo 'LOCAL_C_INCLUDES := $(LOCAL_PATH) \' >Includes.mk
#find $CLIENT_ROOT_PATH/IF/Common -type d -not -wholename '*.svn*' -not -wholename '*ChartIOS*' -not -wholename '*WrapperChatIOS*'|awk '{printf "$(LOCAL_PATH)/%s \\\n",$1}' >> Includes.mk
find $CLIENT_ROOT_PATH/src/main/ -type d -not -wholename '*.svn*' -not -wholename '*ChartIOS*' -not -wholename '*WrapperChatIOS*'|awk '{printf "$(LOCAL_PATH)/%s \\\n",$1}' >> Includes.mk
#find $CLIENT_ROOT_PATH/IF/libs -type d -not -wholename '*.svn*'|awk '{printf "$(LOCAL_PATH)/%s \\\n",$1}' >> Includes.mk
echo >> Includes.mk