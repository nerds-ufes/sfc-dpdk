# Kill all ovs processes currently running
echo "Killing ovs processes currently running..."
killall ovsdb-server ovs-vswitchd
echo "Done."
###################################

# Delete and create again directories necessary for ovs
echo "Removing and recreating directories..."
rm -rf /usr/local/etc/openvswitch
rm -rf /usr/local/var/run/openvswitch/vhost-user*
rm -f /usr/local/etc/openvswitch/conf.db

mkdir -p /usr/local/etc/openvswitch
mkdir -p /usr/local/var/run/openvswitch
echo "Done."

###################################

# Create ovs database
echo "Creating ovs database..."
ovsdb-tool create /usr/local/etc/openvswitch/conf.db \
	$OVS_DIR/vswitchd/vswitch.ovsschema
sleep 1
echo "Done."

###################################

# Start ovsdb-server
echo "Starting ovsdb server..."
DB_SOCK=/usr/local/var/run/openvswitch/db.sock
ovsdb-server \
			--remote=punix:$DB_SOCK \
      --remote=db:Open_vSwitch,Open_vSwitch,manager_options \
      --pidfile --detach &
sleep 1
echo "Done."

###################################

echo "Starting OVS..."
ovs-vsctl --no-wait init
ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-lcore-mask=0x3
ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-socket-mem=2048,0
ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-init=true

#Turn on daemon
ovs-vswitchd \
     --pidfile \
     --detach \
     --log-file=/usr/local/var/log/openvswitch/ovs-vswitchd.log &

###################################


# Configure bridge

echo "Configuring bridge and ports..."
ovs-vsctl --no-wait add-br br0 -- set bridge br0 datapath_type=netdev
sleep 1

# Add ports

ovs-vsctl --no-wait add-port br0 vhost-user1 -- set Interface vhost-user1 type=dpdkvhostuser
echo "Finished configuring vhostuser port 1"
sleep 1

ovs-vsctl --no-wait add-port br0 vhost-user2 -- set Interface vhost-user2 type=dpdkvhostuser
sleep 1

ovs-vsctl --no-wait add-port br0 vhost-user3 -- set Interface vhost-user3 type=dpdkvhostuser
echo "Finished configuring vhostuser port 2"
sleep 1

# Create mirror port to analyze packets with Wireshark
:'ovs-vsctl add-port br0 tap1 \
    -- --id=@p get port tap1 \
    -- --id=@m create mirror name=m0 select-all=true output-port=@p \
    -- set bridge br0 mirrors=@m
'
## At this point, the vhost-user socket will be located at /usr/local/var/run/openvswitch/<vhost port name>
## This is important to set up the VM on next step
echo "Done."
###################################

# Show configured bridge and ports.

ovs-vsctl show

###################################

ovs-ofctl add-flow br0 action=NORMAL
echo "OvS Configuration Finished."
