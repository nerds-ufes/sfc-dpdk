# This script starts the VM using qemu, OvS and DPDK
if [ -z $VM_DIR ]; then
	echo "Variable VM_DIR not set"
	exit 1
fi

rm -d /tmp/qemu_share1
mkdir /tmp/qemu_share1

echo "Starting Generator..."
qemu-system-x86_64 \
 -cpu host \
 -boot c \
 -drive file=$VM_DIR/SFC-Generator.img,media=disk,format=raw \
 -m 2048M \
 -smp cores=4 \
 --enable-kvm -name Generator \
 -vnc :1 -pidfile /tmp/vm_1.pid \
 -drive file=fat:rw:/tmp/qemu_share1,snapshot=off \
 -monitor unix:/tmp/vm_1monitor,server,nowait \
 -chardev socket,id=char1,path=/usr/local/var/run/openvswitch/vhost-user1 \
 -netdev type=vhost-user,id=mynet1,chardev=char1,vhostforce \
 -device virtio-net-pci,mac=00:00:00:00:00:01,netdev=mynet1,id=net1 \
 -chardev socket,id=char2,path=/usr/local/var/run/openvswitch/vhost-user2 \
 -netdev type=vhost-user,id=mynet2,chardev=char2,vhostforce \
 -device virtio-net-pci,mac=00:00:00:00:00:02,netdev=mynet2,id=net2 \
 -chardev socket,id=char3,path=/usr/local/var/run/openvswitch/vhost-user3 \
 -netdev type=vhost-user,id=mynet3,chardev=char3,vhostforce \
 -device virtio-net-pci,mac=00:00:00:00:00:03,netdev=mynet3,id=net3 \
 -object memory-backend-file,id=mem,size=2048M,mem-path=/mnt/huge,share=on \
 -numa node,memdev=mem -mem-prealloc &

if [ $? -eq 0 ]; then
    echo "VM up and running."
else
    echo "Error. Not able to start VM."
fi