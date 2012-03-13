#imports
import string
import os
import re
import time
from subprocess import Popen
import subprocess
import sys

#test_modes = [5]
test_modes = [0,1,2,3,5]
number_of_reps = 5

for i in test_modes:
	sum_total = 0
	per_frame_total = 0.0
	for j in range(0, number_of_reps):
		mode = i
		app = os.getcwd() + "/FrameGrabber.exe"  
		#file_param = "C:\\Foreman.avi" + " mode=" + str(mode) 
		file_param = "C:\\Foreman.avi" 
		mode_param = "mode=" + str(mode)
		# execute process
		output_f = open('output.txt', 'w')
		return_code = subprocess.call([app, file_param, mode_param],stdout=output_f,
                     stderr=output_f)

		output_f.close()					 

		if not return_code == 0:
			print "Error occurred, exiting"
			sys.exit()
		# extract total time from text file
		temp_f = open('output.txt', 'r')
		content = temp_f.read()
		#print "Result: " + content
		temp_f.close()

		results = re.findall(r"[+-]? *(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?", content)
		total = float(results[1])
		per_frame = float(results[3])
		sum_total += total
		per_frame_total += per_frame
	average_total = sum_total/number_of_reps
	average_per_frame = per_frame_total/number_of_reps
	print "Mode: " + str(i) + " Total average: " + str(average_total) + "ms Per frame: " + str(average_per_frame) + "ms"
