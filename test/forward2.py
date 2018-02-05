#!/usr/bin/env python

import struct
import sys,os
import socket
import binascii

rawSocket=socket.socket(socket.PF_PACKET,socket.SOCK_RAW,socket.htons(0x0800))

while True:
	receivedPacket=rawSocket.recv(2048)

	#Extrai o cabecalho Ethernet
	#ethernetHeader=receivedPacket[0:s",ethernetHeader)
	#destinationMAC= binascii.hexlify(ethrheader[0])
	#sourceMAC= binascii.hexlify(ethrheader[1])
	#protocol= binascii.hexlify(ethrheader[2])

	#Extrai o cabecalho IP 
	ipHeader=receivedPacket[14:34]
	ipHdr=struct.unpack("!12s4s4s",ipHeader)
	destinationIP=socket.inet_ntoa(ipHdr[2])
	sourceIP=socket.inet_ntoa(ipHdr[1])
	if sourceIP == "10.10.10.10": # Filtrar os pacotes que serao enviados
		# Printa na tela informacoes do pacote recebido
		# print "Source IP: " +sourceIP
		# print "Destination IP: "+destinationIP
		# print "Destination MAC: "+destinationMAC
		# print "Source MAC: "+sourceMAC
		# print "Protocol: "+protocol
		
		#Envia o pacote recebido de volta para a interface escolhida
		s = socket.socket(socket.PF_PACKET,socket.SOCK_RAW,socket.htons(0x0800))
		s.bind(("eth2", socket.htons(0x0800))) # Alterar para a interface que SFF recebera os pacotes
		s.send(receivedPacket)
		
#################################################################################