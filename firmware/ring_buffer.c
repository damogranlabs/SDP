/**
 ******************************************************************************
 * File Name          : ring_buffer
 * Description        : This file provides code for the initialization
 *                      of circuilar (ring) buffer
 * @date    13-Aug-2017
 * @author  Originally: Chris Karaplis, modified by Domen Jurkovic
 * @version v1.2
 *
 * Copyright (c) 2015, simplyembedded.org
 * Copyright (c) 2017, damogranlabs.com
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  
 * @source  Original Source
 *          http://www.simplyembedded.org/
 *					https://github.com/simplyembedded/msp430_launchpad/
 *
 * @source  Modified:
 *          http://damogranlabs.com/
 *          https://github.com/damogranlabs
 *
 *    1. Create ring buffer descriptor structure, define buffer size:
 *        #define RX_BUFF_SIZE 128
 *        rb_att_t rx_buff;
 *
 *    3. Init ring buffer and check for error (for example, your error report function is called handle_error()):
 *        if(ring_buffer_init(&rx_buff, RX_BUFF_SIZE) != RB_OK) handle_error();
 *
 *    4. Put/get data to/from ring buffer:
 *        ring_buffer_put(&rx_buff, data, 20);
 *        ring_buffer_get(&rx_buff, data, 10);
 */

/* Includes ------------------------------------------------------------------*/
#include "ring_buffer.h"

#include <string.h>
#include <stdlib.h>

/**
 * @brief Initialize a ring buffer
 * @param *rbd - pointer to the ring buffer descriptor
 * @param size - ring buffer size in number of bytes
 * @return RB_ERROR, RB_OK
 */
rb_status_t ring_buffer_init(rb_att_t *rbd, uint32_t size){
  rbd->status = RB_ERROR;
  
  
  if(rbd != NULL){  // rbd must not be pointer to nowhere
    rbd->buff = calloc(size, sizeof(uint8_t)); // allocate memory of "size" bytes, set all values to 0.
    //rbd->buff = malloc(size * sizeof(uint8_t)); // Use this instead of calloc() if you don't wish to set all values to 0 by default.

    if(rbd->buff != NULL){  // buff must not be pointer to nowhere
      rbd->n_elem = size;

      // Initialize the ring buffer internal variables
      rbd->head = 0;
      rbd->tail = 0;
      rbd->count = 0;
      
      rbd->status = RB_OK;
    }
  }

  return rbd->status;
}
        
/**
 * @brief Add a number of elements to the ring buffer
 * @param *rbd - pointer to the ring buffer descriptor
 * @param data - the data to add
 * @param num - number of elements to add
 * @return RB_NOT_ENOUGH_SPACE, RB_OK, RB_ERROR
 */
rb_status_t ring_buffer_put(rb_att_t *rbd, uint8_t *data, uint32_t num){
  rb_status_t status = RB_ERROR;
  uint32_t num_to_end = 0; // number of elements to the last buffer element (including current one (head))
  
  if(rbd != NULL){  // rbd must not be a pointer to nowhere
    if(ring_buffer_free_elements(rbd) >= num){  // is there enough space in buffer for num of data
      if(rbd->head >= rbd->n_elem){ // reset ring buffer head
        rbd->head = 0;
      }
      
      num_to_end = rbd->n_elem - rbd->head; // that many data can be written into buffer, before reaching buffer last element
      if(num_to_end < num){ // if there is not enough space for "num" of data before reaching buffer last element
        memcpy(&(rbd->buff[rbd->head]), data, num_to_end); //write to buffer partial data
        memcpy(rbd->buff, (data + num_to_end), (num - num_to_end)); //write to buffer rest of the data, starting with buffer[0]
        
        rbd->head = num - num_to_end;  //head
        rbd->count = rbd->count + num; // increment counter for num of data
      }
      else{ // there is enough space before reaching buffer's last element
        memcpy(&(rbd->buff[rbd->head]), data, num); //write to buffer all num of data in one piece
        
        rbd->head = rbd->head + num;  // increment head
        rbd->count = rbd->count + num; // increment counter for num of data
      }
      
      status = RB_OK;
    }
    else{ // there is not enough space in buffer for num of data
      status = RB_NOT_ENOUGH_SPACE;
    }
  }
  rbd->status = status;
  return status;
}

/**
 * @brief Get (and remove) a number of elements from the ring buffer
 * @param *rbd - pointer to the ring buffer descriptor
 * @param data - pointer to store the data
 * @param num - number of elements to read
 * @return RB_NOT_ENOUGH_DATA, RB_OK, RB_ERROR
 */
