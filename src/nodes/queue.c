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

#include "queue.h"

/*
 * This function configures the flash memory before storing a message
 */
void configureWriteBegin()
{
	uint8_t watchdog = WDTCTL;
	WDTCTL = WDTPW|WDTHOLD;

	__disable_interrupt();

	while(FCTL3&BUSY);
	FCTL2 = FWKEY + FSSEL_1 + FN0;
	FCTL3 = FWKEY;
	FCTL1 = FWKEY + WRT;

}

/*
 * This function configures the flash memory after storing a message
 */
void configureWriteEnd()
{
	while(!(FCTL3&WAIT));
	FCTL1 = FWKEY;

	while(FCTL3&BUSY);

	FCTL3 = FWKEY+LOCK;
	__enable_interrupt();


}

/*
 * Wait for flash memory to be available
 */
void waitMemoryWrite()
{

	while((FCTL3&WAIT)==0);
}

/*******************************************************************
 *
 * Global Functions
 *
 *******************************************************************/

/**
 * This function adds an array to a queue
 * @param Queue
 * @param message array
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL add(Queue* queue,uint8_t* values,uint8_t size){

	//verifies the queue available space
	if(eSpace(queue,size) == FALSE ) return FALSE;

	volatile uint8_t *address = queue->address + queue->final;
	//first add the array size

	configureWriteBegin();
	*address = size;
	//queue->counter = queue->counter + 1 > 64 ? 0: queue->counter + 1;

	uint16_t final = queue->final;
	final = final  + 1 > queue->size ? 0: final + 1;
	address = queue->address + final;
	waitMemoryWrite();

	//erase
	//if(queue->counter == 0){
	if(final&(0x1FF) == 0){
		FCTL1 = FWKEY + ERASE;                    // Set Erase bit
		FCTL3 = FWKEY;                            // Clear Lock bit
		*address = 0;                           // Dummy write to erase Flash segment
	}

	volatile uint8_t i;
	for(i = 0;i < size;i++)
	{

		//FCTL3 = FWKEY;
		FCTL1 = FWKEY + BLKWRT + WRT;
		waitMemoryWrite();
		*address = values[i];
		//queue->counter = queue->counter + 1 > 64 ? 0: queue->counter + 1;
		waitMemoryWrite();

		final =final  + 1 > queue->size ? 0: final + 1;
		address = queue->address + final;
		//erase
		//if(queue->counter == 0){
		if(final&(0x1FF) == 0){
			FCTL1 = FWKEY + ERASE;                    // Set Erase bit
			FCTL3 = FWKEY;                            // Clear Lock bit
			*address = 0;                           // Dummy write to erase Flash segment
		}

	}
	queue->final = final;
	configureWriteEnd();


	return TRUE;
}

/*
 * This adds an array to a queue with zigbeepadding
 * @param Queue
 * @param message array
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL addWpad(Queue* queue,uint8_t* values,uint8_t size){

	//verifies the queue available space
	if(eSpace(queue,size) == FALSE ) return FALSE;

	volatile uint8_t *address2 = queue->address + queue->final;

	//first add the array size
	configureWriteBegin();
	*address2 = size;

	uint16_t final = queue->final;
	final = final  + 1 >= queue->size ? 0: final + 1;
	address2 = queue->address + final;
	waitMemoryWrite();
	//erase

	if(final&(0x1FF) == 0){
		if((final  >> 9) == (queue->initial>>9) ){
				//queue->counter = countersave;
				configureWriteEnd();
				return FALSE;
		}

		FCTL1 = FWKEY + ERASE;                    // Set Erase bit
		FCTL3 = FWKEY;                            // Clear Lock bit
	    // Dummy write to erase Flash segment
		*address2 = 0;
		waitMemoryWrite();
		FCTL1 = FWKEY + WRT;
	}
	char pad = FALSE;
	volatile uint8_t i;
	for(i = 0;i < size;i++)
	{

		if(values[i] == 0x7D){
			pad = TRUE;
		}else
		{
			waitMemoryWrite();

			if(pad == FALSE){
				*address2 = values[i];
			}
			else{
				*address2 = values[i]^0x20;
			}

			final =final + 1 >= queue->size ? 0: final + 1;
			address2 = queue->address + final;
			pad = FALSE;
			//erase

			if(final&(0x1FF) == 0){
				if((final  >> 9) == (queue->initial>>9) ){
					configureWriteEnd();
					return FALSE;
				}

				FCTL1 = FWKEY + ERASE;                    // Set Erase bit
				address2 =  queue->address + final;
				FCTL3 = FWKEY;                            // Clear Lock bit

				*address2 = 0; // Dummy write to erase Flash segment
				configureWriteBegin();
				waitMemoryWrite();
			}
		}
	}

	if(FCTL3&ACCVIFG) debug();//  FWKEY;
	queue->final = final;
	configureWriteEnd();
	return TRUE;
}

/**
 * This function removes an array from a queue
 * @param Queue
 * @param message array
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL remove(Queue* queue,uint8_t* values,uint8_t* size){

	if( queue->initial == queue->final) return FALSE;

	volatile uint8_t *address = queue->address + queue->initial;
	uint16_t initial = queue->initial;
	//takes the array size
	*size = *address;

	if(*size == 0){
		queue->initial = 0;
		queue->final = 0;

		configureWriteBegin();
		FCTL1 = FWKEY + ERASE;                    // Set Erase bit
		FCTL3 = FWKEY;
		*(queue->address) = 0;
		configureWriteEnd();
		return FALSE;

	}
	initial = initial  + 1 > queue->size ? 0: initial + 1;
	address = queue->address + initial;

	uint8_t i;
	for(i = 0;i < *size;i++)
	{
		values[i] = *address;

		initial = initial  + 1 > queue->size ? 0: initial + 1;
		address = queue->address + initial;

		if( initial == queue->final) {
			queue->initial = initial;

			return TRUE;}
	}

	queue->initial = initial;

	return TRUE;

}

/**
 * This function verifies the queue remaining space before storing a message
 * @param Queue
 * @param message size
 * @return True if was possible to store message. False otherwise
 */
