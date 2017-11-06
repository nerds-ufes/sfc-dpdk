#include <stdlib.h>

#include <rte_ether.h>

#include "nsh.h"

//void nsh_encap(rte_mbuf* pkt_mbuf, uint64_t nsh_hdr){}

void nsh_decap(struct rte_mbuf* pkt_mbuf){
    // Allocate a new mbuf the size of the packet + NSH hdr
    // Copy all info to new mbuf (respecting new space for NSH)
    // Add NSH info
    // Free old mbuf
    
    // If using this approach, will have to return the pointer
    // to the new mbuf
}

//int nsh_dec_si(struct rte_mbuf* pkt_mbuf){}

int nsh_get_header(struct rte_mbuf *mbuf, uint64_t *nsh_info){
    struct ether_hdr *eth_hdr;
    struct ipv4_hdr *ipv4_hdr;
    struct udp_hdr *udp_hdr;
    struct nsh_hdr *nsh_hdr;

    eth_hdr = rte_pktmbuf_mtod(mbuf,struct ether_hdr *);

    /* TODO: Check if packet is well-formed before accessing the value.
     * If not, don't copy any data and return -1 to indicate error.
     */

    nsh_hdr = eth_hdr + sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) +
        sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr);

    nsh_info = rte_memcpy(nsh_info,nsh_hdr,sizeof(nsh_hdr));

    return 0;
}