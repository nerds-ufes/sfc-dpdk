#include <stdlib.h>

#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_cfgfile.h>
#include <rte_ethdev.h>
#include <rte_cfgfile.h>
#include <rte_ether.h>

#include "sfc_proxy.h"
#include "nsh.h"
#include "common.h"

#define VXLAN_NSH_INNER_OFFSET 58

#define CHECK_LKP_FAIL(drop_mask,lkp) \
        if(unlikely(lkp < 0)){ \
            *drop_mask |= 1<<i; \
            continue; \
        }

extern struct sfcapp_config sfcapp_cfg;

static struct rte_hash *proxy_flow_lkp_table;
/* key = ipv4_5tuple ; value = NSH base hdr + SPI + SI (4B) */

static struct rte_hash* proxy_sf_id_lkp_table;
/* key = <spi,si> ; value = sfid (16b) */

static struct rte_hash *proxy_sf_address_lkp_table;
/* key = sfid (16b) ; value = ethernet (48b in 64b) */

void proxy_parse_config_file(struct rte_cfgfile *cfgfile, char** sections, int nb_sections){

    int sfid_ok, mac_ok, sph_ok;
    int nb_entries;
    int i,j,ret;
    uint16_t sfid;
    uint32_t sph;
    struct ether_addr sfmac;
    struct rte_cfgfile_entry entries[PROXY_CFG_MAX_ENTRIES];

    /* Parse sections */
    for(i = 0 ; i < nb_sections ; i++){

        sfid_ok = 0;
        mac_ok = 0;
        sph_ok = 0;

        nb_entries = rte_cfgfile_section_entries_by_index(cfgfile,i,sections[i],
                        entries,PROXY_CFG_MAX_ENTRIES);

        /* Parse SF Sections */
        if(strcmp(sections[i],"SF") == 0){

            if(nb_entries != 2)
                rte_exit(EXIT_FAILURE,
                    "Wrong argument number in SF section in config file. Expected 2, found %d",
                    nb_entries);

            for(j = 0 ; j < nb_entries ; j++){

                //sfid
                if(strcmp(entries[j].name,"sfid") == 0){
                    if(sfid_ok)
                        printf("Duplicated sfid entry in SF section. Ignoring...\n");
                    else{
                        ret = common_parse_uint16(entries[j].value,&sfid);
                        SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to parse SF ID from config file\n");
                        sfid_ok = 1;
                    }

                }

                if(strcmp(entries[j].name,"mac") == 0){
                    if(mac_ok)
                        printf("Duplicated mac entry in SF section. Ignoring...\n");
                    else{
                        ret = common_parse_ether(entries[j].value,&sfmac);
                        SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to parse mac address from config file\n");
                        mac_ok = 1;
                    }

                } 
            }

            if(mac_ok && sfid_ok){
                ret = rte_hash_add_key_data(proxy_sf_address_lkp_table,&sfid, 
                        (void *) common_mac_to_64(&sfmac));
                SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to add SF entry to table.\n");

                char buf[ETHER_ADDR_FMT_SIZE + 1];
                ether_format_addr(buf,ETHER_ADDR_FMT_SIZE,&sfmac);
                printf("Successfully added <sfid=%" PRIx16 ",mac=%s> to proxy" 
                    " SF-address table.\n",sfid,buf);
            }
        }else if(strcmp(sections[i],"PATH_NODE") == 0){

            if(nb_entries != 2)
                rte_exit(EXIT_FAILURE,"Wrong argument number in SF section in config file. Expected 2, found %d",nb_entries);

            for(j = 0 ; j < nb_entries ; j++){
                
                if(strcmp(entries[j].name,"sfid") == 0){
                    if(sfid_ok)
                        printf("Duplicated sfid entry in PATH_NODE section. Ignoring...\n");
                    else{
                        ret = common_parse_uint16(entries[j].value,&sfid);
                        SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to parse SF ID from config file\n");
                        sfid_ok = 1;
                    }
                }

                if(strcmp(entries[j].name,"sph") == 0){
                    if(sph_ok)
                        printf("Duplicated mac entry in PATH_NODE section. Ignoring...\n");
                    else{
                        ret = common_parse_uint32(entries[j].value,&sph);
                        SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to parse service path info from config file\n");
                        sph_ok = 1;
                    }
                }         
            }

            if(sph_ok && sfid_ok){
                ret = rte_hash_add_key_data(proxy_sf_id_lkp_table,&sph, 
                        (void *) ((uint64_t) sfid) );
                SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to add stub entry 1.\n");

                printf("Successfully added <sph=%" PRIx32 ",sfid=%" PRIx16 ">"
                       " to proxy SF ID table.\n",sph,sfid);
            }
        }else{
            rte_exit(EXIT_FAILURE,
                "Section %s unknown, please check config file.\n",
                sections[i]);
        }    
    }
}

