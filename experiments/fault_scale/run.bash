#!/usr/bin/env bash

seed=1000
# this is for the fault type
for fault in "FAULT_PROXIMITYSENSORS_SETRANDOM" "FAULT_LWHEEL_SETZERO"; do
    mkdir $fault
    cd $fault
    # This is for the number of faulty robots
    for n in {1..10}; do
        mkdir $n
        cd $n
        # this is for the number of iterations
        for iter in {1..20}; do
            ln -s ../../../../build build
            argos3 -c epuck_foraging_$seed.argos &
            let "seed += 1"
        done
        cd ../
    done
    cd ../
done
