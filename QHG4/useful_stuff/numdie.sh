#!/bin/bash

# used by sim_and_extract.sh

sim_name=$1
if [ $# -gt 1 ]
then
    echo "simulation;total_count;dieout_time"
fi

sim=`echo ${sim_name} | gawk -F/ '{ print $NF }' | sed 's/.out//'`
numtot=`grep -e "S:  sapiens:  [0-9]*" $1 |  gawk '{ print $NF }' | sed 's/[^0-9]\[0m//g'`
iters=`grep "S:Number of iterations: " $1 | gawk '{ print $NF }' | sed 's/[^0-9]\[0m//g'`

if [[ "${numtot}" != "0" ]]
then
    iters=-1
fi

echo "${sim};${numtot};${iters}"
