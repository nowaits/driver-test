
KERNELDIR ?=/lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

.PHONY: modules
all modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

uninstall: modules
	@-rmmod -s $(MODEL_NAME).ko && echo "$(MODEL_NAME).ko uninstall OK"

install: uninstall
	@insmod $(MODEL_NAME).ko && echo "$(MODEL_NAME).ko install OK"