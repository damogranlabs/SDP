# -*- coding: utf-8 -*-
"""
Simple Data Protocol example
Date: 9.11.2017
@author: Domen Jurkovic
@version 1.0
@source  http://damogranlabs.com/, https://github.com/damogranlabs

Once started, press 's' to send data and press ESC to break out.

"""

import msvcrt
import sys

import sdp

PC_NODE_ID = 0
# mayimum of 50 bytes per payload - must match with firmware code
PC_NODE_MAX_PAYLOAD = 50


def message_handler(data):
    """
    User message handler. Data is correctly received (checked with CRC).
    
    Note: unused since in this example configuration, PC is master 
    so microcontroller does not send data except as response.
    To test this function, modify firmware to send data and check for response.
    """
    sdp_node.send_response([0, 1, 2, 3])

    sys.stdout.write('Message handling function: %s' % data)
    #print('Message handling function: %s' %d)


ser_node = sdp.SDP_serial()   # serial initialisation
ser_status = ser_node.serial_init('COM5', 115200)
sdp_node = sdp.SDP(message_handler, ser_node, PC_NODE_ID,
                   PC_NODE_MAX_PAYLOAD)    # node with ID = 0


def main():
    print('Simple Data Protocol v%s test script\nwww.damogranlabs.com' %
          sdp.__version__)

    if not ser_status:
        print('Serial initialisation error.')
        return

    if sdp_node.status():
        if not sdp_node.enable_receiver():
            print('Can\'t start SDP receiver thread!')
            return
    else:
        print('Serial node status error!')
        return

    sdp_node.s.set_timeouts(0.7, 0.2)   # rx = 0.5 sec, tx = 0.2 sec

    print("\nHit key \'s\' to send data and \'ESC\' to quit.\n")

    while True:
        if msvcrt.kbhit():

            key = ord(msvcrt.getch())
            if key == 27:   # escape

                sdp_node.disable_receiver()
                sdp_node.s.serial_port.close()
                break

            if key == ord('s'):  # send
                # this data is sent. Should be returned
                data = range(50)
                #data = ['a', 'b', 'c', 'd']

                (status, response) = sdp_node.send_data(data)
                if status:
                    print("\nStatus OK")
                    print("\tData sent: %s" % data)

                    print("\tData received: %s" % response)

                else:
                    print("Status != OK. Check SDP debug information")

    print('SDP receiver closed\nover & out')


if __name__ == '__main__':
    main()
