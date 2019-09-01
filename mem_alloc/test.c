#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_TEST_NAME "/proc/mem_alloc"

int
main(int argc, char *argv[])
{
    int *addr;
    int fd;
    int err;

    do {
        int i;
        size_t length = 11;
        void *va, *pa;

        if (-1 == (fd = open(PROC_TEST_NAME, O_RDWR))) {
            printf("open %s error\n", PROC_TEST_NAME);
            err = 1;
            break;
        }

        addr = mmap(10, length, PROT_READ | PROT_WRITE | MAP_FIXED, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            printf("MAMP error\n");
            addr = NULL;
            err = 3;
            break;
        }
        // mem test
        for (i = 0; i < length; i++) {
            printf("[%x] ", ((char *)addr)[i]);
        }

        // mem test
        for (i = 0; i < length; i++) {
            ((char *)addr)[i] = (1 + i);
        }


        printf("[%p => %p] -> %p len=0x%x\n", va, pa, addr, length);

        munmap(addr, length);
    } while (0);

#if 0
    sleep(1000);
#endif

    if (fd > 0) {
        close(fd);
    }

    exit(err);
}