This program demonstrates a chain of multi-hop RT-Link nodes.  There is a static schedule devised to stress test the system by having the farthest away node send a message on a low slot while the rest of the nodes forward the data back towards the gateway.  If any link is broken in the chain, packets will not arrive at the gateway.  The time synchronization is sent in-band using our implicit token passing scheme.  Since the tokens are passed starting from slot 0 of the coordinator, the stream stress tests the system by having the time sync token passed in the opposite direction of the data.

At the top of main, you will see two #defines that you must use to configure the chain.  The first #define COORDINATOR sets which node will generate the time sync token.  This could be any node, however it is best to set it to the highest NODE_ID so that the time synchronization and data streams are  transmitted in opposite directions.  NODE_ID 1 is configured to generate the initial message.  NODE_ID 2 and 3 should forward the nodes.  NODE_ID 4 will also forward the messages, but also act as a COORDINATOR.

Program the nodes with the following configuration at the top of main:
   Node 1:   
	// COORDINATOR  (commented out)
	NODE_ID	1
	
   Node 2:   
	// COORDINATOR  (commented out)
	NODE_ID	2

   Node 3:   
	// COORDINATOR  (commented out)
	NODE_ID	3

   Node 4:   
	COORDINATOR 
	NODE_ID	4

Leave node 4 connected to a terminal so that you can see the messages starting at node 1.
