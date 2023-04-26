from collections import namedtuple
import numpy as np
import sys
import matplotlib.pyplot as plt
import re
import os
import csv
        
NO_FAULT = "FAULT_NONE"
PROX_FAULT = "FAULT_PROXIMITYSENSORS_SETRANDOM"
WHEEL_FAULT = "FAULT_ACTUATOR_LWHEEL_SETZERO"

def extract_data(filename, pattern):
    with open(filename) as f:
        file_content = f.read()
        match = re.search(pattern, file_content)
        if match:
            data = match.group(1)
            return data
        else:
            print(f"No matching data found in file for pattern: {pattern}")
            return "NAN"

def time_to_seconds(time_str):
    minutes, seconds = map(int, time_str.split(':'))
    return minutes * 60 + seconds

def get_error_data(seed_list, human_data, experiment_params):
    num_false_positives, num_false_negatives, num_true_positives, num_true_negatives = 0, 0, 0, 0
    accurate_detection_seed_lists = [[] for _ in range(len(human_data))]
    for seed in seed_list:
        dict_index = -1
        for data_dict in human_data:
            dict_index += 1
            if experiment_params[seed]["fault_behavior"] != NO_FAULT:
                if data_dict[seed]["faulty_id"] == experiment_params[seed]["id_faulty_robot"]:
                    accurate_detection_seed_lists[dict_index].append(seed)
                    num_true_positives += 1
                else:
                    #print(f"{data_dict['data_name']} guessed wrong on seed {seed}. Guessed {data_dict[seed]['faulty_id']} instead of {experiment_params[seed]['id_faulty_robot']}.")
                    num_false_negatives += 1
                    if data_dict[seed]["faulty_id"] != -1:
                        num_false_positives += 1
            else:
                if data_dict[seed]["faulty_id"] != -1:
                    #print(f"{data_dict['data_name']} guessed wrong on seed {seed}. Guessed {data_dict[seed]['faulty_id']} when there was no fault.")
                    num_false_positives += 1
                else:
                    num_true_negatives += 1
    return [accurate_detection_seed_lists, [num_false_positives, num_false_negatives, num_true_positives, num_true_negatives]]

def get_time_data(seed_lists, human_data, experiment_params):
    time_data = []
    for i in range(len(seed_lists)):
        for seed in seed_lists[i]:
            delay_in_detection = human_data[i][seed]["time_detected"] - experiment_params[seed]["injection_step"]/10
            time_data.append(delay_in_detection)
    return time_data
            

def plot_data(seed_lists, human_data):
    # LEDS vs NO LEDS
    time_data_to_plot = {}
    time_data_to_plot["leds_on"] = []
    for seed in seed_lists["leds_on_seeds"]:
        for data_dict in human_data:
            time_data_to_plot["leds_on"].append(data_dict[seed]["time_detected"])
    for seed in seed_lists["leds_off_seeds"]:
        time_data_to_plot[seed] = []

def print_error_data(category_str, data):
    print(f"{category_str}: FP: {data[1][0]}, FN: {data[1][1]}, TP: {data[1][2]}, TN: {data[1][3]}")

