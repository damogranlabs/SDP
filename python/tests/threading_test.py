# -*- coding: utf-8 -*-
"""
Simple Data Protocol example
Date: 9.11.2017
@author: Domen Jurkovic
@version 1.0
@source  http://damogranlabs.com/, https://github.com/damogranlabs

& C:/Python27/python.exe "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/threading_test.py"
& "C:/Program Files (x86)/Python36-32/python.exe" "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/threading_test.py"
python "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/threading_test.py"

& "C:/Program Files (x86)/Python36-32/Scripts/ipython.exe" "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/threading_test.py"

python2 "/home/plejr/Desktop/sdp/threading_test.py"
python3 "/home/plejr/Desktop/sdp/threading_test.py"

"""

import cProfile
import os
import sys
import time as systime
import timeit

import threading
import serial

THREAD_SLEEP = 0.005

flag = False

ser = serial.Serial() 
ser.port = 'COM6'
ser.open()
print(ser.is_open)
ser.timeout = 0.001

def thread_it():
    global flag
    while True:   #on rx_thread_flag=true, end this thread
        print("!")
        if ser.in_waiting > 0:
            flag = True
        #systime.sleep(THREAD_SLEEP)
        

AVERAGE = 10

def to_time_it():
    global flag

    flag = False

    ser.write('cdefghijklmnoprst'.encode())

    systime.sleep(0.02)

    while not flag:
        #print("")
        pass

    print(ser.read(ser.in_waiting))

    print("Data in: OK")


def main():

    print("Serial test - system response time")

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

    avg_time = avg / AVERAGE
    print("\n\tAverage time: %.2f ms" % avg_time)
    print("\tMax time: %.2f ms" % max_time)

    os._exit(1)

if __name__ == '__main__':
    main()
    #os._exit(1)
    #cProfile.run('main()', filename=None, sort='tottime')
