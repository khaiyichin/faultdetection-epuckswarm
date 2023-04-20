# How to run experiments

## Notes on experiment files
This section lists the parameters to change and values to use in running the experiments. Each subheading describes the XML node in the `.argos` file.

### `<epuck_homswarm_controller>` node
- `<params>` node:
    - `experiment_run` node:
        - `swarm_behavior` -- swarm behavior -- attribute (string):
            - `"SWARM_AGGREGATION"`
            - `"SWARM_DISPERSION"`
            - `"SWARM_FLOCKING"`
            - `"SWARM_HOMING"`
            - `"SWARM_STOP"`
        - `swarm_behavior_trans` -- swarm transition behavior -- attribute (string):
            - `"SWARM_AGGREGATION"`
            - `"SWARM_DISPERSION"`
            - `"SWARM_FLOCKING"`
            - `"SWARM_HOMING"`
            - `"SWARM_STOP"`
            - `""`
        - `time_between_robots_trans_behav` -- unknown parameter -- attribute (float):
        - `fault_behavior` -- fault behavior -- attribute (string):
            - `"FAULT_NONE"`
            - `"FAULT_STRAIGHTLINE"`
            - `"FAULT_RANDOMWALK"`
            - `"FAULT_CIRCLE"`
            - `"FAULT_STOP"`
            - `"FAULT_PROXIMITYSENSORS_SETMIN"`
            - `"FAULT_PROXIMITYSENSORS_SETMAX"`
            - `"FAULT_PROXIMITYSENSORS_SETRANDOM"`
            - `"FAULT_PROXIMITYSENSORS_SETOFFSET"`
            - `"FAULT_RABSENSOR_SETOFFSET"`
            - `"FAULT_RABSENSOR_MISSINGRECEIVERS"`
            - `"FAULT_ACTUATOR_LWHEEL_SETZERO"`
            - `"FAULT_ACTUATOR_RWHEEL_SETZERO"`
            - `"FAULT_ACTUATOR_BWHEELS_SETZERO"`
            - `"FAULT_SOFTWARE"`
            - `"FAULT_POWER_FAILURE"`
        - `id_faulty_robot` -- ID of a single faulty robot -- attribute (int)
    - `wheel_turning` node: `max_speed` -- speed of wheels in cm/s? -- attribute (int)

### `<epuck_foraging_controller>` node
- `<params>` node:
    - `experiment_run` node:
        - `swarm_behavior` -- swarm behavior -- attribute (string):
            - `"SWARM_FORAGING"`
        - `fault_behavior` -- fault behavior -- attribute (string):
            - `"FAULT_NONE"`
            - `"FAULT_STRAIGHTLINE"`
            - `"FAULT_RANDOMWALK"`
            - `"FAULT_CIRCLE"`
            - `"FAULT_STOP"`
            - `"FAULT_PROXIMITYSENSORS_SETMIN"`
            - `"FAULT_PROXIMITYSENSORS_SETMAX"`
            - `"FAULT_PROXIMITYSENSORS_SETRANDOM"`
            - `"FAULT_PROXIMITYSENSORS_SETOFFSET"`
            - `"FAULT_RABSENSOR_SETOFFSET"`
            - `"FAULT_RABSENSOR_MISSINGRECEIVERS"`
            - `"FAULT_ACTUATOR_LWHEEL_SETZERO"`
            - `"FAULT_ACTUATOR_RWHEEL_SETZERO"`
            - `"FAULT_ACTUATOR_BWHEELS_SETZERO"`
            - `"FAULT_SOFTWARE"`
            - `"FAULT_POWER_FAILURE"`
        - `id_faulty_robot` -- ID of a single faulty robot -- attribute (int)
    - `wheel_turning` node: `max_speed` -- speed of wheels in cm/s? -- attribute (int)
    - `state` node (work-in-progress):
        - `initial_rest_to_explore_prob` -- some type of probability -- attribute:
        - `minimum_resting_time` -- -- attribute:
        - `minimum_unsuccessful_explore_time` -- -- attribute:
        - `minimum_search_for_place_in_nest_time` -- -- attribute:

# Notes on LED signals
When you set `show_leds` to `true` **under the `<loop_functions>` node**, this is what the signals represent:
1. No LED/Black: Number of tolerators = number of attackers = 0
2. Green: Number of tolerators >= number of attackers
3. Other colors: number of tolerators < number of attackers, but the color type depend on the proportion of attackers.
    - `led_bins` = `2`: Yellow (0 - 50%), Red (50% - 100%)
    - `led_bins` = `3`: Yellow (0 - 33%), Magenta (33% - 67%), Red (67% - 100%)
    - `led_bins` = `2`: Yellow (0 - 25%), Blue (25 - 50%), Magenta (50 - 75%), Red (75% - 100%)