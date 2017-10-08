#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_lcore.h>


#define MEMPOOL_CACHE_SIZE 256

#define NB_MBUF 4096 /* I might change this value later*/
#define NB_RX_QS 1
#define NB_TX_QS 1
#define NB_RX_DESC 128
#define NB_TX_DESC 128

static uint32_t sfcapp_enabled_port_mask = 0;

struct sfcapp_port_assoc {
    uint32_t rx_net;
    uint32_t tx_net;
    uint32_t rx_sf;
    uint32_t tx_sf; 
};

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

static int
parse_portmask(const char *portmask)
{
	char *end = NULL;
	unsigned long pm;

	/* parse hexadecimal string */
	pm = strtoul(portmask, &end, 16);
	if ((portmask[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;

	if (pm == 0)
		return -1;

	return pm;
}

static const char sfcapp_options[] = {
    'p', /* Port mask */
    't', /* SFC entity type*/
    'f',
    'h', /* Print usage */
    'H' /* Hash table size*/
};

static void 
parse_args(int argc, char **argv){
    /* List of possible arguments
     * -t : Type (classifier, proxy, SFF)
     * -f : Configuration file (with rules, list of SFs, etc )
     * -H : Hash table size
     * -h : Print usage information
     */
    int sfcapp_opt;

    while( (sfcapp_opt = getopt(argc,argv,"p:t:hH:")) != -1){
        switch(sfcapp_opt){
            case 'p':
                sfcapp_enabled_port_mask = parse_portmask(optarg);
                break;
            case 't':
                break;
            case 'f':
                break;
            case 'h':
                break;
            case 'H':
                break;
            case '?':
                break;
            default:
                rte_exit(EXIT_FAILURE,"Unrecognized option: %c\n",sfcapp_opt);
                break;
        }
    }
}

/* Function to allocate memory to be used by the application */ 
static void
alloc_mem(unsigned n_mbuf){

    unsigned lcore_id;
    const char* pool_name = "mbuf_pool";

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
                rte_socket_id());
            
            if(sfcapp_pktmbuf_pool == NULL)
                rte_exit(EXIT_FAILURE,
                    "Failed to allocate mbuf pool\n");
            else
                printf("Successfully allocated mbuf pool\n");
        }

    }
}

static int
init_port(uint8_t port, struct rte_mempool *mbuf_pool){
    struct rte_eth_conf port_conf = port_cfg;
    int ret;
    uint16_t q;

    if(port >= rte_eth_dev_count())
        return -1;
    
    ret = rte_eth_dev_configure(port,NB_RX_QS,NB_TX_QS,&port_conf);
    if(ret != 0)
        return ret;
    
    /* Setup TX queues */
    for(q = 0 ; q < NB_TX_QS ; q++){
        ret = rte_eth_tx_queue_setup(port, q, NB_TX_DESC,
            rte_eth_dev_socket_id(port), NULL);

        if(ret < 0)
            return ret;
    }

    /* Setup RX queues */
    for(q = 0 ; q < NB_RX_QS ; q++){
        ret = rte_eth_rx_queue_setup(port, q, NB_RX_DESC,
            rte_eth_dev_socket_id(port), NULL, mbuf_pool);

        if(ret < 0)
            return ret;
    }

    ret = rte_eth_dev_start(port);

    if(ret > 0)
        return ret;

    struct ether_addr eth_addr;
    rte_eth_macaddr_get(port,&eth_addr);
    printf("MAC of port %u: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
            " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
            (unsigned) port,
            eth_addr.addr_bytes[0],eth_addr.addr_bytes[1],
            eth_addr.addr_bytes[2],eth_addr.addr_bytes[3],
            eth_addr.addr_bytes[4],eth_addr.addr_bytes[5]);

    return 0;

}

int 
main(int argc, char **argv){

    int ret;
    int n_ports;
    //unsigned lcore_id;
    uint8_t port;

    ret = rte_eal_init(argc,argv);
    if(ret < 0)
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments.\n");

    argc -= ret;
    argv += ret;

    parse_args(argc,argv);
 
    alloc_mem(NB_MBUF);

    /* setup match table */

    n_ports = rte_eth_dev_count();

    if(n_ports < 2)
        rte_exit(EXIT_FAILURE, 
            "Not enough Ethernet ports (%d) available\n",
            n_ports);

    for(port = 0 ; port < n_ports ; port++){
        if( (sfcapp_enabled_port_mask & (1<<port)) == 0)
            continue;
        
        printf("Configuring port %u...\n",(unsigned) port);
        ret = init_port(port,sfcapp_pktmbuf_pool);

        if(ret < 0)
            rte_exit(EXIT_FAILURE,"Port configuration failed...\n");
    }

    /* Assign one lcore for RX/TX and another for encap/decap tasks
     * Only one queue per port will be used.
     */ 
    

    /* run main loop */

    return ret;
}