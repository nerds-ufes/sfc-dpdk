#ifndef SFCAPP_COMMON_
#define SFCAPP_COMMON_

#include <rte_cfgfile.h>
#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>

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
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t  src_port;
    uint16_t  dst_port;
} __attribute__((__packed__));

enum sfcapp_type {
    SFC_PROXY,
    SFC_CLASSIFIER,
    SFC_FORWARDER,
    NONE
};

struct sfcapp_config {
    uint8_t port1; 
    uint8_t port2;   
    //struct rte_eth_tx_buffer tx_buffer; /* TX buffer for TX port*/         
    struct ether_addr next_hop;                /* MAC address of SFF */
    enum sfcapp_type type;              /* SFC entity type */
    void (*main_loop)(void);

    /* struct sfcapp_stats stats; */  
};

/*struct rte_cfgfile_parameters sfcapp_cfgfile_parameters = {
    .comment_character = '#'
};*/

uint16_t send_pkts(struct rte_mbuf **mbufs, uint8_t tx_port, uint16_t tx_q, uint16_t nb_pkts);

void common_print_ipv4_5tuple(struct ipv4_5tuple *tuple);

void common_ipv4_get_5tuple(struct rte_mbuf *mbuf, struct ipv4_5tuple *tuple, uint16_t offset);

void common_ipv4_get_5tuple_bulk(struct rte_mbuf **mbufs, struct ipv4_5tuple *tuples, 
    struct ipv4_5tuple **tuple_ptrs, uint16_t nb_pkts);

void common_mac_update(struct rte_mbuf *mbuf, struct ether_addr *dest);

void common_dump_pkt(struct rte_mbuf *mbuf, const char *msg);

uint64_t common_mac_to_64(struct ether_addr *mac);

void common_64_to_mac(uint64_t val, struct ether_addr *mac);

int common_parse_portmask(const char *portmask);

enum sfcapp_type common_parse_apptype(const char *type);

int common_parse_uint16(const char *str, uint16_t *res);

int common_parse_uint32(const char* str, uint32_t *res);

int common_parse_uint64(const char* str, uint64_t *res);

int common_parse_ether(const char *str, struct ether_addr *eth_addr);

#endif