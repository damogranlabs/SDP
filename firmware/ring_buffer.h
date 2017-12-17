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
 *					https://github.com/simplyembedded/msp430_launchpad/blob/
 *
 * @source  Modified:
 *          http://damogranlabs.com/
 *          https://github.com/damogranlabs
 */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdint.h>

typedef enum{
  RB_OK = 0,
  RB_ERROR, // NULL pointer or invalid attributes
  RB_EMPTY, 
  RB_FULL,
  RB_NOT_ENOUGH_SPACE, // there is not enaugh room for "num" of data to be written into buffer
  RB_NOT_ENOUGH_DATA // there is not enaugh data in buffer
} rb_status_t;

typedef struct {
  uint8_t *buff;  // actual buffer  
  uint32_t n_elem;  // number of s_elem sized elements in this buffer
  uint32_t head;    // number of first free element in buffer (0 ... n_elem-1)
  uint32_t tail;    // number of last used element in buffer (0 ... n_elem-1)
  uint32_t count;   // number of elements stored in buffer  (0 ... n_elem)
  rb_status_t status;   //current status of ring buffer
} volatile rb_att_t;

rb_status_t ring_buffer_init(rb_att_t *rbd, uint32_t size);

rb_status_t ring_buffer_put(rb_att_t *rbd, uint8_t *data, uint32_t num);
rb_status_t ring_buffer_get(rb_att_t *rbd, uint8_t *data, uint32_t num);

rb_status_t ring_buffer_full(rb_att_t *rbd);
rb_status_t ring_buffer_empty(rb_att_t *rbd);

uint32_t ring_buffer_free_elements(rb_att_t *rbd);
uint32_t ring_buffer_size(rb_att_t *rbd);

void ring_buffer_flush(rb_att_t *rbd);
rb_status_t ring_buffer_get_status(rb_att_t *rbd);

void ring_buffer_send_data(rb_att_t *rbd);

#endif /* __RING_BUFFER_H__ */

