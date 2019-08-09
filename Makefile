
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
	cdev

projects = k70

all clean install uninstall:
	@for i in $(projects) ; do \
		[ ! -d $$i ] || make -C $$i $@ || exit $$? ; \
	done