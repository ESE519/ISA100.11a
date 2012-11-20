#!/bin/bash

while [ $# -gt 0 ]
do
gnome-terminal --working-directory=/home/ayb/nano-RK/projects/network_stack_new/ --command="ssh sauser@sag$1.wv.cc.cmu.edu" --title=SAG$1
shift
done

gnome-terminal --working-directory=/home/ayb/nano-RK/projects/network_stack_new/ --command="ssh sauser@grenache.ece.cmu.edu" --title=GRENACHE_NODE_DOWNLOAD

gnome-terminal --working-directory=/home/ayb/nano-RK/projects/network_stack_new/ --command="ssh sauser@grenache.ece.cmu.edu" --title=GRENACHE_SLIPSTREAM

gnome-terminal --working-directory=/home/ayb/nano-RK/projects/network_stack_new/ --command="ssh sauser@grenache.ece.cmu.edu" --title=GRENACHE_GATEWAY

