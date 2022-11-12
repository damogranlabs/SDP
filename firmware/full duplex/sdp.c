/**
  ******************************************************************************
  * File Name          : sdp.c
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
#include "sdp.h"

#include <stdlib.h>
#include <string.h>

//FRAME: SOF | RESPONSE | STATUS (response) | N x payload | CRC x2 | DLE

#define SDP_SOF_SIZE  1 // number of SOF bytes
#define SDP_RESPONSE_SIZE 1
#define SDP_STATUS_SIZE   1
#define SDP_CRC_SIZE  2 // number of CRC bytes
#define SDP_EOF_SIZE  1 // number of EOF bytes

#define SDP_MIN_FRAME_SIZE (SDP_SOF_SIZE + SDP_RESPONSE_SIZE + SDP_STATUS_SIZE + SDP_CRC_SIZE + SDP_EOF_SIZE)

#define SDP_DLE_XOR  0x20  // Whenever a flag or escape byte appears in the message, it is escaped by 0x7D and the byte itself is XOR-ed with 0x20. So, for example 0x7E becomes 0x7D 0x5E. Similarly 0x7D becomes 0x7D 0x5D. The receiver unsuffs the escape byte and XORs the next byte with 0x20 again to get the original

#define SDP_STATUS_OK     0xFF  // equivalent to ACK
#define SDP_STATUS_ERROR  0x00  // equivalent to NACK
#define SDP_RESPONSE  0xFF  // this message is response to sdp_send_data()
#define SDP_DATA      0x00  // indicates this message is ordinary data (not response to sdp_send_data())

// rx data parser
static void search_for_sof(SDP_data_t *node);
static void search_for_response(SDP_data_t *node);
static void search_for_status(SDP_data_t *node);
static void append_new_data(SDP_data_t *node);
static void check_if_special_character(SDP_data_t *node);
static void handle_data(SDP_data_t *node);
static bool rx_frame_timeout(SDP_data_t *node);
static bool check_rx_message(SDP_data_t *node);
static bool rx_data_put(SDP_data_t *node, uint8_t data);
// TX
static bool transmit_data(SDP_data_t *node);  
static bool compose_frame(SDP_data_t *node, uint8_t response, uint8_t status, uint8_t *data, uint8_t size);
static bool append_crc_bytes(SDP_data_t *node, uint16_t crc_value);
static bool send_err_response(SDP_data_t *node);

/* Init and parsers ------------------------------------------------------------------*/
/**
* @brief Call this function to set default values for each node structure and set ring buffers.
* @param Payload size is number of bytes each frame can contain (before framing)
* @param rx_buff_coun refers to maximum number of framed data that can be stored in rx buffer before processing (unframing & handling)
*/
//bool sdp_init_node(SDP_data_t *node, SDP_uart_t *uart_handle, uint8_t id){
bool sdp_init_node(SDP_data_t *node, SDP_uart_t *uart_handle, uint8_t id, uint8_t payload_size, uint8_t rx_buff_count){
  uint16_t rx_buff_size;
  
  node->uart = *uart_handle;
  node->id = id;
  
  node->rx_tx_max_payload = payload_size;
  node->_max_frame_size = (SDP_SOF_SIZE  + SDP_RESPONSE_SIZE + SDP_STATUS_SIZE + payload_size*2+ SDP_CRC_SIZE*2 + SDP_EOF_SIZE ); // payload/crc worst case = *2 - if every byte is special character, escaped with DLE
  
  // init rx ring buffer for storing all received bytes
  rx_buff_size = (node->_max_frame_size * rx_buff_count) +1;
  if(ring_buffer_init(&node->_rx_buff, rx_buff_size) != RB_OK){
    sdp_debug(node, 40);
    return false;
  }
  node->_rx_state = SDP_RX_IDLE;
  node->_rx_start_time = 0;
  node->rx_msg_timeout = SDP_DEFAULT_RX_MSG_TIMEOUT;
  
  // init rx payload "array"
  node->rx_data = calloc(node->rx_tx_max_payload +1, sizeof(uint8_t)); // allocate memory of payload bytes, set all values to 0.
  if(node->rx_data == NULL){  // buff must not be pointer to nowhere
    sdp_debug(node, 41);
    return false;
  }
  node->rx_data_index = 0;
  
  // init tx data array
  node->_tx_data = calloc(node->_max_frame_size +1, sizeof(uint8_t)); // allocate memory of "frame" bytes, set all values to 0.
  if(node->_tx_data == NULL){  // buff must not be pointer to nowhere
    sdp_debug(node, 42);
    return false;
  }
  node->_tx_data_size = 0;
  node->tx_msg_timeout = SDP_DEFAULT_TX_MSG_TIMEOUT;
  
  node->_expect_response = false;
  node->response_timeout = SDP_DEFAULT_RESPONSE_TIMEOUT;
    
  return true;
}

