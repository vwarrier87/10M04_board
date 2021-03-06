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

print"Scan chain v3.0\nWadhwani Electronics Laboratory, IIT Bombay\n"


# Check for the proper input format
if len(sys.argv) != 2 :
	sys.stdout.write("Error : The correct format is sudo python scan.py <input file>")
	sys.stdout.write(" <output file> <ptx/tiva>\n")
  	sys.stdout.write("<input file>: Input file with all the commands.\n")
  	sys.stdout.write("<output file>: New or Existing Output file. ")
  	sys.stdout.write("If it is existing, all the content will be overwritten.\n")
  	sys.stdout.write("<ptx/tiva>: Type of micro-controller being used.\n")
  	sys.stdout.write("For more information, refer to the User Manual.\n")
	sys.exit(1)
else:
	input_file = open(sys.argv[1],"r")
	#output_file = open(sys.argv[2],"w")
	#device = sys.argv[3]
	#output_file.write("Expected Output    Received Output   Remarks\n")

#----------------- Connecting to device -------------------------------
print "Initiating connection with the device.."
timeout = 1000
sleeptime = 0 #0.0005 = 0.5ms
line_num = 0
success_checks = 0
success_achieved = 0
outvector_verify = 0

#if (device == 'ptx'):
  ## Check if pid and vid are as follows using the command "lsusb"
#	productId = 0x2402
#	vendorId = 0x03eb
#	inEndPoint = 0x02
#	outEndPoint = 0x81
#elif (device == 'tiva'):
productId = 0x0003
vendorId = 0x1cbe
inEndPoint = 0x01
outEndPoint = 0x81
#else:
#	print "Error: Invalid device name. Valid device names: ptx/tiva"
#	sys.exit(1)

# find our device
dev = usb.core.find(idVendor=vendorId, idProduct = productId)
# was it found?
if dev is None:
	raise ValueError('Error : Device not found')

print "Device found.."
# set the active configuration. With no arguments, the first configuration 
# will be the active one
print "Please wait, Setting it's configuration... "
print "Done !"
print "Claiming interface.."
# Note: The code below is for interface 0, check what interface is used

#claim the device
if dev.is_kernel_driver_active(0) is True:
	dev.detach_kernel_driver(0)

try:
	dev.set_configuration()
	usb.util.claim_interface(dev, 0)
except:
	# defence against 'Ctrl+Z' keyboard interrupt
	for i in range(10):
		dev.reset()
	print "\nCould not claim interface ... Processes running in the background."
	print "RUN the command AGAIN. It may work."
	sys.exit(1)
  # The following is not important in this program, but gives additional
  # information on the issue:
	# 1. Ctrl+Z pauses the currently running program, and puts it in a list.
  # 2. running 'jobs' in bash shell lists the jobs where you pressed Ctrl+Z
  # 3. note the job id of the commands
  # 4. run 'fg %1' in bash shell	(replace the number '1' with the job id) to 
  #		run the pending command

print "Connection established.\n"

#--------------------- Hex input to binary conversion function ---------------
def toBin(value, bits):
    value = bin(int(value,16))[2:]
    while ( len(value) < bits ):
    	value = '0' + value
    while ( len(value) > bits ):
    	value = value[1:]   
    return value

#--------------------- Hex input to byte array conversion function -----------
# pad with zero if odd bits
def toByte(value, bits):
    temp_value = value
    if (len(temp_value) % 2 != 0):
    	# if odd number of bits found, pad front with '0'
    	temp_value = '0'+temp_value
    outvalue = ''
    for i in range(0, len(temp_value), 2):
        outvalue += chr(int(temp_value[i]+temp_value[i+1], 16))
    return outvalue

#--------------------- Byte array input to hex conversion function -----------
def toHex(valuearray):
    hexvalue = ''	
    for each in valuearray:
	    hexvalue += str(hex(each)[2:].zfill(2))	
	    # zfill fills leading zeros if they are not there
    return hexvalue    


#-- Function to read DUT output from uC and cross-check with expected output --
def checkDUTOutput(out_sample, last_mask, last_expct_out):
	global success_checks, success_achieved
	dev.write(inEndPoint,'S',timeout)
	time.sleep(sleeptime)
	print "Sampling out data.."
	bit_num = 0
	invalid = 0
	if (device == 'ptx'):
		inn = dev.read(outEndPoint, 16, timeout)  
		# The uC transfers in packets of 8bytes, so there should always be 
		# a multiple of 8 here
		inn = inn [0:out_sample]
		inn = inn[::-1]
		inn = toBin(toHex(inn), out_pins)
		for mask_bit in last_mask:
			if ( mask_bit == '1' ):
				if (inn[bit_num] != last_expct_out[bit_num]):
					invalid = 1
			bit_num += 1
	else:
		inn = dev.read(outEndPoint,out_sample,timeout)
		inn = "".join("%01X" % b for b in inn) #converts decimal inn list to 
		#new inn small x for small letters and X for capital letters
		for mask_bit in last_mask:
			if ( mask_bit == 'F' ):
				if (inn[bit_num] != last_expct_out[bit_num]):
					invalid = 1
			bit_num += 1
