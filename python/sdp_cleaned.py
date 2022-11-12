# -*- coding: utf-8 -*-
"""
Simple Data Protocol
Date: 9.11.2017
@author: Domen Jurkovic
@source  http://damogranlabs.com/, https://github.com/damogranlabs
"""
import sys
import threading
import time as systime

import crcmod
import serial

import timeit

__version__ = 1.2

##################################################################
# USER SETUP
SDP_DEBUG = True  # Set to False if no debug info must be implemented/reported using sdp_debug() + python print()
# Set to False if no read/write data info must be implemented/reported using python print()
SDP_DEBUG_IO_DATA = False

SDP_RETRANSMIT = 2  # number of retries in case of send/receive error
# [s] default timeout while receiving sdp frame (from received SOF character)
SDP_DEFAULT_RX_MSG_TIMEOUT = 0.3
# [s] default timeout while receiving one byte
SDP_DEFAULT_RX_BYTE_TIMEOUT = 0.3
# [s] default timeout while transmitting sdp frame
SDP_DEFAULT_TX_MSG_TIMEOUT = 0.3
# [s] default timeout while response must be received
SDP_DEFAULT_RESPONSE_TIMEOUT = 1
# [s] default wait time before retry with send_data()
SDP_DEFAULT_RETRANSMIT_DELAY = 0.1

SDP_CRC_POLYNOME = 0x18005  # 1 is there because of crcmod package. Polynome is 0x8005
# CRC-16:
#  -> https://www.lammertbies.nl/comm/info/crc-calculation.html
#  -> http://crcmod.sourceforge.net/crcmod.predefined.html
########################################################################################


class SDP_serial():
    """
    SDP node communication interface.
    User can use different communication interface by changing functions in
    SDP_serial class accordingly to description.
    """

    def __init__(self):
        self.serial_port = serial.Serial()

        self.rx_buff = []   # buffer where all incoming data are stored as FIFO

    def serial_init(self, port, baudrate, frame_read_timeout=None, frame_write_timeout=None):
        """
        Init serial port (8, N, 1 start, 1 stop)
        Returns status of port (True = open, False = close)
        """
        status = False

        self.serial_port.port = port
        self.serial_port.baudrate = baudrate
        self.serial_port.parity = serial.PARITY_NONE
        self.serial_port.stopbits = serial.STOPBITS_ONE
        self.serial_port.bytesize = serial.EIGHTBITS
        self.serial_port.xonxoff = False  # disable software flow control
        # disable hardware (RTS/CTS) flow control
        self.serial_port.rtscts = False
        # disable hardware (DSR/DTR) flow control
        self.serial_port.dsrdtr = False
        if frame_read_timeout:  # timeout for read [s]
            self.serial_port.timeout = SDP_DEFAULT_RX_BYTE_TIMEOUT
        else:
            self.serial_port.timeout = frame_read_timeout
        if frame_write_timeout:  # timeout for writing frame [s]
            self.serial_port.write_timeout = SDP_DEFAULT_TX_MSG_TIMEOUT
        else:
            self.serial_port.write_timeout = frame_write_timeout

        try:
            self.serial_port.close()
            self.serial_port.open()

            self.serial_port.reset_input_buffer()
            self.serial_port.reset_output_buffer()

            status = self.serial_port.is_open

            if SDP_DEBUG:
                print("SDP port %s status: %s" %(port, status))

        except Exception as e:
            if SDP_DEBUG:
                print("SDP port %s can\'t be opened, error:\n%s" %(port, str(e)))

        return status

    ########################################################################################
    def set_timeouts(self, rx_frame_timeout=None, tx_frame_timeout=None):
        """
        Set new timeouts for receiving/transmitting one frame.
        All arguments in seconds.
        Note: these values must be chosen accordingly to other node (receiver/transmitter)
        """
        if rx_frame_timeout:
            self.serial_port.timeout = rx_frame_timeout
        if tx_frame_timeout:
            self.serial_port.write_timeout = tx_frame_timeout

    ########################################################################################
    def serial_write(self, data):
        """
        Write data to this node serial port TX
        Returns True on success, false otherwise
        """
        if SDP_DEBUG_IO_DATA:
            print('SDP write: %s' % data)

        status = False

        if self.serial_port.is_open:  # only write to port if it is open
            try:
                num = self.serial_port.write(data)

                if len(data) == num:
                    status = True
                else:
                    print('incomplete write-out')

            except:
                if SDP_DEBUG:
                    print('try ... except fail')

        else:   # serial port is closed
            if SDP_DEBUG:
                print('Serial port is closed, no WRITE operaton ignored')

        return status

    def to_time_it(self):
        self.serial_port.in_waiting

    ########################################################################################
    def serial_read(self):
        """
        Reads all available data from serial port and append them in
        rx_buff as instance of bytes (return bytes(read) - according to pyserial)
        """
        status = False

        if self.serial_port.is_open:  # only write to port if it is open
            try:
                """
                tajm = timeit.timeit(self.to_time_it, number=1) * 1000
                if tajm > 0:
                    print("\t\t\ttimeit: %.2f ms" % tajm)
                """

                num = self.serial_port.in_waiting

                #s_time = systime.time()

                data = self.serial_port.read(num)

                """
                tajm = (systime.time() - s_time) *1000
                if tajm>0:
                    print("time: %f ms" % tajm)
                """

                # if any data is read
                if len(data) > 0:
                    if sys.version[0] >= '3':
                        self.rx_buff += data
                    else:
                        for d in data:  # append data to buffer, byte by byte
                            d = ord(d)
                            self.rx_buff.append(d)

                    if SDP_DEBUG_IO_DATA:
                        print('SDP read: %s' % list(data))

                status = True

            except Exception as e:
                if SDP_DEBUG:
                    print('SDP serial_read() fail:')
                    print(e)

        else:
            if SDP_DEBUG:
                print('Serial port is closed, no WRITE operaton ignored')

        return status

    ########################################################################################
    def get_rx_buff_byte(self):
        """
        Return first rx_buff byte if available as touple (status, byte). If no byte is available,
        False is returned
        """
        if len(self.rx_buff):
            byte = self.rx_buff.pop(0)  # get first byte

            return (True, byte)
        else:
            return (False, 0)


