#!/usr/bin/env python

from scapy.all import *
from scapy.contrib.nsh import NSH
import sys

if len(sys.argv) <= 1:
    with_nsh = True
elif sys.argv[1] == "no-nsh":
    with_nsh = False
else:
    print "Argument unknown. Only \"no-nsh\" allowed."
    exit(1)

html  = """
<html>
<head>
    <title>Messages from the SFs</title>
</head>
<body>
</body>
</html>
"""

http_resp  = "HTTP/1.1 200 OK\r\n"
http_resp += "Server: exampleServer\r\n"
http_resp += "Content-Length: " + str(len(html)) + "\r\n"
http_resp += "\r\n"
http_resp += html

# Generate input packets
i_eth = Ether(src="AA:BB:CC:DD:EE:FF",dst="AA:AA:AA:AA:AA:AA")
i_ip = IP(src="10.1.0.1",dst="10.1.0.2")
i_tcp = TCP(sport=1000,dport=2000)


o_eth = Ether(src="7a:78:3c:f8:be:43",dst="00:00:00:00:00:02")
o_ip = IP(src="10.0.0.1",dst="10.0.0.200")
o_udp = UDP()

vxlan = VXLAN(vni=1515)

if with_nsh:
    in_pkt = o_eth/o_ip/o_udp/vxlan/NSH(MDType=0x2,Len=0x2,SPI=0x1,SI=0xFF)/i_eth/i_ip/i_tcp/http_resp
else:
    in_pkt = o_eth/o_ip/o_udp/vxlan/i_eth/i_ip/i_tcp/http_resp

print "Beautiful packet!\n"
in_pkt.show()
print "Ugly packet!\n"
hexdump(in_pkt)
sendp(in_pkt,iface="br0")
