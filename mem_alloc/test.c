#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_TEST_NAME "/proc/mem_alloc"

int
meminfo(int fd, void **va, void **pa, size_t *size)
{
    char buffer[80] = {0};

    if (read(fd, buffer, sizeof(buffer) - 1) <= 0) {
        return 0;
    }

    return sscanf(buffer, "vmem=0x%p pmem=0x%p size=0x%lx\n", va, pa, size) == 3 ;
}

int
main(int argc, char *argv[])
{
    int *addr;
    int fd;
    int err;

    do {
        int i;
        size_t length;
        void *va, *pa;

        if (-1 == (fd = open(PROC_TEST_NAME, O_RDWR))) {
            err = 1;
            break;
        }

        addr = mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            addr = NULL;
            err = 3;
            break;
        }

        if (!meminfo(fd, &va, &pa, &length)) {
            err = 2;
            break;
        }

        // mem test
        for (i = 0; i < length; i++) {
            ((char *)addr)[i] = i;
        }

        printf("[%p => %p] -> %p len=0x%x\n", va, pa, addr, length);

        munmap(addr, length);
    } while (0);

    if (fd > 0) {
        close(fd);
    }

    exit(err);
}