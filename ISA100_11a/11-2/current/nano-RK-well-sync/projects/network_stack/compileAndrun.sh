#!/bin/bash

clear
make clean
make NODE_ADDR=$1 
make program