static int proxy_init_flow_table(void){

    const struct rte_hash_parameters hash_params = {
        .name = "proxy_flow",
        .entries = PROXY_MAX_FLOWS,
        .reserved = 0,
        .key_len = sizeof(struct ipv4_5tuple),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id()
    };

    proxy_flow_lkp_table = rte_hash_create(&hash_params);

    if(proxy_flow_lkp_table == NULL)
        return -1;
    
    return 0;
}

static int proxy_init_sf_addr_table(void){
    
    const struct rte_hash_parameters hash_params = {
        .name = "proxy_sf_addr",
        .entries = PROXY_MAX_FUNCTIONS,
        .reserved = 0,
        .key_len = sizeof(uint16_t), /* SFID */
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id()
    };

    proxy_sf_address_lkp_table = rte_hash_create(&hash_params);

    if(proxy_sf_address_lkp_table == NULL)
        return -1;

    return 0;
}

static int proxy_init_sf_id_lkp_table(void){
    const struct rte_hash_parameters hash_params = {
        .name = "proxy_next_func",
        .entries = PROXY_MAX_FUNCTIONS,
        .reserved = 0,
        .key_len = sizeof(uint32_t),  /* <SPI,SI> */
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id()
    };

    proxy_sf_id_lkp_table = rte_hash_create(&hash_params);

    if(proxy_sf_id_lkp_table == NULL)
        return -1;

    return 0;
}

int proxy_setup(void){

    int ret = 0;

    ret = proxy_init_flow_table();
    SFCAPP_CHECK_FAIL_LT(ret,0,
        "Proxy: Failed to create flow lookup table.\n");
    ret = proxy_init_sf_addr_table();
    SFCAPP_CHECK_FAIL_LT(ret,0,
        "Proxy: Failed to create SF address lookup table.\n");
    ret = proxy_init_sf_id_lkp_table();
    SFCAPP_CHECK_FAIL_LT(ret,0,
        "Proxy: Failed to create SF id lookup table.\n");
    
    sfcapp_cfg.main_loop = proxy_main_loop;

    return 0;
}

/* This function does all the processing on packets coming from 
 * the SFC network to the Legacy SFs. That includes: 
 * 
 * - Checking if flow info is already on table
 * - Decapsulating the packet
 * - Adding the corresponding SF MAC address
 * 
 * It handles packets in bulks. This can be further optimized by
 * using other DPDK bulk operations.
 */ 
