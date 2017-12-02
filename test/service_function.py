#!/usr/bin/env python       
from scapy.all import *
from scapy.layers.vxlan import VXLAN
from scapy.contrib.nsh import NSH
from bs4 import BeautifulSoup
import netifaces
import sys                                                                                                                 

def append_data(pkt):

    #if pkt[Ether].dst == in_mac:
    #    if VXLAN in pkt:
            if NSH in pkt:
                #print "Received packet with NSH header"
                #pkt.summary()
              #  inner_pkt = pkt[NSH].payload
                pkt[NSH].SI -= 1
            #else: # No NSH
             #   inner_pkt = pkt[VXLAN].payload
                      
            pkt[Ether].src = out_mac
            pkt[Ether].dst = sff_address

            #pkt.show()
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