#===================================================================================================
#
#   Module Name:  local_module.mk
#
#   General Description: main makefile for libdg_loop
#
#===================================================================================================
LOCAL_PATH := $(call my-dir)

DG_FLAGS := -D_GNU_SOURCE

#===================================================
# dg_loop library
#===================================================
include $(CLEAR_VARS)

LOCAL_MODULE := libfpga_net

LOCAL_CFLAGS := $(DG_FLAGS) -msse2

FPGA_NET_EXPORT_INC := hdr

FPGA_NET_INC := $(FPGA_NET_EXPORT_INC)

LOCAL_C_INCLUDES := $(addprefix $(LOCAL_PATH)/, $(FPGA_NET_INC))

LOCAL_EXPORT_C_INCLUDE_DIRS :=  $(addprefix $(LOCAL_PATH)/, $(FPGA_NET_EXPORT_INC))

LOCAL_STATIC_LIBRARIES := libdg_dbg

LOCAL_SRC_FILES := \
  src/fpga_drv.c \
  src/fpga_net.c \
  src/fpga_rx.c \
  src/fpga_tx.c \
  src/hp_malloc.c \
  src/mem_map.c \
  src/rte_ring.c \
  src/rx_mbuf.c \
  src/uio.c

include $(BUILD_STATIC_LIBRARY)

