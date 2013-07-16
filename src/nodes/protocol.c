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

#include "protocol.h"
#define INITSATE 0
#define NETWSATE 1
#define TRANSPORTSTATE 2
#define APPSTATE 3

/*******************************************************************************************************
 *
 *
 * global variables
 *
 *
 *******************************************************************************************************/

//mac -> netw
Queue mac2netw;
//netw->trans
Queue netw2trans;
//trans -> netw
Queue trans2netw;
//
char auxarray[AUXARRAYSIZE];

//this comes from rpl timer in rpl.c
extern uint8_t timerToSend;

/*
 * Local functions
 *
 * */

void macLayer();
void applicationLayer();
void networkLayer();
void transportLayer();
void applicationLayerUp();
void applicationLayerDown();
void networkLayerUp();
void networkLayerDown();
void transportLayerUp();
void transportLayerDown();


/**
 * This function initializes the QUEUE
 */
void applicationLayerUp(){

}

/**
 * This function initializes the QUEUE
 */
void applicationLayerDown(){

}

/**
 * This function read data from lower layer
 */
void networkLayerDown(){
//read data from mac2netw queue
	uint8_t size;
	BOOL result = remove(&mac2netw,(uint8_t*) auxarray,&size);
	volatile uint16_t address = 0;
	if(result == TRUE)
	{
		volatile char apiindex = auxarray[APIIDENT_INDEX];
		switch(apiindex)
		{
			case RECIEVED16BITS:
					address = auxarray[APIIDENT_INDEX+1];
					address = address << 8;
					address |=  auxarray[APIIDENT_INDEX+2];
					reciecedFromMac(auxarray[APIIDENT_INDEX+3],address,auxarray[APIIDENT_INDEX+4],auxarray+8,size-8); //rpl
				break;
			default:
				break;
		}
	}
}

/**
 * This function initializes the QUEUE
 */
void networkLayerUp(){

}

/**
 * This function initializes the QUEUE
 */
void transportLayerUp(){

}

/**
 * This function initializes the QUEUE
 */
void transportLayerDown(){

}

/**
 * This function initializes the QUEUE
 */
void applicationLayer(){
		applicationLayerUp();
		applicationLayerDown();
}

/**
 * This function initializes the QUEUE
 */
void networkLayer(){
	//	networkLayerUp();
	static char rplalg = 0;
	//execute one algorithm at time
	if(rplalg == 1)
		networkLayerDown();
	else
		executeRpl();
	rplalg ^= 1;
}

/**
 * This function initializes the QUEUE
 */
void transportLayer(){
		transportLayerUp();
		transportLayerDown();
}

/**
 * This function executes the rpl protocol
 */
void execNetworkProtocol(){
	static char state = NETWSATE;
		switch (state) {
			case NETWSATE:
				networkLayer();
				break;
			case TRANSPORTSTATE:
				transportLayer();
				break;
			case APPSTATE:
				applicationLayer();
				break;
			default:
				state = INITSATE;
				break;
		}
		state++;
}

/**
 * This function initializes the all the protocol
 */
void NetworkProtocolConfigure()
{
	zigbeeConfigure(&mac2netw,(char*) 0xF4EE);
    init(&mac2netw,(uint8_t*)0xFA00,1024);
	rplInit(auxarray,&mac2netw,0);

}

/**
 * This function sends a message to MAC layer
 */
void toMac(uint16_t macaddress,BOOL up,OPTION opt,char* data,char size){
	sendData(macaddress,up,opt,data,size);
	timerToSend = TIMETOWAIT;
}


