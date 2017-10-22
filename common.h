#ifndef SFCAPP_COMMON_
#define SFCAPP_COMMON_

#include <rte_cfgfile.h>
#include <rte_ethdev.h>
#include <rte_ether.h>

#define MEMPOOL_CACHE_SIZE 256

#define NB_MBUF 4096 /* I might change this value later*/
#define NB_RX_QS 1
#define NB_TX_QS 1
#define NB_RX_DESC 128
#define NB_TX_DESC 128
#define BURST_SIZE 8

#define IP_PROTO_UDP 0x11
#define IP_PROTO_TCP 0x06

#define SFCAPP_CHECK_FAIL_LT(var,val,msg) do { if(var < val) rte_exit(EXIT_FAILURE,msg); } while(0)

struct ipv4_5tuple {
    uint8_t proto;
    uint16_t src_ip;
    uint16_t dst_ip;
    uint8_t  src_port;
    uint8_t  dst_port;
};

enum sfcapp_type {
    SFC_PROXY,
    SFC_CLASSIFIER,
    SFC_FORWARDER
};

struct sfcapp_config {
    uint8_t port1; 
    uint8_t port2;   
    struct rte_eth_tx_buffer tx_buffer; /* TX buffer for TX port*/         
    struct ether_addr next_hop;                /* MAC address of SFF */
    enum sfcapp_type type;              /* SFC entity type */
    void (*main_loop)(void);

    /* struct sfcapp_stats stats; */  
};

struct rte_cfgfile_parameters sfcapp_cfgfile_parameters = {
    .comment_character = '#'
};

void send_packet(struct rte_mbuf *mbufs, uint8_t tx_port);

void ipv4_get_5tuple_bulk(struct rte_mbuf **mbufs, uint16_t nb_pkts, 
    struct ipv4_5tuple *tuples);

#endif