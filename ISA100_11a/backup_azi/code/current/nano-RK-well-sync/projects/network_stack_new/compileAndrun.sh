#!/bin/bash

clear
make clean
make NODE_ADDR=$1 CTG=$2 TAR=$3
make program
