import json
import numpy as np
import matplotlib.pyplot as plt
import os

seed=1000

# This is for the number of faulty robots
tpr_boxplot_data = []
fpr_boxplot_data = []

fnr_boxplot_data = []
tnr_boxplot_data = []

tp_time_boxplot = []
fp_time_boxplot = []

true_positive_rates = []
true_negative_rates = []
false_postive_rates = []
false_negative_rates = []
latenciess = []

false_positive_timess = []
true_positive_timess = []
latencies = []


for iter in range(36):
    path = f'processed_data_{seed}.json'
    try:
        with open(path) as fp:
            data = json.load(fp)
    except FileNotFoundError:
        print(f"File {path} not found, skipping...")
        seed += 1
        continue

    # print(iter, len(data.keys()))
    # if len(data) > 20:
    #     seed += 1
    #     continue

    num_false_positive = 0
    num_true_negative = 0
    num_true_positives = 0
    num_false_negative = 0

    false_positive_times = []
    true_positive_times = []
    for robot_id in range(len(data)):
        robot_id = str(robot_id)
        line = data[robot_id]
        is_faulty = line['is_faulty']
        detected_time = line['time_found']
        percent_time_found = line['percent_time_found']
        injection_step_real = line['injection_step']

        true_positive = is_faulty and detected_time > 0
        false_postive = (not is_faulty) and detected_time > 0
        true_negative = (not is_faulty) and detected_time == 0
        false_negative = is_faulty and detected_time == 0
        
        if true_positive:
            num_true_positives += 1
            true_positive_times.append(percent_time_found)
            latencies.append(detected_time - injection_step_real)
        
        if false_postive:
            num_false_positive += 1
            false_positive_times.append(percent_time_found)

        if true_negative:
            num_true_negative += 1

        if false_negative:
            num_false_negative += 1

    true_positive_rate = num_true_positives / (num_true_positives + num_false_negative)
    true_negative_rate = num_true_negative / (num_false_positive + num_true_negative)
    false_postive_rate = num_false_positive / (num_false_positive + num_true_negative)
    false_negative_rate = num_false_negative / (num_false_negative + num_true_positives)

    true_positive_rates.append(true_positive_rate)
    true_negative_rates.append(true_negative_rate)
    false_postive_rates.append(false_postive_rate)
    false_negative_rates.append(false_negative_rate)

    latenciess.extend(latencies)
    true_positive_timess.extend(true_positive_times)
    false_positive_timess.extend(false_positive_times)

    seed += 1

tpr_boxplot_data.append(true_positive_rates)
fpr_boxplot_data.append(false_postive_rates)
fnr_boxplot_data.append(false_negative_rates)
tnr_boxplot_data.append(true_negative_rates)
tp_time_boxplot.append(true_positive_timess)
fp_time_boxplot.append(false_positive_timess)

# plt.boxplot(tpr_boxplot_data)
# # plt.title("True Positive Rate for Fault Detection with Increasing Number of Faults")
# plt.xlabel("Number of faults")
# plt.ylabel("True Positive Rate")
# plt.savefig(f'swarm-tpr.png', dpi=300)
# plt.clf()


# plt.boxplot(fpr_boxplot_data)
# # plt.title("False Positive Rate for Fault Detection with Increasing Number of Faults")
# plt.xlabel("Number of faults")
# plt.ylabel("False Positive Rate")
# plt.savefig(f'swarm-fpr.png', dpi=300)
# plt.clf()

# plt.boxplot(fnr_boxplot_data)
# # plt.title("False Negative Rate for Fault Detection with Increasing Number of Faults")
# plt.xlabel("Number of faults")
# plt.ylabel("False Negative Rate")
# plt.savefig(f'swarm-fnr.png', dpi=300)
# plt.clf()

# plt.boxplot(tnr_boxplot_data)
# # plt.title("True Negative Rate for Fault Detection with Increasing Number of Faults")
# plt.xlabel("Number of faults")
# plt.ylabel("True Negative Rate")
# plt.savefig(f'swarm-tnr.png', dpi=300)
# plt.clf()
    
# plt.boxplot(tp_time_boxplot)
# # plt.title("Percent Time of True Positives for Increasing Number of Faults")
# plt.xlabel("Number of faults")
# plt.ylabel("Percent Time of True Positives")
# plt.savefig(f'swarm-tp-time.png', dpi=300)
# plt.clf()

# plt.boxplot(fp_time_boxplot)
# # plt.title("Percent Time of False Positives for Increasing Number of Faults")
# plt.xlabel("Number of faults")
# plt.ylabel("Percent Time of False Positives")
# plt.savefig(f'swarm-fp-time.png', dpi=300)
# plt.clf()



plt.boxplot(np.array(latencies) / 10)
# plt.title("False Positive Rate for Fault Detection with Increasing Number of Faults")
plt.xlabel("Swarm Only")
plt.ylabel("Latency in True Positive Detection")
plt.savefig(f'swarm-latency.png', dpi=300)
plt.clf()
