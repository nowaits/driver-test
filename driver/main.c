#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>

MODULE_LICENSE("Dual BSD/GPL");

static int my_match(struct device *dev, struct device_driver *driver)
{
    return !strncmp(dev->kobj.name, driver->name, strlen(driver->name));
}

struct bus_type my_bus_type = {
    .name = "my_bus",  //总线名字
    .match = my_match, //总线match函数指针
};

EXPORT_SYMBOL(my_bus_type);

static int my_probe(struct device *dev)
{
    printk("Driver found device which my driver can handle!\n");
    return 0;
}

static int my_remove(struct device *dev)
{
    printk("Driver found device unpluged!\n");
    return 0;
}

struct device_driver my_driver = {
    .name = "my_dev",
    .bus = &my_bus_type,
    .probe = my_probe,
    .remove = my_remove,
};


static ssize_t mydriver_show(struct device_driver *driver, char *buf)
{
    return sprintf(buf, "%s\n", "This is my driver!");
}

static DRIVER_ATTR(drv, S_IRUGO, mydriver_show, NULL);

static int __init my_driver_init(void)
{
    int ret = 0;

    /*注册驱动*/
    driver_register(&my_driver);

    /*创建属性文件*/
    driver_create_file(&my_driver, &driver_attr_drv);

    return ret;

}

static void my_driver_exit(void)
{
    driver_unregister(&my_driver);
}

module_init(my_driver_init);

module_exit(my_driver_exit);