#!/usr/bin/env python

#---------------------------------------------
# Scan chain implementation v3
# Soumik Sarkar, Raunak Raj Gupta
# WEL Lab, EE Dept. , IIT Bombay

# Credits : Deepak Bhat, designer of v1
#			Titto Thomas, designer of v2
#---------------------------------------------


import os

# Environment variables for debugging
#os.environ['PYUSB_DEBUG'] = 'debug'
#os.environ['LIBUSB_DEBUG'] = '3'

import usb.core
import usb.util
import sys
import time
import subprocess
import array
import struct

print("Scan chain v3.0\nWadhwani Electronics Laboratory, IIT Bombay\n")


# Check for the proper input format
if len(sys.argv) != 3 :
	sys.stdout.write("Error : The correct format is sudo python scan.py <svf file> <output file> \n")
	sys.exit(1)
else:
	input_file = open(sys.argv[1],"r")
	output_file = open(sys.argv[2],"w")
	output_file.write("Expected Output    Received Output   Remarks\n")
	output_file.write("============================================\n")



svf_commands = ["FREQUENCY", "TRST", "ENDDR", "ENDIR", "STATE", "SDR", "SIR", "RUNTEST"]
svf_frequency_arguments = ["HZ"]
svf_trst_arguments = ["ON", "OFF", "Z", "ABSENT"]
svf_enddr_arguments = ["IRPAUSE", "DRPAUSE", "RESET", "IDLE"]
svf_endir_arguments = ["IRPAUSE", "DRPAUSE", "RESET", "IDLE"]
svf_state_path_arguments = ["RESET", "IDLE", "DRSELECT", "DRCAPTURE", "DRSHIFT", 
	"DREXIT1", "DRPAUSE", "DREXIT2", "DRUPDATE", "IRSELECT", "IRCAPTURE", 
	"IRSHIFT", "IREXIT1", "IRPAUSE", "IREXIT2", "IRUPDATE"]
svf_state_stable_arguments = ["IRPAUSE", "DRPAUSE", "RESET", "IDLE"]
svf_sdr_arguments = ["TDI", "TDO", "MASK", "SMASK"]
svf_sir_arguments = ["TDI", "TDO", "MASK", "SMASK"]
svf_runtest_arguments = ["TCK" , "ENDSTATE"]

jtag_curr_state = 0
reset_type = -1
frequency = 0
dr_end_state = svf_state_path_arguments.index("IDLE")
ir_end_state = svf_state_path_arguments.index("IDLE")

sdr_mask = ""
sdr_smask = ""
sdr_mask_bytearray = bytearray.fromhex('00')
sdr_smask_bytearray = bytearray.fromhex('00')

sir_mask = ""
sir_smask = ""
sir_mask_bytearray = bytearray.fromhex('00')
sir_smask_bytearray = bytearray.fromhex('00')

jtag_run_state = svf_state_path_arguments.index("IDLE")

def JTAG_start_program_mode():
	# Writing J will put the TIVA into program state
	#To exit, you have to write 10 into it(as first
	#character of trasnmission)
	print("start program mode")
	
	
def JTAG_set_state(state):
	cmd = array.array('B', [0,state])
	print(cmd)

def JTAG_chage_state(curr_state, next_state):
	cmd = array.array('B', [1,curr_state,next_state])
	print(cmd)

def JTAG_shift_ir(ir_length, ir_addr):
	a = [ir_length & 0xff]
	#b = [(ir_addr >> i & 0xff) for i in (0,8,16,24)]
	cmd = array.array('B', [2] + a + ir_addr)
	print(cmd)
	
def JTAG_read_dr(dr_length, dr_num_bytes):
	cmd = array.array('B', [3] + dr_length + dr_num_bytes)
	print(cmd)
	
def JTAG_runtest(clock_num_times):
	a = [(clock_num_times >> i & 0xff) for i in (0,8,16,24)]
	cmd = array.array('B', [4] + a)
	print(cmd)
	
def JTAG_read_write_dr(dr_length, data, data_expected, num_bytes):
	a = [(dr_length >> i & 0xff) for i in (0,8)]
	cmd = array.array('B', [5] + a + data_expected + num_bytes + data )
	print(cmd)

def JTAG_set_frequency(frequency):
	a = [(frequency >> i & 0xff) for i in (0,8,16,24)]
	cmd = array.array('B', [5] + a)
	print(cmd)
		
def JTAG_exit_program_mode():
	cmd = array.array('B', [3])
	print(cmd)

