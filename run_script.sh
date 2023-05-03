#!/bin/bash

# This script emulates keypress after the ARGoS GUI is launched so that it would run in fast-forward mode.
# This is needed because the experiment config file (generated from Will's script) automatically records (upon playing) the simulation, and terminates automatically upon reaching the end of the experiment.

SEED=1025
set -m
for (( i = 0; i < 11; i++ ))
do
    argos3 -c epuck_foraging_${SEED}.argos & sleep 2
    xdotool keydown Tab && sleep 1 && xdotool keyup Tab && sleep 1 && xdotool keydown Tab && sleep 1 && xdotool keyup Tab && sleep 1 && xdotool keydown Tab && sleep 1 && xdotool keyup Tab && sleep 1 && xdotool keydown f && sleep 1 && xdotool keyup f && fg
    pushd frames/exp${SEED} && bash ../../convert_argos_pngs_to_mp4.bash; popd
    SEED=$(($SEED + 1))
done
