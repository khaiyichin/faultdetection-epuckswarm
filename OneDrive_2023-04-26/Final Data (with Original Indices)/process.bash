#!/usr/bin/env bash

seed=1000
# this is for the fault type
for n in {1..36}; do
    python -m fire ../../analysis/analyze_output.py detailed_analysis_file --exp_file epuck_foraging_$seed.argos --nohup_file nohup_$seed.txt & > /dev/null
    let "seed += 1"
done
