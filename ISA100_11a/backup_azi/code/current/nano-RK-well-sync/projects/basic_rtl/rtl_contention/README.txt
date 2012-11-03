This is an example of using the contention slots for communication. 

Program a single node as the coordinator with the:
#define COORDINATOR
line at the top of main.

Then program a set of nodes with COORDINATOR commented out.
The nodes should be able to communicate in a random access manner with
one another.

Note that is very inefficient and should only be used for mobile agents
or bootstrapping the network for schedules.
