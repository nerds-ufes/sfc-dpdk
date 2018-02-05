if [ -z $OVS_DIR ]; then
  echo "Please set OVS_DIR environment variable first"
  exit 1
fi

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
#ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-lcore-mask=0x42
ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-socket-mem=8192
ovs-vsctl --no-wait set Open_vSwitch . other_config:pmd-cpu-mask=0x186
ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-hugepage-dir=/mnt/huge
ovs-vsctl --no-wait set Open_vSwitch . other_config:dpdk-init=true
ovs-vsctl --no-wait init

#Turn on daemon
ovs-vswitchd \
     --pidfile \
     --detach \
     --log-file=/usr/local/var/log/openvswitch/ovs-vswitchd.log &

###################################