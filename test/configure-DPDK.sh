###### Check if DPDK vars are set ######

: ${RTE_SDK:?"Please set 'RTE_SDK' before running this script."}
: ${RTE_TARGET:?"{Please set 'RTE_TARGET' before running this script."}

###### Setup hugepages ######

huge_dir=/mnt/huge

# Create hugepage dir if it doesn't exist
if [ ! -d $huge_dir ]; then
    mkdir $huge_dir
fi

# Mount 1GB hugepages
#echo 8 > /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
mount -t hugetlbfs -o pagesize=1G none /mnt/huge

###### Insert DPDK driver module ######

if ! lsmod | grep uio &> /dev/null ; then
    modprobe uio
fi

if ! lsmod | grep igb_uio &> /dev/null ; then
    insmod $RTE_SDK/x86_64-native-linuxapp-gcc/kmod/igb_uio.ko
fi

###### Bind interfaces to DPDK ######
$RTE_SDK/tools/dpdk_nic_bind.py -b igb_uio 0000:02:00.0
$RTE_SDK/tools/dpdk_nic_bind.py -b igb_uio 0000:02:00.1
