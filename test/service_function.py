#!/usr/bin/env python       
from scapy.all import *
from scapy.layers.vxlan import VXLAN
from scapy.contrib.nsh import NSH
import netifaces
import sys                                                                                                                 

def append_data(pkt):

    if pkt[Ether].dst == in_mac:
        if NSH in pkt:
            #print "Received packet with NSH header"
            #pkt.summary()

            pkt = pkt/Raw(load=" SF #" + str(sfid) + " says hi! ")
            pkt[NSH].SI -= 1

            pkt[Ether].src = out_mac
            pkt[Ether].dst = sff_address

            sendp(pkt,iface=out_iface)
            
            #print "=== Sent NSH packet: ==="
            #print "Beaufiful: "
            #pkt.show()
            #print "Ugly: "
            #hexdump(pkt)

load_contrib('nsh')

nargs = len(sys.argv)

if nargs < 4 or nargs > 5:
    print "Usage: " + sys.argv[0] + " SFID <SFF MAC> in_face [out_iface]"
    exit(1)
    
sfid = sys.argv[1]
sff_address = sys.argv[2]
in_iface = sys.argv[3]
in_mac = netifaces.ifaddresses(in_iface)[netifaces.AF_LINK][0]['addr']

if nargs == 4:
    out_iface = in_iface
    out_mac = in_mac
else:
    out_iface = sys.argv[4]
    out_mac = netifaces.ifaddresses(out_iface)[netifaces.AF_LINK][0]['addr']

print "In MAC: " + in_mac + "\nOut MAC: " + out_mac

print "Starting sniffing..."
sniff(iface=in_iface,prn=append_data)