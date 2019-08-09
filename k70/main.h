#ifndef __MAIN_H__
#define __MAIN_H__

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

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
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/pci.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long long i64;
typedef int i32;
typedef short i16;
typedef char i8;

typedef struct {
    struct proc_dir_entry *base_dir;
    void *pci_mem ;
    u32 pci_mems;
    void *mem ;
    u32 mems;
    const i8 *root_path;
    const i8 *pci_path;
    const i8 *mem_path;

    struct pci_dev *pci_dev;
} mconfig_t;

extern mconfig_t *mconfig;

#define PRINT(fmt, ...) printk("%s:%d:"fmt, __FILE__, __LINE__, __VA_ARGS__);

#define always_inline static inline __attribute__ ((__always_inline__))

#endif // __MAIN_H__