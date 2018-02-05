#!/bin/sh
# Autor: Gilmar Luiz Vassoler, Cristina Dominicini e Leonardo Ferreira
#
#
#
IPERFTIME=70
SLEEPTIME=70
DELAY=0
MAXTIME=80
SAMPLES=3 # Número de vezes que o teste será realizado.

iperfrx () {
	FILENAME="$1"
	interface="$2"
	
	# -C, --csvchar char specifies the delimiter char for CSV mode. The default is ';'.
	#  -T, --type value. Use one  of  rate  for  the  current rate/s.
	# -u, --unit value selects  which unit to show. It can be one of bytes, bits, packets or errors.
	# -t, --timeout msec displays and gathers stats every n msec (1msec = 1/1000sec). The default is 500msec.

	bwm-ng -t 1000 -o csv -u bytes -T rate -C ',' > tmp.bwm &	
	iperf3 -s -p 50000  &  ### UDP

	sleep $SLEEPTIME	
	pkill iperf3
	pkill bwm-ng

	mkdir -p $j
	grep $interface tmp.bwm > $j/$FILENAME.bwm
	rm tmp.bwm
	#rm numbers
	#rm ${FILENAME}-jitter.log
	#rm tmp_$FILENAME.log
}

iperftx () {
	FILENAME="$1"
	IP="$2"
	bandwitch=$3
	interface="$4"

	bwm-ng -t 1000 -o csv -u bytes -T rate -C ',' > tmp.bwm &
	sleep $(($DELAY))
	iperf3 -c $IP -t $IPERFTIME -N -u -b 500M -p 50000 --cport 50000 -B 10.1.0.2 -l 1400 &  ### UDP
	#iperf3 -c $IP -t $IPERFTIME -N -u -b ${bandwitch}m -p 5000 1>/dev/null &  ### UDP
	sleep $(($SLEEPTIME - $DELAY)) 2> /dev/null

	pkill iperf3
	pkill bwm-ng
	mkdir -p $j
	grep $interface tmp.bwm > $j/$FILENAME.bwm
	rm tmp.bwm
}

iperffw1 () {
	FILENAME="$1"
	interface="$2"

	bwm-ng -t 1000 -o csv -u bytes -T rate -C ',' > tmp.bwm &
	sleep $(($DELAY))	
	sleep 10
	kill `ps -aux | grep -i "forward" | head -n1 | cut -d ' ' -f 6`
	sleep 10
	python forward.py &
	sleep 25
	sleep 15
	pkill bwm-ng
	mkdir -p $j
	#echo $interface
	grep $interface tmp.bwm > $j/$FILENAME.bwm
	#grep eth1 tmp.bwm > $FILENAME.bwm
	echo "Exiting iperffw"
}

iperffw2 () {
	FILENAME="$1"
	interface="$2"

	bwm-ng -t 1000 -o csv -u bytes -T rate -C ',' > tmp.bwm &
	sleep $(($DELAY))
	sleep 25
	sleep 10
	kill `ps -aux | grep -i "forward" | head -n1 | cut -d ' ' -f 7`
	sleep 10
	python forward.py &
	sleep 15
	pkill bwm-ng
	mkdir -p $j
	#echo $interface
	grep $interface tmp.bwm > $j/$FILENAME.bwm
	#grep eth1 tmp.bwm > $FILENAME.bwm
	echo "Exiting iperffw"
}

# Clean old processes
pkill iperf3
pkill forward
pkill bwm-ng
#kill `ps -aux | grep -i "forward" | head -n1 | cut -d ' ' -f 7`

# $1: Type: "rx" or "tx" or "fw"
# $2: Filename
# $3: Delay
# $4: server IP (when rx)
TYPE="$1"
DELAY="$3"
if [ $# -eq 5 ] && [ "tx" = "$TYPE" ]; then
	for j in `seq 1 1 1`
	do
		echo "Considerando uma banda de ${j}Mbits/s"
		for i in `seq 1 $SAMPLES`
		do
			start_time=`date +%s`
			echo "Sample # $i: Started in: $start_time"
			iperftx "a${i}-vm-$2-tx-$j" "$4" $j "$5"
			end_time=`date +%s`
			echo "Sample # $i: Finished in: $end_time"
			sleep $(($MAXTIME - (end_time - start_time))) 2> /dev/null
		done
	done
elif [ $# -eq 4 ] && [ "rx" = "$TYPE" ]; then
	for j in `seq 1 1 1`
	do
		echo "Considerando uma banda de ${j}Mbits/s"
		for i in `seq 1 $SAMPLES`
		do
			start_time=`date +%s`
			echo "Sample # $i: Started in: $start_time"
			iperfrx "a${i}-vm-$2-rx-$j" "$4"
			end_time=`date +%s`
			echo "Sample # $i: Finished in: $end_time"
			sleep $(($MAXTIME - (end_time - start_time))) 2> /dev/null
		done
	done
elif [ $# -eq 4 ] && [ "fw1" = "$TYPE" ]; then
	for j in `seq 1 1 1`
	do
		python forward.py &
		for i in `seq 1 $SAMPLES`
		do
			start_time=`date +%s`
			echo "Sample # $i: Started in: $start_time"
			iperffw1 "a${i}-vm-$2-fw1-$j" "$4"
			end_time=`date +%s`
			echo "Sample # $i: Finished in: $end_time"
			sleep $(($MAXTIME - (end_time - start_time))) 2> /dev/null
		done
		#kill `ps -aux | grep -i "forward" | head -n1 | cut -d ' ' -f 7`
	done
elif [ $# -eq 4 ] && [ "fw2" = "$TYPE" ]; then
	for j in `seq 1 1 1`
	do
		python forward.py &
		for i in `seq 1 $SAMPLES`
		do
			start_time=`date +%s`
			echo "Sample # $i: Started in: $start_time"
			iperffw2 "a${i}-vm-$2-fw2-$j" "$4"
			end_time=`date +%s`
			echo "Sample # $i: Finished in: $end_time"
			sleep $(($MAXTIME - (end_time - start_time))) 2> /dev/null
		done
		#kill `ps -aux | grep -i "forward" | head -n1 | cut -d ' ' -f 7`
	done
else
	echo "$0 tx <host_name> <delay> <ip remote host> <interface>"
	#./iperf-test.sh tx source 10 10.0.101.54 eth0
	echo "or "
	echo "$0 rx <host_name> <delay> <interface>"
	#./iperf-test.sh rx dest 10 eth0
	echo "or "
	echo "$0 fw[1-2] <host_name> <delay> <interface>"
	#./iperf-test.sh fw1 sff1 10
	#./iperf-test.sh fw2 sff2 10
fi