########################################################################################
SDP_ACK = 0x00  # data received OK
# data received ERROR (checked with CRC) - normally sdp_debug() error is sent back as NACK
SDP_NACK = 0xaa

""" Private SDP rx state machine states """
_SDP_RX_IDLE = 0  # waiting for start flag
_SDP_RX_ACK = 1  # waiting for ack field
_SDP_RX_RECEIVING = 2  # receiving data, waiting for end flag
_SDP_RX_DLE = 3  # end flag or DLE byte received
""" Special character definitions and lengths """
_SDP_SOF = 0x7E  # start byte of each frame
_SDP_EOF = 0x66  # END byte of each frame
# Data Link Escape - or Escape (avoid escaping of message if EOF shows up in the middle of data)
_SDP_DLE = 0x7D
_SDP_DLE_XOR = 0x20  # Whenever a flag or escape byte appears in the message, it is escaped by 0x7D and the byte itself is XOR-ed with 0x20. So, for example 0x7E becomes 0x7D 0x5E. Similarly 0x7D becomes 0x7D 0x5D. The receiver unsuffs the escape byte and XORs the next byte with 0x20 again to get the original
_SDP_SOF_SIZE = 1  # number of SOF bytes
_SDP_EOF_SIZE = 1  # number of EOF bytes
_SDP_ACK_SIZE = 1  # number of acknowledgement bytes
_SDP_CRC_SIZE = 2  # number of CRC bytes

_SDP_MAX_PAYLOAD = 255  # C library limitation, maximum payload bytes (<255)

_SDP_THREAD_STOP_TIMEOUT = 1  # [s] timeout when stopping parser thread


