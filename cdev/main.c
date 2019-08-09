#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fcntl.h> /*Helps fix O_ACCMODE*/

#include <linux/sched.h> /*Helps fix TASK_UNINTERRUPTIBLE */

#include <linux/fs.h> /*Helps fix the struct intializer */
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
#include <linux/mm.h>
#include<linux/cdev.h>
#include<linux/slab.h>

MODULE_AUTHOR("x");
MODULE_LICENSE("Dual BSD/GPL");
#define TEST_NAME "sharem"

#define PAGE_ORDER   10
#define PAGES_NUMBER (1<<PAGE_ORDER)

#define PRINT(fmt, ...) printk("%s:%d:"fmt, __FILE__, __LINE__, __VA_ARGS__);

static int shm_test_major = 215;
static struct cdev shm_test_dev;
static struct class *cls;

static void shm_mm_close(struct vm_area_struct *vma)
{
    unsigned long page_num = (vma->vm_end - vma->vm_start) / PAGE_SIZE;

    printk("mem freed: start 0x%p s = %u, 0x%p = 0x%p ok.\n",
        vma->vm_start, vma->vm_end - vma->vm_start,
        vma->vm_private_data, __pa(vma->vm_private_data));

    ClearPageReserved(virt_to_page(vma->vm_private_data));
    free_pages(vma->vm_private_data, get_order(page_num));
}

static const
struct vm_operations_struct vm_ops = {
    .close = shm_mm_close,
};

int
shm_test_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long page_num;
    vma->vm_ops = &vm_ops;
    void *km;

    page_num = (vma->vm_end - vma->vm_start) / PAGE_SIZE;
    if (NULL == (km = __get_free_pages(GFP_DMA/* GFP_KERNEL GFP_USER  GFP_DMA*/,
                    get_order(page_num)))) {
        printk("alloc kernel memory failed!/n");
        return -1;
    }

    SetPageReserved(virt_to_page(km));

    //map
    vma->vm_flags |=  VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP;
    if (remap_pfn_range(vma, vma->vm_start, __pa(km) >> PAGE_SHIFT, (vma->vm_end - vma->vm_start),
            vma->vm_page_prot)) {
        return -1;
    }

    vma->vm_private_data = km;

    printk("%d: vm start:  0x%p (size= 0x%x ) >>>:mem [ 0x%p => 0x%p ] ok.\n", ((int *)km)[0],
        vma->vm_start, vma->vm_end - vma->vm_start,
        km, __pa(km));
    return 0;
}

static const
struct file_operations shm_test_fops = {
    .owner = THIS_MODULE,
    .mmap = shm_test_mmap,
};

static void
shm_test_exit(void)
{
    device_destroy(cls, MKDEV(shm_test_major, 0));
    class_destroy(cls);

    cdev_del(&shm_test_dev);
    unregister_chrdev_region(MKDEV(shm_test_major, 0), 1);
}

int
shm_test_init(void)
{
    int result;
    dev_t devno ;
    struct cdev *p_cdev = &shm_test_dev;

    if (shm_test_major) {
        devno = MKDEV(shm_test_major, 0);
        result = register_chrdev_region(devno, 1, TEST_NAME);
    } else {
        result = alloc_chrdev_region(&devno, 0, 1, TEST_NAME);
        shm_test_major = MAJOR(devno);
    }

    if (result) {
        return result;
    }

    printk("the major device No. is %d\n", shm_test_major);
    cdev_init(p_cdev, &shm_test_fops);
    p_cdev->owner = THIS_MODULE;
    result = cdev_add(p_cdev, devno, 1);
    if (result) {
        printk("Error %d while adding dbg", result);
        return result;
    }


    cls = class_create(THIS_MODULE, TEST_NAME);
    device_create(cls, NULL, MKDEV(shm_test_major, 0), NULL, TEST_NAME"0");
    return 0;
}

module_init(shm_test_init);
module_exit(shm_test_exit);