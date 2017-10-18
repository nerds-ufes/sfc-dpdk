#ifndef SFCAPP_COMMON_
#define SFCAPP_COMMON_

#include <rte_cfgfile.h>

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
    uint8_t rx_port;
    uint8_t tx_port;
    enum sfcapp_type type;
    void (*main_loop)(void*);
    /* struct sfcapp_stats stats; */  
};

struct rte_cfgfile_parameters sfcapp_cfgfile_parameters = {
    .comment_character = '#';
}

#endif