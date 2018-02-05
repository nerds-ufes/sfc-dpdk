#!/usr/bin/python

from helper import *
from time import sleep, time
import numpy as np
import re
import matplotlib.pyplot as plt
import collections
import os
import sys
	
def save_toFile(fname, data):
	myfile = open(fname, "w")
	myfile.writelines(["%s\n" % item  for item in data])
	
def calc_statistics(fname, data):
	print "Data antes: "
	print data
	mean = np.mean(data)  			# media
	std = np.std(data)				# desvio padrao
	var = np.var(data)				# variancia
	se =  std / np.sqrt(len(data))	# erro padrao
	intconf95 = 1.96 * se			# margem de erro para int. conf 95%
	amin = np.amin(data)			# minimo
	amax = np.amax(data)			# maximo
	result = []						# resutlado
	for i, s in enumerate(data):
		result.append("%s, %s" % (i, s))
	#~ print result
	save_toFile(fname, result)
	#~ print "%s,+- %s" % (mean, intconf95)


def parse_bwm_files(dirname, filenames, filemask, target):
	row = []
	data = dict()
	for filename in sorted(filenames):
		m = re.match(filemask, filename)
		if m:
		#~ if filemask in filename:
			filepath = os.path.join(dirname, filename)
			datafile = open(filepath,"r").read().split('\n')
			#datafile = read_list(filepath, ',')
			c_target  = 1	# column with interfaces
			c_data    = 4 	# column with total bytes
			row_no    = 1	# row number
			#print datafile
			for row_d in datafile:
				row = row_d.split(',')
				print row
				try:			
					if row[c_target] in [target]:
						rate = float(row[c_data]) * 8 / (1 << 20) # Mbps
						if row_no not in data:
							data[row_no] = []	# create empty list
						data[row_no].append(rate)
						row_no += 1
						#print data
				except:
					break
	print data
	return data	

def process_bwm_files(dirname, filenames, filetype, graphtitle):
	filemask = "%s.bwm" % filetype
	if "vm" in filemask:
		data = parse_bwm_files(dirname, filenames, filemask, "en0")

	if len(data) > 0:
		values = []
		keys = []
		od = collections.OrderedDict(sorted(data.items()))
		for key, value in od.iteritems():
			keys.append(key)
			values.append(np.mean(value))
			
		figure = plt.figure()
		X = np.arange(len(keys))
		Y = values
		width = 0.5
		plt.plot(X,Y)
		if not os.path.exists("results"):
			os.makedirs("results")
		title = "results/%s_%s.bwm" % (dirname, graphtitle)
		print ("\n\nProcessing %s using %s files" % (title, len(data[1])))
		calc_statistics("%s" % title, Y)

def main():
	hosts    = [ "source", "dest", "sff1", "sff2"]
	dirmask = sys.argv[1]
	
	for (dirname, dirnames, filenames) in os.walk(dirmask):
		if dirmask in dirname:
			for host in hosts:
				graphtitle = "vm-%s" % (host)
				filetype = "((a\d+)(-vm-%s.*))" % (host)
				process_bwm_files(dirname, filenames, filetype, graphtitle)
				

####################
if __name__ == '__main__':
	main()