/**
* @brief Parse all data in rx buffer
* @note This function should be polled frequently to handle incoming data from ring buffer asap.
*/
void sdp_parse_rx_data(SDP_data_t *node){
  uint16_t size = ring_buffer_size(&node->_rx_buff);
  
  if(size >= 1){ // at least one byte is in buffer
    switch(node->_rx_state){
      case SDP_RX_IDLE: 
        search_for_sof(node); 
        break;
      
      case SDP_RX_RESPONSE:
        search_for_response(node); 
        rx_frame_timeout(node); // check for timeout
        break;
      
      case SDP_RX_STATUS:
        search_for_status(node); 
        rx_frame_timeout(node); // check for timeout
        break;
      
      case SDP_RX_RECEIVING: 
        append_new_data(node);
        rx_frame_timeout(node); // check for timeout
        break;
      
      case SDP_RX_DLE: 
        check_if_special_character(node);
        rx_frame_timeout(node); // check for timeout
        break;
      
      default: // invalid rx_state
        sdp_debug(node, 50);
        node->_rx_state = SDP_RX_IDLE; // re-init to IDLE
        break;
    }
  }
  else{ // even if buffer is empty, but data should be received 
    rx_frame_timeout(node); // check for timeout
  } 
  
}

/**
* @brief Receives all available bytes on serial line and store them in rx buffer
* @note Call this function from RXNE interrupt routine.
*/
void sdp_receive_data(SDP_data_t *node){
  uint8_t data;
  
  if(!sdp_user_receive_byte(node, &data)){
    sdp_debug(node, 1);
    return;
  }
  if(ring_buffer_put(&node->_rx_buff, &data, 1) != RB_OK){
    sdp_debug(node, 2); //ring buffer full or not enough space
  }
}

/**
* @brief Sends tx_data array through uart
*/
bool transmit_data(SDP_data_t *node){
  uint32_t timeout = HAL_GetTick() + node->tx_msg_timeout; // frame transmission timeout
  uint16_t num = 0;
  
  if(node->_tx_data_size < SDP_MIN_FRAME_SIZE){ // check if there is anything to send at all
    sdp_debug(node, 10);
    
    return false;
  }
  
  while(num != node->_tx_data_size){
  
    if(!sdp_user_transmit_byte(node, node->_tx_data[num])){
      sdp_debug(node, 11);
      
      return false;
    }
    num++;
    if(HAL_GetTick() > timeout){  // check for frame transmission timeout
      sdp_debug(node, 12);
     
      return false;
    }
  }
  
  return true;  // on success return true
}

