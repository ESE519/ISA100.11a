#!/bin/bash

while [ $# -gt 0 ]
do
    addr=$1
    #let "addr *= 10"    
    if test $addr = "all" ; then
       break
    fi
    
    flag=$2
    if test $flag = "c" ; then
       app="App1-client"
    elif test $flag = "s" ; then
       app="App1-server"
    fi
    
    echo "Compiling for SAG$addr $addr 0 $app" 
    sleep 2
    ./compileAndrunRemote.sh $addr 0 $app
    scp $app.hex sauser@sag$addr.wv.cc.cmu.edu:./aditya/
    sleep 2
    shift
    shift
done

#if test $addr = "all" ; then
#   for((i = 2; i <= 16; i++))
#    do
#	echo "Compiling for SAG$i $i 0" 
#    	sleep 2
#    	./compileAndrunRemote.sh $i 0
#    	scp App1-client.hex sauser@sag$i.wv.cc.cmu.edu:.
#    done 
#fi

app="App1-server"
addr=10
echo "Compiling for GRENACHE $addr 1 $app"
sleep 2
./compileAndrunRemote.sh $addr 1 $app 
scp $app.hex sauser@grenache.ece.cmu.edu:./aditya/
#ssh sauser@grenache.ece.cmu.edu './Aditya-prg-node.sh s && exit'

echo "Compiling network gateway for GRENACHE"
sleep 1
make -f makefileGatewayThreaded
scp ngr sauser@grenache.ece.cmu.edu:./aditya




