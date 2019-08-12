
projects = \
	hello_world \
	hook \
	kobject \
	kset \
	bus \
	device \
	driver \
	sysctl \
	sockopt \
	top \
	k70 \
	cdev \
	mem_alloc \

projects = mem_alloc

all clean install uninstall:
	@for i in $(projects) ; do \
		[ ! -d $$i ] || make -C $$i $@ || exit $$? ; \
	done