#earlier code. unchanged 	
	success_checks += 1
	if ( invalid == 0 ):
		result = "Success"
		success_achieved += 1
	else:
		result = "Failure"


#code adder by varun to replace success/failiure evaluation code
	if ( last_expct_out == inn ):
		result = "Success"
		success_achieved += 1
	else:
		result = "Failure"
		
	print "Output Comparison : ", result
	file_data = last_expct_out + "    " + inn + "   " + result+"\n"
	output_file.write(file_data)

#--------------------- Function to send DUT inputs to the uC ---------------
def sendDUTInput(in_pins, out_pins, data_in):
	if (device == 'ptx'):
		#Form the string to be sent to the microcontroller
		command_out = 'L'+chr(in_pins)+chr(out_pins)+data_in
		#print "Sending",in_pins,out_pins,"bit input data",data_in
		for single in list(command_out):
		 	dev.write(inEndPoint, single, timeout)
			time.sleep(sleeptime)
	else:	
		dev.write(inEndPoint,'L',timeout)
		dev.write(inEndPoint,chr(in_pins),timeout)
		dev.write(inEndPoint,chr(out_pins),timeout)
		dev.write(inEndPoint,data_in,timeout)
		
		
		

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
jtag_run_endstate = svf_state_path_arguments.index("IDLE")

