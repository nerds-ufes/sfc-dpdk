#include <stdlib.h>

#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_byteorder.h>
#include <rte_hexdump.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include "nsh.h"

extern struct rte_mempool *sfcapp_pktmbuf_pool;

void nsh_encap(struct rte_mbuf* mbuf, struct nsh_hdr *nsh_info){
    uint32_t i;
    char *init_inner;
    uint32_t tun_hdr_sz;
    uint32_t offset;
    uint32_t prev_sz;
    struct nsh_hdr *nsh_header;
    
    printf("\n=== Full packet ===\n");
    rte_pktmbuf_dump(stdout,mbuf,mbuf->pkt_len);

    /* TODO: Check if packet is VXLAN or not */
    if(!rte_pktmbuf_is_contiguous(mbuf))
        rte_exit(EXIT_FAILURE,"Not contiguous!! Don't know what to do.\n");

    prev_sz = mbuf->pkt_len;

    offset = sizeof(struct nsh_hdr);

    rte_pktmbuf_append(mbuf,offset);

    if(mbuf == NULL)
        rte_exit(EXIT_FAILURE,"Failed to encapsulate packet. Not enough room.\n");
    
    // Get pointer to end of inner packet data 
    tun_hdr_sz = (sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) 
        + sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr));

    init_inner = (char*) mbuf->buf_addr + rte_pktmbuf_headroom(mbuf) + 
        tun_hdr_sz;

    // Shift inner packet right to free space for NSH header 
    for(i = (int) prev_sz - 1 ; i >= tun_hdr_sz  ; i--)
        init_inner[i+offset] = init_inner[i];    

    nsh_header = rte_pktmbuf_mtod_offset(mbuf,struct nsh_hdr *,tun_hdr_sz);
    nsh_header->basic_info  = nsh_info->basic_info;
    nsh_header->md_type     = nsh_info->md_type;
    nsh_header->next_proto  = nsh_info->next_proto;
    nsh_header->serv_path   = nsh_info->serv_path;

    printf("\n\n\n=== Decapped packet ===\n");
    rte_pktmbuf_dump(stdout,mbuf,mbuf->pkt_len);
}

void nsh_decap(struct rte_mbuf* mbuf){
    int i;
    char *init_new_inner;
    uint16_t offset;

    //printf("\n=== Full packet ===\n");
    //rte_pktmbuf_dump(stdout,mbuf,mbuf->pkt_len);

    if(!rte_pktmbuf_is_contiguous(mbuf))
        rte_exit(EXIT_FAILURE,"Not contiguous!! Don't know what to do.\n");
    
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

/* Initializes the NSH header with default values */
int nsh_init_header(struct nsh_hdr *nsh_header){
   
    if(nsh_header == NULL)
        return -1;
    
    nsh_header->basic_info =  ((uint8_t) 0) | NSH_TTL_DEFAULT | NSH_BASE_LENGHT_MD_TYPE_2;
    nsh_header->md_type = NSH_MD_TYPE_2;
    nsh_header->next_proto = NSH_NEXT_PROTO_ETHER;
    nsh_header->serv_path = 0;
    
    return 0;
}

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