static void proxy_handle_inbound_pkts(struct rte_mbuf **mbufs, uint16_t nb_pkts, 
    uint64_t *drop_mask){

    struct nsh_hdr nsh_header;
    struct ipv4_5tuple tuple = { .proto = 0, .src_ip = 0, .dst_ip = 0, .src_port = 0, .dst_port = 0};
    //struct ipv4_5tuple ip_5tuples[BURST_SIZE];
    //struct ipv4_5tuple *tuple_ptrs[BURST_SIZE];
    //int32_t vals[BURST_SIZE];
    uint16_t sfid;
    uint64_t data;
    struct ether_addr sf_mac;
    int i, lkp;
    uint16_t offset;
    uint64_t sf_mac_64;
    uint64_t nsh_header_64;
    *drop_mask = 0;

    /* Check if flow info is already stored in table */
    /*rte_hash_lookup_bulk(proxy_flow_lkp_table,(void **) tuple_ptrs,
            nb_pkts,vals);
    */
    

    for(i = 0; i < nb_pkts ; i++){
        
        //common_dump_pkt(mbufs[i],"\n=== Input packet ===\n");

        nsh_get_header(mbufs[i],&nsh_header);


        offset = sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) + 
            sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr) +
            sizeof(struct nsh_hdr);

        common_ipv4_get_5tuple(mbufs[i],&tuple,offset);        

        /* Check if flow info is already stored in table */
        lkp = rte_hash_lookup(proxy_flow_lkp_table,&tuple);

        /* Flow not mapped yet in table. Let's add it */
        if(unlikely(lkp < 0)){
            /* Decrement SI before storing header to save 
             * processing time later. 
             */
            if( (nsh_header.serv_path & 0x000000FF) != 0 ){
                nsh_header.serv_path--;
            }else{ /* Drop packet */
                *drop_mask |= 1<<i;
                continue;
            }

            nsh_header_64 = nsh_header_to_uint64(&nsh_header);
            lkp = rte_hash_add_key_data(proxy_flow_lkp_table,
                &tuple, (void *) nsh_header_64);
            
            // Undo decrementing after saving to table
            nsh_header.serv_path++;
        }
        
        /* Decapsulate packet */
        nsh_decap(mbufs[i]);
        
        //printf("Looking for SF for path %" PRIx32 "\n",nsh_header.serv_path);

        /* Get SF MAC address from table */
        lkp = rte_hash_lookup_data(proxy_sf_id_lkp_table, 
                (void *) &nsh_header.serv_path,
                (void **) &data);

        sfid = (uint16_t) data;

        /*if(lkp >= 0){
            printf("Found the SF ID!!! Data: %" PRIx16 "\n",sfid);
        }else{
            printf("Didn't find %" PRIx32 " the corresponding sfid\n",nsh_header.serv_path);
        }*/

        //printf("Trying to get MAC for SF #%" PRIx16 "\n",sfid);

        lkp = rte_hash_lookup_data(proxy_sf_address_lkp_table,
                (void *) &sfid,
                (void **) &sf_mac_64);

        // Currently not droping packets
        if( unlikely(lkp < 0) ){
            //printf("Didnt find MAC for SF #%" PRIx16 "\n",sfid);
            *drop_mask |= 1<<i;
            continue;
        }/*else{
            printf("Found the MAC!!!!\n");
        }*/

        // Convert hash data back to MAC
        common_64_to_mac(sf_mac_64,&sf_mac);

        common_mac_update(mbufs[i],&sfcapp_cfg.port2_mac,&sf_mac);
        
        //common_dump_pkt(mbufs[i],"\n=== Decapsulated packet ===\n");
    }
}

static void proxy_handle_outbound_pkts(struct rte_mbuf **mbufs, uint16_t nb_pkts, 
    uint64_t *drop_mask){
    struct nsh_hdr nsh_header;
    uint64_t nsh_header_64;
    struct ipv4_5tuple tuple;
    uint16_t offset;
    int i,lkp;

    for(i = 0 ; i < nb_pkts ; i++){

        offset = sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) + 
            sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr);

        common_ipv4_get_5tuple(mbufs[i],&tuple,offset);

        /* Get packet header from hash table */
        lkp = rte_hash_lookup_data(proxy_flow_lkp_table,
                (void*) &tuple,(void**) &nsh_header_64);
        CHECK_LKP_FAIL(drop_mask,lkp);
        
        nsh_uint64_to_header(nsh_header_64,&nsh_header);
        
        /* Encapsulate packet */
        nsh_encap(mbufs[i],&nsh_header);

        /* Add SFF's MAC address */
        common_mac_update(mbufs[i],&sfcapp_cfg.port1_mac,&sfcapp_cfg.sff_addr);

        //common_dump_pkt(mbufs[i],"\n=== Encapsulated packet ===\n");
    }
}

void proxy_main_loop(void){

    struct rte_mbuf *rx_pkts[BURST_SIZE];
    uint16_t nb_rx;//, nb_tx;
    uint64_t drop_mask;
    
    for(;;){
    
        /* Receive packets from network */
        nb_rx = rte_eth_rx_burst(sfcapp_cfg.port1,0,
                     rx_pkts,BURST_SIZE);

	    /* Process and decap received packets*/
        proxy_handle_inbound_pkts(rx_pkts,nb_rx,&drop_mask);

        /* TODO: Only send packets not marked for dropping */
        send_pkts(rx_pkts,sfcapp_cfg.port2,0,nb_rx);

        drop_mask = 0;

        /* Receive packets from SFs */
        nb_rx = rte_eth_rx_burst(sfcapp_cfg.port2,0,
            rx_pkts,BURST_SIZE);
        
        /* Process and encap packets from SFs */
        proxy_handle_outbound_pkts(rx_pkts,nb_rx,&drop_mask);

        /* TODO: Only send packets not marked for dropping  */
        send_pkts(rx_pkts,sfcapp_cfg.port1,0,nb_rx);  
    }
}


