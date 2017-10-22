#include <stdlib.h>

#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include "common.h"

void send_packet(struct rte_mbuf *mbufs, uint8_t tx_port){

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

        ether_hdr =  = rte_pktmbuf_mtod(mbufs[i], struct ether_hdr *);
        eth_type = rte_be_to_cpu_16(ether_addr->ether_type);
        
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
                    break;
                default:
                    break;
            }
        }
    }
}