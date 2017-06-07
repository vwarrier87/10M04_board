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

print"Scan chain v3.0\nWadhwani Electronics Laboratory, IIT Bombay\n"


# Check for the proper input format
if len(sys.argv) != 4 :
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
	output_file = open(sys.argv[2],"w")
	device = sys.argv[3]
	output_file.write("Expected Output    Received Output   Remarks\n")
	output_file.write("============================================\n")

#----------------- Connecting to device -------------------------------
print "Initiating connection with the device.."
timeout = 1000
sleeptime = 0 #0.0005 = 0.5ms
line_num = 0
success_checks = 0
success_achieved = 0
outvector_verify = 0

if (device == 'ptx'):
  ## Check if pid and vid are as follows using the command "lsusb"
	productId = 0x2402
	vendorId = 0x03eb
	inEndPoint = 0x02
	outEndPoint = 0x81
elif (device == 'tiva'):
	productId = 0x0003
	vendorId = 0x1cbe
	inEndPoint = 0x01
	outEndPoint = 0x81
else:
	print "Error: Invalid device name. Valid device names: ptx/tiva"
	sys.exit(1)

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

def JTAG_readregister(addr):
	dev.write(inEndPoint, 'Q', timeout)
	time.sleep(sleeptime)
	inn = dev.read(outEndPoint, 40, timeout) 
	if (inn[0] == 100):
		print "JTAG Reg fn entered!!!"
		print("Data returned is :")
		print(inn)	
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
	
	dev.write(inEndPoint, 'P', timeout)
	time.sleep(sleeptime)
	inn = dev.read(outEndPoint, 16, timeout) 
	if (inn[0] == 100):
		print "JTAG fn entered!!!"
		print("No of devices is :")
		print(inn)	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)

	dev.write(inEndPoint, 'I', timeout)
	time.sleep(sleeptime)
	inn = dev.read(outEndPoint, 40, timeout) 
	if (inn[0] == 100):
		print "JTAG ID fn entered!!!"
		print("The device ID is :")
		print(inn)	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)
	
	dev.write(inEndPoint, 'Q', timeout)
	time.sleep(sleeptime)
	inn = dev.read(outEndPoint, 40, timeout) 
	if (inn[0] == 100):
		print "JTAG Reg fn entered!!!"
		print("No of devices is :")
		print(inn)	
	else:
		print "Something is wrong with JTAG!!!"
		sys.exit(1)

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
