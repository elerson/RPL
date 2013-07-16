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

#include "zigbee.h"
#define ATCN "ATCN\r\n"
#define ATAP "ATAP2\r\n"
#define ATPL "ATPL0\r\n"
#define ATRO "ATRO9\r\n"

#define HC1_HDR2 31027//01111001 00110011 without address

volatile uint16_t txData;
volatile uint8_t rxBuffer;
volatile uint8_t commandResponse[5];
char frameID;
Queue* queue_;
extern Queue mac2netw;
etxAddr extstruct;

char recievedMessage_[MAXMSGSIZE];
volatile char dataIndex = 0;
volatile char msgsize = 255;
volatile char readwrite = 0;

uint16_t ETXARRAY[NUMEXT];
/*
SOFTWARE UART for zigbee - 9600 bps - it can be changed
*/

//------------------------------------------------------------------------------
// Timer_A UART
//------------------------------------------------------------------------------
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer_A1(void)
{
  static unsigned char txBitCnt = 10;

  switch (__even_in_range(TA1IV, 10))        // Efficient switch-implementation
  {
  case  2:
    TA1CCR1 += UART_TBIT;                     // Add Offset to CCRx
    if (txBitCnt == 0) {                    // All bits TXed?
      TA1CCTL1 &= ~CCIE;                  // All bits TXed, disable interrupt
      txBitCnt = 10;                      // Re-load bit counter

    }
    else {
      if (txData & 0x01) {
        TA1CCTL1 &= ~OUTMOD2;              // TX Mark '1' //set
      }
      else {
        TA1CCTL1|= OUTMOD2;               // TX Space '0' //reset
      }
      txData >>= 1;
      txBitCnt--;
    }
  }
}
//------------------------------------------------------------------------------
// Outputs one byte using the Timer_A UART
//------------------------------------------------------------------------------
void zigbeePutChar(unsigned char byte)
{

  while (TA1CCTL1 & CCIE);                 // Ensure last char got TX'd
  TA1CCR1 = TA1R;                          // Current state of TA1 counter
  TA1CCR1 += UART_TBIT;                    // One bit time till first bit

  txData = byte;                          // Load global variable
  txData |= 0x0100;                        // Add mark stop bit to TXData
  txData <<= 1;                           // Add space start bit

  TA1CCTL1= OUTMOD0 + CCIE;               // Set TXD on EQU0, Int

}

//------------------------------------------------------------------------------
// Outputs one byte using the Timer_A UART
//------------------------------------------------------------------------------
void zigbeePutString(char* string){
	char i = 0;

	for(;;)
	{
		if(string[i] == '\n') return;
		zigbeePutChar(string[i++]);
		__delay_cycles(20000);
	}
}

/* to do not lose messages this function must be called as fast as possible*/

void zigbeeDataProcessing(){

	if(readwrite == 0) return;
	 //__disable_interrupt();
	if(recievedMessage_[3] == 0x81){ // recieved data 16bits
		if((recievedMessage_[8] == 0)  && (recievedMessage_[9] == 0) && (recievedMessage_[10] ==0))
		{	//ignore for while
		}else{
			addWpad(&mac2netw,(uint8_t*) recievedMessage_ , msgsize );
		}
	}
	else if(recievedMessage_[3] == 0x89){ // status
		if(recievedMessage_[4]&ACKMASK){ // expecting ack
			if(recievedMessage_[5] == 0) //ok
				ETXARRAY[recievedMessage_[4]&(~ACKMASK)]+= 256 ;
		}
	}else if(recievedMessage_[3] == 0x88){ // command response
		if(recievedMessage_[4] == COMMANDFRAME){
			commandResponse[0] = recievedMessage_[7];
			commandResponse[1] = recievedMessage_[8];
			commandResponse[2] = recievedMessage_[9];
			commandResponse[3] = recievedMessage_[10];
			commandResponse[4] = recievedMessage_[11];
		}
	}
	dataIndex = 0;
	msgsize = 255;
	readwrite = 0;
	//__enable_interrupt();
}

