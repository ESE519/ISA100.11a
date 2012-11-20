The SAMPL protocol uses a tree routing scheme where a packet is broadcast across the networking setting up a loose tree depth based synchronization.  Nodes at each level contend with each other as they pass data up the tree.  Nodes on different levels do not contend with each other because they are at different depths in the tree and hence reply at different times.  The bottom of the tree begins replying first so that data can be easily aggregated towards the gateway.  Routes are set by each downstream message from the gateway.  The reply messages are called upstream messages.


SAMPL/gateway
This project acts as the gateway running on a sensor node that has a serial connection to a gateway device.  This project communicates using the SLIPstream-server to the slip-client.

SAMPL/client
This is the code which runs on each sensor node in the tree. 

SAMPL/slip-client
This is linux software which sends and receives data to the gateway node using the SLIPstream-server.  The xmpp project allows data to be published to an XMPP server.

SAMPL/mobile-example
This contains various example projects that act as mobile nodes communicating directly with the client nodes.

SAMPL/include
This directory contains any common header files that are used to specify packet formats across the different projects

nano-RK/tools/SLIPstream/SLIPstream-server
This is the basic SLIPstream server which is a linux program that forwards SLIP packets between the slip-client and the gateway.


Quick Start
  1)Flash a single sensor node with the “gateway” project code
  2)Flash a set of nodes with the “client” project 
  3)Set the client nodes MAC address using nano-RK/tools/EEPROM_mac_set.
  4)Start the SLIPstream-server linux program on a device connected over serial to the “gateway” node.
  5)Start the slip-client and set it to connect to the SLIPstream-server
  6)You should now see the slip-client make requests and receive responses from the network 
