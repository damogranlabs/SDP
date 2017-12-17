# SPD Firmware guidelines
This is python module used to communicate on one of computer serial ports.
Tested with PC stm32 on port COM5 with baud rate 115200.

Example code is the best place to start exploring how this module works.

## Instructions how to setup & use
1. Import sdp module in your script.

2. Create message handler function.
Do not forget to send response on every correctly received message.

3. Create and initiane node serial interface:
```
ser_node = sdp.SDP_serial()   # serial initialisation
ser_status = ser_node.serial_init('COM5', 115200)
sdp_node = sdp.SDP(message_handler, ser_node, 0, 50)    # node with ID = 0
```

4. Init sdp node with serial node and start message parser (enable receiver):
```
sdp_node.status()
sdp_node.enable_receiver()
```
Optionally, set timeouts with: `sdp_node.s.set_timeouts()`

5. Send & receive data: `(status, response) = sdp_node.send_data(data)`

6. Note: sdp module supports printing debug informations. Turn on/off:
```
SDP_DEBUG = True
SDP_DEBUG_IO_DATA = False
```