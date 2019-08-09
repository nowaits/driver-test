
MODEL_NAME := top
SRC_LIST := main.c
HOSTCXXFLAGS = -O2
export HOSTCXXFLAGS
ifneq ($(KERNELRELEASE),)

    $(MODEL_NAME)-objs := $(patsubst %.c,%.o,$(SRC_LIST))
    obj-m := $(MODEL_NAME).o

else
    include ../mk/module_build.mk
endif