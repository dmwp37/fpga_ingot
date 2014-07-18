#===================================================================================================
#
##   Module Name:  local_module.mk
#
##   General Description: main makefile for dg_int_loop
#
##===================================================================================================
LOCAL_PATH := $(call my-dir)

#===================================================
# dg_int_loop
#===================================================
include $(CLEAR_VARS)

LOCAL_MODULE := fpga_test

LOCAL_CFLAGS :=

DG_INT_LOOP_INC := ..

LOCAL_C_INCLUDES := $(addprefix $(LOCAL_PATH)/, $(DG_INT_LOOP_INC))

LOCAL_STATIC_LIBRARIES := libfpga_net libdg_dbg
LOCAL_LDLIBS += -lrt

LOCAL_REQUIRED_MODULES :=

LOCAL_SRC_FILES := main.c

include $(BUILD_EXECUTABLE)

