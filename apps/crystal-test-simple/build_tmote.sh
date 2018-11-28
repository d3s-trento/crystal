#!/bin/bash

# set to 1 for motelab testbeds
#export CFLAGS="$CFLAGS -DTINYOS_SERIAL_FRAMES=1"

# set COOJA to 1 for simulating Glossy in Cooja
#CFLAGS+=" -DCOOJA=1"


CFLAGS+=" -DSINK_ID=1" # the sink node id

CFLAGS+=" -DTX_POWER=31 -DRF_CHANNEL=26"
CFLAGS+=" -DPAYLOAD_LENGTH=2"
#CFLAGS+=" -DCRYSTAL_CONF_LOGLEVEL=CRYSTAL_LOGS_EPOCH_STATS"
CFLAGS+=" -DCRYSTAL_CONF_LOGLEVEL=CRYSTAL_LOGS_NONE"
CFLAGS+=" -DCRYSTAL_CONF_N_FULL_EPOCHS=0"

if [ $# -lt 1 ]; 
    then echo "Specify the node ID"
    exit
fi

NODE_ID=$1
CFLAGS+=" -DNODE_ID=$NODE_ID"


export CFLAGS

make clean
rm crystal-test.sky

make && mv crystal-test.sky crystal-test-$NODE_ID.sky
rm -f sndtbl.c symbols.h symbols.c
