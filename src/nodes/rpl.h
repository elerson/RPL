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

#ifndef RPL_H_
#define RPL_H_
#include "platform.h"
//#define SIMULATION

//#define DEBUGMULTIHOP
#define DEBUG

#define DAORETRIES 15
#define DAOTIME 500

#define IMAX 65200 // TODO 65200
#define IMIN 4
#define ID 2
//HEADER
#define HC1_HDR 31027//01111001 00110011 without address
#define HC2_HDR 26658//01101000 00100010 16bits address

#define FALINGTHRESHOLD 4
#define DISTRIES 5
#define DISTIME 250

#define NUMPARENTS 4
#define NUMMSGADDR 4
#define PARENTNULL 0xffff
#define INFRANK 0xffff
#define NUMINFDIO 8

#define DONOTHING 0
#define RETRANSMIT 1
#define TRANSMITTOPARENT 2

#define MINHOPINCREASE 8
#define MAXHOPINCREASE 64

#define NULLADDR 0xff

#define LASTFRAGMENT 1

#ifdef SIMULATION

#else
#include <msp430g2553.h>
#endif

#include "ip6.h"
#include "queue.h"
#include "protocol.h"
#include "utils.h"

typedef struct msgaddr_{
	char id[2];
	uint16_t address;
}Msgaddress;

typedef struct parent_{
	uint16_t macaddress;
	uint16_t rank;
}Parent;

typedef struct rpl_{
	    Parent parent[NUMPARENTS];
	    Msgaddress msgaddress[NUMMSGADDR];
	    uint16_t dodagid;
	    uint8_t rplinstanceid;
	    uint8_t versionNumber;
	    uint8_t GO_MOP_Prf;
	    uint8_t DTSN;
	    uint16_t rank;
	    uint16_t minrank;
	    BOOL initialized;
	    uint16_t selfaddr;
}Rpl;


typedef struct Diotimer_
{
	uint16_t t;
	uint16_t I;
	uint16_t counter;
}Diotimer;

typedef enum RCVOPT_ {RESERVED = BIT0,BROADCAST = BIT1,PANBROADCAST = BIT2} RCVOPT;

/* This function receives a message from mac layer
* @param rssi of recieved packdge
* @param address of message
* @param received status
* @param raw message
* @param message size
* @return void
*/
void reciecedFromMac(char rssi,uint16_t address, char rcv,char *data,char size);

/**
 * This function initializes the rpl instance
 *
 * @param message
 *  @param Queue
 *  @param timerTosend_
 * @return
 */
void rplInit(char* array,Queue* mac2netw_,uint8_t** timerTosend_);
/**
 * This function send DIO`s and DAO`s messages (this depends the timer values)
 *
 */
void executeRpl();


#endif /* RPL_H_ */