BOOL eSpace(Queue* queue,uint8_t size){
	int16_t remainS = (int16_t)queue->final - (int16_t)queue->initial;
	int16_t aux = (int16_t)queue->final + (int16_t)size;
	if((aux >>9) != (((int16_t)queue->final) >>9)){
		if( aux > queue->size){
			aux = aux - queue->size;
		}

		if((aux >>9) == (((int16_t)queue->initial) >>9))
			return FALSE;
	}

	if(remainS >= 0)
	{
		if((queue->size - remainS) > (int16_t)size + 1)	return TRUE;
		return FALSE;
	}else
	{
		if(-remainS > (int16_t)size + 1) return TRUE;
		return FALSE;
	}
}

/**
 * Debug function
 */
void debug()
{
	P1DIR |= 0x01;					// Set P1.0 to output direction
		for(;;) {
			volatile unsigned int i;	// volatile to prevent optimization

			P1OUT ^= 0x01;				// Toggle P1.0 using exclusive-OR

			i = 10000;					// SW Delay
			do i--;
			while(i != 0);
		}
}

/**
 * This function initializes the QUEUE
 * @param Queue
 * @param message array
 * @param message size
 * @return
 */

void init(Queue* queue,uint8_t* address,uint16_t size){

	queue->final = 0;
	queue->initial = 0;

	queue->size = size;
	queue->address = address;
}


/* example code
void writeBlock(uint8_t* address,uint8_t* array,uint8_t size )
{
	uint8_t watchdog = WDTCTL;
	WDTCTL = WDTPW+WDTHOLD;
	asm("DINT");
	while(FCTL3&BUSY);
	FCTL2 = FWKEY+FSSEL1+FN0;
	FCTL3 = FWKEY;
	FCTL1 = FWKEY+BLKWRT+WRT;
	int i;
	for(i = 0; i < size; i++)
	{
		address = array[i];
		address++;
		while(FCTL3&WAIT);
	}
	FCTL1 = FWKEY;
	while(FCTL3&BUSY);
	FCTL3 = FWKEY+LOCK;
	asm("EINT");
	WDTCTL = watchdog;
}*/





