# SDP
Simple Data Protocol - data link protocol for embedded systems and python bindings

Date: 08-Nov-2017
Author:  Domen Jurkovic
Version: v1
Source:  http://damogranlabs.com/
              https://github.com/damogranlabs

## About SDP - Simple Data Protocol
Main idea of this protocol is that user can send data between diferent "nodes" using simple functions, 
while protocol will make sure the correct data is delivered and if not, retry or give up and report that to user.

It was primary designed for smaller embeded systems and payloads (up to 255 bytes per payload/message) to use with 
UART (specifically STM32 LL UART drivers), but can be easily ported to other communication hardware. 
Tested for speeds up to 115200 bps.
Although each node can initiate communication to the other end, it was primary meant for Master-Slave point-to-point 
communication, where master poll other node for any new available data (or user must take care of possible 
data collision and timing, or use other technique to notify other system for available data - like unused CTS/RTS line).

Protocol check each payload data with CRC-16 and responds with ACK (+ optional user defined respond). If CRC values does not match,
NACK  is returned. If no response is received, this is considered as error and protocol retry.

Note: In many ways SDP is very simillar to MIN (https://github.com/min-protocol/min) which was discovered after this library was already created. SDP is simpler and more straight forward therefore simpler to modify and customize. MIN has some other nice features which are lacking in SDP, but is more complex.

##. More info:
- Each communication port is called "node". One uC can communicate on more than one port, so more nodes can be initialised.
  Each node must have its own rx and tx port. 
- Communication peripheral initialisation (UART in this case) is done outside of this library. Hardware/driver must allow to
  receive 1 byte in interrupt mode (like RX not empty ISR) where bytes are put in node's rx buffer. User must take care of 
  all enabled interrupts and flags. (There are guidelines below on how to configure library).
- Rx buffer is handled in sdp_parse_rx_data(), which should be polled frequently to handle messages and response to sender asap.
  Messages, if valid, are passed to sdp_user_handle_message() where user can handle it.
- Framing: payloads up to 255 bytes (or more with customisation).
  '''
  SOF | ACK | PAYLOAD (+DLE) (+CRC) | EOF
  '''
- ACK field is used for acknowledgement of correctly received data and retransmission process. After payload CRC check:
  - CRC OK - ACK == 0x00
  - CRC data error - NACK != 0x00
  Important note: ACK field is used of internal retransmission of the packet and is not for aplication level error reporting. 
        Aplication/invalid data errors should be implemented in higher layer, merged into payload by user.
        
For even more information explore "firmware" and "python" subfolders.
