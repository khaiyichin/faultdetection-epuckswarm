#!/bin/bash

# This scripts generate experiment config file automatically based on the provided seed using Will's script.

SEED=1011
FAULT_TYPES=("FAULT_NONE" "FAULT_ACTUATOR_LWHEEL_SETZERO" "FAULT_PROXIMITYSENSORS_SETRANDOM")
SWARM_SIZES=(16 64)

for (( i=0; i < ${#FAULT_TYPES[@]}; i++ ))
do
    for (( j = 0; j < ${#SWARM_SIZES[@]}; j++ ))
    do
        for (( k = 0; k < 2; k ++ ))
        do
            SEED=$(($SEED + 1))
            python3 experiments/generate_experiments.py ${SWARM_SIZES[j]} ${FAULT_TYPES[i]} $SEED --led_bins=4 --lower=450 --upper=900

            SEED=$(($SEED + 1))
            python3 experiments/generate_experiments.py ${SWARM_SIZES[j]} ${FAULT_TYPES[i]} $SEED --led_bins=0 --lower=450 --upper=900
        done
    done
done
