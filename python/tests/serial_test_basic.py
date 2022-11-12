# -*- coding: utf-8 -*-
"""
Simple Data Protocol example
Date: 9.11.2017
@author: Domen Jurkovic
@version 1.0
@source  http://damogranlabs.com/, https://github.com/damogranlabs

# & C:/Python27/python.exe "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/serial_test_basic.py"
# & "C:/Program Files (x86)/Python36-32/python.exe" "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/serial_test_basic.py"
# python "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/serial_test_basic.py"

python2 "/home/plejr/Desktop/sdp/serial_test_basic.py"
python3 "/home/plejr/Desktop/sdp/serial_test_basic.py"

"""

import cProfile
import os
import sys
import time as systime
import timeit

import threading
import serial

serial_port = serial.Serial()

Flag = False
status = True

def serial_init():
    """
    Init serial port (8, N, 1 start, 1 stop)
    Returns status of port (True = open, False = close)
    """
    status = False

    #win
    serial_port.port = 'COM6'   # cp210x board
    #serial_port.port = 'COM9'  # mcp2200
    
    #linux
    #serial_port.port = '/dev/ttyACM0' # mcp2200 board
    #serial_port.port = '/dev/ttyUSB0' # cp210x board
    
    serial_port.baudrate = 115200
    serial_port.parity = serial.PARITY_NONE
    serial_port.stopbits = serial.STOPBITS_ONE
    serial_port.bytesize = serial.EIGHTBITS
    serial_port.xonxoff = False  # disable software flow control
    # disable hardware (RTS/CTS) flow control
    serial_port.rtscts = False
    # disable hardware (DSR/DTR) flow control
    serial_port.dsrdtr = False
    serial_port.timeout = 0.5
    serial_port.write_timeout = 0.5

    try:
        serial_port.close()
        serial_port.open()

        serial_port.reset_input_buffer()
        serial_port.reset_output_buffer()

        status = serial_port.is_open
    
    except Exception as e:
        print("Port can\'t be opened, error:", str(e))

    return status

AVERAGE = 10

def to_time_it():
    
    global status
    status = False
    
    d_out = bytearray(1)
    d_out[0] = ord('C')
    
    serial_port.write(d_out)

    while serial_port.in_waiting == 0:
        pass

    d_in = serial_port.read(serial_port.in_waiting)

    print("Data in: %s" %d_in)



def main():
    global Flag

    print("Serial test - system response time")

    if not serial_init():
        print("Serial initialisation error.")
        return

    max_time = 0
    take = 0
    avg = 0
    tajm = 0

    print("\nResponse timing test start:")

    while take < AVERAGE:
        
        tajm = timeit.timeit(to_time_it, number=1) * 1000
        #to_time_it()

        avg = avg + tajm
        if tajm > max_time:
            max_time = tajm

        #print("\t%s Execution time: %.2f ms" % (take - 1, tajm))

        take = take + 1
    
    avg_time = avg / AVERAGE
    print("\n\tAverage time: %.2f ms" % avg_time)
    print("\tMax time: %.2f ms" % max_time)

    systime.sleep(15)

    serial_port.close()

    print("\nover & out")

if __name__ == '__main__':
    main()
    # os._exit(1)
    #cProfile.run('main()', filename=None, sort='tottime')
