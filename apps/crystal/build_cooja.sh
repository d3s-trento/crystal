#!/bin/bash

# set to 1 for motelab testbeds
#export CFLAGS="$CFLAGS -DTINYOS_SERIAL_FRAMES=1"

# set COOJA to 1 for simulating Glossy in Cooja
CFLAGS+=" -DCOOJA=1"
#CFLAGS+=" -DDISABLE_ETIMER=1"

CFLAGS+=" -DDISABLE_UART=0"
CFLAGS+=" -DCRYSTAL_SINK_ID=1" 

CFLAGS+=" -DTX_POWER=31 -DRF_CHANNEL=26 -DCONCURRENT_TXS=5 -DNUM_ACTIVE_EPOCHS=1"
CFLAGS+=" -DPAYLOAD_LENGTH=2"

CFLAGS+=" -DSTART_EPOCH=1"

export CFLAGS

cp sndtbl_cooja.c sndtbl.c

#make clean
rm crystal.sky

make 
