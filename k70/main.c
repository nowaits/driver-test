#include "main.h"
#include "pci_k70.h"

mconfig_t _mconfig = {
    .mems = (1 << 5 << 10 << 10),  // 32M
    .root_path = "k70",
    .pci_path = "pci",
    .mem_path = "mem"
}, *mconfig = &_mconfig;

static unsigned char mems = 0; // M
module_param(mems, byte, S_IRUGO);

static inline int
ispath(struct file *f, const char *path)
{
    char *tmp;
    char *pathname;
    int ok = 0 ;

    path_get(&f->f_path);

    tmp = (char *)__get_free_page(GFP_TEMPORARY);
    if (!tmp) {
        return ok;
    }

    pathname = d_path(&f->f_path, tmp, PAGE_SIZE);
    path_put(&f->f_path);

    if (IS_ERR(pathname)) {
        free_page((unsigned long)tmp);
        return ok;
    }

    ok = strcmp(pathname, path) == 0;
    free_page((unsigned long)tmp);

    return  ok;
}

static   int
proc_mmap(struct file *f, struct vm_area_struct *vma)
{
    void *m = NULL;
    u32 s = 0;

    do {
        char tmp_s[50];

        // mem
        sprintf(tmp_s, "/proc/%s/%s", mconfig->root_path, mconfig->mem_path);
        if (ispath(f, tmp_s)) {
            m = mconfig->mem;
            s = mconfig->mems;
            break;
        }

        // pci
        sprintf(tmp_s, "/proc/%s/%s", mconfig->root_path, mconfig->pci_path);
        if (ispath(f, tmp_s)) {
            m = mconfig->pci_mem;
            s = mconfig->pci_mems;
            break;
        }
    } while (0);

    if (m == 0) {
        PRINT("path error: %p %u\n", m, s);
        return -1;
    }

    if (vma->vm_end - vma->vm_start > s) {
        PRINT("out of memory![0x%lx > 0x%x]\n", vma->vm_end - vma->vm_start, s);
        return -1;
    }

    //map
    if (remap_pfn_range(vma, vma->vm_start, __pa(m) >> PAGE_SHIFT,
            vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
        return -1;
    }

    vma->vm_flags |=  VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP;

    PRINT("vm start:  0x%p (size= 0x%lx ) >>>:mem [ 0x%p => 0x%p ] ok.\n",
        (void *)vma->vm_start, vma->vm_end - vma->vm_start,
        m, (void *)__pa(m));
    return 0;
}

static int
_proc_show(struct seq_file *sf, void *v)
{
    void *m = NULL;
    u32 s = 0;

    do {
        char tmp_s[50];
        struct  file *f = sf->private;
        if (!f) {
            break;
        }

        // mem
        sprintf(tmp_s, "/proc/%s/%s", mconfig->root_path, mconfig->mem_path);
        if (ispath(f, tmp_s)) {
            m = mconfig->mem;
            s = mconfig->mems;
            break;
        }

        // pci
        sprintf(tmp_s, "/proc/%s/%s", mconfig->root_path, mconfig->pci_path);
        if (ispath(f, tmp_s)) {
            m = mconfig->pci_mem;
            s = mconfig->pci_mems;
            break;
        }
    } while (0);

    if (!m) {
        PRINT("path error: %p %u\n", m, s);
        return 0;
    }

    seq_printf(sf, "vmem=0x%p pmem=0x%p size=0x%x\n", m, (void *)__pa(m), s);
    return 0;
}

static int
_proc_open(struct inode *inode, struct  file *file)
{
    return single_open(file, _proc_show, file);
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = _proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
    .mmap = proc_mmap,
};

static int
_proc_init(void)
{
    i32 err;

    err = pci_register_driver(&pci_k70_driver);
    if (err != 0) {
        PRINT("Error: %s", "faild to regist k70 pci driver");
        return err;
    }

    //
    // @TOMO:
    //      assume: pci probe finish
    //

    if (!mconfig->pci_mem) {
        PRINT("Error(%s)", "no k70 pci card");
        return -1;
    }

    mconfig->base_dir = proc_mkdir(mconfig->root_path, NULL);
    if (mems != 0) {
        mconfig->mems = mems * (1 << 20);
    }

    proc_create(mconfig->pci_path, 0, mconfig->base_dir, &proc_fops);
    proc_create(mconfig->mem_path, 0, mconfig->base_dir, &proc_fops);

    if (NULL == (mconfig->mem = (void *)__get_free_pages(GFP_DMA/* GFP_KERNEL GFP_USER  GFP_DMA*/,
                    get_order(mconfig->mems / PAGE_SIZE)))) {
        PRINT("alloc kernel memory:[0x%x] failed!\n", mconfig->mems);
        return -1;
    }

    SetPageReserved(virt_to_page(mconfig->mem));

    PRINT("DMA mem prepare ok: 0x%p => 0x%p size = 0x%x \n",
        mconfig->mem, (void *)__pa(mconfig->mem), mconfig->mems);

    return 0;
}

static void
_proc_exit(void)
{
    pci_unregister_driver(&pci_k70_driver);

    if (!mconfig->base_dir) {
        return;
    }

    remove_proc_entry(mconfig->pci_path, mconfig->base_dir);
    remove_proc_entry(mconfig->mem_path, mconfig->base_dir);
    proc_remove(mconfig->base_dir);

    ClearPageReserved(virt_to_page(mconfig->mem));
    free_pages((unsigned long)mconfig->mem, get_order(mconfig->mems / PAGE_SIZE));
}

MODULE_LICENSE("GPL");
module_init(_proc_init);
module_exit(_proc_exit);

// insmod {}.ko mems=32768