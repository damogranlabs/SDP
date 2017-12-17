/**
  ******************************************************************************
  * File Name          : sdp.h
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
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDP_H
#define __SDP_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Private includes ------------------------------------------------------------------*/   
#include <stdbool.h>
#include "ring_buffer.h"
   
   
/* User includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
   
/* User setup ------------------------------------------------------------------*/      
//#define SDP_DEBUG // undefine if no debug info must be implemented/reported using sdp_debug()

#define SDP_RETRANSMIT 2 // number of retries in case of send/receive error
  
#define SDP_DEFAULT_RX_MSG_TIMEOUT 300  // [ms] 
#define SDP_DEFAULT_TX_MSG_TIMEOUT 300  // [ms] 
#define SDP_DEFAULT_RESPONSE_TIMEOUT  300 //[ms]
#define SDP_CRC_POLYNOME  0x8005 // CRC-16 -> https://www.lammertbies.nl/comm/info/crc-calculation.html

/* Private ------------------------------------------------------------------*/     
#define SDP_SOF 0x7E  // start byte of each frame
#define SDP_EOF 0x66  // END byte of each frame
#define SDP_DLE 0x7D  // Data Link Escape - or Escape (avoid escaping of message if EOF shows up in the middle of data)


typedef enum{
  SDP_RX_IDLE = 0, // waiting for start flag
  SDP_RX_ACK, // waiting for ack field
  SDP_RX_RECEIVING,  //receiving data, waiting for end flag
  SDP_RX_DLE  // end flag or DLE byte received
} SDP_rx_state_t;

// LL UART layer - initialisation must be done with HAL CubeMX or other
typedef struct{
  // user MUST SET this variables
  USART_TypeDef *handle;  // uart handle -> User must change this type definition if not using STM32 LL UART library
  uint32_t rx_timeout;    // [ms] RX timeout (for receiving 1 byte) - include interrupt times
  uint32_t tx_timeout;    // [ms] TX timeout (for transmiting 1 byte) - include interrupt times
} SDP_uart_t;

typedef struct{
  // user MUST SET this variables (set with sdp_init_node())
  SDP_uart_t uart;  // communicaton port
  uint8_t id;       // node ID
  uint8_t rx_tx_max_payload;  // each message/frame can contain max this number of payload bytes
  
  // user CAN SET this variables -> inn sdp.h or after init() function call
  uint32_t rx_msg_timeout;    // if EOF does not arrive in this time, message is discarded and set as invalid
  uint32_t tx_msg_timeout;    // if EOF does not arrive in this time, message is discarded and set as invalid
  uint32_t response_timeout;  // receiver must respond in this time 
  // user CAN READ this buffer when data is received (response)
  uint8_t *rx_data; // pointer to received data payload (used as array)
  uint16_t rx_data_index;   // buffer index (also size of received payload)
  uint8_t ack;  // if message is a response to transmited data, ack holds reception status value
  
  // private variables
  bool _expect_response; // interval variable to expect response from receiver after transmiting data
  SDP_rx_state_t _rx_state;  // internal state machine state
  rb_att_t _rx_buff; // uart stores all received characters in this buffer
  uint32_t _rx_start_time; // message SOF timestamp
  uint8_t *_tx_data; // pointer to outgoing framed data (used as array)
  uint16_t _tx_data_size;  // frame payload size
  uint16_t _max_frame_size; // framed payload maximum size
} SDP_data_t;

/* Setup ------------------------------------------------------------------*/  
bool sdp_init_node(SDP_data_t *node, SDP_uart_t *uart_handle, uint8_t id, uint8_t payload_size, uint8_t rx_buff_count);
void sdp_parse_rx_data(SDP_data_t *node);
void sdp_receive_data(SDP_data_t *node); // call this from RXNE ISR

/* Update according your HW and application -----------------------------------------------*/
// Edit sdp_user.c file
bool sdp_user_receive_byte(SDP_data_t *node, uint8_t *byte);
bool sdp_user_transmit_byte(SDP_data_t *node, uint8_t byte);
void sdp_user_handle_message(SDP_data_t *node, uint8_t *payload, uint8_t size);
uint16_t sdp_user_calculate_crc(SDP_data_t *node, uint8_t *payload, uint16_t size);

/* Transmit & receive data ------------------------------------------------*/
bool sdp_send_data(SDP_data_t *node, uint8_t *payload, uint8_t payload_size);
bool sdp_send_response(SDP_data_t *node, uint8_t *payload, uint8_t payload_size);
bool sdp_send_dummy_response(SDP_data_t *node);

uint8_t * sdp_get_response(SDP_data_t *node);
uint16_t sdp_get_rx_data_size(SDP_data_t *node);

/* Other ------------------------------------------------------------------*/
void sdp_debug(SDP_data_t *node, uint8_t err);
void sdp_reset_node(SDP_data_t *node);
   
#ifdef __cplusplus
}
#endif

#endif 

