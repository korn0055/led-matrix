/**
 * \file
 *
 * \brief Ring buffer
 *
 * Copyright (c) 2011 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
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
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#ifndef RING_BUFFER_H_INCLUDED
#define RING_BUFFER_H_INCLUDED

#include "compiler.h"

typedef struct ring_buffer_tag ring_buffer_t;

/**
 * \defgroup ring_buffer_group Ring buffer
 *
 * This is a generic ring buffer that can be used for data storage e.g. for
 * communication peripherals like the UART.
 *
 * @{
 */

/**
 * \brief Struct for holding the ring buffer
 *
 * This struct is used to hold one ring buffer
 *
 * \note The maximum size of the ring buffer is 256 (0xFF) bytes
 */
struct ring_buffer_tag {
	volatile int16_t write_offset;
	volatile int16_t read_offset;
	//volatile uint8_t iter_offset[RING_MAX_ITERATORS];
	int16_t size;
	uint8_t *buffer;
};

///**
 //* \brief Function to get the next offset in a ring buffer.
 //*
 //* \param cur_offset the current offset in the ring buffer
 //* \param size       the size of the ring buffer in bytes
 //*
 //* \returns next offset or 0 if we are wrapping
 //*/
//static inline uint8_t get_next(uint8_t cur_offset, uint8_t size);
///**
 //* \brief Function to get the next write offset in a ring buffer.
 //*
 //* \param ring pointer to a struct of type ring_buffer_t
 //*
 //* \returns the next write offset in the ring buffer
 //*/
//static inline uint8_t ring_buffer_get_next_write(const struct ring_buffer_t *ring);
//
//
///**
 //* \brief Function to get the next read offset in a ring buffer.
 //*
 //* \param ring pointer to a struct of type ring_buffer_t
 //*
 //* \returns the next read offset in the ring buffer
 //*/
//static inline uint8_t ring_buffer_get_next_read(const struct ring_buffer_t *ring);
//
//
///**
 //* \brief Function for checking if the ring buffer is full
 //*
  //* \param ring pointer to a struct of type ring_buffer_t
 //*
 //* \retval true  if the buffer is full
 //* \retval false if there is space available in the ring buffer
 //*/
//static inline bool ring_buffer_is_full(const struct ring_buffer_t *ring);
//
//
///**
 //* \brief Function for checking if the ring buffer is empty
 //*
 //* \param ring pointer to a struct of type ring_buffer_t
 //*
 //* \retval true   if the buffer is empty
 //* \retval false  if there is still data in the buffer
 //*/
//static inline bool ring_buffer_is_empty(const struct ring_buffer_t *ring);
//
//
///**
 //* \brief Function for initializing a ring buffer
 //*
 //* \param buffer pointer to the buffer to use as a ring buffer
 //* \param size   the size of the ring buffer
 //*
 //* \retval struct ring_buffer_t a struct containing the ring buffer
 //*/
//static inline struct ring_buffer_t ring_buffer_init(uint8_t *buffer, uint8_t size);
//
//
///**
 //* \brief Function for getting one byte from the ring buffer
 //*
 //* Call this function to get a byte of data from the ring buffer.
 //* Make sure buffer is not empty (using \ref ring_buffer_is_empty)
 //* before calling this function.
 //*
 //* \param ring pointer to a struct of type ring_buffer_t
 //*
 //* \returns next data byte in buffer
 //*/
//static inline uint8_t ring_buffer_get(struct ring_buffer_t *ring);
//
//
///**
 //* \brief Function for putting a data byte in the ring buffer
 //*
 //* Call this function to put a byte of data in the ring buffer.
 //* Make sure buffer is not full (using \ref ring_buffer_is_full)
 //* before calling this function.
 //*
 //* \param ring pointer to a struct of type ring_buffer_t
 //* \param data the byte to put to the buffer
 //*
 //*/
