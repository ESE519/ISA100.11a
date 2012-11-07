#!/bin/bash

ssh-keygen -t dsa -f .ssh/id_dsa
cd .ssh

for((i=2; i <= 16; i++))
do
  echo "Copying key into SAG$i"
  scp id_dsa.pub sauser@sag$i.wv.cc.cmu.edu:~/.ssh/id_dsa.pub
  echo "SSHing into SAG$i"
  ssh sauser@sag$i.wv.cc.cmu.edu 'cd .ssh && cat id_dsa.pub >>  authorized_keys && chmod 640 authorized_keys && rm id_dsa.pub && exit'
done

echo "Copying key into GRENACHE"
scp id_dsa.pub sauser@grenache.ece.cmu.edu:~/.ssh/id_dsa.pub
echo "SSHing into grenache"
ssh sauser@grenache.ece.cmu.edu 'cd .ssh && cat id_dsa.pub >> authorized_keys && chmod 640 authorized_keys && rm id_dsa.pub && exit'

# go back to work directory
cd /home/ayb/nano-RK/projects/network_stack_new

