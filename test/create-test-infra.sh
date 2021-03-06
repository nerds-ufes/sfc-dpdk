ovs_get_iface_number () {
    ovs-vsctl get interface $1 ofport
}

ovs_add_vhost_port () {
    # $1 = bridge ; $2 = portname
    ovs-vsctl --may-exist add-port $1 $2 -- set Interface $2 type=dpdkvhostuser
}

ovs_del_port () {
    # $1 = bridge ; $2 = portname
    ovs-vsctl --if-exists del-port $1 $2
}

# Create bridges
ovs-vsctl --no-wait --may-exist add-br brmgmt -- set bridge brmgmt datapath_type=netdev
ovs-vsctl --no-wait --may-exist add-br brpxy -- set bridge brpxy datapath_type=netdev
ovs-vsctl --no-wait --may-exist add-br brsfc -- set bridge brsfc datapath_type=netdev

# Create ifaces

for i in $(seq 1 3 16); do
    ovs_add_vhost_port brmgmt vhost-user$i
done

ovs-vsctl --may-exist add-port brmgmt vmnet0 -- set interface vmnet0 type=internal
ifconfig vmnet0 10.0.0.254/24

for i in {9,12,14,15,17,18}; do
    ovs_add_vhost_port brpxy vhost-user$i
done

for i in {2,3,5,6,8,11}; do
    ovs_add_vhost_port brsfc vhost-user$i
done

ovs-vsctl --may-exist add-port brsfc enp10s0f0 -- set Interface enp10s0f0 type=dpdk options:dpdk-devargs=0000:0a:00.0
ovs-vsctl --may-exist add-port brsfc enp10s0f2 -- set Interface enp10s0f2 type=dpdk options:dpdk-devargs=0000:0a:00.2

# Add flows

# brnet
ovs-ofctl add-flow brmgmt actions=NORMAL

# brpxy
ovs-ofctl add-flow brpxy in_port=$(ovs_get_iface_number vhost-user9),actions=output:$(ovs_get_iface_number vhost-user14)
ovs-ofctl add-flow brpxy in_port=$(ovs_get_iface_number vhost-user15),actions=mod_dl_dst:00:00:00:00:00:09,output:$(ovs_get_iface_number vhost-user9)

ovs-ofctl add-flow brpxy in_port=$(ovs_get_iface_number vhost-user12),actions=output:$(ovs_get_iface_number vhost-user17)
ovs-ofctl add-flow brpxy in_port=$(ovs_get_iface_number vhost-user18),actions=mod_dl_dst:00:00:00:00:00:0C,output:$(ovs_get_iface_number vhost-user12)

# brsfc
# ingress flows : send ingress packets to classifier
ovs-ofctl add-flow brsfc priority=3,in_port=$(ovs_get_iface_number enp10s0f0),actions=output:$(ovs_get_iface_number vhost-user2)
ovs-ofctl add-flow brsfc priority=3,in_port=$(ovs_get_iface_number enp10s0f2),actions=output:$(ovs_get_iface_number vhost-user2)

# egress flows
ovs-ofctl add-flow brsfc priority=2,dl_dst=A0:36:9F:3D:FC:A0,actions=output:$(ovs_get_iface_number enp10s0f0)
ovs-ofctl add-flow brsfc priority=2,dl_dst=A0:36:9F:3D:FC:A2,actions=output:$(ovs_get_iface_number enp10s0f2)

for i in {2,3,5,6,8,11}; do
    mac=$(printf "00:00:00:00:00:%02x" $i)
    ovs-ofctl add-flow brsfc priority=1,dl_dst=$mac,actions=output:$(ovs_get_iface_number vhost-user$i)
done

# Start VMS
./start-guest.sh Classifier 1 /home/user/SFC-Classifier.img 1 3 0x8 4096
./start-guest.sh Forwarder 2 /home/user/SFC-Forwarder.img 4 6 0x104 8192
./start-guest.sh Proxy1 3 /home/user/SFC-Proxy.img 7 9 0x610 4096
./start-guest.sh Proxy2 4 /home/user/SFC-SF3.img 10 12 0x610 4096
./start-guest.sh SF1 5 /home/user/SFC-SF1.img 13 15 0x610 4096
./start-guest.sh SF2 6 /home/user/SFC-SF2.img 16 18 0x610 4096


# Flows to bypass the SFC arch and create a loopback between the physical interfaces
# ovs-ofctl add-flow brsfc priority=2,in_port=$(ovs_get_iface_number enp10s0f2),actions=output:$(ovs_get_iface_number enp10s0f0)
# ovs-ofctl add-flow brsfc priority=2,in_port=$(ovs_get_iface_number enp10s0f0),actions=output:$(ovs_get_iface_number enp10s0f2)
