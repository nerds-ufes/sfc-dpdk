if [ -z $1 ] || [ -z $2 ] || [ -z $3 ] || [ -z $4 ] || [ -z $5 ] || [ -z $6 ] || [ -z $7 ]; then
	echo "Usage: $0 name id img_path netid_low netid_high cpu_mask"
	exit 1
fi

name=$1
id=$2
path=$3
low_idx=$4
high_idx=$5
cpu_mask=$6
mem_mb="$7M"

if [ $low_idx -gt $high_idx ]; then
    echo "netid_low should be less than or equal to netid_high"
    exit 1
fi

share_dir=/dev/qemu_share$id
if [ ! -d "$share_dir" ]; then
	mkdir $share_dir
fi

# Define CPU mask based on vm ID.
#cpu_mask=$(printf "0x%x" $((1 << $(($id % 6)))) )

echo "Starting $name..."
qemu_cmd_base="qemu-system-x86_64 -cpu host -boot c -drive file=$path,media=disk,format=raw -m $mem_mb -smp cores=2 --enable-kvm -name $name -vnc :$id -pidfile /tmp/vm_$id.pid  -drive file=fat:rw:$share_dir,snapshot=off -monitor unix:/tmp/vm_$idmonitor,server,nowait"

#echo "low: $low_idx high: $high_idx"

for i in $( seq $low_idx 1 $high_idx );
do
	mac=$(printf "00:00:00:00:00:%02x" $i)
	#echo $mac
	cmd_dev="$cmd_dev -chardev socket,id=char$i,path=/usr/local/var/run/openvswitch/vhost-user$i -netdev type=vhost-user,id=mynet$i,chardev=char$i,vhostforce -device virtio-net-pci,mac=$mac,netdev=mynet$i,id=net$i"
done

cmd_end="-object memory-backend-file,id=mem,size=$mem_mb,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc &"

#cmd="taskset $cpu_mask $qemu_cmd_base $cmd_dev $cmd_end"
cmd="$qemu_cmd_base $cmd_dev $cmd_end"
eval $cmd