//static inline void ring_buffer_put(struct ring_buffer_t *ring, uint8_t data);
//
//
//static inline uint8_t ring_buffer_get_count(struct ring_buffer_t *ring);
//
//
//static inline void ring_buffer_flush(struct ring_buffer_t *ring);


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup ring_buffer_group Ring buffer
 *
 * This is a generic ring buffer that can be used for data storage e.g. for
 * communication peripherals like the UART.
 *
 * \section dependencies Dependencies
 * This ring buffer does not depend on other modules.
 * @{
 */

/**
 * \brief Function to get the next offset in a ring buffer.
 *
 * \param cur_offset the current offset in the ring buffer
 * \param size       the size of the ring buffer in bytes
 *
 * \returns next offset or 0 if we are wrapping
 */
static inline int16_t get_next(int16_t cur_offset, int16_t size)
{
	return (cur_offset == (size - 1) ? 0 : cur_offset + 1);
}

static inline int16_t get_prev(int16_t cur_offset, int16_t size)
{
	return (cur_offset == 0 ? (size - 1) : cur_offset - 1);
}

static inline int16_t get_forward_offset(int16_t cur_offset, int16_t size, int16_t forward_offset)
{
	if(cur_offset + forward_offset > (size - 1))
	{
		return (forward_offset - (size - cur_offset));
	}
	else
	{
		return (cur_offset + forward_offset);
	}	
}

static inline int16_t get_reverse_offset(int16_t cur_offset, int16_t size, int16_t reverse_offset)
{
	if(reverse_offset > cur_offset)
	{
		return ((size - 1) - (reverse_offset - cur_offset));
	}
	else
	{
		return (cur_offset - reverse_offset);
	}
}



/**
 * \brief Function to get the next write offset in a ring buffer.
 *
 * \param ring pointer to a struct of type ring_buffer_t
 *
 * \returns the next write offset in the ring buffer
 */
static inline int16_t ring_buffer_get_next_write(const ring_buffer_t *ring)
{
	return get_next(ring->write_offset, ring->size);
}

/**
 * \brief Function to get the next read offset in a ring buffer.
 *
 * \param ring pointer to a struct of type ring_buffer_t
 *
 * \returns the next read offset in the ring buffer
 */
static inline int16_t ring_buffer_get_next_read(const ring_buffer_t *ring)
{
	return get_next(ring->read_offset, ring->size);
}



/**
 * \brief Function for checking if the ring buffer is full
 *
  * \param ring pointer to a struct of type ring_buffer_t
 *
 * \retval true  if the buffer is full
 * \retval false if there is space available in the ring buffer
 */
inline bool ring_buffer_is_full(const ring_buffer_t *ring)
{
	return (ring->read_offset == ring_buffer_get_next_write(ring));
}

/**
 * \brief Function for checking if the ring buffer is empty
 *
 * \param ring pointer to a struct of type ring_buffer_t
 *
 * \retval true   if the buffer is empty
 * \retval false  if there is still data in the buffer
 */
inline bool ring_buffer_is_empty(const ring_buffer_t *ring)
{
	return (ring->read_offset == ring->write_offset);
}

/**
 * \brief Function for getting one byte from the ring buffer
 *
 * Call this function to get a byte of data from the ring buffer.
 * Make sure buffer is not empty (using \ref ring_buffer_is_empty)
 * before calling this function.
 *
 * \param ring pointer to a struct of type ring_buffer_t
 *
 * \returns next data byte in buffer
 */
inline uint8_t ring_buffer_get(ring_buffer_t *ring)
{
	//Assert(!ring_buffer_is_empty(ring));
		
	//if(ring->read_offset > ring->size -1)
	//{
		////set_led(UART_RX_OVFL_LED_MASK);
			//
		////blink_error_forever();
	//}
	uint8_t data = ring->buffer[ring->read_offset];
	
	ring->read_offset = ring_buffer_get_next_read(ring);
	return data;
}

inline uint8_t ring_buffer_pop(ring_buffer_t *ring)
{
	ring->read_offset = ring_buffer_get_next_read(ring);
}

/**
 * \brief Function for putting a data byte in the ring buffer
 *
 * Call this function to put a byte of data in the ring buffer.
 * Make sure buffer is not full (using \ref ring_buffer_is_full)
 * before calling this function.
 *
 * \param ring pointer to a struct of type ring_buffer_t
 * \param data the byte to put to the buffer
 *
 */
inline void ring_buffer_put(ring_buffer_t *ring, uint8_t data)
{
	//Assert(!ring_buffer_is_full(ring));	
	ring->buffer[ring->write_offset] = data;
	ring->write_offset = ring_buffer_get_next_write(ring);		
}

inline uint8_t ring_buffer_index(ring_buffer_t *ring, int16_t index)
{
	return ring->buffer[get_forward_offset(ring->read_offset, ring->size, index)];
}

inline uint8_t ring_buffer_peek(ring_buffer_t *ring)
{
	return ring->buffer[ring->read_offset];
}

/**
 * \brief Function for initializing a ring buffer
 *
 * \param buffer pointer to the buffer to use as a ring buffer
 * \param size   the size of the ring buffer
 *
 * \retval struct ring_buffer_t a struct containing the ring buffer
 */
inline ring_buffer_t ring_buffer_init(uint8_t *buffer, int16_t size)
{
	ring_buffer_t ring;
	ring.write_offset = 0;
	ring.read_offset = 0;	
	ring.size = size;
	ring.buffer = buffer;
	return ring;
}

inline int16_t ring_buffer_get_count(ring_buffer_t *ring)
{
	if(ring->read_offset > ring->write_offset)
	{
		//return (ring->size - ring->read_offset) + ring->read_offset;
		return (ring->size - ring->read_offset) + ring->write_offset;
	}
	else
	{
		return (ring->write_offset - ring->read_offset);
	}
}

inline void ring_buffer_flush(ring_buffer_t *ring)
{
	ring->read_offset = ring->write_offset;	
}

//static inline void ring_increment_iterator(struct ring_buffer_t *ring)
//{
	//ring->iter_offset = ring_buffer_get_next_iter(ring);
//}
//
//static inline void ring_decrement_iterator(struct ring_buffer_t *ring)
//{
	//ring->iter_offset = ring_buffer_get_prev_iter(ring);
//}
//
//static inline void ring_advance_iterator(struct ring_buffer_t *ring, uint8_t offset)
//{
	//ring->iter_offset = get_forward_offset(ring->iter_offset, ring->size, offset);
//}
//
//static inline void ring_reverse_iterator(struct ring_buffer_t *ring, uint8_t offset)
//{
	//ring->iter_offset = get_reverse_offset(ring->iter_offset, ring->size, offset);
//}
//
//static inline void ring_set_iterator_index(struct ring_buffer_t *ring, uint8_t offset)
//{
	//ring->iter_offset = get_reverse_offset(ring->write_offset, ring->size, offset);
//}


#endif /* RING_BUFFER_H_INCLUDED */
