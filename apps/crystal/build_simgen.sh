#!/bin/bash

# set to 1 for motelab testbeds
# export CFLAGS="$CFLAGS -DTINYOS_SERIAL_FRAMES=1" # set in the simgen_ta.py

#export CFLAGS="$CFLAGS -DTINYOS_NODE_ID=1" # set in the simgen_ta.py

# set COOJA to 1 for simulating Glossy in Cooja
#export "CFLAGS=$CFLAGS -DCOOJA=1"

#export CFLAGS="$CFLAGS -DDISABLE_ETIMER=1"

ENV=crystal.sky.env
make clean

rm -f crystal.sky
rm -f crystal.sky.ihex
rm -f $ENV


rm $ENV
pwd 2>&1 | tee -a $ENV
msp430-gcc --version 2>&1 | tee -a $ENV
export 2>&1 | tee -a $ENV


echo "-- git status -------" >> $ENV
git log -1 --format="%H" >> $ENV
git --no-pager status --porcelain 2>&1 | tee -a $ENV
git --no-pager diff >> $ENV

echo "-- build log --------" >> $ENV
make 2>&1 | tee -a $ENV
msp430-objcopy -O ihex crystal.sky crystal.sky.ihex | tee -a $ENV

echo "-- sndtbl.c --------" >> $ENV
cat sndtbl.c >> $ENV
