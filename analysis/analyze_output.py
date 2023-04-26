from collections import namedtuple
import numpy as np
import sys
import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET
import pandas as pd

Dataline = namedtuple("Dataline", "time robot tolerators attackers")

def parse_votes(token):
	voters = []
	for subtoken in token.split(' ')[1:]:
		if subtoken == '-1' or subtoken == '' or subtoken == "\n":
			break
		voters.append(int(subtoken))
	return voters

def process_dataline(line):
	"""
	Processes a raw dataline into a dict of useful information
	"""

	if len(line.split('\t')) != 5:
		return None # there is no data here 

	
	tokens = line.split('\t')
	time = int(tokens[0].split(' ')[1])
	robot = int(tokens[1].split(' ')[1])
	tolerators = parse_votes(tokens[3])
	attackers = parse_votes(tokens[4])

	data = Dataline(time, robot, tolerators, attackers)

	return data


def process_file(file):
	"""
	Returns a list of processed datalines for the given file
	"""

	# Read in the data from the nohup.txt file
	lines = []
	with open(file, 'r') as datafile:
		for line in datafile.readlines():
			processed = process_dataline(line)
			if processed is not None:
				lines.append(processed)
	return lines

def experiment_details(file):
	"""
	Returns the number of robots and faulty robot information from the XML file
	"""
	
	root = ET.parse(file)
	run = root.find('.//experiment_run')
	id_faulty = run.get('id_faulty_robot')
	injection_step = run.get('injection_step')

	num_robots = root.find('arena').find('distribute').find('entity').get('quantity')

	seed = root.find('.//experiment').get('random_seed')

	return id_faulty.split(' '), injection_step.split(' '), num_robots, seed



def time_sus(data: list[Dataline], num_robots):
	"""
	Returns the proportion of time the robot is considered faultly for each robot

	Robot is considered faulty if a majority of other robots considers it faulty, and back to normal when no one thinks so
	"""
	total_faulty_time = np.zeros(num_robots, dtype=int)
	currently_faulty = np.zeros(num_robots, dtype=int)
	declared_faulty_time = np.zeros(num_robots, dtype=int)
	first_faulty_time = np.zeros(num_robots, dtype=int)

	max_time = data[-1].time
	min_time = data[0].time

	for line in data:
		is_comm_timestep = int(str(line.time)[-2:]) >= 90

		if not is_comm_timestep:
			continue

		robot_ind = line.robot
		sus = len(line.attackers) > len(line.tolerators) # and len(line.attackers) > 5
		antisus = not sus #len(line.attackers) <= len(line.tolerators) and len(line.tolerators) > 5

		# Case 1: Neighbors now think you are faultly
		if sus and not currently_faulty[robot_ind]:
			currently_faulty[robot_ind] = 1
			declared_faulty_time[robot_ind] = line.time
			print(f"Robot {robot_ind} declared fault at {line.time}")
			if first_faulty_time[robot_ind] == 0:
				first_faulty_time[robot_ind] = line.time

		# # Case 2: Neighbors no longer think you are faulty
		elif antisus and currently_faulty[robot_ind]:
			currently_faulty[robot_ind] = 0
			total_faulty_time[robot_ind] = total_faulty_time[robot_ind] + (line.time - declared_faulty_time[robot_ind])
			declared_faulty_time[robot_ind] = 0
			print(f"Robot {robot_ind} declared safe at {line.time}")

	for robot, faulty in enumerate(list(currently_faulty)):
		if faulty:
			total_faulty_time[robot] = total_faulty_time[robot] + (max_time - declared_faulty_time[robot])




	return total_faulty_time / (max_time - min_time), first_faulty_time


def detailed_analysis_file(exp_file, nohup_file, dest=''):
	id_faulty, injection_step, num_robots, seed = experiment_details(exp_file)
	num_robots = int(num_robots)
	time_data, first_times = time_sus(process_file(nohup_file), num_robots)

	injection_steps = np.zeros(num_robots)
	injection_steps[np.array(id_faulty, dtype=int)] = np.array(injection_step)
	data = {
		'is_faulty': [str(robot) in id_faulty for robot in range(num_robots)],
		'injection_step' : injection_steps,
		'time_found' : first_times,
		'percent_time_found' : time_data
	}
	df = pd.DataFrame.from_dict(data, 'index')
	df.to_json(dest+f'processed_data_{seed}.json')














# if __name__ == "__main__":
	# if len(sys.argv) < 3:
	# 	raise Exception("usage python analysis_output.py <num_robots> <path>")
    
	# data = process_file(sys.argv[2])
	# time_faulty = time_sus(data, int(sys.argv[1]))
	# print(time_faulty)

	# tf15 = []
	# for i in range(20):
	# 	exp = str(i+1)
	# 	data = process_file('original_data/SWARM_FORAGING/FAULT_ACTUATOR_LWHEEL_SETZERO/nohup_' + exp*3)
	# 	time_faulty, _ = time_sus(data, 20)
	# 	print(time_faulty)
	# 	tf15.append(time_faulty[15])
	
	# plt.boxplot(tf15)
	# plt.ylim(0, 1)
	# plt.show()


        