/**
* @brief Transmits data and waits for response.
* @param payload_size >= 1
* @note Response is parsed normally while handled with node->expect_response flag
*/
bool sdp_send_data(SDP_data_t *node, uint8_t *payload, uint8_t payload_size){
  uint8_t retransmit_count;
  uint32_t response_timeout;
  uint32_t waiting_timeout = HAL_GetTick() + node->response_timeout;
  
  // data is already receiving, wait for complete data, parse , handle and send response
  while(node->_rx_state != SDP_RX_IDLE){
    sdp_parse_rx_data(node);

    if(HAL_GetTick() > waiting_timeout){
      sdp_debug(node, 64);
      break;
    }
  }  
  
  for(retransmit_count = 0; retransmit_count < SDP_RETRANSMIT; retransmit_count++){
    node->_response = SDP_DATA;
    if(compose_frame(node, SDP_DATA, SDP_STATUS_OK, payload, payload_size)){ // compose frame and store it in tx_data array
      
      if(transmit_data(node)){ // transmit tx_data array
        // transmission OK, poll for response
        response_timeout = HAL_GetTick() + node->response_timeout*2; // note that node->rx_start_time is updated on SOF
        
        node->_expect_response = true;
        while(node->_expect_response){ // wait until parser clears flag or timeout          
          sdp_parse_rx_data(node);  // parse all incoming rx buffer data
          
          if(HAL_GetTick() > response_timeout){
            sdp_debug(node, 60);
            break; // data didn't arrive in time, break out of loop
          }
        }
        
        if(node->_expect_response == false){ // parser cleared flag, response received
          node->_expect_response = false; 
          
          if(node->_status != SDP_STATUS_OK){
            // response received, but receiver side is reporting data failure (CRC)
            sdp_debug(node, 63);
            
            // failure, retry
            
            // TODO
            // delay? - to avoid receiver overrun?
          }
          else{ // ACK OK
            return true; // success, read rx_data for response payload
          }
        } // expect_response flag not cleared, timeout
        
        // _expect_response flag not cleared, retry
        node->_expect_response = false;
        node->_rx_state = SDP_RX_IDLE;
        
      }
      else{ // transmition unsuccessfull, retry
        sdp_debug(node, 61);
      }
      
    } // end of for loop reached - retransmit if error
    else{ // frame can't be composed - larger than SDP_MAX_FRAME_SIZE
      sdp_debug(node, 62);
      
      return false;
    }
  }// end of for loop (retransmission)
  
  
  return false; // loop didn't return while executing, error occured
}

/**
* @brief Sends response to node.
* @param payload_size >= 1
* @note Response is not send accordingly to retransmit - try once and return
*/
bool sdp_send_response(SDP_data_t *node, uint8_t *payload, uint8_t payload_size){
    
  if(!compose_frame(node, SDP_RESPONSE, SDP_STATUS_OK, payload, payload_size)){ // compose frame and store it in tx_data array
    sdp_debug(node, 71);
    return false;
  }

  if(transmit_data(node)){ // transmit tx_data array
    return true;
  }
  else{ // transmit error
    sdp_debug(node, 70);
  }
  
  return false; // transmit_data() not succedded  
}

/**
* @brief Sends response to node on CRC validation fail - set _ctrl to R=1, S=0
* @note There is no payload parameter.
* @note Response is not send accordingly to retransmit - try once and return
*/
static bool send_err_response(SDP_data_t *node){
  
  node->_tx_data[0] = SDP_SOF;  // first byte of message is always SOF
  node->_tx_data[1] = SDP_RESPONSE;
  node->_tx_data[2] = SDP_STATUS_ERROR;
  node->_tx_data[3] = SDP_EOF; 
  node->_tx_data_size = 4;
  
  if(transmit_data(node)){ // transmit tx_data array
    return true;
  }
  else{ // transmit error
    sdp_debug(node, 161);
    return false;
  }
    
  return false; // transmit_data() not succedded  
}

/**
* @brief Sends response to node without payload, with ack and response field "true".
* @note Response is not send accordingly to retransmit - try once and return
*/
bool sdp_send_dummy_response(SDP_data_t *node){
    
  node->_tx_data[0] = SDP_SOF;  // first byte of message is always SOF
  node->_tx_data[1] = SDP_RESPONSE;
  node->_tx_data[2] = SDP_STATUS_OK;
  node->_tx_data[3] = SDP_EOF; 
  node->_tx_data_size = 4;
  
  if(transmit_data(node)){ // transmit tx_data array
    return true;
  }
  else{ // transmit error
    sdp_debug(node, 150);
    return false;
  }
}

