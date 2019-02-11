#!/bin/bash

# set to 1 for motelab testbeds
#export CFLAGS="$CFLAGS -DTINYOS_SERIAL_FRAMES=1"

# set COOJA to 1 for simulating Glossy in Cooja
#CFLAGS+=" -DCOOJA=1"


CFLAGS+=" -DSINK_ID=1" # the sink node id


if [ $# -lt 1 ]; 
    then echo "Specify the node ID"
    exit
fi

NODE_ID=$1
CFLAGS+=" -DNODE_ID=$NODE_ID"

export CFLAGS

make clean
rm -f crystal-test.sky crystal-test.ihex

make crystal-test.ihex && mv crystal-test.sky crystal-test-$NODE_ID.sky && mv crystal-test.ihex crystal-test-$NODE_ID.ihex 
rm -f symbols.h symbols.c
