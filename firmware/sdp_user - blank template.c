/**
  ******************************************************************************
  * File Name          : sdp_user.c
  * Description        : Simple Data Protocol
  *                      
  * @date    08-Nov-2017
  * @author  Domen Jurkovic
  * @version v1
  * @source  http://damogranlabs.com/
  *          https://github.com/damogranlabs
  *
  * @note    https://eli.thegreenplace.net/2009/08/12/framing-in-serial-communications/
  ******************************************************************************
*/

/*
  Find readme file to learn more about protocol and setup instructions.
*/ 

#include "sdp.h"

/* User includes ------------------------------------------------------------------*/   

/**
* @brief Receive one byte on serial line
* @note This function is called from ISR (from sdp_receive_byte()). User should handle all flags that are enabled 
*       by peripheral either here or in corresponding ISR. 
* @note User should check for node->uart.rx_timeout occured while receiving byte
* @retval Function should return "true" on success, "false" otherwise
*/
bool sdp_user_receive_byte(SDP_data_t *node, uint8_t *byte){
  
  // RXNE flag not empty - receive one byte and store it in *byte pointer. 
  // return receive status (true/false)
  
}

/**
* @brief Transmit one byte on serial line
* @note User should handle all flags that are enabled by peripheral either here or in corresponding ISR.
* @note User should check for node->uart.tx_timeout occured while transmitting byte
* @retval Function should return "true" on success, "false" otherwise
*/
bool sdp_user_transmit_byte(SDP_data_t *node, uint8_t byte){
  
  // Transmitt byte over SDP node. If multiple nodes are used, make sure to transmitt over correct one.
  // return transmitt status (true/false)
  
}

/**
* @brief This function is called when message is received and checked with CRC.
* @note If multiple nodes are used, user should add node id selector to select right message handler
*/
void sdp_user_handle_message(SDP_data_t *node, uint8_t *payload, uint8_t size){
  
  // Handle this correctly received message and send response

}

/**
* @brief Calculate CRC value of payload data
* @retval Must return crc value
*/
uint16_t sdp_user_calculate_crc(SDP_data_t *node, uint8_t *payload, uint16_t size){
  
  // implement your CRC calculation function. 
  // retur CRC value of a given payload
  
}

/**
* @brief Implement error log/debug informations
*/
void sdp_debug(SDP_data_t *node, uint8_t err){
  
  //  add your code to log SDP errors (debugging SDP or your custom error reporting implementation
  
  #ifdef SDP_DEBUG
    /*
    1 - sdp_receive_data()->sdp_user_receive_data() failed to retrive byte
    2 - sdp_receive_data() - ring buffer put error
    
    10 - sdp_transmit_data() - no payload, frame size error
    11 - sdp_transmit_data()-> sdp_user_transmit_data() - byte transmission timeout
    12 - sdp_transmit_data() - frame transmission timeout

    20 - sdp_user_transmit_byte() - waiting for TXE flag timeout
    21 - sdp_user_transmit_byte() - waiting for TXC flag timeout
  
    40 - sdp_init_node() - rx_buff init error
    41 - sdp_init_node() - rx_data payload malloc() error
    42 - sdp_init_node() - tx_data malloc() error
  
    50 - sdp_parse_rx_data() - invalid rx_state
    
    60 - sdp_send_data() - response timeout
    61 - sdp_send_data() - transmission unsuccessfull
    62 - sdp_send_data() - composed message larger than SDP_MAX_FRAME
    63 - sdp_send_data() - NACK received
    
    70 - sdp_send_response()-> compose_frame() - composed message larger than SDP_MAX_FRAME
    71 - sdp_send_response()-> sdp_transmit_data() - transmission unsuccessfull
    
    80 - append_new_data() - payload exceded SDP_MAX_PAYLOAD
    81 - append_new_data() - payload CRC error (CRC values does not match), node->ack updated
    82 - append_new_data() - frame with no payload while not expecting response (maybe send with sdp_send_dummy_response() but timing or other error occured)
    
    90 - check_if_eof() - payload size out of range before EOF
    91 - check_if_eof() - framing error, DLE should never appear on its own in message
    
    100 - rx_frame_timeout() - rx frame timeout
  
    110 - compose_frame() - payload size > SDP_MAX_PAYLOAD
    111, 112, 113 - compose_frame() - frame size > SDP_MAX_FRAME_SIZE
    114 - compose_frame()->append_crc_bytes() frame size error
    
    120 - sdp_handle_message()->sdp_send_response() - response transmitt failure
    
    130, 131, 132 - append_crc_bytes() - frame size > SDP_MAX_FRAME_SIZE
    133 - append_crc_bytes() - CRC size != 2
        
    150 - sdp_send_dummy_response()->sdp_transmit_data() - transmission error
    
    */
  #endif
}


