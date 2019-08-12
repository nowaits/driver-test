#include "main.h"

typedef struct {
    struct {
        void *vmem;
        void *pmem;
        u32 msize;
    };
} file_private_data_t;

#define FILE_PRIVATE_DATA(f) ((file_private_data_t*)((struct seq_file *)f->private_data)->private)

void reserve_pages(void *mem, u32 size)
{
    while (size > 0) {
        SetPageReserved(virt_to_page(mem));
        mem += PAGE_SIZE / (sizeof(mem[0]));
        size -= PAGE_SIZE;
    }
}

void
clear_reserve_pages(void *mem, u32 size)
{
    while (size > 0) {
        ClearPageReserved(virt_to_page(mem));
        mem += PAGE_SIZE / (sizeof(mem[0]));
        size -= PAGE_SIZE;
    }
}

inline void
_release_vmem(file_private_data_t *fpd)
{
    clear_reserve_pages(fpd->vmem, fpd->msize);
    free_pages((unsigned long)fpd->vmem, get_order(fpd->msize / PAGE_SIZE));

    fpd->vmem = NULL;
    fpd->pmem = NULL;
    fpd->msize = 0;
}

static void
_shm_mm_close(struct vm_area_struct *vma)
{
    file_private_data_t *fpd = (file_private_data_t *)vma->vm_private_data;

    PRINT(INFO, "mem freed: start 0x%lx s = %lx, 0x%p = 0x%p ok.\n",
        vma->vm_start, vma->vm_end - vma->vm_start,
        fpd->vmem, fpd->pmem);

    _release_vmem(fpd);
}

static const struct vm_operations_struct vm_ops = {
    .close = _shm_mm_close,
};

static int
_proc_mmap(struct file *f, struct vm_area_struct *vma)
{
    file_private_data_t *fpd = FILE_PRIVATE_DATA(f);

    if (fpd->vmem) {
        PRINT(ERR, "file is already maped!\n");
        return -1;
    }

    fpd->msize = vma->vm_end - vma->vm_start;

    if (NULL == (fpd->vmem = (void *)__get_free_pages(GFP_DMA/* GFP_KERNEL GFP_USER  GFP_DMA*/,
                    get_order(fpd->msize / PAGE_SIZE)))) {
        PRINT(ERR, "alloc kernel memory:[0x%x] failed!\n", fpd->msize);
        return -1;
    }

    fpd->pmem = (void *)__pa(fpd->vmem);
    reserve_pages(fpd->vmem, fpd->msize);

    //map
    if (remap_pfn_range(vma, vma->vm_start, ((u64)fpd->pmem) >> PAGE_SHIFT,
            fpd->msize, vma->vm_page_prot)) {
        PRINT(ERR, "remap mem:[0x%p] failed!\n", fpd->vmem);

        _release_vmem(fpd);
        return -1;
    }

    vma->vm_ops = &vm_ops;
    vma->vm_flags |=  VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP;
    vma->vm_private_data = fpd;

    PRINT(INFO, "map [ 0x%p => 0x%p ] -> 0x%p len=%x ok.\n",
        fpd->vmem, fpd->pmem,
        (void *)vma->vm_start, fpd->msize);
    return 0;
}

static int
_proc_show(struct seq_file *sf, void *v)
{
    file_private_data_t *fpd = sf->private;

    if (!fpd->vmem) {
        seq_printf(sf, "Use mmap to alloc dma memory\n");
        return 0;
    }

    seq_printf(sf, "vmem=0x%p pmem=0x%p size=0x%x\n", fpd->vmem, fpd->pmem, fpd->msize);
    return 0;
}

static int
_proc_open(struct inode *inode, struct  file *file)
{
    file_private_data_t *fpd = (file_private_data_t *)kzalloc(sizeof(file_private_data_t), GFP_KERNEL);
    PRINT(INFO, "file open: %p %p.\n", file, fpd);

    return single_open(file, _proc_show, fpd);
}

static int
_proc_rlease(struct inode *inode, struct file *file)
{
    file_private_data_t *fpd = FILE_PRIVATE_DATA(file);

    kfree(fpd);

    PRINT(INFO, "release %p\n", fpd);
    return single_release(inode, file);
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = _proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = _proc_rlease,
    .mmap = _proc_mmap,
};

#define PROC_TEST "mem_alloc"

static int
_proc_init(void)
{
    proc_create(PROC_TEST, 0, NULL, &proc_fops);
    PRINT(INFO, "INIT\n");
    return 0;
}

static void
_proc_exit(void)
{
    remove_proc_entry(PROC_TEST, NULL);
    PRINT(INFO, "EXIT\n");
}

MODULE_LICENSE("GPL");
module_init(_proc_init);
module_exit(_proc_exit);

// insmod {}.ko mems=32768