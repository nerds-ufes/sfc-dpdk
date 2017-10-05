#include <stdio.h>
#include <stdlib.h>

#include <rte_eal.h>

static sfcapp_enabled_port_mask = 0;

struct sfcapp_port_assoc {
    uint32_t rx_net;
    uint32_t tx_net;
    uint32_t rx_sf;
    uint32_t tx_sf; 
}

void 
parse_args(int argc, char **argv){
    /* TODO: Implement argument parsing
     * List of arguments possible arguments
     * (Considering single configurable application)
     * - Default EAL arguments
     * - Type (classifier, proxy, SFF)
     * - Configuration file (with rules, list of SFs, etc )
     * - Port assoc (rx/tx to SF/net) [rx_net,tx_net,rx_sf,tx_sf]
     * 
     */

    /* Read port assoc and save in sfcapp_port_assoc*/
     
}

int 
main(int argc, char **argv){

    int ret;
    int n_ports;

    ret = rte_eal_init(argc,argv);
    if(ret < 0)
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments.\n");

    argc -= ret;
    argv += ret;

    /* parse_args(argc,argv); */
 
    /* allocate mbuf pool */

    n_ports = rte_eth_dev_count();
    if(n_ports == 0)
        rte_exit(EXIT_FAILURE, "No Ethernet ports available\n");

    



}