def parse_svf(file):
	global sir_mask, sir_smask, sir_mask_bytearray, sdr_mask, sdr_smask, sdr_mask_bytearray, \
	sir_smask_bytearray, sdr_smask_bytearray, jtag_curr_state, jtag_run_endstate
	lines = file.readlines()
	line_no = -1
	while line_no < len(lines) -1:
		line_no +=1
		#lines marked with ! are comments
		line = lines[line_no]
		if lines[line_no][0] != '!':
			line_contents = lines[line_no].split(' ')
			#checking integrity of file. all lines end with ;
			while not (';' in line):
				line_no +=1
				line += lines[line_no].replace("\t", "")
				#print("Error in svf file at line no " + str(line_no))
			#first word of line is the command
			if line.find("\n") > -1 :
				line = line.replace("\n", "")
			line_contents = line.split(' ')
			command = line_contents[0]
			args = []
			for text in line_contents:
				if text != command:
					if not (';' in text):
						args.append(text)
					else:
						args.append(text[:-1])
			
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
					print(line)
					for i in range(0,len(args)):
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
				if len(line) < 500:
					print(line)
					#print("Length " + str(sdr_length))
					#print("TDI " + str(sdr_tdi))
					#print("TDO " + str(sdr_tdo))
					#print("Mask " + str(sdr_mask))
					#print("Smask" + str(sdr_smask))
				else:
					print("Length " + str(sdr_length))
					print("line too long")
				
				
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
					data_expected = 1
				else:
					data_expected = 0
					
				if sdr_tdi != "" :
					if len(sdr_tdi) % 2 !=0 :
						sdr_tdi = '0' + sdr_tdi					
					sdr_tdi_bytearray = bytearray.fromhex(sdr_tdi)
					sdr_tdi_bytearray= sdr_tdi_bytearray[::-1]
					
				for i in range(0,len(sdr_tdo_bytearray)):
					sdr_tdo_masked.append(sdr_tdo_bytearray[i] & sdr_mask_bytearray[i])
						
				for i in range(0,len(sdr_tdi_bytearray)):
					sdr_tdi_masked.append(sdr_tdi_bytearray[i])
						
				#print("Masked TDO : " + str(sdr_tdo_masked))
				#print("Masked TDI : " + str(sdr_tdi_masked))
				JTAG_chage_state(jtag_curr_state, svf_state_path_arguments.index("DRSHIFT"))
				eom=0
				count_left = round(len(sdr_tdi_masked)/50, 0)
				while len(sdr_tdi_masked) > 50:
					count_left -= 1
					print("Count left : " + str(count_left))
					if data_expected == 0:
						JTAG_write_dr(400, sdr_tdi_masked[:50], [eom])
					else :
						sdo_read = JTAG_read_write_dr(400, sdr_tdi_masked[:50], [data_expected], [50], [eom])
						for i in range(0,len(sdo_read)):
							if sdr_tdo_masked[i] != sdo_read[i] :
								print("Error in received data")
						
					sdr_tdi_masked = sdr_tdi_masked[50:]
					sdr_tdo_masked = sdr_tdo_masked[50:]
					sdr_length -= 400
				
				eom=1
				if data_expected == 0 :
					JTAG_write_dr(sdr_length, sdr_tdi_masked, [eom])
				else :
					JTAG_read_write_dr(sdr_length, sdr_tdi_masked, [data_expected], [len(sdr_tdo_masked)], [eom])
				JTAG_chage_state(svf_state_path_arguments.index("DREXIT1"),dr_end_state)
				jtag_curr_state = dr_end_state
				#jtag_curr_state = svf_state_path_arguments.index("DREXIT1")
				
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
				#print("Length " + str(sir_length))
				#print("TDI " + str(sir_tdi))
				#print("TDO" + str(sir_tdo))
				#print("Mask " + str(sir_mask))
				#print("Smask" + str(sir_smask))
				
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
						
				#print("Masked TD0 : " + str(sir_tdo_masked))
				#print("Masked TDI : " + str(sir_tdi_masked))
				
				JTAG_chage_state(jtag_curr_state, svf_state_path_arguments.index("IRSHIFT"))
				JTAG_shift_ir(sir_length, sir_tdi_masked)
				JTAG_chage_state(svf_state_path_arguments.index("IREXIT1"),ir_end_state)
				jtag_curr_state = ir_end_state
				#jtag_curr_state = svf_state_path_arguments.index("IREXIT1")
			
			elif command == "RUNTEST" :
				run_count = 0
				run_clk = 0
				min_time = 0
				max_time = 0
				
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
						jtag_run_endstate = svf_state_path_arguments.index(args[args.index("ENDSTATE") + 1])
				print(line)	
				#print("Run State " + str(jtag_run_state))
				#print("Run Count " + str(run_count))
				#print("Run Clock " + str(run_clk))
				#print("Min TIme " + str(min_time))
				#print("Max Time " + str(max_time))
				#print("End State " + str(jtag_run_endstate))	
				
				if run_clk == 0 :
					JTAG_chage_state(jtag_curr_state, jtag_run_state)
					count_left = round(run_count/50000,0)
					while run_count > 50000:
						count_left -=1
						print("Count left : " + str(count_left))
						JTAG_runtest(50000)
						run_count -= 50000
					JTAG_runtest(run_count)
					
				if jtag_run_endstate != -1:
					JTAG_chage_state(jtag_run_state, jtag_run_endstate)
				

def JTAG_start_program_mode():
	# Writing J will put the TIVA into program state
	#To exit, you have to write 10 into it(as first
	#character of trasnmission)

	dev.write(inEndPoint, 'J', timeout)
	inn = dev.read(outEndPoint, 16, timeout) 
	if (inn[0] == 100):
		print("Program mode entered")
		print(inn)	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
	
def JTAG_set_state(state):
	cmd = array.array('B', [0,state])
	dev.write(inEndPoint, cmd, timeout)
	inn = dev.read(outEndPoint, 16, timeout) 
	if (inn[0] == 100):
		if(inn[1] == state):
			print("New State entered : " + str(state) + "\n")	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)

def JTAG_chage_state(curr_state, next_state):
	cmd = array.array('B', [1,curr_state,next_state])
	dev.write(inEndPoint, cmd, timeout)
	inn = dev.read(outEndPoint, 4, timeout) 
	if (inn[0] == 100):
		if(inn[1] == next_state):
			print("State Changed from " + str(curr_state) + " to " + str(next_state) + "\n")	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)

def JTAG_shift_ir(ir_length, ir_addr):
	a = [(ir_length & 0xff)]
	#b = [(ir_addr >> i & 0xff) for i in (0,8,16,24)]
	cmd = array.array('B', [2] + a + ir_addr + [0,0])
	print(cmd)
	dev.write(inEndPoint, cmd, timeout)
	inn = dev.read(outEndPoint, 16, timeout) 
	if (inn[0] == 100):
		print("IR Shifted\n")
		#print(inn)	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
		
def JTAG_write_dr(dr_length, data, eom):
	a = [(dr_length >> i & 0xff) for i in (0,8)]
	cmd = array.array('B', [6] + a +  eom + data )
	print("Write DR command: " + str(cmd))
	dev.write(inEndPoint, cmd, timeout)
	inn = dev.read(outEndPoint, 16, timeout) 	
	if (inn[0] == 100):
		print("DR written to\n")
		print(inn)	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
	return(inn)

