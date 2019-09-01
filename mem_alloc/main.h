#ifndef __MAIN_H__
#define __MAIN_H__

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/mm.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long long i64;
typedef int i32;
typedef short i16;
typedef char i8;

#define PRINT(LEVEL, fmt, ...) printk(KERN_##LEVEL "%s:%d:" fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#endif // __MAIN_H__