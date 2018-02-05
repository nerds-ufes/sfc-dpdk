#include <stdlib.h>
#include <inttypes.h>

#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_byteorder.h>
#include <rte_hash_crc.h>
#include <rte_ip.h>
#include <rte_udp.h>

#include "common.h"
#include "vxlan_gpe.h"

extern struct sfcapp_config sfcapp_cfg;
extern long int n_rx, n_tx;

#define PORT_MIN	49152
#define PORT_MAX	65535
#define PORT_RANGE ((PORT_MAX - PORT_MIN) + 1)
#define SFCAPP_DEFAULT_VNI 1000
#define IP_VERSION 0x40
#define IP_HDRLEN  0x05 
#define IP_DEFTTL  64
#define IP_VHL_DEF (IP_VERSION | IP_HDRLEN)

void common_flush_tx_buffers(void){
    rte_eth_tx_buffer_flush(sfcapp_cfg.port1,0,sfcapp_cfg.tx_buffer1);
    rte_eth_tx_buffer_flush(sfcapp_cfg.port2,0,sfcapp_cfg.tx_buffer2);
}

uint16_t send_pkts(struct rte_mbuf **mbufs, uint8_t tx_port, uint16_t tx_q, 
    struct rte_eth_dev_tx_buffer* tx_buffer, uint16_t nb_pkts, uint64_t drop_mask){
    uint16_t i;
    uint16_t sent=0;
    uint16_t total_sent=0;

    for(i = 0 ; i < nb_pkts ; i++){
        if( (drop_mask & (1<<i)) == 0 ){
            sent = rte_eth_tx_buffer(tx_port,tx_q,tx_buffer,mbufs[i]);
            total_sent += sent;
        }else
            sfcapp_cfg.dropped_pkts++;
    }
    
    return total_sent;
}

