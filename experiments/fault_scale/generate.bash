#!/usr/bin/env bash

seed=1000
# this is for the fault type
for fault in "FAULT_PROXIMITYSENSORS_SETRANDOM" "FAULT_ACTUATOR_LWHEEL_SETZERO"; do
    mkdir $fault
    cd $fault
    # This is for the number of faulty robots
    for n in {1..10}; do
        mkdir $n
        cd $n
        ln -s ../../../../build build
        # this is for the number of iterations
        for iter in {1..20}; do
            python3 ../../../generate_experiments.py --size 16 --fault $fault --seed $seed --num_faulty $n --led_bins 4 --length 300 --lower 0 --upper 451 --headless
            let "seed += 1"
        done
        cd ../
    done
    cd ../
done
