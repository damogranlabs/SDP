
import os
import sys
import time as systime
import timeit

import cProfile

import sdp_cleaned as sdp

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
    #sdp_node.send_response([0, 1, 2, 3])

    sys.stdout.write("Message handling function: %s" % d)
    # print('Message handling function: %s' %d)

ser_node = sdp.SDP_serial()   # serial initialisation
    ser_status = ser_node.serial_init('COM8', 115385)
    sdp_node = sdp.SDP(message_handler, ser_node, PC_NODE_ID, PC_NODE_MAX_PAYLOAD)    # node with ID = 0

def main():
    

cProfile.run('main()', filename=None, sort='tottime')
