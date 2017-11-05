: ${OVS_DIR:?"Please set 'OVS_DIR' before running this script."}
: ${RTE_SDK:?"Please set 'RTE_SDK' before running this script."}
: ${RTE_TARGET:?"Please set 'RTE_TARGET' before running this script."}

cd $OVS_DIR
./configure --with-dpdk=$RTE_SDK/$RTE_TARGET
make
make install