//------------------------------------------------------------------------------
// Timer_A UART - Receive Interrupt Handler
//------------------------------------------------------------------------------
#pragma vector = TIMER1_A0_VECTOR
__interrupt void zigbeeGetChar_ISR(void)
{
  static unsigned char rxBitCnt = 8;
  static unsigned char rxData = 0;


  TA1CCR0 += UART_TBIT;                 // Add Offset to CCRx
  if (TA1CCTL0 & CAP) {                 // Capture mode = start bit edge
    TA1CCTL0 &= ~CAP;                 // Switch capture to compare mode
    TA1CCR0 += UART_TBIT_DIV_2;       // Point CCRx to middle of D0
  }
  else {
    rxData >>= 1;
    if (TA1CCTL0 & SCCI) {            // Get bit waiting in receive latch
      rxData |= 0x80;
    }
    rxBitCnt--;
    if (rxBitCnt == 0) {             // All bits RXed?

     if(readwrite == 0){

    	 if(dataIndex == 2){
    			 msgsize = rxData + 0x03;
         }else if(rxData == 0x7e){
    			 dataIndex = 0;
    	 }

		 if(msgsize > dataIndex){

				 recievedMessage_[dataIndex++] = rxData;
		 }else{
			 	 readwrite = 1;
			 	 zigbeeDataProcessing();

		 }

     }

     rxBitCnt = 8;                // Re-load bit counter
     TA1CCTL0 |= CAP;              // Switch compare to capture mode
      __bic_SR_register_on_exit(LPM0_bits);  // Clear LPM0 bits from 0(SR)
    }
  }
}


//------------------------------------------------------------------------------
// put the data pad before send to zigbee
//------------------------------------------------------------------------------
void send2Zigbee(char data)
{
	//zigbeePutChar('A');

	if( (data == 0x7E) || (data == 0x7D) ||  (data == 0x11) || (data == 0x13))
	{
		zigbeePutChar(0x7D);
		zigbeePutChar(data^0x20);
	}else
	{
		zigbeePutChar(data);
	}
}
//------------------------------------------------------------------------------
// send data
//------------------------------------------------------------------------------
uint8_t sendData(uint16_t address,BOOL up,OPTION option,char* data,uint16_t size){
	//frame mark
	zigbeePutChar(0x7E);
	//size

	send2Zigbee((char)(((size+5)&0xFF00)>>8));
	send2Zigbee((char)(size+5)&0xFF);
	send2Zigbee(0x01); // 16bits address
	frameID = frameID + 1 == 0 ? 1 : frameID + 1; // new frameid

	//it will identify message that were not successfully  delivered
	char frameid_ = frameID;

	if((up == TRUE) && (option == NONE)){
		frameid_ = etxindex(address);
		if(frameid_ >= NUMEXT)
			frameid_ = frameID&(~ACKMASK);
		else{
			ETXARRAY[frameid_] = ((ETXARRAY[frameid_]+1)&0xFF) == 0xFF ? (ETXARRAY[frameid_]>>1)&(0x7F7F) : ETXARRAY[frameid_] + 1;
			frameid_ |= ACKMASK;
		}
	}else{
		frameid_ &= ~ACKMASK;
	}

	send2Zigbee(frameid_); //frameID
	//destination address
	send2Zigbee((char)(address>>8)&0xFF);
	send2Zigbee((char)address&0xFF);
	//option
	send2Zigbee(option);
	char i;
	char checksum = 0;
	//checksum on frame data
	checksum+=0x01;
	checksum+=frameid_;
	checksum+=(char)(address>>8)&0xFF;
	checksum+=(char)address&0xFF;
	checksum+=option;
	//data
	for(i = 0 ; i < size ; i++)
	{
		send2Zigbee(data[i]);
		checksum += data[i];
	}
	send2Zigbee((0xFF - checksum)); //checksum*

	return frameid_;
}

//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------
void zigbeeInit()
{
  TA1CCTL1 = OUT;                          // Set TXD Idle as Mark = '1'
  TA1CCTL0 = SCS + CM1 + CAP + CCIE;       // Sync, Neg Edge, Capture, Int
  TA1CTL = TASSEL_2 + MC_2 + TAIE + ID_3;  // SMCLK, start in continuous mode
}

//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------