/**
* @brief Get pointer to rx data buffer. Same as directly reading node->rx_data.
*/
uint8_t * sdp_get_response(SDP_data_t *node){
  return node->rx_data;
}

/**
* @brief Get received data size. Same as directly reading node->rx_data_index
*/
uint16_t sdp_get_rx_data_size(SDP_data_t *node){
  return node->rx_data_index;
}

/* Private RX ------------------------------------------------------------------*/
/**
* @brief Scans through rx buffer and search for SOF (start of frame byte). If found, return.
* @note This function is only called if rx state is IDLE
*/
static void search_for_sof(SDP_data_t *node){
  uint8_t data;

  while(ring_buffer_get(&(node->_rx_buff), &data, 1) == RB_OK){
    if(data == SDP_SOF){  // check if byte is SOF
      // byte is SOF, update rx state
      node->_rx_state = SDP_RX_RESPONSE;
      node->_rx_start_time = HAL_GetTick();
            
      return; // SOF found, start reading ack data
    }
    // else, garbage data, search for SOF continues
  }
  // else - ring buffer get error
}

/**
* @brief Get next (after SOF) byte - response byte
* @note This function is only called if rx state is SDP_RX_CTRL
*/
static void search_for_response(SDP_data_t *node){
  uint8_t data;

  if(ring_buffer_get(&(node->_rx_buff), &data, 1) == RB_OK){
    node->_response = data;
    node->_rx_state = SDP_RX_STATUS;
  }
  // else - ring buffer get error /no data
}

/**
* @brief Get next (after response) byte  - status byte (valid only if response = true)
* @note This function is only called if rx state is SDP_RX_CTRL
*/
static void search_for_status(SDP_data_t *node){
  uint8_t data;

  if(ring_buffer_get(&(node->_rx_buff), &data, 1) == RB_OK){
    node->_status = data;
    
    node->_rx_state = SDP_RX_RECEIVING; // start receiving payload.
    node->rx_data_index = 0;
  }
  // else - ring buffer get error /no data
}

/**
* @brief This function is called while receiving payload (after SOF, RESPONSE and STATUS, before EOF)
* @note This function is only called if rx state is SDP_RX_RECEIVING
*/
static void append_new_data(SDP_data_t *node){
  uint8_t data;
    
  while(ring_buffer_get(&(node->_rx_buff), &data, 1) == RB_OK){
    if(data == SDP_DLE){ // xor-ed EOF character is expected
      node->_rx_state = SDP_RX_DLE;
      // don't do anything with this byte, just set rx state to expect another DLE, SOF or EOF.
      return;
    }
    else if(data == SDP_EOF){ // end of payload
      handle_data(node);
      
      node->_rx_state = SDP_RX_IDLE; // continue parsing data 
      return; // even if bytes are still in rx buffer, start with searching for SOF
    }
    else{ // pure data
      if(!rx_data_put(node, data)){
        node->_rx_state = SDP_RX_IDLE; // discard data, payload size out of range before EOF
                
        sdp_debug(node, 80);
        return;
      }
        
      
      // else - continue appending data
    }
  }// end of ring buffer data
}

/**
* @brief Rx state is SDP_RX_DLE. Expecting another special character: DLE, SOF or EOF byte.
*/
static void check_if_special_character(SDP_data_t *node){
  uint8_t data;
  
  if(ring_buffer_get(&node->_rx_buff, &data, 1) == RB_OK){
    if((data == (SDP_DLE ^ SDP_DLE_XOR)) || (data == (SDP_SOF^ SDP_DLE_XOR) ) || (data == (SDP_EOF ^ SDP_DLE_XOR) )){  // check for DLE and SOF/EOF flags
      node->_rx_state = SDP_RX_RECEIVING;
      data = data ^ SDP_DLE_XOR; // XOR-ing allows a participant to identify frames by only listening for STX/ETX. Otherwise, it would need to account for DLEs too, because the user data might contain an escaped STX/ETX.
      
      if(!rx_data_put(node, data)){
        node->_rx_state = SDP_RX_IDLE; // discard data, payload size out of range before EOF
        
        sdp_debug(node, 90);
        return;
      }
      
    }
    else{ // framing error, DLE should never appear on its own in message
      node->_rx_state = SDP_RX_IDLE;
      
      sdp_debug(node, 91);
    }
  }
}

