#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/jiffies.h>
#include <linux/time.h>

static int mem_major = 3;
extern unsigned long volatile jiffies;
module_param(mem_major, int, S_IRUGO);

void print_skb_info(struct sk_buff *skb, const struct net_device *in,
    const struct net_device *out)
{
    char ip_s[16], ip_d[16];
    char m_s[18], m_d[18];
    struct timeval tv = {0};
    char p_s[5], p_d[5];

    snprintf(p_s, 5, "%d", tcp_hdr(skb)->source);
    snprintf(p_d, 5, "%d", tcp_hdr(skb)->dest);

    jiffies_to_timeval(jiffies, &tv);
    snprintf(ip_s, 16, "%pI4", &ip_hdr(skb)->saddr);
    snprintf(ip_d, 16, "%pI4", &ip_hdr(skb)->daddr);

    snprintf(m_s, 18, "%pM", eth_hdr(skb)->h_source);
    snprintf(m_d, 18, "%pM", eth_hdr(skb)->h_dest);

    printk("->:%ld %s:MAC:%s IP:%s:%s <->%s %s %s:%s \n",
        tv.tv_sec,
        in->name, m_s, ip_s, p_s,
        out->name, m_d, ip_d, p_d
    );
};

#if 1

int ip_rcv(struct sk_buff *skb,
    struct net_device *dev,
    struct packet_type *pt,
    struct net_device *orig_dev)
{

    print_skb_info(skb, dev, orig_dev);
    return NET_RX_DROP;
}

struct packet_type net_filters = {
    .type = __constant_htons(ETH_P_ALL),
    .func = ip_rcv,
};

static int hook_init(void)
{
    dev_add_pack(&net_filters);
    printk(KERN_ALERT "%s init!\n", __FILE__);
    return 0;
}

static void hook_exit(void)
{
    dev_remove_pack(&net_filters);
    printk(KERN_ALERT "%s exit!\n", __FILE__);
}

#else

static unsigned int my_handler(const struct nf_hook_ops *ops,
    struct sk_buff *skb,
    const struct net_device *in,
    const struct net_device *out,
#ifndef __GENKSYMS__
    const struct nf_hook_state *state
#else
    int (*okfn)(struct sk_buff *)
#endif
)
{
    print_skb_info(skb);
    return NF_ACCEPT;
    //return NF_DROP;
}

// Handler registering struct
static struct nf_hook_ops net_hook __read_mostly = {
    .hook = my_handler,
    .pf = NFPROTO_IPV4,
    .hooknum = (1 << NF_INET_PRE_ROUTING),
    .priority = NF_IP_PRI_FIRST // My hook will be run before any other netfilter hook
};

static int hook_init(void)
{
    int err = nf_register_hook(&net_hook);
    if (err) {
        printk(KERN_ERR "Could not register hook\n");
    }

    printk(KERN_ALERT "%s init!\n", __FILE__);
    return err;
}

static void hook_exit(void)
{
    nf_unregister_hook(&net_hook);
    printk(KERN_ALERT "%s exit!\n", __FILE__);
}

#endif

MODULE_AUTHOR("HOOK");
MODULE_LICENSE("GPL");

module_init(hook_init);
module_exit(hook_exit);