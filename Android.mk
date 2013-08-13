LOCAL_PATH := $(call my-dir)

# define vars for json-cpp library
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cpp
LOCAL_MODULE := json-cpp
LOCAL_C_INCLUDES := jni/libraries/third_party/json-cpp/include
LOCAL_SRC_FILES := third_party/json-cpp/src/lib_json/json_reader.cpp \
	third_party/json-cpp/src/lib_json/json_value.cpp \
	third_party/json-cpp/src/lib_json/json_writer.cpp

LOCAL_CFLAGS := -Wall -Werror -DOS_ANDROID 

include $(BUILD_STATIC_LIBRARY)

# define vars for m-lab library
include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := mlab
LOCAL_C_INCLUDES := jni/libraries/include/ \
	jni/libraries/third_party/json-cpp/include
LOCAL_SRC_FILES := \
	src/accepted_socket.cc \
	src/client_socket.cc \
	src/host.cc \
	src/log.cc \
	src/mlab.cc \
	src/ns.cc \
	src/listen_socket.cc \
	src/socket.cc
LOCAL_STATIC_LIBRARIES := json-cpp

LOCAL_CFLAGS := -Wall -Werror -DOS_ANDROID

include $(BUILD_SHARED_LIBRARY)
