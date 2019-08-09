#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <net/ip.h>
#include "./top.h"

extern unsigned long volatile jiffies;
MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("sockopt module,simple module");
MODULE_VERSION("1.0");

static inline int tcp_add_opt(struct sk_buff *skb)
{
    struct iphdr *iph = ip_hdr(skb);
    struct tcphdr *tcp = tcp_hdr(skb);
    struct tcp_opt *to = NULL;

    if (iph->protocol != IPPROTO_TCP) {
        return -1;
    }

    if (unlikely(!SKB_APPEND(skb, OP_LEN))) {
        printk("%s: %pI4:%d => %pI4:%d append error!\n", __func__, &iph->saddr, ntohs(tcp->source),
            &iph->daddr, ntohs(tcp->dest));

        return -1;
    }

    to = (struct tcp_opt *)((__u8 *)tcp + tcp->doff * 4);

    memmove((__u8 *)to + OP_LEN, (__u8 *)to, (ntohs(iph->tot_len) - (iph->ihl + tcp->doff) * 4));

    iph->tot_len = htons(ntohs(iph->tot_len) + OP_LEN);

    tcp->doff += OP_LEN >> 2;

    to->ali.opcode = ALI_OPCODE;
    to->ali.opsize = ALI_OPT_SIZE;
    to->ali.port = tcp->source;
    to->ali.ip = iph->saddr;
    ALI_SET_PADDING(&to->ali);

    to->ws.opcode = WS_OPCODE;
    to->ws.opsize = WS_OPT_SIZE;
    to->ws.ip = iph->saddr;
    WS_SET_PADDING(&to->ws);

    calc_tcp_ckm(skb);

    return 0;
}

unsigned int nf_in(const struct nf_hook_ops *ops,
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

    if (tcp_add_opt(skb) == 0) {
        struct iphdr *iph = ip_hdr(skb);
        struct tcphdr *tcp = tcp_hdr(skb);

        printk("%s: %pI4:%d => %pI4:%d\n", __func__,
            &iph->saddr, ntohs(tcp->source),
            &iph->daddr, ntohs(tcp->dest));
    }

    return NF_ACCEPT;
}


unsigned int nf_out(const struct nf_hook_ops *ops,
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
    if (tcp_add_opt(skb) == 0) {
        struct iphdr *iph = ip_hdr(skb);
        struct tcphdr *tcp = tcp_hdr(skb);

        printk("%s: %pI4:%d => %pI4:%d\n", __func__,
            &iph->saddr, ntohs(tcp->source),
            &iph->daddr, ntohs(tcp->dest));
    }

    return NF_ACCEPT;
}

static struct nf_hook_ops nf_tp_ops[] = {
    {
        .hook = nf_in,
        .owner = THIS_MODULE,
        .pf = PF_INET,
        .hooknum = NF_INET_LOCAL_IN,
    },
    {
        .hook = nf_out,
        .owner = THIS_MODULE,
        .pf = PF_INET,
        .hooknum = NF_INET_LOCAL_OUT,
    }
};

static int __init _init(void)
{
    int ret = 0;

    ret = nf_register_hooks(nf_tp_ops, ARRAY_SIZE(nf_tp_ops));
    if (ret) {
        printk(KERN_ERR "register hook failed\n");
        return -1;
    }

    printk("%s:%d\n", __func__, __LINE__);

    return 0;
}

static void __exit _exit(void)
{
    nf_unregister_hooks(nf_tp_ops, ARRAY_SIZE(nf_tp_ops));
    printk("%s:%d\n", __func__, __LINE__);
}

module_init(_init);
module_exit(_exit);