
MODEL_NAME := sockopt
SRC_LIST := main.c

.PHONY:
test:
	gcc test.c -o test

ifneq ($(KERNELRELEASE),)

    $(MODEL_NAME)-objs := $(patsubst %.c,%.o,$(SRC_LIST))
    obj-m := $(MODEL_NAME).o

else
    include ../mk/module_build.mk
endif