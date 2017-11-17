if [ -z $1 ] || [ -z $2 ] || [ -z $3 ] || [ -z $4 ] || [ -z $5 ]; then
	echo "Usage: $0 name id img_path netid_low netid_high"
	exit 1
fi

name=$1
id=$2
path=$3
low_idx=$4
high_idx=$5

if [ $low_idx -gt $high_idx ]; then
    echo "netid_low should be less than or equal to netid_high"
    exit 1
fi

rm -d /tmp/qemu_share$id
mkdir /tmp/qemu_share$id

echo "Starting $name..."
cmd_base="qemu-system-x86_64 -cpu host -boot c -drive file=$path,media=disk,format=raw -m 1024M -smp cores=2 --enable-kvm -name $name -vnc :$id -pidfile /tmp/vm_$id.pid  -drive file=fat:rw:/tmp/qemu_share$id,snapshot=off -monitor unix:/tmp/vm_$idmonitor,server,nowait"

echo "low: $low_idx high: $high_idx"

for i in $( seq $low_idx 1 $high_idx );
do
	mac=$(printf "00:00:00:00:00:%02x" $i)
	echo $mac
	cmd_dev="$cmd_dev -chardev socket,id=char$i,path=/usr/local/var/run/openvswitch/vhost-user$i -netdev type=vhost-user,id=mynet$i,chardev=char$i,vhostforce -device virtio-net-pci,mac=$mac,netdev=mynet$i,id=net$i"
done

cmd_end="-object memory-backend-file,id=mem,size=1024M,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc &"

cmd="$cmd_base $cmd_dev $cmd_end"

eval $cmd
