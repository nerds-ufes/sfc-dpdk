#!/usr/bin/python

from helper import *
from time import sleep, time
import numpy as np
import re
import matplotlib.pyplot as plt
import collections
import os
import Gnuplot, Gnuplot.funcutils


def gnuplot_bwm(in_file1, in_file2, in_file3, in_file4, title, out_file,  
				key="off", xmin=-5, xmax=75, line=3):
	g = Gnuplot.Gnuplot(debug=0)
	g("set terminal postscript eps enhanced color font 'Helverica,18'")
	#g("set terminal pngcairo size 350,262 enhanced font 'Verdana,10'")
	g("set output '%s.eps'" % out_file)
	g("set multiplot")
	g("set datafile commentschars '#%'")
	g("set encoding utf8")
	g("set key %s" % key)			# legenda
	#g("set grid")
	g("set xrange [%s:%s]" % (xmin, xmax))
	g("set yrange [%s:%s]" % (0, 450))
	g("set xtics 10")
	g("set style line 1 lc rgb '#0060ad' lt 1 lw 2 ps 0.5")   # --- blue
	g("set style line 2 lc rgb '#dd181f' lt 1 lw 2 ps 0.5")   # --- red
	g("set style line 3 lc rgb '#09ad00' lt 1 lw 2 ps 0.5")   # --- green
	g("set style line 4 lc rgb '#edc951' lt 1 lw 2 ps 0.5")   # --- yellow
	g("set title '%s' font 'Helverica,22'" % title)
	g("set datafile separator ','")
	g("set ylabel 'Throughput (Mbps)'")
	g("set xlabel 'Time (s)'")

	g("plot '%s' using 1:2 with linespoints ls %s" % (in_file1, 2))
	g("plot '%s' using 1:2 with linespoints ls %s" % (in_file2, 1))
	g("plot '%s' using 1:2 with linespoints ls %s" % (in_file3, 3))
	g("plot '%s' using 1:2 with linespoints ls %s" % (in_file4, 4))
	g.reset()
	
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
		result.append("%s, %s, %s" % (i, s, intconf95))
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


def parse_cpu_files(dirname, filenames, filemask, target="all"):
	data = dict()
	for filename in sorted(filenames):
		m = re.match(filemask, filename)
		if m:
		#~ if filemask in filename:
			filepath = os.path.join(dirname, filename)
			datafile = read_list(filepath)
			c_target = 1	# column with cpu
			c_data 	 = 10 	# column with total bytes
			row_no   = 1	# row number
			for row in datafile:
				try:			
					if row[c_target] == target:
						rate = 100.0 - float(row[c_data])
						if row_no not in data:
							data[row_no] = []	# create empty list
						data[row_no].append(rate)
						row_no += 1
				except:
					pass
	return data

	

def process_bwm_files(dirname, filenames, filetype, graphtitle):
	filemask = "%s.bwm" % filetype
	if "phy" in filemask:
		data = parse_bwm_files(dirname, filenames, filemask, "total")
	elif "vm" in filemask:
		data = parse_bwm_files(dirname, filenames, filemask, "eth0")

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
		title = "%s_%s.bwm" % (dirname.strip().split('/')[1], graphtitle)
		print ("\n\nProcessing %s using %s files" % (title, len(data[1])))
		#~ print X
		#~ print Y
		calc_statistics("SFC/plot_files/%s" % title, Y)
		#~ plt.title("Rate %s" % title )
		#~ plt.ylabel("Mbps")
		#~ plt.xlabel("Time")
		#~ plt.gcf().subplots_adjust(bottom=0.18, left=0.1, right=0.97, top=0.93)
		#~ plt.grid(True)
		#~ figure.savefig("plot/%s" % title, format="png", dpi = 200)
		#~ plt.show()

	
def process_cpu_files(dirname, filenames, filetype, graphtitle):
	filemask = "%s.cpu" % filetype
	data = parse_cpu_files(dirname, filenames, filemask, "all")	

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
		title = "%s_%s.cpu" % (dirname.strip().split('/')[1], graphtitle)
		print ("\n\nProcessing %s using %s files" % (title, len(data[1])))
		#~ print X
		#~ print Y
		calc_statistics("SFC/plot_files/%s" % title, Y)
		#~ plt.title("CPU %s" % title )
		#~ plt.ylabel("%")
		#~ plt.xlabel("Time")
		#~ plt.gcf().subplots_adjust(bottom=0.18, left=0.1, right=0.97, top=0.93)
		#~ plt.grid(True)
		#~ figure.savefig("plot/%s" % title, format="png", dpi = 200)
		#~ plt.show()

def main():
	hosts    = [ "source", 
				 "dest",
				 "sff1",
				 "sff2"]
	#hosts = ["node-"]
	dirmask = "SFC/all"
	
	
	for (dirname, dirnames, filenames) in os.walk('SFC/all'):
		#~ if "results" in dirname:
			#~ continue
		#~ elif dirmask in dirname:
		if dirmask in dirname:
			for host in hosts:
				print "oi"
				#~ filetype = "vm-%s-rx" % (host)
				#~ graphtitle = "vm-%s-rx" % (host)
				#~ filetype = "((a\d+)(-vm-%s\d*)(-rx))" % (host)
				#~ process_bwm_files(dirname, filenames, filetype, graphtitle)
				#~ process_cpu_files(dirname, filenames, filetype, graphtitle)

				#~ filetype = "vm-%s-tx" % (host)
				#~ graphtitle = "vm-%s-tx" % (host)
				#~ filetype = "((a\d+)(-vm-%s\d*)(-tx))" % (host)
				#~ process_bwm_files(dirname, filenames, filetype, graphtitle)
				#~ process_cpu_files(dirname, filenames, filetype, graphtitle)

				#~ filetype = "phy-%s" % (host)
				graphtitle = "phy-%s" % (host)
				filetype = "((a\d+)(-vm-%s.*))" % (host)
				#print filetype
				#print filenames
				#print dirname
				process_bwm_files(dirname, filenames, filetype, graphtitle)
				#process_cpu_files(dirname, filenames, filetype, graphtitle)
				

####################
if __name__ == '__main__':
	title = "Iperf test between two machine in the same host"
	#gnuplot_bwm(in_file1='plot_files/test1_phy-source.bwm', in_file2='plot_files/test1_phy-sff1.bwm', in_file3='plot_files/test1_phy-sff2.bwm', in_file4='plot_files/test1_phy-dest.bwm', title=title, out_file='test1')
	#gnuplot_bwm(in_file='plot_files/test1_phy-sff1.bwm', title=title, out_file='test1_phy-sff1')
	#gnuplot_bwm(in_file='plot_files/test1_phy-sff2.bwm', title=title, out_file='test1_phy-sff2')
	#gnuplot_bwm(in_file='plot_files/test1_phy-source.bwm', title=title, out_file='test1_phy-source')
	main()
