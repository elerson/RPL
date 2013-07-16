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

#ifndef ZIGBEE_H_
#define ZIGBEE_H_
#include "platform.h"

#include <stdint.h>
#include "queue.h"
#include "utils.h"

#ifdef SIMULATION
#else
#include <msp430g2553.h>
#endif


#define UART_TXD   BIT1                     // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD   BIT0                     // RXD on P1.2 (Timer0_A.CCI1A)

#define RX BIT1
#define TX BIT2

#define UART_TBIT_DIV_2     ((8000000/8) / (9600*2 ))
#define UART_TBIT           ((8000000/8) / 9600)
#define MAXMSGSIZE 130

#define RECIVEDDATAINDEX 8
#define RECIVEDRSSINDEX 6
#define RECIVEDADDRINDEX 4

#define UPMASK 128
#define ACKMASK 64

#define AVGACK 10

#define BROADCASTMAC 0xffff
#define COMMANDFRAME 10

#define NUMEXT 8
typedef struct etxstruc_{
	char* address;
	uint8_t size;
} etxAddr;

typedef enum OPTION_ {NONE = 0x00,DISBLEACK = 0x01,BROADCASTPANID = 0x04} OPTION;
typedef struct dataRecieved_{
	uint16_t size;
	char* data;
}dataRecieved;

void zigbeeInit();
/*assumes the zigbee in AT state*/
void zigbeeConfigure(Queue* queue,char* extaddr);
void zigbeeDataProcessing();

/*send data to address and return the cmdID*/
uint8_t sendData(uint16_t address,BOOL up,OPTION option,char* data,uint16_t size);
void zigbeePutChar(unsigned char byte);
uint16_t getZigbeeAddress();
void sendCommand(char* command,BOOL set,uint8_t* value);
void configureWriteEnd();
void configureWriteBegin();
void waitMemoryWrite();
uint8_t etxindex(uint16_t addrSeach);
void measureETX(uint16_t address,uint8_t id);
uint8_t getETX(uint8_t id);

#endif /* ZIGBEE_H_ */