def parse_svf(file):
	global sir_mask, sir_smask, sir_mask_bytearray, sdr_mask, sdr_smask, sdr_mask_bytearray, sir_smask_bytearray, sdr_smask_bytearray
	lines = file.readlines()
	line_no = 0
	for line in lines:
		line_no +=1
		#lines marked with ! are comments
		if line[0] != '!':
			line_contents = line.split(' ')
			#checking integrity of file. all lines end with ;
			if not (';' in line_contents[-1]):
				print("Error in svf file at line no " + str(line_no))
			#first word of line is the command
			command = line_contents[0]
			args = []
			for text in line_contents:
				if text != command:
					if not (';' in text):
						args.append(text)
					else:
						args.append(text[:-2])
			
			#specifies the largest frequency the clock can support
			if command == "FREQUENCY" :
				if len(args) > 2 or len(args) < 2:
					 print("Error in svf file at line no " + str(line_no))
				else :
					frequency = int(float(args[0]))
					print("Frequency : " + str(frequency))
					JTAG_set_frequency(frequency)
				
			#ON, OFF, Z, and ABSENT are the valid trst_mode states	
			elif command == "TRST" :
				if len(args) > 1 or len(args) < 1:
					 print("Error in svf file at line no " + str(line_no))
				else :
					reset_type = svf_trst_arguments.index(args[0])
					print("Reset Type : " + str(reset_type))
			
			#specifies the state of the TAP controller after every
			#SDR or SIR instruction
			#ON, OFF, Z, and ABSENT are the valid trst_mode states	
			elif command == "ENDDR" :
				if len(args) > 1 or len(args) < 1:
					 print("Error in svf file at line no " + str(line_no))
				else :
					dr_end_state = svf_state_path_arguments.index(args[0])
					print("End DR State : " + svf_state_path_arguments[dr_end_state])
					
			elif command == "ENDIR" :
				if len(args) > 1 or len(args) < 1:
					 print("Error in svf file at line no " + str(line_no))
				else :
					ir_end_state = svf_state_path_arguments.index(args[0])
					print("End IR State : " + svf_state_path_arguments[ir_end_state])
			
			#moves the TAP controller from one stable state to another
			#might move through a number of other states on the way
			elif command == "STATE" :
				if len(args) < 1:
					 print("Error in svf file at line no " + str(line_no))
				else :
					for i in range(0,len(args)):
						print(i)
						if i == 0:
							JTAG_set_state(svf_state_path_arguments.index(args[0]))
							jtag_curr_state = svf_state_path_arguments.index(args[0])
						else :
							JTAG_chage_state(svf_state_path_arguments.index(args[i-1]),
								svf_state_path_arguments.index(args[i]))
							jtag_curr_state = svf_state_path_arguments.index(args[i])
								
			elif command == "SDR" :
				sdr_length = 0
				sdr_tdi = ""
				sdr_tdo = ""
				data_expected = []
				sdr_tdo_bytearray = bytearray.fromhex('00')
				sdr_tdi_bytearray = bytearray.fromhex('00')
				sdr_tdo_masked = []
				sdr_tdi_masked = []
				
				if len(args) < 3:
					 print("Error in svf file at line no " + str(line_no))
				else :
					sdr_length = int(args[0])
					if "MASK" in args:
						sdr_mask = args[args.index("MASK") + 1][1:-1]
					if "SMASK" in args:
						sdr_smask = args[args.index("SMASK") + 1][1:-1]
					if "TDI" in args:
						sdr_tdi = args[args.index("TDI") + 1][1:-1]
					if "TDO" in args:
						sdr_tdo = args[args.index("TDO") + 1][1:-1]
				print(line)
				print("Length " + str(sdr_length))
				print("TDI " + str(sdr_tdi))
				print("TDO" + str(sdr_tdo))
				print("Mask " + str(sdr_mask))
				print("Smask" + str(sdr_smask))
				
				if sdr_mask!= "" :
					if len(sdr_mask) % 2 !=0 :
						sdr_mask = '0' + sdr_mask
					sdr_mask_bytearray = bytearray.fromhex(sdr_mask)
					sdr_mask_bytearray[::-1]
						
				if sdr_smask!= "" :
					if len(sdr_smask) % 2 !=0 :
						sdr_mask = '0' + sdr_smask
					sdr_smask_bytearray = bytearray.fromhex(sdr_smask)
					sdr_smask_bytearray = sdr_smask_bytearray[::-1]
						
				if sdr_tdo != "" :
					if len(sdr_tdo) % 2 !=0 :
						sdr_tdo = '0' + sdr_tdo					
					sdr_tdo_bytearray = bytearray.fromhex(sdr_tdo)
					sdr_tdo_bytearray= sdr_tdo_bytearray[::-1]
					data_expected = [1]
				else:
					data_expected = [0]
					
				if sdr_tdi != "" :
					if len(sdr_tdi) % 2 !=0 :
						sdr_tdi = '0' + sdr_tdi					
					sdr_tdi_bytearray = bytearray.fromhex(sdr_tdi)
					sdr_tdi_bytearray= sdr_tdi_bytearray[::-1]
					
				for i in range(0,len(sdr_tdo_bytearray)):
					sdr_tdo_masked.append(sdr_tdo_bytearray[i] & sdr_mask_bytearray[i])
						
				for i in range(0,len(sdr_tdi_bytearray)):
					sdr_tdi_masked.append(sdr_tdi_bytearray[i])
						
				print(sdr_tdo_masked)
				print(sdr_tdi_masked)
				
				JTAG_chage_state(jtag_curr_state, svf_state_path_arguments.index("DRSHIFT"))
				JTAG_read_write_dr(sdr_length, sdr_tdi_masked, data_expected, [len(sdr_tdo_masked)])
				JTAG_chage_state(svf_state_path_arguments.index("DREXIT1"),dr_end_state)
				jtag_curr_state = dr_end_state
				
			elif command == "SIR" :
				
				sir_length = 0
				sir_tdi = ""
				sir_tdo = ""
				data_expected = 0
				sir_tdo_bytearray = bytearray.fromhex('00')
				sir_tdi_bytearray = bytearray.fromhex('00')
				sir_tdo_masked = []
				sir_tdi_masked = []
				if len(args) < 3:
					 print("Error in svf file at line no " + str(line_no))
				else :
					sir_length = int(args[0])
					if "MASK" in args:
						sir_mask = args[args.index("MASK") + 1][1:-1]
					if "SMASK" in args:
						sir_smask = args[args.index("SMASK") + 1][1:-1]
					if "TDI" in args:
						sir_tdi = args[args.index("TDI") + 1][1:-1]
					if "TDO" in args:
						sir_tdo = args[args.index("TDO") + 1][1:-1]
				print(line)
				print("Length " + str(sir_length))
				print("TDI " + str(sir_tdi))
				print("TDO" + str(sir_tdo))
				print("Mask " + str(sir_mask))
				print("Smask" + str(sir_smask))
				
				if sir_mask!= "" :
					if len(sir_mask) % 2 !=0 :
						sir_mask = '0' + sir_mask
					sir_mask_bytearray = bytearray.fromhex(sir_mask)
					sir_mask_bytearray[::-1]
						
				if sir_smask!= "" :
					if len(sir_smask) % 2 !=0 :
						sir_mask = '0' + sir_smask
					sir_smask_bytearray = bytearray.fromhex(sir_smask)
					sir_smask_bytearray = sir_smask_bytearray[::-1]
						
				if sir_tdo != "" :
					if len(sir_tdo) % 2 !=0 :
						sir_tdo = '0' + sir_tdo				
					sir_tdo_bytearray = bytearray.fromhex(sir_tdo)
					sir_tdo_bytearray= sdr_tio_bytearray[::-1]
					data_expected = 1
				else:
					data_expected = 0
					
				if sir_tdi != "" :
					if len(sir_tdi) % 2 !=0 :
						sir_tdi = '0' + sir_tdi					
					sir_tdi_bytearray = bytearray.fromhex(sir_tdi)
					sir_tdi_bytearray= sir_tdi_bytearray[::-1]
					
				for i in range(0,len(sir_tdo_bytearray)):
					sir_tdo_masked.append(sir_tdo_bytearray[i] & sir_mask_bytearray[i])
				
				for i in range(0,len(sir_tdi_bytearray)):
					sir_tdi_masked.append(sir_tdi_bytearray[i])
						
				print(sir_tdo_masked)
				print(sir_tdi_masked)
				
				JTAG_chage_state(jtag_curr_state, svf_state_path_arguments.index("IRSHIFT"))
				JTAG_shift_ir(sir_length, sir_tdi_masked)
				JTAG_chage_state(svf_state_path_arguments.index("IREXIT1"),ir_end_state)
				jtag_curr_state = ir_end_state
			
			elif command == "RUNTEST" :
				run_count = 0
				run_clk = 0
				min_time = 0
				max_time = 0
				end_state = 0
				
				if len(args) < 2:
					 print("Error in svf file at line no " + str(line_no))
				else :
					if args[0] in svf_state_path_arguments:
						jtag_run_state = svf_state_path_arguments.index(args[0])
					if "TCK" in args:
						run_clk = 0
						run_count = int(args[args.index("TCK") - 1])
					if "SCK" in args:
						run_clk = 1
						run_count = int(args[args.index("SCK") - 1])
					if "SEC" in args:
						min_time = float(args[args.index("SEC") - 1])
					if "MAXIMUM" in args:
						max_time = float(args[args.index("MAXIMUM") + 1])
					if "ENDSTATE" in args:
						end_state = svf_state_path_arguments.index(args[args.index("ENDSTATE") + 1])
				print(line)	
				print("Run State " + str(jtag_run_state))
				print("Run Count " + str(run_count))
				print("Run Clock " + str(run_clk))
				print("Min TIme " + str(min_time))
				print("Max Time " + str(max_time))
				print("End State " + str(end_state))	
				
				if run_clk == 0 :
					JTAG_chage_state(jtag_curr_state, jtag_run_state)
					JTAG_runtest(run_count)
					
				if end_state != "":
					JTAG_chage_state(jtag_run_state, end_state)
				
		
parse_svf(input_file)
