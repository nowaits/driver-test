#ifndef _TOP_H_
#define _TOP_H_

struct tcp_opt {
    struct {
#define ALI_OPCODE 254
#define ALI_OPT_SIZE 8
        __u8 opcode;
        __u8 opsize;
        __u16 port;
        __u32 ip;
#define ALI_SET_PADDING(a)
    } __attribute__((packed)) ali;

    struct {
#define WS_OPCODE 78
#define WS_OPT_SIZE 6
        __u8 opcode;
        __u8 opsize;
        __u32 ip;
        __u8 padding[2];
#define WS_SET_PADDING(a) (memset((a)->padding, 1, sizeof((a)->padding)))
    } __attribute__((packed)) ws;

} __attribute__((packed));

#define OP_LEN (sizeof(struct tcp_opt))

#define SKB_APPEND(m, len) (skb_tailroom(m) >= len ? skb_put(m, len) : NULL)

static inline void calc_tcp_ckm(struct sk_buff *skb)
{
    struct iphdr *iph = ip_hdr(skb);
    int t_off = skb_transport_offset(skb);
    struct tcphdr *tcp = tcp_hdr(skb);

    if (iph->protocol != IPPROTO_TCP) {
        return;
    }

    tcp->check = 0;

    tcp->check = ~csum_tcpudp_magic(iph->saddr, iph->daddr, skb->len - t_off, iph->protocol, 0);

    tcp->check = csum_fold(skb_checksum(skb, t_off, skb->len - t_off, 0));

    skb->ip_summed = CHECKSUM_UNNECESSARY;

    ip_send_check(iph);
}

#endif // _TOP_H_