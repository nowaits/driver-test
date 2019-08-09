#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

#include <linux/types.h>
#include <linux/string.h>
#include <linux/netfilter_ipv4.h>
#include <linux/init.h>
#include <asm/uaccess.h>

#define SOCKET_OPS_BASE 128
#define SOCKET_OPS_SET (SOCKET_OPS_BASE)
#define SOCKET_OPS_GET (SOCKET_OPS_BASE)
#define SOCKET_OPS_MAX (SOCKET_OPS_BASE + 1)

#define KMSG "--------kernel---------"
#define KMSG_LEN sizeof("--------kernel---------")

MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("sockopt module,simple module");
MODULE_VERSION("1.0");

static int recv_msg(struct sock *sk, int cmd, void __user *user, unsigned int len)
{
    int ret = 0;
    printk(KERN_INFO "sockopt: recv_msg()\n");

    if (cmd == SOCKET_OPS_SET) {
        char umsg[64];
        int len = sizeof(char) * 64;
        memset(umsg, 0, len);
        ret = copy_from_user(umsg, user, len);
        printk("recv_msg: umsg = %s. ret = %d\n", umsg, ret);
    }
    return 0;
}

static int send_msg(struct sock *sk, int cmd, void __user *user, int *len)
{
    int ret = 0;
    printk(KERN_INFO "sockopt: send_msg()\n");
    if (cmd == SOCKET_OPS_GET) {
        ret = copy_to_user(user, KMSG, KMSG_LEN);
        printk("send_msg: umsg = %s. ret = %d. success\n", KMSG, ret);
    }
    return 0;
}

struct nf_sockopt_ops test_sockops = {
    .pf = AF_INET,
    .set_optmin = SOCKET_OPS_SET,
    .set_optmax = SOCKET_OPS_MAX,
    .set = recv_msg,
    .get_optmin = SOCKET_OPS_GET,
    .get_optmax = SOCKET_OPS_MAX,
    .get = send_msg,
    .owner = THIS_MODULE,
};

static int __init init_sockopt(void)
{
    printk(KERN_INFO "sockopt: init_sockopt()\n");
    return nf_register_sockopt(&test_sockops);
}

static void __exit exit_sockopt(void)
{
    printk(KERN_INFO "sockopt: fini_sockopt()\n");
    nf_unregister_sockopt(&test_sockops);
}

module_init(init_sockopt);
module_exit(exit_sockopt);