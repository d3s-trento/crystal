#!/bin/bash

ENV=crystal-test.sky.env
make clean

rm -f crystal-test.sky
rm -f crystal-test.sky.ihex
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
msp430-objcopy -O ihex crystal-test.sky crystal-test.sky.ihex | tee -a $ENV

echo "-- sndtbl.c --------" >> $ENV
cat sndtbl.c >> $ENV
rm -f sndtbl.c symbols.h symbols.c
