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
  Find readme.txt to learn more about protocol and setup instructions.

About SDP - Simple Data Protocol
  Main idea of this protocol is that user can send data between diferent "nodes" using simple functions, 
while protocol will make sure correct data is delivered and if not, retry or give up and report that to user.

  It was primary designed for smaller embeded systems and payloads (up to 255 bytes per payload) to use with 
UART (specifically STM32 LL UART drivers), but can be easily ported to other communication hardware. 
Tested for speeds up to 115200 bps.
  Although each node can initiate communication to the other end, it was primary meant for Master-Slave point-to-point 
communication, where master poll other node for any new available data (or user must take care of possible 
data collision and timing, or use other technique to notify other system for available data - like unused CTS/RTS line).

Protocol check each payload data with CRC-16 and responds with ACK + user defined respond. If CRC values does not match,
NACK + received payload is returned. If no response is received, this is considered as error and protocol retry.
*/ 

#include "sdp.h"

/* User includes ------------------------------------------------------------------*/   
#include "misc.h"
#include "comm.h"
#include "data_io.h"
#include "stm32xx_hal_liquid_crystal.h"

#include "stm32f0xx_ll_crc.h"

/**
* @brief Receive one byte on serial line
* @note This function is called from ISR (from sdp_receive_byte()). User should handle all flags that are enabled 
*       by peripheral either here or in corresponding ISR. 
* @note User should check for node->uart.rx_timeout occured while receiving byte
* @retval Function should return "true" on success, "false" otherwise
*/
bool sdp_user_receive_byte(SDP_data_t *node, uint8_t *byte){
  
  // RXNE flag already checked in ISR
  *byte = LL_USART_ReceiveData8(node->uart.handle); // RXNE flag will be cleared by reading of RDR register (done in call)
    
  return true;
}

/**
* @brief Transmit one byte on serial line
* @note User should handle all flags that are enabled by peripheral either here or in corresponding ISR.
* @note User should check for node->uart.tx_timeout occured while transmitting byte
* @retval Function should return "true" on success, "false" otherwise
*/
bool sdp_user_transmit_byte(SDP_data_t *node, uint8_t byte){
  uint32_t timeout = HAL_GetTick() + node->uart.tx_timeout;
   
  // wait while TX data register is not empty
  while(!LL_USART_IsActiveFlag_TXE(node->uart.handle)){
    if(HAL_GetTick() > timeout){  // check for timeout
      sdp_debug(node, 20);
      
      return false;
    }
  }
  
  LL_USART_TransmitData8(node->uart.handle, byte);
        
  // wait while TX complete (data transmited)
  while(!LL_USART_IsActiveFlag_TC(node->uart.handle)){
    if(HAL_GetTick() > timeout){  // check for timeout
      sdp_debug(node, 21);
      
      return false;
    }
  }
    
  return true; // on success return true
}

/**
* @brief This function is called when message is received and checked with CRC.
* @note If multiple nodes are used, user should add node id selector to select right message handler
*/
void sdp_user_handle_message(SDP_data_t *node, uint8_t *payload, uint8_t size){
  
  LED_2_ON();
  
  handle_received_data(node);
    
  LED_2_OFF();
}

/**
* @brief Calculate CRC value of payload data
* @retval Must return crc value
*/
uint16_t sdp_user_calculate_crc(SDP_data_t *node, uint8_t *payload, uint16_t size){
  uint32_t data = 0;
  uint32_t index = 0;
  
  if((size == 0) || (payload == NULL)){
    return 0;
  }
  
  LL_CRC_ResetCRCCalculationUnit(CRC);  // clear accumulated data
  
  // Compute the CRC - as 32 bit while possible, than 16 or 8 bit (modified from STM32 LL example)
  for (index = 0; index < (size / 4); index++){
    data = (uint32_t)((payload[4 * index] << 24) | (payload[4 * index + 1] << 16) | (payload[4 * index + 2] << 8) | payload[4 * index + 3]);
    LL_CRC_FeedData32(CRC, data);
  }  
  // Last bytes specific handling
  if ((size % 4) != 0){
    if(size % 4 == 1){
      LL_CRC_FeedData8(CRC, payload[4 * index]);
    }
    if  (size% 4 == 2){
      LL_CRC_FeedData16(CRC, (uint16_t)((payload[4 * index]<<8) | payload[4 * index + 1]));
    }
    if  (size % 4 == 3){
      LL_CRC_FeedData16(CRC, (uint16_t)((payload[4 * index]<<8) | payload[4 * index + 1 ]));
      LL_CRC_FeedData8(CRC, payload[4 * index + 2]);
    }
  }
  
  return(LL_CRC_ReadData16(CRC)); // Return computed CRC value
}

/**
* @brief Implement error log/debug informations
*/
void sdp_debug(SDP_data_t *node, uint8_t err){
  //node->ack = err; // do not modify
  
  #ifdef SDP_DEBUG
    LCD_ClearArea(1, 13, 15);
    LCD_PrintNumber(1, 13, err);
    HAL_Delay(500);
    /*
    1 - sdp_receive_data()->sdp_user_receive_data() failed to retrive byte
    2 - sdp_receive_data() - ring buffer put error
    
    10 - sdp_transmit_data() - no payload, frame size error
    11 - sdp_transmit_data()->sdp_user_transmit_data() - byte transmission timeout
    12 - sdp_transmit_data() - frame transmission timeout

    20 - sdp_user_transmit_byte() - waiting for TXE flag timeout
    21 - sdp_user_transmit_byte() - waiting for TXC flag timeout
  
    40 - sdp_init_node() - rx_buff init error
    41 - sdp_init_node() - rx_data payload malloc() error
    42 - sdp_init_node() - tx_data malloc() error
    43 - sdp_init_node() - rx_data_temp malloc() error
  
    50 - sdp_parse_rx_data() - invalid rx_state
    
    60 - sdp_send_data() - response timeout
    61 - sdp_send_data() - transmission unsuccessfull
    62 - sdp_send_data() - composed message larger than SDP_MAX_FRAME
    63 - sdp_send_data() - NACK received
    
    70 - sdp_send_response()->sdp_transmit_data() - transmission unsuccessfull
    71 - sdp_send_response()->compose_frame() - composed message larger than SDP_MAX_FRAME
    72 - sdp_send_response()->compose_nack_frame() - composed message larger than SDP_MAX_FRAME
    
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
    
    140 - compose_nack_frame()->compose_frame() - frame not succesfully created 
        
    150 - sdp_send_dummy_response()->sdp_transmit_data() - transmission error
    
    160 - _send_response() - compose frame error
    161 - _send_response() - transmit error
    
    
    190 - handle_data()->send_response() error
    */
  #endif
}


