#!/usr/bin/env python       
from scapy.all import *
from scapy.layers.vxlan import VXLAN
from scapy.contrib.nsh import NSH
from bs4 import BeautifulSoup
import netifaces
import sys                                                                                                                 

def append_data(pkt):

    if pkt[Ether].dst == in_mac:
        if VXLAN in pkt:
            if NSH in pkt:
                #print "Received packet with NSH header"
                #pkt.summary()
                inner_pkt = pkt[NSH].payload
                pkt[NSH].SI -= 1
            else: # No NSH
                inner_pkt = pkt[VXLAN].payload
            
            if TCP in inner_pkt:
                http_msg = str(inner_pkt[TCP].payload)
                s = http_msg.split("\r\n\r\n")
            
                # Add content to html file
                if len(s) > 1:
                    headers = s[0]
                    content = s[1]
                    soup = BeautifulSoup(content,'html.parser')
                    soup.body.string += "SF #" + sfid + " says hi!\n"
                    inner_pkt[TCP].remove_payload()
                    pkt /= headers + "\r\n\r\n" + str(soup)
                    print "=== Modified HTML file ===="
                    print soup.prettify()
                else:
                    print "Split failed!"

            
            pkt[Ether].src = out_mac
            pkt[Ether].dst = sff_address

            pkt.show()
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