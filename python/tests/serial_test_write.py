# -*- coding: utf-8 -*-
"""
Simple Data Protocol example
Date: 9.11.2017
@author: Domen Jurkovic
@version 1.0
@source  http://damogranlabs.com/, https://github.com/damogranlabs

& C:/Python27/python.exe "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/serial_test_write.py"
& "C:/Program Files (x86)/Python36-32/python.exe" "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/serial_test_write.py"
python "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/serial_test_write.py"

& "C:/Program Files (x86)/Python36-32/Scripts/ipython.exe" "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/serial_test_write.py"

python2 "/home/plejr/Desktop/sdp/serial_test_write.py"
python3 "/home/plejr/Desktop/sdp/serial_test_write.py"

"""

import cProfile
import os
import sys
import time as systime
import timeit

import threading
import serial

RX_THREAD_SLEEP = 0.3

serial_port = serial.Serial()

rx_thread_flag = False
data_received_flag = False

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

def thread_it():
    global rx_thread_flag
    global data_received_flag

    while not rx_thread_flag:   #on rx_thread_flag=true, end this thread
        if serial_port.in_waiting != 0:
            data_received_flag = True

            
            #systime.sleep(RX_THREAD_SLEEP)
        #print("!")

AVERAGE = 10

def to_time_it():
    global data_received_flag

    #d_out = bytearray(1)
    #d_out[0] = ord('C')
    
    data_received_flag = False

    #serial_port.write(d_out)
    serial_port.write('cdefghijklmnoprst'.encode())

    while not data_received_flag:
        #print(".")
        #systime.sleep(0)
        pass

    
    while not data_received_flag:
        #print(".")
        #systime.sleep(0)
        pass

    d_in = serial_port.read_all()   #read(serial_port.in_waiting)
    
    print("Data in: %s" %d_in)
    

def main():
    global rx_thread_flag
    global data_received_flag

    print("Serial test - system response time")

    if not serial_init():
        print("Serial initialisation error.")
        return

    th = threading.Thread(target=thread_it)
    th.start()

    max_time = 0
    take = 0
    avg = 0
    tajm = 0

    print("\nResponse timing test start:")

    while take < AVERAGE:
        
        tajm = timeit.timeit(to_time_it, number=1) * 1000
        
        #to_time_it()
        #tajm = 0

        avg = avg + tajm
        if tajm > max_time:
            max_time = tajm

        #print("\t%s Execution time: %.2f ms" % (take - 1, tajm))

        take = take + 1
    
    rx_thread_flag = True   # end serial_read thread

    avg_time = avg / AVERAGE
    print("\n\tAverage time: %.2f ms" % avg_time)
    print("\tMax time: %.2f ms" % max_time)

    serial_port.close()

    print("\nSDP receiver closed -> over & out")

if __name__ == '__main__':
    main()
    # os._exit(1)
    #cProfile.run('main()', filename=None, sort='tottime')
