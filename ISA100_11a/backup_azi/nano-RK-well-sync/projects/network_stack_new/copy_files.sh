#!/bin/bash

DIR=/home/ayb/nano-RK/projects/basic_signals

cd $DIR
make clean
make
    
for((i = 2; i <= 16; i++))
do
    echo "Copying file into SAG$i"
    scp $DIR/main.hex sauser@sag$i.wv.cc.cmu.edu:./aditya/
done
