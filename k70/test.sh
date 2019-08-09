ROOT_DIR=`realpath $(dirname $0)`

rmmod k70 >/dev/null 2>&1

insmod $ROOT_DIR/k70.ko mems=32
dmesg -c