/* Copyright 2013 - Elerson Rubens da S. Santos <elerss@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef QUEUE_
#define QUEUE_

#include "platform.h"
#include <stdint.h>

#ifdef SIMULATION
#else
#include <msp430g2553.h>
#endif
#include "utils.h"

typedef struct Queue_{
	volatile uint8_t *address;
	volatile uint16_t size;
	volatile uint16_t initial;
	volatile uint16_t final;
}Queue;

/*****************************************************************
/				Functions to manipulate the QUEUE
****************************************************************/
/**
 * Debug function
 */
void debug();
/*
 * This adds an array to a queue with zigbeepadding
 * @param Queue
 * @param message array
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL addWpad(Queue* queue,uint8_t* values,uint8_t size);
/**
 * This function adds an array to a queue
 * @param Queue
 * @param message array
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL add(Queue*,uint8_t* values,uint8_t size);
/**
 * This function removes an array from a queue
 * @param Queue
 * @param message array
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL remove(Queue*,uint8_t* values,uint8_t* size);
/**
 * This function verifies the queue remaining space before storing a message
 * @param Queue
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL eSpace(Queue*,uint8_t size);
/**
 * This function initializes the QUEUE
 * @param Queue
 * @param message array
 * @param message size
 * @return
 */
void init(Queue* queue,uint8_t* address,uint16_t size);

#endif /* QUEUE_ */
