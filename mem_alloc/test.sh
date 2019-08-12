ROOT_DIR=`realpath $(dirname $0)`

rmmod mem_alloc >/dev/null 2>&1
insmod $ROOT_DIR/mem_alloc.ko
dmesg -c