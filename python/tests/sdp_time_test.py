# -*- coding: utf-8 -*-
"""
Simple Data Protocol example
Date: 9.11.2017
@author: Domen Jurkovic
@version 1.0
@source  http://damogranlabs.com/, https://github.com/damogranlabs

# & C:/Python27/python.exe "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/sdp_time_test.py"
# & "C:/Program Files (x86)/Python36-32/python.exe" "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/sdp_time_test.py"
# python "d:/Stromarija/Damogran Labs/Simple Data Protocol/python/sdp_time_test.py"

"""

import os
import sys
import time as systime
import timeit

import cProfile

#import sdp_cleaned as sdp
import sdp

PC_NODE_ID = 0
# mayimum of 50 bytes per payload - must match with firmware code
PC_NODE_MAX_PAYLOAD = 50


def message_handler(d):
    """
    User message handler. Data is correctly received (checked with CRC).

    Note: unused since in this example configuration, PC is master
    so microcontroller does not send data except as response.
    To test this function, modify firmware to send data and check for response.
    """
    sdp_node.send_response([0, 1, 2, 3])

    sys.stdout.write("Message handling function: %s" % d)
    # print('Message handling function: %s' %d)


ser_node = sdp.SDP_serial()   # serial initialisation
#ser_status = ser_node.serial_init('COM9', 115200)
#ser_status = ser_node.serial_init('/dev/ttyACM0', 115200)
ser_status = ser_node.serial_init('/dev/ttyUSB0', 115200)
sdp_node = sdp.SDP(message_handler, ser_node, PC_NODE_ID,
                   PC_NODE_MAX_PAYLOAD)    # node with ID = 0

data = ['C']
#data = ['C'] * 40
AVERAGE = 20


def to_time_it():
    (status, response) = sdp_node.send_data(data)
    #cProfile.run('sdp_node.send_data(data)', sort='tottime')
    
    if not status:
        print("ERROR: response not received: %s" % response)
    else:
        print("OK")


def main():
    print("Simple Data Protocol v%s system response time test script\nwww.damogranlabs.com" %
          sdp.__version__)

    if not ser_status:
        print("Serial initialisation error.")
        return

    if sdp_node.status():
        if not sdp_node.enable_receiver():
            print("Can't start SDP receiver thread!")
            sdp_node.s
            return
    else:
        print("Serial node status error!")
        return

    sdp_node.s.set_timeouts(0.5, 0.5)   # rx = 0.5 sec, tx = 0.2 sec
    sdp_node.set_response_timeout(0.7)
    max_time = 0
    take = 0
    avg = 0

    # 0x04D8
    # 0x00DF


    print("\nResponse timing test start:")

    while take <= AVERAGE:
        take = take + 1

        tajm = timeit.timeit(to_time_it, number=1) * 1000

        if tajm > max_time:
            max_time = tajm
        else:
            avg = avg + tajm

        #print("\t%s Execution time: %.2f ms" % (take - 1, tajm))

    avg_time = avg / AVERAGE
    print("\n\tAverage time: %.2f ms" % avg_time)
    print("\tMax time: %.2f ms" % max_time)

    sdp_node.disable_receiver()
    sdp_node.s.serial_port.close()

    print("\nSDP receiver closed -> over & out")


if __name__ == '__main__':
    main()
    # os._exit(1)
    #cProfile.run('main()', filename=None, sort='tottime')