class SDP():
    """
    Init SDP  - Simple Data Protocol node.
    Serial interface must be intitalised before with SDP_serial() class.
    """

    def __init__(self, msg_handler, node_serial, node_id, max_payload):
        self.user_message_handler = msg_handler  # user message handler function
        self.s = node_serial  # node's serial port
        # node ID (relevant for debugging if more than one node is in use)
        self.id = node_id

        # each message can contain up to this number of bytes (without framing)(MAX = 256 bytes)
        if max_payload > _SDP_MAX_PAYLOAD:  # check if payload is larger than C library
            self.debug(
                'payload size - C library can handle payloads up to 255 bytes')
        # still use parameter max_payload - this library can be used as python <-> python communication
        self.max_payload_size = max_payload
        self.rx_frame_timeout = SDP_DEFAULT_RX_MSG_TIMEOUT
        self.tx_frame_timeout = SDP_DEFAULT_TX_MSG_TIMEOUT
        self.response_timeout = SDP_DEFAULT_RESPONSE_TIMEOUT

        # user can read
        self.ack = SDP_ACK
        self.rx_payload = []

        # private variables
        self._expect_response = False
        self._rx_state = _SDP_RX_IDLE
        self._rx_start_time = 0
        # payload worst case = *2 - if every byte of payload is special character, escaped with DLE
        self._max_frame_size = (_SDP_SOF_SIZE + _SDP_ACK_SIZE +
                                self.max_payload_size * 2 + _SDP_CRC_SIZE * 2 + _SDP_EOF_SIZE)

        self.crc16 = crcmod.mkCrcFun(SDP_CRC_POLYNOME, initCrc=0, rev=False)

        # thread that receives and parses data
        self.parser_thread = threading.Thread(
            target=self._receive_parse_thread)
        self._thread_stop_flag = False

    ########################################################################################
    def set_response_timeout(self, response_timeout):
        """
        Override default SDP timeout for receiving response frame [in seconds].
        These value must be chosen accordingly to other node (receiver/transmitter).
        """
        self.response_timeout = response_timeout

    ########################################################################################
    def status(self):
        """
        Return True if node's serial port is open, False otherwise
        """
        try:
            status = self.s.serial_port.is_open
        except:
            self.disable_receiver()
            status = False

        return status

    ########################################################################################
    def enable_receiver(self):
        """
        Enable receiver - start thread which is reading all incoming data and store them in
        rx_buffer so parser can build messages. Performs serial input/output data flush.
        Returns True on success, False otherwise
        """
        if self.status():   # is serial port open?
            if not self.parser_thread.is_alive():   # is thread already ongoing?
                self.s.serial_port.reset_input_buffer()
                self.s.serial_port.reset_output_buffer()
                self.parser_thread.start()
                self._thread_stop_flag = False
                if SDP_DEBUG:
                    print('SDP parser thread started.')

            return True
        else:   # serial port is not open
            self.debug('SDP serial port is not open')
            return False

    ########################################################################################
    def disable_receiver(self):
        """
        Disable receiver - thread which is reading all incoming data and store them in
        rx_buffer so parser can build messages.
        Returns True on success, False otherwise
        """
        self._thread_stop_flag = True

        if self.parser_thread.is_alive():   # is thread already ongoing?
            start_time = systime.time()
            while self._thread_stop_flag:
                if systime.time() > (start_time + _SDP_THREAD_STOP_TIMEOUT):
                    # timeout
                    return False
                pass    # wait while thread is not over

        return True

    ########################################################################################
    def _receive_parse_thread(self):
        """
        Call parse_rx_data() to handle all available data. Checks for thread terminate flag.
        """
        # This function is run by SDP "parser_thread", check for exit/terminate flag
        while not self._thread_stop_flag:
            self.parse_rx_data()

        self._thread_stop_flag = False  # reset flag
        if SDP_DEBUG:
            print('SDP Parser thread over.')

    ########################################################################################
    def parse_rx_data(self):
        """
        Read all available data from serial port, store them in rx_buffer and parse (un-frame)
        all bytes. When frame is received and parsed, this function calls message handler.
        Called from parser_thread().
        """
        self.s.serial_read()  # read all available data from serial port

        if len(self.s.rx_buff):  # if rx buffer is not empty
            if self._rx_state == _SDP_RX_IDLE:
                self._search_for_sof()

            elif self._rx_state == _SDP_RX_ACK:
                self._search_for_ack()

            elif self._rx_state == _SDP_RX_RECEIVING:
                self._append_new_data()
                self._rx_frame_timeout()

            elif self._rx_state == _SDP_RX_DLE:
                self._check_if_eof()
                self._rx_frame_timeout()

            else:
                self.debug(50)
                self._rx_state = _SDP_RX_IDLE

        else:   # buffer is empty
            # if frame reception is in progress, check timeout
            if self._rx_state != _SDP_RX_IDLE:
                self._rx_frame_timeout()