/**
* @brief Once parser builds a message, data is passed to this function. 
*/
static void handle_data(SDP_data_t *node){
 
  // 1. check message with CRC if it is valid
  if(!check_rx_message(node)){
    // invalid message (CRC checked)
    if(!send_err_response(node)){ // send response to let receiver now some error occured.
      sdp_debug(node, 190);
    }
  }
  else{ 
    //message is CRC checked, valid
    node->rx_data_index = node->rx_data_index - SDP_CRC_SIZE; // update payload index (discarding received CRC data)
    
    if(node->_response == SDP_RESPONSE){
      // response message
      if(node->_expect_response == true){ // response message, expecting response
        node->_expect_response = false; // clear response flag - response received, continue in sdp_send_data()
        
      }
      else{ // response message, not expecting response
        // transmitter error or corrupted data
        node->rx_data_index = 0;  // discard this data
        
        sdp_debug(node, 191);
      }
    }
    else{ // this is not response message (= SDP_DATA)
      sdp_user_handle_message(node, node->rx_data, node->rx_data_index);  // pass message to user. User must send response to this message.
    }
  }
}

/**
* @brief Check for message timeout
* @retval Returns false if timeout occured, resets state and index
*/
static bool rx_frame_timeout(SDP_data_t *node){
  if(node->_rx_state != SDP_RX_IDLE){
    if(HAL_GetTick() > (node->_rx_start_time + node->rx_msg_timeout)){
      node->_rx_state = SDP_RX_IDLE;
      node->rx_data_index = 0;
      
      sdp_debug(node, 100);
      return false;
    }
    else{
      return true;
    }
  }
  else{
    return true;
  }
  
}

/**
* @brief Get data from rx_data and calculate CRC value, if CRC check = 0, data is OK
* @retval Returns false if values does not match, true otherwise
*/
static bool check_rx_message(SDP_data_t *node){
  uint16_t crc_value;
  crc_value = sdp_user_calculate_crc(node, node->rx_data, node->rx_data_index); // index points to first free aray field
  
  if(crc_value == 0){
    
    return true; //success, no errors detected with CRC
  }
  
  return false;
}

/**
* @brief Put byte into rx_data buffer
* @note node->rx_data_index always points at first free element of array
* @retval Returns false if buffer is full, true otherwise
*/
static bool rx_data_put(SDP_data_t *node, uint8_t data){
  if(node->rx_data_index >= (node->rx_tx_max_payload + SDP_CRC_SIZE)){
    // index already out of range, no free place in array
    return false;
  }
  node->rx_data[node->rx_data_index] = data;  // put data into array
  node->rx_data_index ++; // increment index
  
  return true;
}

