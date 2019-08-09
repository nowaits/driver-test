#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_MEM_NAME "/proc/k70/mem"
#define PROC_PCI_NAME "/proc/k70/pci"

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

        if (-1 == (fd = open(PROC_MEM_NAME, O_RDWR))) {
            err = 1;
            break;
        }

        if (!meminfo(fd, &va, &pa, &length)) {
            err = 2;
            break;
        }

        addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            addr = NULL;
            err = 3;
            break;
        }

        // mem test
        for (i = 0; i < length; i++) {
            ((char *)addr)[i] = i;
        }

        printf("va=%p pa=%p s=0x%lx\n", va, pa, length);

        munmap(addr, length);
    } while (0);

    if (fd > 0) {
        close(fd);
    }

    exit(err);
}