def JTAG_read_dr(dr_length, dr_num_bytes):
	cmd = array.array('B', [3] + dr_length + dr_num_bytes)
	dev.write(inEndPoint, cmd, timeout)
	inn = dev.read(outEndPoint, dr_num_bytes[0]+5, timeout) 
	if (inn[0] == 100):
		print("DR Shifted. Data is :")
		print(inn)	
		print("\n")
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
		
		
def JTAG_read_write_dr(dr_length, data, data_expected, num_bytes, eom):
	a = [(dr_length >> i & 0xff) for i in (0,8)]
	cmd = array.array('B', [5] + a + data_expected + num_bytes + eom + data)
	print("Read Write DR command: " + str(cmd))
	dev.write(inEndPoint, cmd, timeout)
	if data_expected[0] == 1:
		inn = dev.read(outEndPoint, num_bytes[0]+16, timeout) 
	else :
		inn = dev.read(outEndPoint, 2, timeout) 
	
	if (inn[0] == 100):
		print("DR Shifted. Data is :")
		print(inn)	
		print("\n")
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
	return(inn[6:])
	
def JTAG_runtest(clock_num_times):
	a = [(clock_num_times >> i & 0xff) for i in (0,8,16,24)]
	cmd = array.array('B', [4] + a)
	print("Runtest command: " + str(cmd))
	dev.write(inEndPoint, cmd, timeout)
	inn = dev.read(outEndPoint, 16, timeout) 
	if (inn[0] == 100):
		print("Runtest ran")
		print(inn)	
		print("\n")
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
	return(inn)
		
def JTAG_set_frequency(frequency):
	a = [(frequency >> i & 0xff) for i in (0,8,16,24)]
	cmd = array.array('B', [5] + a)
	print("Set frequency command: " + str(cmd))
	#dev.write(inEndPoint, cmd, timeout)
	#inn = dev.read(outEndPoint, dr_num_bytes[0]+5, timeout) 
	#if (inn[0] == 100):
	#	print("New frequency set")
	#	#print(inn)	
	#else:
	#	print "Something is wrong with JTAG!!!"
	#	sys.exit(1)

def JTAG_exit_program_mode():
	cmd = array.array('B', [9])
	print("Exit program command: " + str(cmd))
	dev.write(inEndPoint, cmd, timeout)
	time.sleep(sleeptime)
	inn = dev.read(outEndPoint, 16, timeout) 
	if (inn[0] == 100):
		print("Exited program mode")
		print("TIVA in default mode now");
		#print(inn)	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
		


dev.reset()

try:
	# defence against 'Ctrl+C' keyboard interrupt
	# check for uC response
	
	dev.write(inEndPoint, 'T', timeout)
	time.sleep(sleeptime)
	inn = dev.read(outEndPoint, 16, timeout) 
	if (inn[0] == 170):
		print "Ready to roll!!!"	
	else:
		print "Something is wrong. Wrong output from uC!!!"
		sys.exit(1)
		

	
	#jtag
	JTAG_start_program_mode()

	JTAG_set_state(1)
	#state set to IRSHIFT
	JTAG_chage_state(1,11)
	
	#write 6 to IR
	JTAG_shift_ir(10, [6,0])

	#change state from IREXIT1 to DRSHIFT
	JTAG_chage_state(12,4)

	#read 32 bits + return char from DR
	JTAG_read_dr([32,0], [4])
	

	JTAG_set_state(0)
	
	print("Parsing SVF File \n\n\n")
	print(time.ctime())
	
	parse_svf(input_file)
	print(time.ctime())
	print("Parsed SVF File \n\n\n")
	
	print(jtag_curr_state)
	
	#JTAG_chage_state(ir_end_state,11)
	#JTAG_chage_state(jtag_curr_state,4)

	#read 32 bits + return char from DR
	#JTAG_read_dr([32,0], [4])
	

	#exit program state
	JTAG_exit_program_mode()

except:
	for i in range(30):
		dev.reset()
	print "Something is wrong. No response from uC!!!"
	print "RUN AGAIN."
	print "Still doesn't run? Press the RESET button on uC"
	sys.exit(1)	



# flush the scan chain
#dev.write(inEndPoint, 'Z', timeout)
#time.sleep(sleeptime)
#inn = dev.read(outEndPoint, 16, timeout) 
#if (inn[0] == 85):
#	print "Transaction Complete."
#else:
#	print "uC is not responding!!!"
