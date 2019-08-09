#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define MODULE_NAME "hello_world"

static int mem_major = 3;
module_param(mem_major, int, S_IRUGO);

static int test_init(void)
{
    printk(KERN_ALERT "==>>> %s(%d) \n", __FILE__, __LINE__);
    return 0;
}

static void test_exit(void)
{
    printk(KERN_ALERT "==>>> %s(%d) \n", __FILE__, __LINE__);
}

MODULE_AUTHOR(MODULE_NAME);
MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);