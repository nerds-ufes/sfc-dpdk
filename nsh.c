#include <stdlib.h>

#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_byteorder.h>
#include <rte_hexdump.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include "nsh.h"
#include "common.h"

extern struct rte_mempool *sfcapp_pktmbuf_pool;

void nsh_encap(struct rte_mbuf* mbuf, struct nsh_hdr *nsh_info){
    int i;
    char *init_inner;
    uint16_t tun_hdr_sz;
    uint16_t offset;
    struct nsh_hdr *nsh_header;

    /* TODO: Check if packet is VXLAN or not */
    if(!rte_pktmbuf_is_contiguous(mbuf))
        rte_exit(EXIT_FAILURE,"Not contiguous!! Don't know what to do.\n");

    offset = sizeof(struct nsh_hdr);

    rte_pktmbuf_append(mbuf,offset);

    if(mbuf == NULL)
        rte_exit(EXIT_FAILURE,"Failed to encapsulate packet. Not enough room.\n");
    
    tun_hdr_sz = (sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) 
        + sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr));

    init_inner = (char*) mbuf->buf_addr + rte_pktmbuf_headroom(mbuf);

    for(i = (int) mbuf->pkt_len ; i >= tun_hdr_sz  ; i--)
        init_inner[i] = init_inner[i-offset];
    
    nsh_header = rte_pktmbuf_mtod_offset(mbuf,struct nsh_hdr *,tun_hdr_sz);
    nsh_header->basic_info  = rte_cpu_to_be_16(nsh_info->basic_info);
    nsh_header->md_type     = nsh_info->md_type;
    nsh_header->next_proto  = nsh_info->next_proto;
    nsh_header->serv_path   = rte_cpu_to_be_32(nsh_info->serv_path);
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

int nsh_dec_si(struct rte_mbuf* mbuf){
    struct nsh_hdr *nsh_hdr;
    uint32_t serv_path;

    nsh_hdr = rte_pktmbuf_mtod_offset(mbuf,struct nsh_hdr *,
        sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) + 
        sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr));

    serv_path = rte_be_to_cpu_32(nsh_hdr->serv_path);

    printf("SPI|SI before: %08" PRIx32 "\n",serv_path);
    if( (serv_path & 0x000000FF) != 0 ){
        serv_path--;
        nsh_hdr->serv_path = rte_cpu_to_be_32(serv_path);
        printf("SPI|SI after: %08" PRIx32 "\n",serv_path);
        return 0;
    }

    printf("Failed to decrement SI\n");
    return -1;
}

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


    nsh_info->basic_info = rte_be_to_cpu_16(nsh_hdr->basic_info);
    nsh_info->md_type = nsh_hdr->md_type;
    nsh_info->next_proto = nsh_hdr->next_proto;
    nsh_info->serv_path = rte_be_to_cpu_32(nsh_hdr->serv_path);

    return 0;
}

uint64_t nsh_header_to_uint64(struct nsh_hdr *nsh_info){
    return (((uint64_t) nsh_info->basic_info)<<48) |
           (((uint64_t) nsh_info->md_type)   <<40) |
           (((uint64_t) nsh_info->next_proto)<<32) |
           (((uint64_t) nsh_info->serv_path));
}

void nsh_uint64_to_header(uint64_t hdr_int, struct nsh_hdr *nsh_info){
    nsh_info->basic_info = (uint8_t)  (hdr_int>>48);
    nsh_info->md_type    = (uint8_t)  (hdr_int>>40);
    nsh_info->next_proto = (uint16_t) (hdr_int>>32);
    nsh_info->serv_path  = (uint32_t) (hdr_int);
}