/* Private TX ------------------------------------------------------------------*/
/**
* @brief Compose frame from data, SOF, DLE and EOF
* @param size >= 1
* @note frame & size are stored in node's tx_data array and tx_data_size
* @retval Returns false if frame size is exceded, true otherwise
*/
static bool compose_frame(SDP_data_t *node, uint8_t response, uint8_t status, uint8_t *data, uint8_t size){
  uint8_t data_index; // uint8_t -> PAYLOAD size up to 255 bytes
  uint16_t tmp_index; // points to first available element of tx_data array
  uint8_t temp_data;
  uint32_t crc_value;
  
  if(size > node->rx_tx_max_payload){
    sdp_debug(node, 110);
    return false;
  }

  crc_value = sdp_user_calculate_crc(node, data, size);
  
  node->_tx_data[0] = SDP_SOF;    // first byte of message is always SOF
  node->_tx_data[1] = response;  
  node->_tx_data[2] = status;
  tmp_index = 3;
  
  for(data_index = 0;  data_index < size; data_index++){
    temp_data = *data;
    if( (temp_data == SDP_SOF) || (temp_data == SDP_DLE) || (temp_data == SDP_EOF)){  // check if is special character
      node->_tx_data[tmp_index] = SDP_DLE;
      
      tmp_index++;
      if(tmp_index >= node->_max_frame_size){
        
        sdp_debug(node, 111);
        return false;
      }
      node->_tx_data[tmp_index] = temp_data ^ SDP_DLE_XOR;  // append XOR-ed data
      
      tmp_index++;
      data++; // increment pointer
      if(tmp_index >= node->_max_frame_size){
        
        sdp_debug(node, 112);
        return false;
      }      
    }
    else{ // data is not special character
      node->_tx_data[tmp_index] = temp_data;
      
      tmp_index++;
      data++;
      if(tmp_index >= node->_max_frame_size){
        
        sdp_debug(node, 113);
        return false;
      }
    }
  }// data appended and checked for special character
  
  node->_tx_data_size = tmp_index; // update size field so CRC can append data
  
  if(!append_crc_bytes(node, crc_value)){
    
    sdp_debug(node, 114);
    return false;
  }
    
  node->_tx_data[node->_tx_data_size] = SDP_EOF; // last byte of message is always EOF
  node->_tx_data_size = node->_tx_data_size+1;
  
  return true;
}

/**
* @brief Add already calculated CRC value to tx frame array
* @note This function is tailored to 16bit CRC - 2 bytes. TODO if not CRC-16
* @retval Returns false if frame size is exceded, true otherwise
*/
static bool append_crc_bytes(SDP_data_t *node, uint16_t crc_value){
  uint8_t c;
  uint8_t crc_data[SDP_CRC_SIZE];
  
  if(SDP_CRC_SIZE != 2){
    // Function not valid for other than CRC-16
    sdp_debug(node, 133);
    return true; // return true and no bytes are added to payload
  }
  crc_data[0] = (crc_value >> 8); // msb
  crc_data[1] = (crc_value & 0x00FF); //lsb
  
  for(c=0; c < SDP_CRC_SIZE; c++){
    if( (crc_data[c] == SDP_SOF) || (crc_data[c] == SDP_DLE) || (crc_data[c] == SDP_EOF)){  // check if is special character
      node->_tx_data[node->_tx_data_size] = SDP_DLE;
      node->_tx_data_size++;
      if(node->_tx_data_size >= node->_max_frame_size){
        
        sdp_debug(node, 130);
        return false;
      }
      
      node->_tx_data[node->_tx_data_size] = crc_data[c] ^ SDP_DLE_XOR;  // append XOR-ed data
      node->_tx_data_size++;
      if(node->_tx_data_size >= node->_max_frame_size){
        
        sdp_debug(node, 131);
        return false;
      } 
    }
    else{ // data is not special character
      node->_tx_data[node->_tx_data_size] = crc_data[c];
      
      node->_tx_data_size++;
      if(node->_tx_data_size >= node->_max_frame_size){
        
        sdp_debug(node, 132);
        return false;
      }
    }
  }
  
  return true;  // success
}

/**
* @brief Resets/flush rx buffer, reset index and receiver state machine to default state
* @note This function can be called on UART/interface error handler (like overrun, noise or frame error)
*/
void sdp_reset_node(SDP_data_t *node){
  ring_buffer_flush(&node->_rx_buff); // flush all buffer stored data
  node->rx_data_index = 0;  // reset payload data index/size
  node->_rx_state = SDP_RX_IDLE;
  node->_response = SDP_DATA;
  node->_status = SDP_STATUS_OK;
  node->_expect_response = false;
}