def main():
    if len(sys.argv) != 2:
        print("Usage: python extract_robot_id.py <path-to-directory>")
        sys.exit(1)

    directory = sys.argv[1]
    random_seed_pattern = r'random_seed="(\d+)"'
    fault_behavior_pattern = r'fault_behavior="([^"]+)"'
    faulty_robot_pattern = r'id_faulty_robot="(\d+)"'
    injection_step_pattern = r'injection_step="(\d+)"'
    show_leds_pattern = r'concise_output="true"\n.*show_leds="(true|false)"'
    quantity_pattern = r'quantity="(\d+)"'

    experiment_params = {}
    human_data = []
    human_data_files_index = -1
    for filename in os.listdir(directory):
        if filename.endswith(".argos"):
            argos_filename = os.path.join(directory, filename)
            random_seed = int(extract_data(argos_filename, random_seed_pattern))
            fault_behavior = extract_data(argos_filename, fault_behavior_pattern)
            id_faulty_robot = int(extract_data(argos_filename, faulty_robot_pattern))
            injection_step = int(extract_data(argos_filename, injection_step_pattern))
            show_leds_str = extract_data(argos_filename, show_leds_pattern)
            quantity = int(extract_data(argos_filename, quantity_pattern))
            show_leds = True if show_leds_str.lower() == "true" else (False if show_leds_str.lower() == "false" else "NAN")  # NAN if typo
            if show_leds == "NAN":
                print("Typo in the show_leds argument, received: ", show_leds_str, ". Aborting...")
                return
            experiment_params[random_seed] = {
                "fault_behavior": fault_behavior,
                "id_faulty_robot": id_faulty_robot,
                "injection_step": injection_step,
                "show_leds": show_leds,
                "quantity": quantity
            }
        elif filename.endswith(".csv"):
            human_data_files_index += 1
            # Create a dictionary for the current file
            human_data.append({})
            human_data[human_data_files_index]["data_name"] = filename.split(".csv")[0]
            # Open the CSV file
            with open(os.path.join(directory, filename)) as csv_file:
                # Read the CSV data using a DictReader
                csv_reader = csv.DictReader(csv_file)
                for row in csv_reader:
                    # Extract the seed, faulty_id, and time from the current row
                    seed = int(row["seed"])
                    faulty_id = -1 if row["faulty_id"] == "null" else int(row["faulty_id"])
                    time =  -1 if row["time"] == "null" else time_to_seconds(row["time"])
                    # Add the data to the file_dict using the seed as the key
                    human_data[human_data_files_index][seed] = {"faulty_id": faulty_id, "time_detected": time} 

    # print(experiment_params)
    # print("\n", "\n", human_data)

    # Get lists of random seeds for experiments
    SMALL_ROBOT_NUM = 16
    LARGE_ROBOT_NUM = 64
    seed_group_lists = {}
    leds_on_seeds = [seed for seed in experiment_params.keys() if experiment_params[seed]["show_leds"]]
    leds_off_seeds = [seed for seed in experiment_params.keys() if not experiment_params[seed]["show_leds"]]
    no_fault_seeds = [seed for seed in experiment_params.keys() if experiment_params[seed]["fault_behavior"] == NO_FAULT]
    prox_fault_seeds = [seed for seed in experiment_params.keys() if experiment_params[seed]["fault_behavior"] == PROX_FAULT]
    wheel_fault_seeds = [seed for seed in experiment_params.keys() if experiment_params[seed]["fault_behavior"] == WHEEL_FAULT]
    small_quantity_seeds = [seed for seed in experiment_params.keys() if experiment_params[seed]["quantity"] == SMALL_ROBOT_NUM]
    large_quantity_seeds = [seed for seed in experiment_params.keys() if experiment_params[seed]["quantity"] == LARGE_ROBOT_NUM]
    leds_on_seeds.sort()
    leds_off_seeds.sort()
    no_fault_seeds.sort()
    prox_fault_seeds.sort()
    wheel_fault_seeds.sort()
    small_quantity_seeds.sort()
    large_quantity_seeds.sort()
    seed_group_lists["leds_on_seeds"] = leds_on_seeds
    seed_group_lists["leds_off_seeds"] = leds_off_seeds
    seed_group_lists["no_fault_seeds"] = no_fault_seeds
    seed_group_lists["prox_fault_seeds"] = prox_fault_seeds
    seed_group_lists["wheel_fault_seeds"] = wheel_fault_seeds
    seed_group_lists["small_quantity_seeds"] = small_quantity_seeds
    seed_group_lists["large_quantity_seeds"] = large_quantity_seeds

    total_analysis = get_error_data(range(1000, 1036), human_data, experiment_params) # all experiments
    led_on_analysis = get_error_data(seed_group_lists["leds_on_seeds"], human_data, experiment_params)
    led_off_analysis = get_error_data(seed_group_lists["leds_off_seeds"], human_data, experiment_params)
    no_fault_analysis = get_error_data(seed_group_lists["no_fault_seeds"], human_data, experiment_params)
    prox_fault_analysis = get_error_data(seed_group_lists["prox_fault_seeds"], human_data, experiment_params)
    wheel_fault_analysis = get_error_data(seed_group_lists["wheel_fault_seeds"], human_data, experiment_params)
    small_quantity_analysis = get_error_data(seed_group_lists["small_quantity_seeds"], human_data, experiment_params)
    large_quantity_analysis = get_error_data(seed_group_lists["large_quantity_seeds"], human_data, experiment_params)

    print_error_data("total_analysis", total_analysis)
    print_error_data("led_on_analysis", led_on_analysis)
    print_error_data("led_off_analysis", led_off_analysis)
    print_error_data("no_fault_analysis", no_fault_analysis)
    print_error_data("prox_fault_analysis", prox_fault_analysis)
    print_error_data("wheel_fault_analysis", wheel_fault_analysis)
    print_error_data("small_quantity_analysis", small_quantity_analysis)
    print_error_data("large_quantity_analysis", large_quantity_analysis)

    
    total_time_data     = get_time_data(total_analysis[0], human_data, experiment_params)
    leds_on_data        = get_time_data(led_on_analysis[0], human_data, experiment_params)
    led_off_data        = get_time_data(led_off_analysis[0], human_data, experiment_params)
    prox_fault_data     = get_time_data(prox_fault_analysis[0], human_data, experiment_params)
    wheel_fault_data    = get_time_data(wheel_fault_analysis[0], human_data, experiment_params)
    small_quantity_data = get_time_data(small_quantity_analysis[0], human_data, experiment_params)
    large_quantity_data = get_time_data(large_quantity_analysis[0], human_data, experiment_params)

    time_data = [total_time_data, leds_on_data, led_off_data, prox_fault_data, wheel_fault_data, small_quantity_data, large_quantity_data]
    # Create a dictionary to store the data to be plotted for each seed
    # data_to_plot = {}
    # for seed in seeds:
    #     data_to_plot[seed] = []

    # # Extract the times for the relevant seeds from the human_data dictionaries
    # for human_dict in human_data:
    #     data_name = human_dict["data_name"]
    #     for seed in seeds:
    #         if seed in human_dict:
    #             data_to_plot[seed].append(human_dict[seed]["time"])
    #         else:
    #             print(f"No data found for seed {seed} in {data_name}")

    # Create a boxplot for each relevant seed
    plt.boxplot(time_data, labels=["all_data", "LEDs_on", "LEDs_off", "prox_sensor", "wheel_broke", "small_group", "large_group"])
    plt.title(f"Time between injection and detection")
    plt.ylabel("Time (s)")
    plt.ylim(0, 255)
    plt.show()


if __name__ == "__main__":
    main()
