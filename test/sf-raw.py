#!/usr/bin/env python

import sys,os
import socket
import binascii
import signal

def signal_handler(signal, frame):
        print "\n\n" + str(count) + " packets received"
        sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

rawSocket=socket.socket(socket.PF_PACKET,socket.SOCK_RAW,socket.htons(0x0800))
#rawSocket.bind(("eth1",socket.htons(0x0800)))

s = socket.socket(socket.PF_PACKET,socket.SOCK_RAW,socket.htons(0x0800))
s.bind(("eth2", socket.htons(0x0800)))
count=0

print "Sniffing..."
while True:
    receivedPacket=rawSocket.recv(4096)
    #if destinationIP != "10.0.0.5" and sourceIP != "10.0.0.5":
        #Envia o pacote recebido de volta para a interface escolhida
    count += 1   
    s.send(receivedPacket)

#################################################################################