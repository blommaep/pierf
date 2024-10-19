# pierf
pierf is an ethernet packet generation/injection and capture/analysis tool. It builds packets and puts them directly on the network driver, bypassing the operating system. Therefore, it is not limited to the features of the OS. It is intended as a test tool for ethernet/ip devices. Trying to respect a single executable concept that doesn't require any installation. 


The source was maintained on sourceforge until release 0.137.0 - some binaries up to 0.195.0

# ToC
+ [Legal Notice](#legal-notice)
+ [Introduction](#introduction)
+ [Installation](#installation)
+ [Very brief summary of packet layers](#very-brief-summary-of-packet-layers)
+ [Sending a basic packet](#sending-a-basic-packet)



# Introduction

A short overview of the capabilities. _The referred samples are full working sample files that can be found under the configs directory. Furthermore, the file configs/tst_loopback.pierf contains a test set that covers all types of packet layers, both transmit and capture, be it in a rather irrelevant sequence._

Some of the features:
- Basic protocols: ethernet, ip, udp, arp, icmp, igmp, raw bytes (hex string)
- Repetitive packets. Combining asleep tag with a repeat attribute, the tool can send packets periodically. E.g. igmp.pierf implements an igmp querier.
- Writing to a file (pcap/ethereal capture format) rather then to a real port. Can e.g. be used for test purpose (Cf. tst_file.pierf) or to convert a raw dump to a capture file (Cf. tst_raw.pierf).
- Replay of a capture file: combining the mirror feature (to mirror a captured packet from one port to another) with the possibility to read from a capture file, one can replay a capture file in realtime. Cf. tst_mirror.pierf. 
- Log the packets on a port while running. Gives you a log of everything you sent, as also any reply and anything else that was captured during that time. Cf. tst_log.pierf. (As such, it can also act as a simple capture tool, if all you'd do while logging is a sleep.)
- Count and summarize number of packets, packet rate, number of bytes and bitrate. Cf. tst_counter.pierf and tst_counter2.pierf (the second is supposed to be executed after the first to get a full sample.)
- Use of config include files, e.g. easy for predefining commonly used ports or scenarios. Cf. a.o. tst_raw.pierf
- Analyze and print the received packet in xml format. The print format follows the pierf syntax and hence the print output can be used as input to the tool. Cf. a.o. tst_print_prerare.pierf and tst_print.pierf, that illustrate this feature when run in sequence.
- Build conditional actions, depending on the structure and field values of received packets. E.g. send igmp report when igmp query is received, increment a counter for each udp packet received, mirror all packets for destination mac address x, etc. Cf. a.o. tst_match.pief, which can be run after executing tst_print_prepare.pierf
- Send out packets at a predefined rate. Cf. tst_shaper.pierf. Also multiple streams in (quasi-)parallel possible. Cf. tst_multishaper.pierf
- Use variables to dynamically assign values to a field. E.g. for port scanning, mac flooding, etc. Also useful for field values that keep coming back or for a flexible tuning of configs. Cf. configs/tst_var.pierf
- Assign variables to fields of received (captured) packets. Cf. configs/icmp.pierf

# Installation

No installation. Build (old fashioned make) or download zipped release files with executable.
Unzip release files on the correct platform, the result is ready to use
- until v0.195.0 : cygwin and linux 32 bit executables, statically linked to include all libraries, are provided. 
- v0.213.0 : linux 64 bit executable with dynamically linked libpcap: install libpcap per your linux distro practice

Further manuals and help:
See [pierf.html](https://htmlpreview.github.io/?https://github.com/blommaep/pierf/blob/main/src/pierf.html)
(A few brief items also below)

# Very brief summary of packet layers

All tags and all packet layers are listed in Appendix A with a full explanation. This chapter gives a quick overview for quickpick. To learn how to create a packet and get it sent, check the next chapter. Note that typically not all attributes must be specified. Some attributes have a default value (where there is a meaningful default). They are marked in italic. Some values get an automatic value based on higher layers in the packet. They are marked in italic underline. Refer to Appendix A and Appendix B for details on the default value or automatically calculated value..

```
<eth from="..." to="..." ethertype="..."/>
<vlans stack="..." vlanEthertype="..." bodyEthertype="..."  />
<arp type="..." fromMac="..." toMac="..." fromIp="..." toIp="..." />
<raw type="..." data="..." filler="..." size="...">...</raw>
<iphdr from="..." to="..." protocol="..." packetid="..." ttl="..." contentlength="..." options="..." dscp="..." flags="..." fragmentoffset="..." checksum="..."/>
<ipv6 from="..." to="... trafficClass="..." flowLabel="..." hopLimit="..." payloadlength="..." />
<udp sourceport="..." destport="..." length="..." checksum="..."  />
<tcp sourceport="..." destport="..." sequencenr="..." acknowledgenr="..." flags="..." windowsize="..." urgentpointer="0" options="..." checksum="..."  />
<igmp version="2" type="..." to="..." responsetime="..." checksum="..." />
<igmp version="3" type="query" to="0.0.0.0" responsetime="..." sflag="..." qrv="..." qqic="..." checksum="...">
    <source address="..." />
</igmp>
<igmp version="3" type="report" to="0.0.0.0" responsetime="..." checksum="...">
    <group type="..." to="..." />
</igmp>
<icmp type="echoRequest" code="..." checksum="..." identifier="..." sequencenr="..." />
<icmp type="..." code="..." checksum="..." identifier="..." sequencenr="..." nexthopmtu="..." ipaddress="..." advertisementCount="..." addressEntrySize="..." lifetime="..." pointer="..." offset="..." checksum="..." />
<signature name="..." />
```

# Sending a basic packet 
Lets start with a small example to illustrate, sending one arp. (More sample pierf files may be found bundled with the binary, in the configs, cf. above.)

```  
<pierf>
  <port id="eth0" device="\Device\NPF_{F8C7658E-8536-4DA1-BC85-F2EC10B37656}" />
  <port id="eth1" device="\Device\NPF_{F8C7698B-0123-4EF0-CC55-F2E13852CF5A}" />
  <scene id="arp">
  <seq repeat="2">
    <packet port="eth0">
      <eth from="02:02:02:02:02:02" to ="FF:FF:FF:FF:FF:FF" />
      <vlans stack="700:120"></vlans>
      <arp type="req" fromMac="02:02:02:02:02:02" fromIp="192.168.10.2" toIp="192.168.10.1" />
    </packet>
    <packet port="eth1">
      <eth from="01:01:01:01:01:01" to ="02:02:02:02:02:02" />
      <vlans stack="700:120"></vlans>
      <arp type="rep" fromMac="01:01:01:01:01:01" toMac="02:02:02:02:02:02" fromIp="192.168.10.1" toIp="192.168.10.2" />
    </packet>

    <sleep milliseconds="100" />
  </seq>
  </scene>

<play scene="arp" />
</pierf>
```

Clarification of the above example:
- The file starts and stops resp. with a <pierf> and </pierf> tag, confirming that it is a pierf file.
- The ports (ethernet cards) on your PC must be identified by their device id. To find out the device id of your ports, just fill someting dummy and run the tool. It will print an error but also provide a list of devices. Using Ethereal, you'll also find the device id there, when starting a capture. For further use, you can give these ports a friendly name. The <port> tag is used for that.
- You can define multiple "scenario's", identified by <scene> tags.  A scene has an id, that can be referenced later to run it. As such, the definition of a scene does not cause anything to be sent. It only loads the scenario and packet definitions into memory. Everything between the start tag <scene> and the end tag </scene> is part of the scene.
- A scene currently can only exist of a sequence of packets. A sequence starts with a <seq> tag and ends with a </seq> tag. If you want to sent a packet (or sequence of packets) repetitively, add repeat="..." as an attripbute to the sequence tag.
- A sequence is built up by packets. Each packet needs a port attribute to know on which port to send the packet. This is the <packet>/</packet> tag. 
- A packet consists of a series of layers. In this example, an arp request in a stacked vlan (outer 700, inner 120), over ethernet. Currently, the layers in a packet have no correlation, so, e.g. in this case, you must explicitly specify the source mac address both in the ethernet and in the arp layer.
- A sequence can contain more then packets. E.g. a delay (<sleep milliseconds="...">) was added here. In this case, two arp requests will be sent with 100 milliseconds delay between them.
- Finally, there's one tag left: the <play> tag. This is the only one that effectively does something. It sends all the packets defined in the <scene> tag.

# Legal Notice

Copyright (c) 2024, Pieter Blommaert

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
<br/>
Small clarification of my choice for this type (free BSD) style of license:
- At first i liked copyleft style (e.g. GPL) of licenses until I came to realise that enforced freedom is a contradiction in terminis. Once i really want to start using GPL licensed code, i need to carefully check what is allowed and what not. I also wanted at any time to keep all of my rights to do with the code whatever i want. So i decided that the above, free BSD style license, is the only license that makes my small contribution somewhat useful to the community. 
- I intended my tool for network and equipment tests. I disclaim all responsibility for the use of this software, however I would be personally offended if it is intentionally used for illegal or aggressive purpose. There are no excuses that justify misusing or breaking well-intended equipment and/or services.

