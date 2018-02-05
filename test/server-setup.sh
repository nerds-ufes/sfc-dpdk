: ${OVS_DIR:?"Please set 'OVS_DIR' before running this script."}
: ${RTE_SDK:?"Please set 'RTE_SDK' before running this script."}
: ${RTE_TARGET:?"Please set 'RTE_TARGET' before running this script."}


apt-get update
apt-get install make gcc libnuma-dev linux-headers-$(uname -r) libpcap-dev

# Download and install dpdk
# dpdk_rel=dpdk-17.05.2
# wget fast.dpdk.org/rel/$dpdk_rel.tar.xz
# tar -xf $dpdk_rel.tar.xz 

## Setup DPDK kernel parameters

# Download and compile OVS
apt-get install libssl-dev python2.7 python-pip
pip install six netifaces
cd /home/mscastanho/
wget http://openvswitch.org/releases/openvswitch-2.8.0.tar.gz
cd $OVS_DIR
./configure --with-dpdk=$RTE_SDK/$RTE_TARGET
make
make install

apt-get install qemu qemu-kvm python-openvswitch