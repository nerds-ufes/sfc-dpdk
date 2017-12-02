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
#define NB_RX_DESC 2048
#define NB_TX_DESC 2048
#define BURST_SIZE 64

#define TX_BUFFER_SIZE 1024


#define IP_PROTO_UDP 0x11
#define IP_PROTO_TCP 0x06

#define VXLAN_PORT 4789

#define CFG_FILE_MAX_SECTIONS 1024

#define SFCAPP_CHECK_FAIL_LT(var,val,msg) do { if(var < val) rte_exit(EXIT_FAILURE,msg); } while(0)

#define COND_MARK_DROP(lkp,drop_mask) \
        if(unlikely(lkp < 0)){ \
            /*printf("Dropping packet!\n");*/ \
            *drop_mask |= 1<<i; \
            continue; \
        }

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
    struct rte_eth_dev_tx_buffer *tx_buffer1; /* TX buffer for TX port*/         
    struct ether_addr port1_mac; 
    struct rte_eth_dev_tx_buffer *tx_buffer2; /* TX buffer for TX port*/         
    uint8_t port2;   
    struct ether_addr port2_mac;
    struct ether_addr sff_addr;                /* MAC address of SFF */
    enum sfcapp_type type;              /* SFC entity type */
    void (*main_loop)(void);
    uint64_t rx_pkts, tx_pkts, dropped_pkts;
};

/*struct rte_cfgfile_parameters sfcapp_cfgfile_parameters = {
    .comment_character = '#'
};*/

void common_flush_tx_buffers(void);

uint16_t send_pkts(struct rte_mbuf **mbufs, uint8_t tx_port, uint16_t tx_q, struct rte_eth_dev_tx_buffer* tx_buffer,
 uint16_t nb_pkts, uint64_t drop_mask);

void common_print_ipv4_5tuple(struct ipv4_5tuple *tuple);

int common_ipv4_get_5tuple(struct rte_mbuf *mbuf, struct ipv4_5tuple *tuple, uint16_t offset);

void common_ipv4_get_5tuple_bulk(struct rte_mbuf **mbufs, struct ipv4_5tuple *tuples, 
    struct ipv4_5tuple **tuple_ptrs, uint16_t nb_pkts);

void common_mac_update(struct rte_mbuf *mbuf, struct ether_addr *src, struct ether_addr *dst);

void common_dump_pkt(struct rte_mbuf *mbuf, const char *msg);

uint64_t common_mac_to_64(struct ether_addr *mac);

void common_64_to_mac(uint64_t val, struct ether_addr *mac);

int common_check_destination(struct rte_mbuf *mbuf, struct ether_addr *mac);

void common_vxlan_encap(struct rte_mbuf *mbuf);

#endif