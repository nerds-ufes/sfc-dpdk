#include <stdio.h>
#include <stdlib.h>

#include <rte_eal.h>

int 
main(int argc, char **argv){

    int ret;

    ret = rte_eal_init(argc,argv);
    if(ret < 0)
        rte_exit(EXIT_FAILURE, "Invalid EAL arguments.\n");

    argc -= ret;
    argv += ret;

}