########################################################################################
    def send_data(self, payload):
        """
        Transmit data and wait for response. Retry if neccessary.
        Return status and received response (array of bytes).
        """
        if not self.status():  # check if serial port is opened
            self.debug('serial port is not open')
            return (False, [])

        # check if data elements fit in byte (0 - 255)
        (status, payload) = self._convert_data(payload)
        if not status:
            self.debug('invalid payload data')
            return (False, [])

        retransmit_count = 0
        while retransmit_count < SDP_RETRANSMIT:
            (status, frame) = self._compose_frame(payload)
            if status:
                if self._transmit_data(frame):

                    response_timeout = systime.time() + self.response_timeout
                    self._rx_state = _SDP_RX_IDLE
                    self._expect_response = True

                    while self._expect_response:
                        print("alo")

                        # all incoming data are parsed in parser thread
                        if systime.time() > response_timeout:  # check for response timeout
                            # response not received in time
                            self.debug('timeout expecting reseponse')
                            break

                    if not self._expect_response:  # parser cleared flag - response received
                        if self.ack == SDP_ACK:
                            return (True, self.rx_payload)  # success
                        else:
                            # response received, but CRC validation failed
                            self.debug('CRC validation failure')
                            # failure, retry

                            # delay to avoid receiver overrun
                            systime.sleep(SDP_DEFAULT_RETRANSMIT_DELAY)

                    # else:  parser didn't clear expect_response flag, reseponse not received in time

                else:  # frame transmission unsuccessful
                    self.debug('transmission failure (take %s' %
                               (retransmit_count + 1))
                    # retry
                    systime.sleep(SDP_DEFAULT_RETRANSMIT_DELAY)

            else:  # frame composition error
                self.debug('frame composition')
                return (False, [])

            retransmit_count = retransmit_count + 1

        return (False, [])  # loop didn't return while executing, error occured

    ########################################################################################
    def send_response(self, payload):
        """
        Compose frame with given payload, and transmit it if frame composition succedded.
        Return True on successfull transmission, false otherwise.
        """
        if not self.status():  # check if serial port is opened
            self.debug('serial port is not open')
            return False

        # check if data elements fit in byte (0 - 255)
        (status, payload) = self._convert_data(payload)
        if not status:
            self.debug('invalid payload data')
            return (False, [])

        if self.ack == SDP_ACK:
            (status, frame) = self._compose_frame(payload)
            if not status:
                self.debug('frame composition')
                return False

        else:  # ack = NACK (CRC values does not match)
            (status, frame) = self._compose_nack_frame(payload)
            if not status:
                self.debug('NACK frame composition')
                return False

        # frame composition OK, send data
        if self._transmit_data(frame):
            return True
        else:
            self.debug('transmission failure')
            return False

    ########################################################################################
    def send_dummy_response(self):
        """
        Builds minimum frame accordingly to SDP frame definition (without payload, only ack field)
        Returns True on success, false otherwise
        """
        if not self.status():  # check if serial port is opened
            self.debug('serial port is not open')
            return False

        frame = []
        self.ack = SDP_ACK
        frame.append(_SDP_SOF)
        frame.append(self.ack)
        frame.append(_SDP_EOF)

        if self._transmit_data(frame):
            return True
        else:  # transmission failed
            self.debug('transmission failure')
            return False

    ########################################################################################
    def _transmit_data(self, frame):
        """
        Transmit frame array through node's serial port.
        Returns True on success, False otherwise
        """
        status = self.s.serial_write(frame)
        if not status:
            self._thread_stop_flag = True

        return status

    ########################################################################################
    def _handle_message(self):
        """
        Handle received message or returned response (clears expecting_response flag).
        This function calls user defined message handler function (parameter of SDP class)
        """
        if self._expect_response:
            self._expect_response = False
        else:
            if self.ack == SDP_ACK:  # if message received correctly, pass it to user
                self.user_message_handler(self.rx_payload)
            # message CRC failure, send response (return received payload)
            else:
                if not self.send_response(self.rx_payload):
                    self.debug('send response failure')

    ########################################################################################
    def _search_for_sof(self):
        """ Search for "start of frame" character """
        (status, byte) = self.s.get_rx_buff_byte()
        while status:
            if byte == _SDP_SOF:
                self._rx_state = _SDP_RX_ACK
                self.ack = SDP_ACK
                self._rx_start_time = systime.time()

                return
            else:  # else, garbage data, search for SOF continues
                (status, byte) = self.s.get_rx_buff_byte()

    ########################################################################################
    def _search_for_ack(self):
        """ First byte after SOF is ack byte """
        (status, byte) = self.s.get_rx_buff_byte()
        if status:
            self.ack = byte
            self._rx_state = _SDP_RX_RECEIVING
            self.rx_payload = []  # clear payload buffer

    ########################################################################################
    def _append_new_data(self):
        """ Append new data and check for special characters or EOF flag"""
        (status, byte) = self.s.get_rx_buff_byte()
        while status:
            if byte == _SDP_DLE:
                self._rx_state = _SDP_RX_DLE
                return

            elif byte == _SDP_EOF:
                self._rx_state = _SDP_RX_IDLE

                if len(self.rx_payload) == 0:  # empty payload, dummy response or frame error
                    if self._expect_response:
                        self._expect_response = False  # reset flag
                        # node->ack field is than checked in send_data()
                    else:  # node is not expecting response, so this frame is corrupted or other error occured.
                        self.debug(
                            'empty payload while not expecting response')

                    # in both cases (expecting response or frame error, return)
                    return

                # payload not empty, continue checking and handling message
                if not self._check_rx_message():
                    self.ack = SDP_NACK
                    # message CRC validation error
                    self.debug('CRC validation failure')

                for _ in range(_SDP_CRC_SIZE):
                    self.rx_payload.pop()  # clear last elements of payload, since they are CRC

                self._handle_message()  # handle message upon expect_response flag, NACK and payload
                return  # even if bytes are still in rx buffer, start with searching for SOF

            else:  # received character is not DLE or EOF, append data to payload
                if len(self.rx_payload) < (self.max_payload_size + _SDP_CRC_SIZE):
                    self.rx_payload.append(byte)
                else:  # discard data, payload size out of range before EOF
                    self._rx_state = _SDP_RX_IDLE

                    self.debug('payload oversized')
                    return

            # handle all bytes in serial rx buffer
            (status, byte) = self.s.get_rx_buff_byte()

        # end of while loop, no data to handle

    ########################################################################################
    def _check_if_eof(self):
        """ Search for "end of frame" character """
        (status, byte) = self.s.get_rx_buff_byte()
        if status:
            if (byte == (_SDP_DLE ^ _SDP_DLE_XOR)) or \
                (byte == (_SDP_SOF ^ _SDP_DLE_XOR)) or \
                    (byte == (_SDP_EOF ^ _SDP_DLE_XOR)):

                self._rx_state = _SDP_RX_RECEIVING

                if len(self.rx_payload) < (self.max_payload_size + _SDP_CRC_SIZE):
                    self.rx_payload.append(byte ^ _SDP_DLE_XOR)
                else:
                    self.debug('payload oversized')
                    return
            else:  # framing error, DLE should never appear on its own in message
                self._rx_state = _SDP_RX_IDLE
                self.debug('corrupted data, standalone DLE')

    ########################################################################################
    def _rx_frame_timeout(self):
        """ Check if frame (and character EOF) arrived in rx_frame_timeout """
        if systime.time() > (self._rx_start_time + self.rx_frame_timeout):
            self._rx_state = _SDP_RX_IDLE

            self.rx_payload = []    # discard payload data

            self.debug('receiving frame timeout')
            return False
        else:
            return True

    ########################################################################################
    def _check_rx_message(self):
        """
        Get message CRC value and compare it with received payload calulated CRC value
        Returns True if crc values match, False otherwise
        """
        (status, crc_value) = self._calculate_crc(self.rx_payload)
        if status:
            # TODO if _SDP_CRC_SIZE is changed to diferent value
            if crc_value == [0, 0]:
                return True
            else:
                return False
        else:  # calculating crc value error
            self.debug('calculating CRC value failure')
            return False

    ########################################################################################
    def _calculate_crc(self, data):
        """
        Calculate CRC-16 value upon data (array of bytes)
        Returns tuple of status and array of bytes
        """

        if len(data) == 0:
            return (True, [0, 0])

        # prepare data for crc calculation, check python version
        if sys.version[0] >= '3':
            # python 3.x
            data = bytes(data)
        else:
            # python 2.x
            x = ''
            for d in data:
                x = x + chr(d)
            data = x

        # data = bytes(data)

        crc_value = self.crc16(data)

        # TODO if _SDP_CRC_SIZE is changed to diferent value
        if crc_value <= 0xFFFF:  # crc_value must fit in _SDP_CRC_SIZE number of bytes
            msb = crc_value >> 8
            lsb = crc_value & 0x00FF
            crc = [msb, lsb]

            return (True, crc)
        else:
            self.debug('CRC value > SDP_CRC_SIZE bytes')
            return (False, [])

    ########################################################################################
    def _compose_frame(self, payload):
        """
        Compose frame accordingly to SDP protocol
        Returns status and array of bytes
        """
        frame = []

        frame.append(_SDP_SOF)
        frame.append(SDP_ACK)

        for b in payload:
            # check for special characters
            if (b == _SDP_SOF) or \
               (b == _SDP_DLE) or \
               (b == _SDP_EOF):

                frame.append(_SDP_DLE)
                frame.append(b ^ _SDP_DLE_XOR)
            else:  # byte is not a special character
                frame.append(b)

        (status, crc) = self._calculate_crc(
            payload)  # calculate payload CRC data
        if not status:
            self.debug('calculating CRC failure')
            return (False, [])
        for c in crc:  # append crc data with special characters check
            # check for special characters
            if (c == _SDP_SOF) or \
               (c == _SDP_DLE) or \
               (c == _SDP_EOF):

                frame.append(_SDP_DLE)
                frame.append(c ^ _SDP_DLE_XOR)
            else:  # byte is not a special character
                frame.append(c)

        frame.append(_SDP_EOF)

        if len(frame) > self._max_frame_size:   # check if frame is inside of SDP size setup
            self.debug('frame oversized')
            return (False, [])
        else:
            return (True, frame)

    ########################################################################################
    def _compose_nack_frame(self, payload):
        """
        Compose frame with payload for response purposes (CRC verification failure at receiver)
        Returns status and array of bytes
        """
        (status, frame) = self._compose_frame(payload)  # build normal frame
        if status:
            # on success, change ACK to NACK
            # accordingly to SDP frame, ack/nack positions is second byte
            frame[1] = SDP_NACK

            return (True, frame)
        else:
            return (False, [])

    ########################################################################################
    def _convert_data(self, data):
        """
        Convert data (characters, strings and integers(0-255) or list of mixed types)
        to bytearray. No escape encoding.
        Returns tuple: (status, converted_data)
        """
        converted_data = bytearray()
        if type(data) == list:
            for element in data:
                if type(element) == str:
                    # element of list is string ("something")
                    # append integer->bytes representation of this character
                    [converted_data.append(ord(char)) for char in element]

                elif type(element) == int:
                    # check if element fits into one byte (0 - 255)
                    if (element < 256) and (element >= 0):
                        converted_data.append(element)
                    else:
                        # element does not fit in one byte, data error
                        return (False, [])
                else:
                    # element type invalid
                    return (False, [])

        elif type(data) == str:
            # append integer->bytes representation of this character
            [converted_data.append(ord(char)) for char in element]

        elif type(data) == int:
            if (data < 256) and (data >= 0):
                # converted_data.append(str(element).encode(element)) #fits, convert and add
                converted_data.append(data.to_bytes(
                    1, byteorder='big', signed=False))
            else:
                # element does not fit in one byte, data error
                return (False, [])
        else:
            # element type invalid
            return (False, [])

        return (True, converted_data)

    ########################################################################################
    def debug(self, err):
        """
        Print out caller of this function and user readable string error, if SDP_DEBUG == True
        You can add your custom error logger here, or call external function.
        """
        if SDP_DEBUG:
            func = sys._getframe().f_back.f_code.co_name + '()'
            print('ERR: SDP node %s in function %s: %s' % (self.id, func, err))
