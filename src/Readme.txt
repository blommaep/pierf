Features: (finished todo's)
- Send packets onto any network port of a pc (that allows promiscue mode)
- Xml input files
- Predefined sets of scenes. Make config files with a whole list of possible things to send (each thing is called a scene). Then select what to run.
- Identify (network) ports and use them with a simple name
- Ethernet and Arp (request/reply)
- Vlan with any level of stacking and priorities (per level)
- Raw packet elements (hex)
- IGMP v2 and v3
- Only limited by your PC's memory limitations. Packet size, vlan stacks,... all use stl and heap allocation.
- Clear error reporting (syntax in input, device not ready,...)
- Tool version support: the tool version has three numbers. The first is an indication of the supported feature level. The second is a tracked source version. The third is always 0 for released (distributed) version and an indidication of the number of sources changed in debug versions.
- Auto protocol fill: e.g. if IP is followed by igmp: no need to specify protocol. If Ethernet is followed by IP/IGMP: auto translate multicast ip to mac@=. "auto" will be a new tag argument. When set to none, no updates are done, when set to all, as much as possible is guessed.
- ICMP 
- UDP
- Port: when an unknown device is specified, list all the available devices
- Logging of all traffic on an interface (pcap format)
- Receiving and sending to file (pcap format) i.o. a real device. Allows e.g. selective logging
- Repeating sequences x times
- Delay (sleep) in between two packets sent.
- Capture of packets (receive) and local mirror: send out on another port. Allow also replay of capture files.
- Counter variables, counting number, rate, captured bytes and byterates
- Size and filler for "raw" packets, allowing easy sizing of packets
- Linux build. Common Makefiles for windows/cygwin and Linux.
- Stack improvements, e.g. ethertype is now properly part of the Ethernet layer, Vlan ethertype can be specified,... Several layers have been extended or completed.
- Includes of config files, to allow definition of multiple seq's and then run 1
- Analysis of received packets and possibility to print them in xml (same as the send syntax, so that it can be used for building scenarios).
- Packet matching system by <match><eth>...</match> for conditional constructs
- Shaping for packets sent, including a multishaper that allows quasi-parallel sending of multiple steams.
- Variable definition for re-use. E.g. <var type="string" id="$mac1" value="02:02:02:02:02:02"> and then <ethernet from="$mac1">
- Timing and traffic load generation with fixed bandwidth (limited accuracy depending on the platform)
- Loopback port that recieves the packets it sends itself and logs them also to  a file. Useful for testing: the receive and send can be tested by one config file
- Detection of parameters while running, e.g. extracting target mac@ from arp reply
- Tcp
- Signature frames and detection of missed frames
- IPv6 header
- Increment/Reset counters to a variable value (<counter ... var="..." />)
- String Match in addition to regular (binary) match. String Match is only slightly slower. Binary match may be entirely removed later?
- Counter increment by or reset to a variable value
- Names sequences that may be referenced anywhere else
- Text fields (update of raw fields to support text (ascii) input)
- New matching method (<match method="compare">) that allows more flexibility, especially for matching <raw> data


Todo: (in order of "planned" implementation)
- Self testing scripts
- Interface definition, which is nothing more then a set of parameters and tag to apply. e.g. <param value="02:02:02:02:02:02" tag="eth" arg="from"> and applying
- Use of variables also for ICMP (Flexfield) fields (redesign of Flexfield).
- Protocol stack definitions <stack id="ipoe"><eth><ip></stack>. May, but must not contain argument values. 
- Scan entire packet for a "signature frame" without the need for full stack match.
- Improve/rewrite the threading with a messaging system. Possibly get rid of zthread fancy library and go back to pthread and build own to-the-point mini-platfor instead. Goal is to allow also parallel receive/send threads.
- Remove the scene as a tag, it has little advantages. Allow <seq> to have an id and store them with id when specified. Allow <seq ref="..."> to put in place a sequence defined before. This allows to predifine sequences (or packets if the seq contains only a single packet) that is needed on several places.
- Other protocols (DHCP, PPP,...)
- Remote operation by starting servers on several machines, waiting for control messages containing xml files.
- Console application to read and modify the servers "database" (xml files) and logs.
- GUI that does the same.
- Local port mirror with modification (e.g. : insert a vlan tag,...)
- Remote port mirror (kind of pipe)
- support of netpdl xml languange to describe protocols. Would allow to extend with any protocol without having to add code
- creation of a compiler that compiles the netpdl files to c++ files that can be included in the tool. Would allow easy conversion of netpdl to built-in supported protocols. Advantages is that no additional files are needed for execution and that execution should be more optimal (less memory needed, faster) vs. netpdl parsing.
- compiler mode that generates C++ code, that can be far more efficient then the interpreted mode. May give better thoughput and also be easier for delivery of dedicated test scripts?


Ideas for future features:

- ease implementation of elements by introducing a fieldstack. The fieldstack has a vector of fieldrecords which contain a field pointer and field attributes.


Proposal parallel threads

<par>
  <syncpoint id="rxsynx"> // merely a predeclaration
  <seq>
    <receive syncto="rxsync" /> // all occurences of rxsync will wait for eachother. There may be only one per thread, else this will block forever, so check this while parsing
  </seq>

  <seq>
    <receive syncto="rxsync" />
  </seq>

  <seq>
    <packet ... />
    <sleep ... />
  </seq>
</par>


Proposal gui (dream implementation)
-----------------------------------
- Different frames that can be structured in one window or split off into multiple windows, entirely up to wish. Several prefered lineups can be saved (can e.g. depend on the screen(s) available, task to be done,...)
- One frame will contain a list (editable table) of the ports with their name and attributes. Context menu offers
-- creation of a new port, with browse for files if creating a port of type file.
- One frame will contain a list of sequences/scenes, that fold open until the lowest level. Structure matches obviously the xml structure.

Code structure
- Code has consciously been designed in c++, using limitted external libraries. Intention is a cross platform executable, that does not require any installation.
- Makefile supports released versions. Sources of released versions are automatically committed in subversion and the subversion version is used for the tool version.
- Consciosly choose using stl. It is the most available and efficient way to be limitted only by memory and processor. I believe that hard coded limits are a level of complexity you want to avoid in testing tools. 
- This is not a protocol stack, optimised for efficiency. People used to making low level protocol stacks would probably find the code horrible.
- Packet class has a stack of Elements. Element class is "virtual". All Elements, like eth, vlans, arp,... are derived from Element class. The Packet object has a main member of ElemStack class. The ElemStack is usable by the Elements and can be searched through for auto completion (typComplete function).
- Elements are constructed of Fields. All Fields are derived from the Field class. The field class allows tracking of the state of a field (undefined, has been assigned a default value, manually configured value, ...)
- Some always come back fields, like mac address and IP address have their own class, mainly for string (xml ascii input) to value (hex, network format) parsing. Others use generic Field classes, like BitField16.
- Both containters (scene, seq,...) and Elements are derived from the AutoObject, that keeps track of the auto flag and implements enheritence of this flag.
- The ParseConfig currently does both the config parsing and keeping the config and launching the execution. Probably needs split up later. Needed to get to packet generation fastly.
- Scene and Seq class contain stacks of PlayStep objects that transparently represent their xml counterpart
- Packets are only one type of PlayStep. Another important PlayStep is ReceiveStep. It waits for capture of a packet on a specific Port. It then plays in sequence sub-steps, like MirrorStep, CounterStep, FirstofStep, MatchStep... 
- All steps part of the "receive" are made general playsteps, which are run without arguments and have no return values (so that they can be easily stacked). Because some things need to be passed (not in the least the received packet) between the different layers in the receive (e.g. between receive and mirror step), these steps keep pointers to each other, so that they can request the necessary information and push the relevant result.

Idea for new structure:
 - General Packet class that does contain the sequence of elements, the serialized packet and basic operations
 - RxPacket, TxPacket and CmpPacket class for the specific play() etc., that has Packet as a member or that's derived from it.
 - For GUI: 
   - Everything must have a "parseAttrib". getString to be split in getOpenString, getCloseString and getBodyString. getBodyString is to get all of the sub-tags. A Pierf class to encapsulate everything for which getString should return the entire xml must allow write to file.
   - There must also be functions to remove sub-elements, not only add. Requires profound study of the delete (currently mostly ignored).
   - Everything must have a "parseAttrib" or so. Updates in the gui must result in a "reparse" of xml attributes (from gui point of view, all are string operations). Exceptions must be catch'd and result in errors displayed in the gui. For things like "auto", a re-parse of entire element seems better then to introduce pareseSingleAttrib: anyway more things must be re-evaluated. But how to do save reparse?

Some typical debug problems:
- Memory corruption: check that the size calculation is correct. Otherwise wrong packet size is allocated
- Warning for non virtual destructor: abstract classes must always have a constructor.
- gdb/ddd in analyze functions where fromPtr is actually a reference: x/20 (uchar *) fromPtr : will give the actual content  (x/20 fromPtr is misinterpreted due to the reference)
