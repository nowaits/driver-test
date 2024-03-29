#include "main.h"

char *mem = NULL;
int msize = 44;

static void
_shm_mm_close(struct vm_area_struct *vma)
{
    int i = 0;
    char buf[1024], *p = buf;

    for (i = 0; i < msize; i++) {
        int l = sprintf(p, "%0x%s", mem[i], i % 20 == 0 ? "\n" : " ");
        if (l <= 0) {
            break;
        }

        p += l;
    }

    PRINT(INFO, "close: %s", buf);
}

static const struct vm_operations_struct vm_ops = {
    .close = _shm_mm_close,
};

static int
_proc_mmap(struct file *f, struct vm_area_struct *vma)
{
    //map
    if (remap_pfn_range(vma, vma->vm_start, __pa(mem) >> PAGE_SHIFT,
            vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
        PRINT(ERR, "remap mem:[0x%p] failed!\n", mem);
        return -1;
    }

    vma->vm_ops = &vm_ops;
    vma->vm_flags |=  VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP;

    PRINT(INFO, "map [ 0x%p => 0x%p ] -> 0x%p len=%x ok.\n",
        (void *)__pa(mem), mem,
        (void *)vma->vm_start, msize);

    return 0;
}

static int
_proc_show(struct seq_file *sf, void *v)
{
    int i = 0;
    char buf[1024], *p = buf;

    for (i = 0; i < msize; i++) {
        int l = sprintf(p, "%x%s", mem[i], i % 20 == 0 ? "\n" : " ");
        if (l <= 0) {
            break;
        }

        p += l;
    }

    seq_printf(sf, "%p=>%p: %s\n", (void *)__pa(mem), mem, buf);
    return 0;
}

static int
_proc_open(struct inode *inode, struct  file *file)
{
    try_module_get(THIS_MODULE);
    return single_open(file, _proc_show, NULL);
}

static int
_proc_rlease(struct inode *inode, struct file *file)
{

    module_put(THIS_MODULE);
    PRINT(INFO, "put module %u", module_refcount(THIS_MODULE));

    return single_release(inode, file);;
}

static void
reset()
{
    PRINT(INFO, "success");
}

typedef struct {
    char *cmd;
    void (*proc)(void);
} cmd_t;

static ssize_t
_proc_write(struct file *file, const char __user *buffer,
    size_t count, loff_t *pos)
{
    int res;
    char *buf = (char *) __get_free_page(GFP_USER);
    int i;

    cmd_t cmds[] = {
        {"reset", reset},
    };

    if (!buf) {
        return -ENOMEM;
    }

    res = -EINVAL;
    if (count >= PAGE_SIZE) {
        goto out;
    }

    res = -EFAULT;
    if (copy_from_user(buf, buffer, count)) {
        goto out;
    }

    buf[count] = 0;

    for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
        cmd_t *c = cmds + i;

        if (strncmp(buf, c->cmd, min(strlen(c->cmd) - 1, count)) == 0) {
            c->proc();
            res = count;
            break;
        }
    }

    if (res <= 0) {
        res = -EINVAL;
        PRINT(INFO, "invalid args: %s", buf);
    }

out:
    free_page((unsigned long)buf);
    return res;
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = _proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = _proc_rlease,
    .mmap = _proc_mmap,
    .write = _proc_write,
};

#define PROC_TEST "mem_alloc"

static int
_proc_init(void)
{
    int cpu;
    mem = (char *)kzalloc(msize, GFP_DMA);
    proc_create(PROC_TEST, 0644 /*0644/*S_IRWXUGO*/, NULL, &proc_fops);
    PRINT(INFO, "alloc: %p=>%p\n", (void *)__pa(mem), mem);
    PRINT(INFO, "module state: %d", THIS_MODULE->state);
    PRINT(INFO, "module name: %s %s", THIS_MODULE->name, KBUILD_MODNAME);
    PRINT(INFO, "module version:%s", THIS_MODULE->version);

    return 0;
}

static void
_proc_exit(void)
{
    PRINT(INFO, "module state: %d", THIS_MODULE->state);
    PRINT(INFO, "find_module bye...");

    kfree(mem);
    remove_proc_entry(PROC_TEST, NULL);
    PRINT(INFO, "EXIT");
}

MODULE_DESCRIPTION(KBUILD_MODNAME);
MODULE_VERSION("0.0.1");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("XDJA");

module_init(_proc_init);
module_exit(_proc_exit);

// insmod {}.ko mems=32768