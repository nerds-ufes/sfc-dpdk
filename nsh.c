#include <stdlib.h>

#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_byteorder.h>
#include <rte_hexdump.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include "nsh.h"

extern struct rte_mempool *sfcapp_pktmbuf_pool;

/*void nsh_encap(struct rte_mbuf* pkt_mbuf, uint64_t nsh_info){
    struct ether_hdr *inner_eth_hdr;
    struct nsh_hdr *nsh_hdr;

    nsh_hdr = rte_pktmbuf_mtod_offset(mbuf,struct ether_hdr *, 
        sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) +
        sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr) );
    
    // Does this work?
    inner_eth_hdr = nsh_hdr + sizeof(struct nsh_hdr);
    
    // Append size of NSH header to end of packet
    mbuf = rte_pktmbuf_append(mbuf,sizeof(struct nsh_hdr));

    if(mbuf == NULL)
        rte_exit(EXIT_FAILURE,"Failed to encapsulate packet. Not enough room.\n");
        
    // Shift inner packet data to end
    // HOW TO DO THIS???

    // Add NSH info
    *nsh_hdr = nsh_info;  
}*/

void nsh_decap(struct rte_mbuf* mbuf){
    int i;
    char *init_new_inner;
    uint16_t offset;

    //printf("\n=== Full packet ===\n");
    //rte_pktmbuf_dump(stdout,mbuf,mbuf->pkt_len);

    if(!rte_pktmbuf_is_contiguous(mbuf))
        rte_exit(EXIT_FAILURE,"Not contiguous!! Don't know what to do.\n");

    if(mbuf == NULL)
        rte_exit(EXIT_FAILURE,"Failed to encapsulate packet. Not enough room.\n");
    
    /* Get pointer to beginning of inner packet data */
    init_new_inner = (char*) mbuf->buf_addr + rte_pktmbuf_headroom(mbuf) + 
        sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) + 
        sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr);
    
    offset = sizeof(struct nsh_hdr);

    /* Shift inner packet left to use NSH header space */
    for(i = 0; i < (int) mbuf->pkt_len ; i++)
        init_new_inner[i] = init_new_inner[i+offset];    
    
    rte_pktmbuf_trim(mbuf,offset);

    //printf("\n\n\n=== Decapped packet ===\n");
    //rte_pktmbuf_dump(stdout,mbuf,mbuf->pkt_len);
}

//int nsh_dec_si(struct rte_mbuf* pkt_mbuf){}

int nsh_get_header(struct rte_mbuf *mbuf, struct nsh_hdr *nsh_info){
    struct nsh_hdr *nsh_hdr;

    nsh_hdr = rte_pktmbuf_mtod_offset(mbuf,struct nsh_hdr *, sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) +
        sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr));


    nsh_info->basic_info = rte_be_to_cpu_32(nsh_hdr->basic_info);
    nsh_info->md_type = rte_be_to_cpu_32(nsh_hdr->md_type);
    nsh_info->next_proto = rte_be_to_cpu_32(nsh_hdr->next_proto);
    nsh_info->serv_path = rte_be_to_cpu_32(nsh_hdr->serv_path);

    //printf("Get header >> src_IP: %08" PRIx32 " dst_IP: %08" PRIx32 " SPI+SI: %" PRIx32 "\n",ipsrc,ipdst,hdrinfo);

    return 0;
}
