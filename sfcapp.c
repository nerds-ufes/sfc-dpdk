#include <stdio.h>
#include <stdlib.h>

#include <rte_eal.h>

#define MEMPOOL_CACHE_SIZE 256

static nb_mbuf = 4096; /* I might change this value later*/

static sfcapp_enabled_port_mask = 0;

struct sfcapp_port_assoc {
    uint32_t rx_net;
    uint32_t tx_net;
    uint32_t rx_sf;
    uint32_t tx_sf; 
}

static struct rte_mempool *sfcapp_pktmbuf_pool;

static const struct rte_eth_conf port_cfg = {
    .rxmode = {
        .header_split   = 0, /* Header Split disabled*/
        .split_hdr_size = 0,
        .hw_ip_checksum = 0, /* Disable IP Checksum */
        .hw_vlan_filter = 0, /* Disable VLAN filtering */
        .jumbo_frame    = 0, /* No jumbo frames */
        .hw_strip_crc   = 1, /* Enable HW CRC strip*/
    },

    .txmode = {
        .mq_mode = ETH_MQ_TX_NONE,
    },
};

void 
parse_args(int argc, char **argv){
    /* TODO: Implement argument parsing
     * List of arguments possible arguments
     * (Considering single configurable application)
     * - Default EAL arguments
     * - Type (classifier, proxy, SFF)
     * - Configuration file (with rules, list of SFs, etc )
     * - Port assoc (rx/tx to SF/net) [rx_net,tx_net,rx_sf,tx_sf]
     * - Hash table size
     */

    /* Read port assoc and save in sfcapp_port_assoc*/   
}

/* Function to allocate memory to be used by the application */ 
void
alloc_mem(unsigned n_mbuf){

    unsigned lcore_id;
    char* pool_name = "mbuf_pool";
    int socket_id = 0; /* Using only one socket */

    for(lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++){
        if(!rte_lcore_is_enabled(lcore_id))
            continue;
        
        if(sfcapp_pktmbuf_pool == NULL){
            sfcapp_pktmbuf_pool = rte_pktmbuf_pool_create(
                pool_name,
                n_mbuf,
                MEMPOOL_CACHE_SIZE,
                0,
                RTE_MBUF_DEFAULT_BUF_SIZE,
                socket_id);
            
            if(sfcapp_pktmbuf_pool == NULL)
                rte_exit(EXIT_FAILURE,
                    "Failed to allocate mbuf pool\n");
            else
                printf("Successfully allocated mbuf pool\n");
        }

    }
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
 
    alloc_mem(nb_mbuf);

    /* setup match table */

    n_ports = rte_eth_dev_count();
    if(n_ports == 0)
        rte_exit(EXIT_FAILURE, "No Ethernet ports available\n");

    /* configure ports according to port mask and port assoc*/
    
    /* init slave cores with configuration */

    /* run main loop */

}