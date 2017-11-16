#include <stdio.h>
#include <stdlib.h>

#include <rte_ether.h>

#include "parser.h"
#include "common.h"
#include "sfc_classifier.h"
#include "sfc_proxy.h"
#include "sfc_forwarder.h"

extern struct sfcapp_config sfcapp_cfg;

static void parse_sf_section(struct rte_cfgfile_entry *entries, int nb_entries){
    int j,ret;
    int sfid_ok, mac_ok;
    struct ether_addr sfmac;
    uint16_t sfid;
    const char* SECTION_NAME = "SF";

    sfid_ok = 0;
    mac_ok = 0;
    sfid = 0;

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

        }else if(strcmp(entries[j].name,"mac") == 0){
            if(mac_ok)
                printf("Duplicated mac entry in SF section. Ignoring...\n");
            else{
                ret = common_parse_ether(entries[j].value,&sfmac);
                SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to parse mac address from config file\n");
                mac_ok = 1;
            }

        }else{
            rte_exit(EXIT_FAILURE,
                "Entry %s unknown in section %s, please check config file.\n",
                entries[j].name,SECTION_NAME); 
        } 
    }

    if(mac_ok && sfid_ok){
        switch(sfcapp_cfg.type){
            case SFC_FORWARDER:
                forwarder_add_sf_address_entry(sfid,&sfmac);
                break;
            case SFC_PROXY:
                proxy_add_sf_address_entry(sfid,&sfmac);
                break;
            default:
                rte_exit(EXIT_FAILURE,
                "Config file parsing failed. \"SF\" sections do not" 
                "apply to this type of application.\n");
        }
    }
}

static void parse_sfc_node_section(struct rte_cfgfile_entry *entries, int nb_entries){
    int j,ret;
    int sfid_ok,sph_ok;
    uint16_t sfid;
    uint32_t sph;
    const char* SECTION_NAME = "SFC_NODE";

    if(nb_entries != 2)
        rte_exit(EXIT_FAILURE,
            "Wrong argument number in \"SFC_NODE\" section in config file."
            "Expected 2, found %d",
            nb_entries);

    sph = sfid = 0;
    sfid_ok = sph_ok = 0;

    for(j = 0 ; j < nb_entries ; j++){
        
        if(strcmp(entries[j].name,"sfid") == 0){
            if(sfid_ok)
                printf("Duplicated sfid entry in PATH_NODE section. Ignoring...\n");
            else{
                ret = common_parse_uint16(entries[j].value,&sfid);
                SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to parse SF ID from config file\n");
                sfid_ok = 1;
            }
        }else if(strcmp(entries[j].name,"sph") == 0){
            if(sph_ok)
                printf("Duplicated mac entry in PATH_NODE section. Ignoring...\n");
            else{
                ret = common_parse_uint32(entries[j].value,&sph);
                SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to parse service path info from config file\n");
                sph_ok = 1;
            }
        }else{
            rte_exit(EXIT_FAILURE,
                "Entry %s unknown in section %s, please check config file.\n",
                entries[j].name,SECTION_NAME); 
        }         
    }

    if(sph_ok && sfid_ok){
        switch(sfcapp_cfg.type){
            case SFC_FORWARDER:
                forwarder_add_sph_entry(sph,sfid);
                break;
            case SFC_PROXY:
                proxy_add_sph_entry(sph,sfid);
                break;
            default:
                rte_exit(EXIT_FAILURE,
                "Config file parsing failed. \"SFC_NODE\" sections do not" 
                "apply to this type of application.\n");
        }
    }
}