static void sprint_ipv4(uint32_t ip, char* buffer){
    uint8_t a,b,c,d;
    a = (ip>>24);
    b = (ip>>16);
    c = (ip>>8);
    d = ip;

    sprintf(buffer,"%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8 "",a,b,c,d);
}

void common_print_ipv4_5tuple(struct ipv4_5tuple *tuple){
    char ip1[16],ip2[16];
    sprint_ipv4(tuple->src_ip,ip1);
    sprint_ipv4(tuple->dst_ip,ip2);

    printf("<ipsrc: %s, ipdst: %s, proto: %02" PRIu8 ", psrc: %" PRIu16 
           ", pdst: %" PRIu16 ">",ip1,ip2,tuple->proto,
           tuple->src_port,tuple->dst_port);
}

int common_ipv4_get_5tuple(struct rte_mbuf *mbuf, struct ipv4_5tuple *tuple, uint16_t offset){
    struct ipv4_hdr *ipv4_hdr;
    struct tcp_hdr *tcp_hdr;
    struct udp_hdr *udp_hdr;
    struct ether_hdr *eth_hdr;

    eth_hdr = rte_pktmbuf_mtod(mbuf,struct ether_hdr *);

    if(rte_be_to_cpu_16(eth_hdr->ether_type) != ETHER_TYPE_IPv4)
        return -1;

    ipv4_hdr = rte_pktmbuf_mtod_offset(mbuf,struct ipv4_hdr *,
                    (offset + sizeof(struct ether_hdr)));


    tuple->src_ip = rte_be_to_cpu_32(ipv4_hdr->src_addr);
    tuple->dst_ip = rte_be_to_cpu_32(ipv4_hdr->dst_addr);
    tuple->proto  = ipv4_hdr->next_proto_id;
    
    switch(tuple->proto){
        case IP_PROTO_UDP:
            udp_hdr = (struct udp_hdr *) ( (unsigned char*) ipv4_hdr + sizeof(struct ipv4_hdr));
            tuple->src_port = rte_be_to_cpu_16(udp_hdr->src_port);
            tuple->dst_port = rte_be_to_cpu_16(udp_hdr->dst_port);
            break;
        case IP_PROTO_TCP:
            tcp_hdr = (struct tcp_hdr *) ( (unsigned char*) ipv4_hdr + sizeof(struct ipv4_hdr));
            tuple->src_port = rte_be_to_cpu_16(tcp_hdr->src_port);
            tuple->dst_port = rte_be_to_cpu_16(tcp_hdr->dst_port);
            break;
        default:
            tuple->src_port = 0;
            tuple->dst_port = 0;
            break;
    }

    return 0;
}

void common_mac_update(struct rte_mbuf *mbuf, struct ether_addr *src, struct ether_addr *dst){
    struct ether_hdr *eth_hdr;

    eth_hdr = rte_pktmbuf_mtod(mbuf,struct ether_hdr *);

    ether_addr_copy(src,&eth_hdr->s_addr);
    ether_addr_copy(dst,&eth_hdr->d_addr);
}

void common_dump_pkt(struct rte_mbuf *mbuf, const char *msg){
    printf("%s",msg);
    rte_pktmbuf_dump(stdout,mbuf,mbuf->pkt_len);
}

uint64_t common_mac_to_64(struct ether_addr *mac){

    return (((uint64_t) mac->addr_bytes[5])<<40) | 
           (((uint64_t) mac->addr_bytes[4])<<32) |
           (((uint64_t) mac->addr_bytes[3])<<24) |
           (((uint64_t) mac->addr_bytes[2])<<16) |
           (((uint64_t) mac->addr_bytes[1])<<8)  |
           (((uint64_t) mac->addr_bytes[0]))     |
           (uint64_t) 0;
}

void common_64_to_mac(uint64_t val, struct ether_addr *mac){
    mac->addr_bytes[0] = (uint8_t) (val);// & 0xFF);
    mac->addr_bytes[1] = (uint8_t) (val>>8);// & 0xFF);
    mac->addr_bytes[2] = (uint8_t) (val>>16);// & 0xFF);
    mac->addr_bytes[3] = (uint8_t) (val>>24);// & 0xFF);
    mac->addr_bytes[4] = (uint8_t) (val>>32);// & 0xFF);
    mac->addr_bytes[5] = (uint8_t) (val>>40);// & 0xFF);
}

int common_check_destination(struct rte_mbuf *mbuf, struct ether_addr *mac){
    struct ether_hdr *eth_hdr;
    char mac1[64],mac2[64];
    int res;
    eth_hdr = rte_pktmbuf_mtod(mbuf,struct ether_hdr *);
    ether_format_addr(mac1,64,&eth_hdr->d_addr);
    ether_format_addr(mac2,64,mac);
    res = memcmp(&eth_hdr->d_addr,mac,sizeof(struct ether_addr));
    /*if(res == 0)
        printf("Comparing %s == %s => result: %d\n",mac1,mac2,res);
    */return res;
}

void common_vxlan_encap(struct rte_mbuf *mbuf){
    struct ether_hdr *eth_hdr, *inner_ether;
    struct ipv4_hdr *ipv4_hdr;
    struct udp_hdr *udp_hdr;
    struct vxlan_hdr *vxlan_hdr;
    uint16_t old_pkt_len;
    uint32_t hash;

    old_pkt_len = mbuf->pkt_len;
    inner_ether = rte_pktmbuf_mtod(mbuf,struct ether_hdr *);
    hash = rte_hash_crc(inner_ether,2*ETHER_ADDR_LEN,inner_ether->ether_type);

    eth_hdr = (struct ether_hdr *) rte_pktmbuf_prepend(mbuf,sizeof(struct ether_hdr) + 
        sizeof(struct ipv4_hdr) + sizeof(struct udp_hdr) + 
        sizeof(struct vxlan_hdr));
    
    ipv4_hdr  = (struct ipv4_hdr *) (((char*) eth_hdr) + sizeof(struct ether_hdr));
    udp_hdr   = (struct udp_hdr *) (((char*) ipv4_hdr) + sizeof(struct ipv4_hdr));
    vxlan_hdr = (struct vxlan_hdr *) ( ((char*) udp_hdr) + sizeof(struct udp_hdr));

    /* Outer Ethernet must be updated by caller function later */
    eth_hdr->ether_type = rte_cpu_to_be_16(ETHER_TYPE_IPv4);

    /* These are just placeholders for now.
     * Currently all communication is done in L2.
     */
    ipv4_hdr->version_ihl = IP_VHL_DEF;
    ipv4_hdr->type_of_service = 0;
    ipv4_hdr->fragment_offset = 0;
    ipv4_hdr->time_to_live = IP_DEFTTL;
    ipv4_hdr->packet_id = 0;
    ipv4_hdr->hdr_checksum = 0;
    ipv4_hdr->src_addr = rte_cpu_to_be_32(IPv4(10,10,10,10));
    ipv4_hdr->dst_addr = rte_cpu_to_be_32(IPv4(10,10,10,11));
    ipv4_hdr->next_proto_id = IP_PROTO_UDP;
    ipv4_hdr->total_length = rte_cpu_to_be_16(mbuf->pkt_len - 
        sizeof(struct ether_hdr));

    udp_hdr->dgram_cksum = 0;
    udp_hdr->dgram_len = rte_cpu_to_be_16(old_pkt_len + 
        sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr));
    udp_hdr->dst_port = rte_cpu_to_be_16(VXLAN_PORT);
    udp_hdr->src_port = rte_cpu_to_be_16((((uint64_t) hash * PORT_RANGE) >> 32)
					+ PORT_MIN);

    vxlan_hdr->vx_flags = rte_cpu_to_be_32(VXLAN_INSTANCE_FLAG);
    vxlan_hdr->vx_vni = rte_cpu_to_be_32(SFCAPP_DEFAULT_VNI << 8);
}