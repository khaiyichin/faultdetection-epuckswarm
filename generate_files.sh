#!/bin/bash

# This scripts generate experiment config file automatically based on the provided seed using Will's script.

SEED=110
FAULT_TYPES=("FAULT_NONE" "FAULT_ACTUATOR_LWHEEL_SETZERO" "FAULT_PROXIMITYSENSORS_SETRANDOM")
SWARM_SIZES=(16 64)

for (( i=0; i < ${#FAULT_TYPES[@]}; i++ ))
do
    for (( j = 0; j < ${#SWARM_SIZES[@]}; j++ ))
    do
        for (( k = 0; k < 5; k ++ ))
        do
            SEED=$(($SEED + 1))
            python3 experiments/generate_experiments.py ${SWARM_SIZES[j]} ${FAULT_TYPES[i]} $SEED --length=245 --led_bins=2 --lower=450 --upper=1250

            SEED=$(($SEED + 1))
            python3 experiments/generate_experiments.py ${SWARM_SIZES[j]} ${FAULT_TYPES[i]} $SEED --length=245 --led_bins=0 --lower=450 --upper=1250
        done
    done
done