static void parse_flow_class_section(struct rte_cfgfile_entry *entries, int nb_entries){
    struct ipv4_5tuple tuple;
    int ipsrc_ok,ipdst_ok;
    int dport_ok,sport_ok;
    int proto_ok,sfp_ok;
    int dup,ret,j;
    uint32_t ipsrc,ipdst;
    uint16_t sport,dport;
    uint8_t proto;
    uint32_t sfp;
    const char* SECTION_NAME = "FLOW_CLASS";

    if(nb_entries != 6)
        rte_exit(EXIT_FAILURE,
            "Wrong argument number in \"%s\" section in config file."
            "Expected 5, found %d",
            SECTION_NAME,
            nb_entries);
    dup = 0;
    ret = 0;
    ipsrc_ok = ipdst_ok = 0;
    dport_ok = sport_ok = 0;
    proto_ok = sfp_ok = 0;

    for(j = 0 ; j < nb_entries ; j++){
        
        if(strcmp(entries[j].name,"ipsrc") == 0){
            if(ipsrc_ok)
                dup = -1;
            else{
                ret = common_parse_uint32(entries[j].value,&ipsrc);
                ipsrc_ok = 1;
            }
        }else if(strcmp(entries[j].name,"ipdst") == 0){
            if(ipdst_ok)
                dup = -1;
            else{
                ret = common_parse_uint32(entries[j].value,&ipdst);
                ipdst_ok = 1;
            }
        }else if(strcmp(entries[j].name,"sport") == 0){
            if(sport_ok)
                dup = -1;
            else{
                ret = common_parse_uint16(entries[j].value,&sport);
                sport_ok = 1;
            }
        }else if(strcmp(entries[j].name,"dport") == 0){
            if(dport_ok)
                dup = -1;
            else{
                ret = common_parse_uint16(entries[j].value,&dport);
                dport_ok = 1;
            }
        }else if(strcmp(entries[j].name,"proto") == 0){
            if(proto_ok)
                dup = -1;
            else{
                ret = common_parse_uint8(entries[j].value,&proto);
                proto_ok = 1;
            }
        }else if(strcmp(entries[j].name,"sfp") == 0){
            if(sfp_ok)
                dup = -1;
            else{
                ret = common_parse_uint32(entries[j].value,&sfp);
                sfp_ok = 1;
            }
        }else{
            rte_exit(EXIT_FAILURE,
                "Entry %s unknown in section %s, please check config file.\n",
                entries[j].name,SECTION_NAME); 
        }

        if(ret < 0) rte_exit(EXIT_FAILURE,"Failed to parse params in %s section from config file\n",
            SECTION_NAME);    
        if(dup < 0) rte_exit(EXIT_FAILURE,"Found duplicate entries in %s section from config file.\n",
            SECTION_NAME); 
    }

    if(ipsrc_ok && ipdst_ok && sport_ok && dport_ok && proto_ok && sfp_ok){
        if(sfcapp_cfg.type == SFC_CLASSIFIER){
            if(sfp <= 0xFFFFFF)
                classifier_add_flow_class_entry(&tuple,sfp);
            else
                rte_exit(EXIT_FAILURE,
                    "SFP id too big. Maximum is 0xFFFFFF");
        }
        else rte_exit(EXIT_FAILURE,
                "Config file parsing failed. \"SFC_NODE\" sections do not" 
                "apply to this type of application.\n");
    }else
        rte_exit(EXIT_FAILURE,"Missing parameters in \"%s\" section from config file\n",SECTION_NAME);

}

void parse_config_file(char* cfg_filename){

    int nb_entries;
    int i;
    struct rte_cfgfile_entry entries[PROXY_CFG_MAX_ENTRIES];
    struct rte_cfgfile *cfgfile;
    char** sections;
    int nb_sections;
    struct rte_cfgfile_parameters cfg_params = {.comment_character = '#'};

    cfgfile = rte_cfgfile_load_with_params(cfg_filename,0,&cfg_params);
    
    if(cfgfile == NULL)
        rte_exit(EXIT_FAILURE,
            "Failed to load proxy config file\n");
    
    nb_sections = rte_cfgfile_num_sections(cfgfile,NULL,0);

    if(nb_sections <= 0)
        rte_exit(EXIT_FAILURE,
            "Not enough sections in config file\n");

    sections = (char**) malloc(nb_sections*sizeof(char*));

    if(sections == NULL)
        rte_exit(EXIT_FAILURE,
            "Failed to allocate memory when parsing proxy config file.\n");
    
    for(i = 0 ; i < nb_sections ; i++){
        sections[i] = (char*) malloc(CFG_NAME_LEN*sizeof(char));    
     
        if(sections[i] == NULL)
            rte_exit(EXIT_FAILURE,
                "Failed to allocate memory when parsing proxy config file.\n");
    }

    rte_cfgfile_sections(cfgfile,sections,CFG_FILE_MAX_SECTIONS);
    
    /* Parse sections */
    for(i = 0 ; i < nb_sections ; i++){

        nb_entries = rte_cfgfile_section_entries_by_index(cfgfile,i,sections[i],
                        entries,PROXY_CFG_MAX_ENTRIES);

        /* Parse SF Sections */
        if(strcmp(sections[i],"SF") == 0)
            parse_sf_section(entries,nb_entries);
        else if(strcmp(sections[i],"SFC_NODE") == 0)
            parse_sfc_node_section(entries,nb_entries);
        else if(strcmp(sections[i],"FLOW_CLASS") == 0)
            parse_flow_class_section(entries,nb_entries);
        else
            rte_exit(EXIT_FAILURE,
                "Section %s unknown, please check config file.\n",
                sections[i]);  
    }

    free(sections);
    rte_cfgfile_close(cfgfile);
}