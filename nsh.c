#include <stdlib.h>

#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_ether.h>
#include <rte_byteorder.h>
#include <rte_hexdump.h>
#include "nsh.h"

//void nsh_encap(rte_mbuf* pkt_mbuf, uint64_t nsh_hdr){}

void nsh_decap(struct rte_mbuf* pkt_mbuf){
    // Allocate a new mbuf the size of the packet + NSH hdr
    // Copy all info to new mbuf (respecting new space for NSH)
    // Add NSH info
    // Free old mbuf
    
    // If using this approach, will have to return the pointer
    // to the new mbuf
    pkt_mbuf += 1;
}

//int nsh_dec_si(struct rte_mbuf* pkt_mbuf){}

int nsh_get_header(struct rte_mbuf *mbuf, struct nsh_hdr *nsh_info){
    struct ether_hdr *eth_hdr;
    struct ipv4_hdr *ip4;
    //struct ipv4_hdr *ipv4_hdr;
    //struct udp_hdr *udp_hdr;
    struct nsh_hdr *nsh_hdr;
    uint32_t hdrinfo;
    uint32_t ipdst,ipsrc;

    rte_pktmbuf_dump(stdout,mbuf,112);
    eth_hdr = rte_pktmbuf_mtod(mbuf,struct ether_hdr *);
    ip4 = rte_pktmbuf_mtod_offset(mbuf,struct ipv4_hdr *,sizeof(struct ether_hdr));

    /* TODO: Check if packet is well-formed before accessing the value.
     * If not, don't copy any data and return -1 to indicate error.
     */

    nsh_hdr = rte_pktmbuf_mtod_offset(mbuf,struct nsh_hdr *, sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) +
        sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr));
    hdrinfo = rte_be_to_cpu_32(nsh_hdr->serv_path);
    ipsrc = rte_be_to_cpu_32(ip4->src_addr);
    ipdst = rte_be_to_cpu_32(ip4->dst_addr);

    nsh_info->basic_info = rte_be_to_cpu_32(nsh_hdr->basic_info);
    nsh_info->md_type = rte_be_to_cpu_32(nsh_hdr->md_type);
    nsh_info->next_proto = rte_be_to_cpu_32(nsh_hdr->next_proto);
    nsh_info->serv_path = rte_be_to_cpu_32(nsh_hdr->serv_path);

    printf("Get header >> src_IP: %08" PRIx32 " dst_IP: %08" PRIx32 " SPI+SI: %" PRIx32 "\n",ipsrc,ipdst,hdrinfo);
    nsh_info = rte_memcpy(nsh_info,nsh_hdr,sizeof(struct nsh_hdr));

    return 0;
}
