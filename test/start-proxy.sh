# This script starts the VM using qemu, OvS and DPDK
if [ -z $VM_DIR ]; then
	echo "Variable VM_DIR not set."
	exit 1
fi

rm -d /tmp/qemu_share2
mkdir /tmp/qemu_share2

echo "Starting Generator..."
qemu-system-x86_64 \
 -cpu host \
 -boot c \
 -drive file=$VM_DIR/SFC-Proxy.img,media=disk,format=raw \
 -m 2048M \
 -smp cores=4 \
 --enable-kvm -name Proxy \
 -vnc :2 -pidfile /tmp/vm_2.pid \
 -drive file=fat:rw:/tmp/qemu_share2,snapshot=off \
 -monitor unix:/tmp/vm_2monitor,server,nowait \
 -chardev socket,id=char2,path=/usr/local/var/run/openvswitch/vhost-user2 \
 -netdev type=vhost-user,id=mynet2,chardev=char2,vhostforce \
 -device virtio-net-pci,mac=00:00:00:00:00:02,netdev=mynet2,id=net2 \
 -object memory-backend-file,id=mem,size=2048M,mem-path=/mnt/huge,share=on \
 -numa node,memdev=mem -mem-prealloc &

if [ $? -eq 0 ]; then
    echo "VM up and running."
else
    echo "Error. Not able to start VM."
fi