void zigbeeConfigure(Queue* queue,char* extaddr){

	_delay_cycles(12000000);
	zigbeePutString("+++\n");
	__delay_cycles(8000000);
	__delay_cycles(800000);
	zigbeePutString(ATAP);
	zigbeePutString(ATPL);
	zigbeePutString(ATRO);
	queue_ = queue;
	zigbeePutString(ATCN);

	//configurezigbee
	commandResponse[0] = 255;
	while(commandResponse[0] == 255){
		sendCommand("SL",FALSE,0);
		__delay_cycles(500000);
	}

	commandResponse[0] = 255;
	while(commandResponse[0] == 255){
		sendCommand("MY",TRUE,(uint8_t*)&commandResponse[3]);
		__delay_cycles(500000);
	}

	commandResponse[0] = 255;
	while(commandResponse[0] == 255){
		sendCommand("WR",FALSE,0);
		__delay_cycles(500000);
	}


	//ETX
	uint8_t i;
	for(i = 0; i < NUMEXT; i++)
		ETXARRAY[i] = 0;

	extstruct.address = extaddr;
	extstruct.size = NUMEXT;
	//erase etx area
	configureWriteBegin();
	FCTL1 = FWKEY + ERASE;
	FCTL3 = FWKEY;
	*extstruct.address = 0;
	waitMemoryWrite();
	configureWriteEnd();
}
/**/
void sendCommand(char* command,BOOL set,uint8_t* value){
	zigbeePutChar(0x7E);
	send2Zigbee(0x00);
	if(set == TRUE){
		send2Zigbee(0x08);
	}else{
		send2Zigbee(0x04);
	}
	send2Zigbee(0x08);
	send2Zigbee(COMMANDFRAME);
	send2Zigbee(command[0]);
	send2Zigbee(command[1]);
	if(set == TRUE){
		send2Zigbee(0);
		send2Zigbee(0);
		send2Zigbee(value[0]);
		send2Zigbee(value[1]);
	}
	char checksum = 0;
	checksum+= 0x08;
	checksum+= COMMANDFRAME;
	checksum+= command[0];
	checksum+= command[1];
	if(set == TRUE){
		checksum+= value[0];
		checksum+= value[1];
	}
	send2Zigbee((0xFF - checksum));
}

uint16_t getZigbeeAddress(){

	commandResponse[0] = 255;
	while(commandResponse[0] == 255){
		sendCommand("MY",FALSE,0);
		__delay_cycles(100000);
	}
	uint16_t address;
	address = commandResponse[1];
	address <<=8;
	address |= commandResponse[2];
	return address;

}


/* ETX */
void measureETX(uint16_t address,uint8_t id){

	zigbeePutChar(0x7E);
	//size

	send2Zigbee((char)(((8)&0xFF00)>>8));
	send2Zigbee((char)(8)&0xFF);
	send2Zigbee(0x01); // 16bits address
	frameID = frameID + 1 == 0 ? 1 : frameID + 1; // new frameid

	//it will identifie message that were not successfully  delivered
	if(id > NUMEXT) return;

	char frameid_ = id | ACKMASK;

	ETXARRAY[frameid_] = ((ETXARRAY[frameid_]+1)&0xFF) == 0xFF ? (ETXARRAY[frameid_]>>1)&(0x7F7F) : ETXARRAY[frameid_] + 1;

	send2Zigbee(frameid_); //frameID
	//destination address
	send2Zigbee((char)(address>>8)&0xFF);
	send2Zigbee((char)address&0xFF);
	//option
	send2Zigbee(NONE);

	char checksum = 0;
	//checksum on frame data
	checksum+=0x01;
	checksum+=frameid_;
	checksum+=(char)(address>>8)&0xFF;
	checksum+=(char)address&0xFF;
	checksum+=NONE;
	//data
	//send2Zigbee(HC1_HDR2&0xFF);
	//send2Zigbee((HC1_HDR2>>8)&0xFF);TODO:
	send2Zigbee(0);
	send2Zigbee(0);
	send2Zigbee(0);
	//checksum +=HC1_HDR2&0xFF;
	//checksum +=(HC1_HDR2>>8)&0xFF;
	send2Zigbee((0xFF - checksum)); //checksum*

}

uint8_t etxindex(uint16_t addrSeach){

	uint8_t i;
	uint16_t macaddr;
	uint8_t* address;
	address = (uint8_t*) extstruct.address;
	for(i = 0;i < extstruct.size;i++)
	{
		macaddr = (*address)&0xFF;
		address++;
		macaddr |= (*address << 8);
	    address++;

		if(macaddr==0xFFFF){
			address-=2;
			configureWriteBegin();
			*address = (addrSeach&0xFF);
			address++;
			waitMemoryWrite();
			*address = ((addrSeach>>8)&0xFF);
			waitMemoryWrite();
			configureWriteEnd();
			return i;
		}

		if(macaddr == addrSeach){
			return i;
		}
	}

return 255;
}

uint8_t getETX(uint8_t id){
	uint16_t acks;
	uint16_t numPackets;
	acks = (ETXARRAY[id]>>8)&0x00FF;
	numPackets = ETXARRAY[id]&0x00FF;
	if(acks > numPackets) {
		ETXARRAY[id] |= acks;
		return 1;
	}
	if(acks == 0) return 255;
	return (numPackets<<3)/acks;
}