rb_status_t ring_buffer_get(rb_att_t *rbd, uint8_t *data, uint32_t num){
  rb_status_t status = RB_ERROR;
  uint32_t num_to_end = 0;
  
  if(rbd != NULL){  // rbd must not be a pointer to nowhere
    if(ring_buffer_size(rbd) >= num){ //buffer is not empty and there is at least num of data stored
      if(rbd->tail >= rbd->n_elem){ // reset ring buffer tail
        rbd->tail = 0;
      }
      
      num_to_end = rbd->n_elem - rbd->tail;
      if(num >= num_to_end){ // is there enough ("num") of data to read before reaching buffer last element
        memcpy(data, &(rbd->buff[rbd->tail]), num_to_end); //read from buffer partial data
        memcpy(data + num_to_end, rbd->buff, (num - num_to_end)); //read from buffer remaining partial data
          
        rbd->tail = num - num_to_end;  // tail increment
        rbd->count = rbd->count - num; // decrement counter for num of data
      }
      else{ //there is enough data to be read before reaching last element of ring buffer
        memcpy(data, &(rbd->buff[rbd->tail]), num); //read from buffer
        
        rbd->tail = rbd->tail + num;  // increment tail
        rbd->count = rbd->count - num; // decrement counter for num of data
      }
      status = RB_OK;
    }
    else{
      status = RB_NOT_ENOUGH_DATA;
    }
  }
  
  rbd->status = status;
  return status;
}

/**
 * @brief Check if ring buffer is full
 * @param *rbd - pointer to the ring buffer descriptor
 * @return RB_FULL if empty, RB_OK if not, RB_ERROR if invalid parameters. 
 */
rb_status_t ring_buffer_full(rb_att_t *rbd){
  rb_status_t status = RB_ERROR;
  
  if(rbd != NULL){
   if(rbd->count >= rbd->n_elem){
     status = RB_FULL;
   }
   else{
     status = RB_OK;
   }
  }
  
  rbd->status = status;
  return status;
}

/**
 * @brief Check if ring buffer is empty
 * @param *rbd - pointer to the ring buffer descriptor
 * @return RB_EMPTY if empty, RB_OK if not, RB_ERROR if invalid parameters. 
 */
rb_status_t ring_buffer_empty(rb_att_t *rbd){
  rb_status_t status = RB_ERROR;
  
  if(rbd != NULL){
   if(rbd->count == 0){
     status = RB_EMPTY;
   }
   else{
     status = RB_OK;
   }
  }
  
  rbd->status = status;
  return status;
}

/**
 * @brief Check how many elements can be written to the buffer
 * @param *rbd - pointer to the ring buffer descriptor
 * @return 0 on invalid parameter, number of elements otherwise
 */
uint32_t ring_buffer_free_elements(rb_att_t *rbd){
  uint32_t ret_val = 0;
  
  if(rbd != NULL){  // rbd must not be a pointer to nowhere
   ret_val = rbd->n_elem - rbd->count;
  }
  
  return ret_val;
}

/**
 * @brief Get the number of bytes stored in ring buffer
 * @param *rbd - pointer to the ring buffer descriptor
 * @return 0 if invalid parameters, number of data stored in buffer otherwise
 */
uint32_t ring_buffer_size(rb_att_t *rbd){
  uint32_t size = 0;
  
  if(rbd != NULL){  // rbd must not be a pointer to nowhere
    size = rbd->count;
  }
  
  return size;
}

/**
 * @brief Flush data from ring buffer (discard head, tail and count data)
 * @param *rbd - pointer to the ring buffer descriptor
 */
void ring_buffer_flush(rb_att_t *rbd){
  rbd->head = 0;
  rbd->tail = 0;
  rbd->count = 0;
  
  memset(rbd->buff, 0, rbd->n_elem);  // set all values back to 0.
}

/**
 * @brief Get ring buffer current status
 * @param *rbd - pointer to the ring buffer descriptor
 */
rb_status_t ring_buffer_get_status(rb_att_t *rbd){
  return rbd->status;
}

/**
 * @brief User function to print out buffer details
 * @param *rbd - pointer to the ring buffer descriptor
 */
//#include "comm.h"
void ring_buffer_send_data(rb_att_t *rbd){
  //send_data(CU_TO_PC, rbd->buff);
}


