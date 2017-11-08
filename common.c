#include <stdlib.h>

#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include "common.h"

uint16_t send_pkts(struct rte_mbuf **mbufs, uint8_t tx_port, uint16_t tx_q, uint16_t nb_pkts){
    
    uint16_t nb_tx;
    uint16_t buf;

    nb_tx = rte_eth_tx_burst(tx_port,tx_q,
        mbufs,nb_pkts);

    if(unlikely(nb_tx < nb_pkts)){
        for(buf = nb_tx ; buf < nb_pkts ; buf++)
            rte_pktmbuf_free(mbufs[buf]);
    }

    return nb_tx;
}

void ipv4_get_5tuple_bulk(struct rte_mbuf **mbufs, uint16_t nb_pkts, 
    struct ipv4_5tuple *tuples)
{
    struct ether_hdr *eth_hdr;
    struct ipv4_hdr *ipv4_hdr;
    struct tcp_hdr *tcp_hdr;
    struct udp_hdr *udp_hdr;
    struct ipv4_5tuple *curr_tuple;
    uint16_t eth_type;

    int i;

    for(i = 0 ; i < nb_pkts ; i++){
        
        curr_tuple = &tuples[i];

        eth_hdr = rte_pktmbuf_mtod(mbufs[i], struct ether_hdr *);
        eth_type = rte_be_to_cpu_16(eth_hdr->ether_type);
        
        if(eth_type == ETHER_TYPE_IPv4){
            ipv4_hdr = (struct ipv4_hdr *) (eth_hdr + sizeof(struct ether_hdr));
            curr_tuple->src_ip = ipv4_hdr->src_addr;
            curr_tuple->dst_ip = ipv4_hdr->dst_addr;
            curr_tuple->proto = ipv4_hdr->next_proto_id;

            switch(curr_tuple->proto){
                case IP_PROTO_UDP:
                    udp_hdr = (struct udp_hdr *) (ipv4_hdr + sizeof(struct ipv4_hdr));
                    curr_tuple->src_port = udp_hdr->src_port;
                    curr_tuple->dst_port = udp_hdr->dst_port;
                    break;
                case IP_PROTO_TCP:
                    tcp_hdr = (struct tcp_hdr *) (ipv4_hdr + sizeof(struct ipv4_hdr));
                    curr_tuple->src_port = tcp_hdr->src_port;
                    curr_tuple->dst_port = tcp_hdr->dst_port;
                    break;
                default:
                    break;
            }
        }else{
            ;/* Currently only treating IPv4 payloads*/
        }
    }
}

void common_mac_update(struct rte_mbuf *mbuf, struct ether_addr *dest){
    struct ether_hdr *eth_hdr;

    eth_hdr = rte_pktmbuf_mtod(mbuf,struct ether_hdr *);

    ether_addr_copy(&eth_hdr->d_addr,&eth_hdr->s_addr);
    ether_addr_copy(dest,&eth_hdr->d_addr);
}

void common_dump_pkt(struct rte_mbuf *mbuf, const char *msg){
    printf("%s",msg);
    rte_pktmbuf_dump(stdout,mbuf,mbuf->pkt_len);
}