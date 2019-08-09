
MODEL_NAME := cdev_test
SRC_LIST := main.c

ifneq ($(KERNELRELEASE),)

    $(MODEL_NAME)-objs := $(patsubst %.c,%.o,$(SRC_LIST))
    obj-m := $(MODEL_NAME).o

else
    include ../mk/module_build.mk
endif