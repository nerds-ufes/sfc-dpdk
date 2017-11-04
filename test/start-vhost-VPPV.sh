# Kill all ovs processes currently running
echo "Killing ovs processes currently running..."
pkill -9 ovs
echo "Done."
###################################

# Delete and create again directories necessary for ovs
echo "Removing and creating directories..."
rm -rf /usr/local/etc/openvswitch
rm -rf /usr/local/var/run/openvswitch
mkdir -p /usr/local/etc/openvswitch
mkdir -p /usr/local/var/run/openvswitch
rm /usr/local/etc/openvswitch/conf.db
echo "Done."
###################################

# Create ovs database
echo "Creating ovs database..."
$OVS_DIR/ovsdb/ovsdb-tool create /usr/local/etc/openvswitch/conf.db \
/usr/src/openvswitch-2.4.0/vswitchd/vswitch.ovsschema
sleep 1
echo "Done."
###################################

# Start ovsdb-server
echo "Starting ovsdb server..."
$OVS_DIR/ovsdb/ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock \
          --remote=db:Open_vSwitch,Open_vSwitch,manager_options \
          --pidfile --detach &

$OVS_DIR/utilities/ovs-vsctl --no-wait init
sleep 1
echo "Done."
###################################

# Turn on daemon
echo "Start vswitch daemon..."
$OVS_DIR/vswitchd/ovs-vswitchd --dpdk -c 0x2 -n 4 --socket-mem 2048,0 \
	 -- unix:/usr/local/var/run/openvswitch/db.sock --pidfile --detach
###################################


# Add bridge
echo "Configuring bridge and ports..."
$OVS_DIR/utilities/ovs-vsctl --no-wait add-br br0 -- set bridge br0 datapath_type=netdev
sleep 1
echo "Finished configuring bridge."
echo "Configuring ports now..."
###################################


# Add ports
$OVS_DIR/utilities/ovs-vsctl --no-wait add-port br0 dpdk0 -- set Interface dpdk0 type=dpdk
#echo "Finished configuring port0."
$OVS_DIR/utilities/ovs-vsctl --no-wait add-port br0 vhost-user1 -- set Interface vhost-user1 type=dpdkvhostuser

echo "Finished configuring vhostuser port 0"

## At this point, the vhost-user socket will be located at /usr/local/var/run/opeenvswitch/<vhost port name>
## This is important to set up the VM on later step

sleep 1
echo "Done."
###################################

# Show configured bridge and ports.
$OVS_DIR/utilities/ovs-vsctl show
###################################

# Delete current flows and add new ones
echo "Deleting current flows..."
$OVS_DIR/utilities/ovs-ofctl del-flows br0
echo "Done."

echo "Adding new flows..."
$OVS_DIR/utilities/ovs-ofctl add-flow br0 in_port=1,idle_timeout=0,action=output:2
$OVS_DIR/utilities/ovs-ofctl add-flow br0 in_port=2,idle_timeout=0,action=output:1
echo "Done."
###################################

echo "OvS Configuration Finished."
