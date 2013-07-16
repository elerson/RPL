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

#include "rpl.h"

/*******************************************************************************************************
 *
 *
 * global variables
 *
 *
 *******************************************************************************************************/

static uint16_t nextrand= 1;
uint16_t daotime = 0;
uint16_t daoretries = 0;
uint16_t diocounter = 0;
uint8_t timerToSend = 0;
uint8_t pingrequestnumber = 0;
BOOL sendDIO_;
BOOL sendDAO_;
char sendDIS_ = 0;
char DISTimer = 0;
char DAOSEQ = 0;
char sendDIOInfRank = 0;
volatile char *memArray;
Diotimer diotimer;
Rpl rpl;
Queue* mac2netwPointer;

//this came from zigbee
extern uint16_t ETXARRAY[NUMEXT];
/*********************************************************************
 *
 *
 * local functions
 *
 *
 *********************************************************************/
void recieveDIO(DIO* dio,uint16_t macaddress);
char recieveDAO(DAO* dao,uint16_t *addr);
void recieveDAOACK(DAO_ACK* daoack);
char recieveHC2HDR(uint16_t* address, char *data);
char createDIO(Rpl *rpl,char* outData);
char create6LowpanHeader1(char* data,uint16_t hc1);
void sendDio();
void resetTimer();
void incrementTimer();


/*****************************************************************************************
 *
 *
 * 									CRC16 code
 *
 *
 *****************************************************************************************/

#define CRC16_POLY			0x8005
#define CRC16_FINAL_XOR     0x0
#define CRC16_INIT_REM      0x0
#define CRC16_CHECK         0xFEE8

// this is a C-optimized implementation
uint16_t crc16MakeBitwise2(uint16_t crc, uint16_t poly,
						unsigned char *pmsg, unsigned int msg_size)
{
    unsigned int i, j;
    uint16_t msg;

    for(i = 0 ; i < msg_size ; i ++)
    {
        msg = (*pmsg++ << 8);

		for(j = 0 ; j < 8 ; j++)
        {
            if((msg ^ crc) >> 15) crc = (crc << 1) ^ poly;
			else crc <<= 1;
			msg <<= 1;
        }
    }

    return(crc ^ CRC16_FINAL_XOR);
}

/*****************************************************************************************
 *
 *
 * 									Rand functions
 *
 *
 *****************************************************************************************/

int rand(void){
#ifdef SIMULATION

#else
	nextrand = nextrand << 1 + 13;
    return nextrand;
#endif
}

void srand(){
#ifdef SIMULATION

#else
	nextrand = TA0R;
#endif
}

/*****************************************************************************************
 *
 *
 * 									Timer code
 *
 *
 *****************************************************************************************/

void resetDAO(){
	sendDAO_ = FALSE;
	daotime = DAOTIME;
	daoretries = DAORETRIES;
}

void stopDAO(){
	daotime = 0;
	daoretries = 0;
}

void resetTimer(){
	diotimer.I = IMIN;
	srand();
	diotimer.t = IMIN/2 + rand()%IMIN/2;
	diotimer.counter = 0;
}

void incrementTimer(){
	if((diotimer.I << 1) < IMAX){
		diotimer.I = diotimer.I << 1;
	}
	srand();
	diotimer.t = diotimer.I/2 + rand()%(diotimer.I>>1);
	diotimer.counter = 0;
}

void RplTimerConfig(){
#ifdef SIMULATION

#else
	  TACTL = TASSEL_2+MC_3+ID_3;   // SMCLK = 8 MHz, SMCLK/2 = 500 KHz (  2 us) , up/down
	  CCR0 =  625*8;                // 0.01 segundo
	  CCTL0 = CCIE;                 // Habilita interrupção de comparação do timer A
#endif
}

// Timer A0 interrupt service routine
#ifdef SIMULATION
void TimerRPL (void) {
#else
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerRPL (void) {
#endif
	//DIO Timer
	if(rpl.initialized == TRUE){
		diotimer.counter++;

	}
	if(diotimer.counter == diotimer.t){
		sendDIO_ = TRUE;
		diocounter++;
	}else if(diotimer.counter == diotimer.I){
		incrementTimer();
	}

	//DAO Timer
	if(sendDAO_ == FALSE)
		if(daoretries > 0){
			if(daotime > 1){
				daotime--;

			}else{
				sendDAO_ = TRUE;
				daotime = DAOTIME;
				daoretries--;
			}
		}

	if(timerToSend > 0)
		timerToSend--;

	if(DISTimer > 0)
		DISTimer--;
}

/*****************************************************************************************
 *
 *
 * 									RPL code
 *
 *
 *****************************************************************************************/

/**
 * This function receives a message from mac layer
 * @param rssi of recieved packdge
 * @param address of message
 * @param received status
 * @param raw message
 * @param message size
 * @return void
 */
void reciecedFromMac(char rssi,uint16_t address, char rcv,char *data,char size){

	char *dataaux = data;

	LowPan1* lowpan;
	DIS* dis;
	icmp6_hdr* hdr;


	volatile char returndata;
	uint16_t addr;

	lowpan = (LowPan1*) dataaux;
	uint16_t iphc;
	COPYARRAY2INT16(iphc,lowpan->IPHC);


	switch (iphc) {
		case HC1_HDR:

			if(lowpan->next == ICMPv6){

				dataaux += sizeof(LowPan1);
				hdr = (icmp6_hdr*)dataaux;
				//recieved dio
				if((hdr->icmp6_type == RPL_TYPE) && (hdr->icmp6_code == DOGAG_DIO_CODE)){
						recieveDIO((DIO*)dataaux,address);
				}

				//recieved dao
				if((hdr->icmp6_type == RPL_TYPE) && (hdr->icmp6_code == DOGAG_DAO_CODE)){
						returndata = recieveDAO((DAO*)dataaux,&addr);
						if((returndata != DONOTHING) && (addr != BROADCASTMAC))
							toMac(addr,TRUE,NONE,data,size); //resend
				}

				//recieved dis
				if((hdr->icmp6_type == RPL_TYPE) && (hdr->icmp6_code == DOGAG_DIS_CODE)){
						dis = (DIS*) dataaux;
						if((dis->code == 0) && (dis->flags == 0)){
							resetTimer(); // reset dio timer
						}
				}
			}

			break;

		case HC2_HDR:

			returndata = recieveHC2HDR((&addr),dataaux);
			returndata;
			if((returndata != DONOTHING) && (addr != BROADCASTMAC))
			{
				if(returndata == RETRANSMIT)
					toMac(addr,FALSE,NONE,data,size);
				else
					toMac(addr,TRUE,NONE,data,size); // to parent
			}


			break;
		default:
			addr = 0;
			break;
	}
}

/**
 * This function creates DAO messages
 * @param Rpl struct instance
 * @param array to store message
 * @return size of stored message
 */
char createDAO(Rpl *rpl,char* outData){
	DAO* dao = (DAO*) outData;
	dao->type = RPL_TYPE;
	dao->code = DOGAG_DAO_CODE;
	dao->rplinstanceid = rpl->rplinstanceid;
	dao->kdflags = 192; // expects daoack and dodaid is present
	dao->reserved = 0;
	dao->dao_sequence = DAOSEQ; //TODO: dao nao incrementa sempre
	COPYUINT16(dao->dodagid,rpl->dodagid);
    uint16_t checksum = calculateDAOChecksum(dao);
    COPYUINT16(dao->checksum,checksum);
    return sizeof(DAO);
}

/**
 * This function creates DAOTarget messages
 * @param Rpl struct instance
 * @param array to store message
 * @return size of stored message
 */
char createDAOTarget(Rpl *rpl, char* outData){
	RPLTARGET* rpltarget = (RPLTARGET*) outData;
	rpltarget->type = 0x05;
	rpltarget->len = 1; // it will be followed by a transit information
	rpltarget->flags = 0;
	rpltarget->prefixlength = 16; // 16bits dodagid
	COPYUINT16(rpltarget->prefix,rpl->selfaddr);
	return sizeof(RPLTARGET);
}

/**
 * This function creates DAO Transit info messages
 * @param address
 * @param array to store message
 * @return size of stored message
 */
char createDAOTransinfo(uint16_t transaddr, char* outData){
	TRANSINFO* rpltransinfo = (TRANSINFO*) outData;
	rpltransinfo->type = 0x06;
	rpltransinfo->len = 0; // nothing more
	rpltransinfo->eflags = 0;
	rpltransinfo->pathcontrol = 0;
	rpltransinfo->pathsequence = 0;
	rpltransinfo->pathlifetime = 0xff; // infinity
	COPYUINT16(rpltransinfo->_parentaddress,transaddr);
	return sizeof(TRANSINFO);
}

/**
 * This function creates ping resquest
 * @param array to store message
 * @return size of stored message
 */
char createPingRequest(char* data){
	ECHOREQUEST* reply = (ECHOREQUEST*) data;
	reply->type = PINGREQUEST_TYPE;
	reply->code = 0;
	reply->checksum[0] = pingrequestnumber;
	reply->checksum[1] = 0;
	reply->identifier[0] = 0;
	reply->identifier[0] = 0;
	reply->sequence[0] = pingrequestnumber;
	reply->sequence[1] = 0;
	return ECHOREQUESTSIZE;
}

/**
 * This function creates ping reply message
 * @param ECHOREQUEST message
 * @param array to store message
 * @return size of stored message
 */
char createPingReply(ECHOREQUEST* request,char* data){
	ECHOREPLY* reply = (ECHOREPLY*) data;
	reply->type = PINGREPLY_TYPE;
	reply->code = 0;
	reply->checksum[0] = request->checksum[0];
	reply->checksum[1] = request->checksum[1];
	reply->identifier[0] = request->identifier[0];
	reply->identifier[1] = request->identifier[1];
	reply->sequence[0] = request->sequence[0];
	reply->sequence[1] = request->sequence[1];
	return ECHOREPLYSIZE;
}

/**
 * This function creates DIS messages
 * @param Rpl instance
 * @param array to store message
 * @return size of stored message
 */
char createDIS(Rpl *rpl,char* outData){
	DIS * dis = (DIS*) outData;
	dis->type = RPL_TYPE;
	dis->code = DOGAG_DIS_CODE;
	dis->flags = 0;
	dis->reserved = 0;
	dis->checksum[0] = 0;
	dis->checksum[1] = 0;
	return sizeof(DIS);

}

/**
 * This function creates a DIO message
 * @param Rpl instance
 * @param array to store message
 * @return size of stored message
 */
char createDIO(Rpl *rpl,char* outData){

	DIO* dio = (DIO*) outData;
	dio->type = RPL_TYPE;
	dio->code = DOGAG_DIO_CODE;
	dio->rplinstanceid = rpl->rplinstanceid;
	dio->versionNumber = rpl->versionNumber;
	dio->reserved = 126; // TODO
	//verifies id it is need send dio with inf rank
	if(sendDIOInfRank == 0){
		COPYUINT16(dio->rank,rpl->rank);
	}else
		if(sendDIOInfRank > 1){
			sendDIOInfRank--;
			COPYUINT16(dio->rank,INFRANK);
		}else if(sendDIOInfRank == 1){
			sendDIOInfRank--;
			COPYUINT16(dio->rank,INFRANK);
			resetTimer(); // it was sent all the inf dio // reset timer
		}

	dio->DTSN = rpl->DTSN;
	dio->GO_MOP_Prf = rpl->GO_MOP_Prf;
	dio->flags = 0;
    //in6_addr dodagid;
	COPYUINT16(dio->dodagid,rpl->dodagid);
	uint16_t checksum = calculateDIOChecksum(dio);
	COPYUINT16(dio->checksum,checksum);
	return sizeof(DIO);
}

/**
 * This function creates 6lowpan header 1
 *
 * @param array to store message
 * @param hc1 type
 * @return size of stored message
 */
char create6LowpanHeader1(char* data,uint16_t hc1){
	LowPan1* lowpan;
	lowpan = (LowPan1*) data;
	COPYUINT16(lowpan->IPHC, hc1)//(uint16_t) DIO_HC1;
	lowpan->next = (uint8_t) ICMPv6 ;//(uint8_t) ICMP6;
	return sizeof(LowPan1) ;
}

/**
 * This function creates 6lowpan header 2
 *
 * @param array to store message
 * @param source address
 * @param destination address
 * @param array to store message
 * @param next message field
 * @param hc2 type
 *
 * @return size of stored message
 */
char create6LowpanHeader2(uint16_t src,uint16_t dst,char* data,uint8_t next,uint16_t hc2){
	LowPan2* lowpan;
	lowpan = (LowPan2*) data;
	COPYUINT16(lowpan->IPHC, hc2)//(uint16_t) DIO_HC1;
	lowpan->next = next;
	COPYUINT16(lowpan->addr.ip6_dst,dst)
	COPYUINT16(lowpan->addr.ip6_src,src)
	return LOWPAN2SIZE;
}

/**
 * This function sends a message to the network
 * @param address
 * @param array with message
 * @param message size
 * @return void
 */
void send(uint16_t address,uint8_t* data, uint8_t msgsize){
	char size = create6LowpanHeader2(rpl.selfaddr,address,(char*)memArray,DATA_TYPE,HC2_HDR);
	char* dataLocation = (char*)memArray;
	dataLocation+= size;
	size += msgsize;
	uint8_t i;
	for(i = 0;i < msgsize;i++)
		dataLocation[i] = data[i];
	toMac(rpl.parent[0].macaddress,TRUE,NONE,(char*)memArray,size);
}

/**
 * This function sends a ping reply
 * @param echo request
 * @param message destination address
 * @return void
 */
void sendPingReply(ECHOREQUEST* request,char* dst){
	uint16_t address;
	COPYARRAY2INT16(address,dst)
	char size = create6LowpanHeader2(rpl.selfaddr,address,(char*)memArray,ICMPv6,HC2_HDR);
	size += createPingReply(request,(char*)memArray + size);
	toMac(rpl.parent[0].macaddress,TRUE,NONE,(char*)memArray,size);

#ifndef SIMULATION

#ifdef DEBUG
	P1DIR |= 0x01;
	P1OUT ^= 0x01;
#endif

#endif

}

/**
 * This function sends a DIO message
 * @return void
 */
void sendDIO(){
	if(rpl.initialized == TRUE){
		char size = create6LowpanHeader1((char*)memArray,HC1_HDR);
		size += createDIO(&rpl,(char*)memArray+size);
		toMac(BROADCASTMAC,TRUE,DISBLEACK,(char*)memArray,size);
	}
}

/**
 * This function sends a DIS message
 * @return void
 */
void sendDIS(){
	if(rpl.initialized == TRUE){
		char size = create6LowpanHeader1((char*)memArray,HC1_HDR);
		size += createDIS(&rpl,(char*)memArray+size);
		toMac(BROADCASTMAC,TRUE,DISBLEACK,(char*)memArray,size);
	}
}

/**
 * This function sends a DAO message
 * @return void
 */
void sendDAO(){
	if(rpl.initialized == TRUE){
		//TODO DAO timer
		if(rpl.rank == INFRANK) return; // decide if will or not send DAO
		uint16_t addr = rpl.parent[0].macaddress; // send DAO to parent
		char size = create6LowpanHeader1((char*)memArray,HC1_HDR);
		size += createDAO(&rpl,(char*)memArray+size);
		size += createDAOTarget(&rpl,(char*)memArray+size);
		size += createDAOTransinfo(addr,(char*)memArray+size);
		//calculate crc
		uint16_t crc16 = crc16MakeBitwise2(CRC16_INIT_REM, CRC16_POLY, (unsigned char*)memArray, size);
		char *data = (char*)(memArray+size);
		COPYUINT16(data,crc16);
		size +=2;
		toMac(addr,TRUE,NONE,(char*)memArray,size);
	}
}

#define SAMEPARENTS 0
#define NEWPARENTS 1

/**
 * This function selects a new parent
 * @param diorank
 * @param a new calculated rank
 * @param macaddress of dio sender
 * @return void
 */
void selectRplParents(uint16_t diorank,uint16_t possiblenewrank,uint16_t macaddress){
    char i,j;
    //the new calculated rank is better - take it and remove already selected parents
	if((possiblenewrank>>3) < (rpl.rank>>3)){

		rpl.parent[0].rank = diorank;
		rpl.parent[0].macaddress = macaddress;
		rpl.rank = possiblenewrank;
		for(i = 1; i < NUMPARENTS; i++){
				rpl.parent[i].rank = INFRANK;
				rpl.parent[i].macaddress = PARENTNULL;

		}
		return ;
	}

     //the new calculated rank is equal - take it and insert it on parents list
	if((possiblenewrank>>3) == (rpl.rank>>3)){

			for(i = 0; i < NUMPARENTS; i++){
					if(rpl.parent[i].macaddress == macaddress)
					{
						rpl.parent[i].rank = diorank;
						return ;
					}

				    if(rpl.parent[i].rank == 0xffff){
				    	rpl.parent[i].rank = diorank;
				        rpl.parent[i].macaddress = macaddress;
				        return ;
				    }
			}
			return ;
	}
	 //the new calculated rank is worst - take the better rank from the list if it exists
	if((possiblenewrank>>3) > (rpl.rank>>3)){

		if(rpl.parent[1].rank == INFRANK){
			if(rpl.parent[0].macaddress == macaddress){ // just one parent in the list
				if(possiblenewrank < (rpl.minrank + MAXHOPINCREASE)){// avoids count to infinity
					rpl.parent[0].rank = diorank;
					rpl.rank = possiblenewrank;
				}else{
					rpl.parent[0].rank = INFRANK;
					rpl.rank = INFRANK;
					rpl.minrank = INFRANK;
				}
			}
		}else{
			for(i = 0; i < NUMPARENTS; i++){
					if(macaddress == rpl.parent[i].macaddress){ // remove the parent from list
						for(j = i + 1; j < NUMPARENTS; j++){
							 rpl.parent[j-1].macaddress = rpl.parent[j].macaddress;
							 rpl.parent[j-1].rank = rpl.parent[j].rank;
						}
						rpl.parent[NUMPARENTS-1].rank = INFRANK;
						rpl.parent[NUMPARENTS-1].macaddress = PARENTNULL;
						break;
					}
				}
		}
	}

}

/**
 * This function refresh the preferential parent rank
 * @return void
 */
void refreshParentRank(){
	if(rpl.parent[0].macaddress != 0xffff){
		uint8_t index = etxindex(rpl.parent[0].macaddress);
		uint8_t etx = getETX(index);
		uint16_t rank = rpl.parent[0].rank;

		if(etx == 255) {rpl.rank = 0xffff; return;}

		if((((rank*etx)>>3)+ MINHOPINCREASE) > INFRANK ) return;
		rpl.rank = ((rank*etx)>>3)+ MINHOPINCREASE;
	}
}

/**
 * This function calculates the rank of one dio message
 * @param DIO instance
 * @param macaddress of dio sender
 * @return a rank value
 */
uint16_t calculateRank(DIO* dio,uint16_t macaddress){

	uint8_t index = etxindex(macaddress);
	uint8_t i;
	uint16_t rank;
	COPYARRAY2INT16(rank,dio->rank);

	if(index == 255) return INFRANK; // TODO:
    if(rank == INFRANK) return INFRANK;

	if(ETXARRAY[index] == 0){
		for(i = 0 ; i < 14 ; i++){ // 14 tries
			measureETX(macaddress,index);
		}
	}

	i = getETX(index);
	if(i == 255) return INFRANK;

	if(((rank*i)+ MINHOPINCREASE) > INFRANK ) return INFRANK;

	return ((rank*i )>>3)+ MINHOPINCREASE; //TODO:;
}

/**
 * This function creates a rpl instance
 * @param DIO instance
 * @param macaddress of dio sender
 * @return void
 */
void createRPL(DIO* dio,uint16_t macaddress)
{
	COPYARRAY2INT16(rpl.dodagid, dio->dodagid);
	rpl.rplinstanceid = dio->rplinstanceid;
	rpl.versionNumber = dio->versionNumber;
	rpl.GO_MOP_Prf = dio->GO_MOP_Prf;
	rpl.DTSN = dio->DTSN;
	rpl.rank = INFRANK;
	int i;
	for(i = 0 ; i < NUMEXT ; i++)
		ETXARRAY[i] = 0;

	uint16_t diorank;
	COPYARRAY2INT16(diorank,dio->rank);
	uint16_t newrank = calculateRank(dio,macaddress);
	selectRplParents(diorank,newrank,macaddress);
	rpl.rank = calculateRank(dio,macaddress);
	rpl.minrank = rpl.rank;
	resetTimer();
	rpl.initialized = TRUE;
}


/**
 * This function removes a macaddres from the parents list
 * @param macaddress
 * @return true if the first parent is removed
 */
BOOL removeFromParents(uint16_t macaddress){
	char i,j;
	for(i = 0; i < NUMPARENTS; i++){
			if(rpl.parent[i].macaddress == macaddress){
				if(i == (NUMPARENTS-1)) {
					rpl.parent[i].rank = INFRANK;
					rpl.parent[i].macaddress = PARENTNULL;
					return FALSE;
				}
				for(j = i; j < NUMPARENTS-1; j++){
					rpl.parent[j].rank = rpl.parent[j+1].rank ;
					rpl.parent[j].macaddress = rpl.parent[j+1].macaddress;
				}
				rpl.parent[NUMPARENTS-1].rank = INFRANK;
				rpl.parent[NUMPARENTS-1].macaddress = PARENTNULL;

				if(i == 0){
					return TRUE;
				}else{
					return FALSE;
				}
			}
	}
	return FALSE;
}

/**
 * This function receives a DIO message and refresh the global rpl instance
 * @param dio instance
 * @param macaddress
 * @return
 */
void recieveDIO(DIO* dio,uint16_t macaddress){

#ifdef DEBUGMULTIHOP
	if((dio->rank[0] == 8) || (dio->rank[0] == 8) ) return;
#endif

		volatile uint16_t aux;
		aux  = calculateDIOChecksum(dio); // aux = checksum
		if(!COMPARE(aux,dio->checksum)) return;

		if(rpl.initialized == FALSE){
			createRPL(dio,macaddress);
		}else{

			if(!(COMPARE(rpl.dodagid,dio->dodagid)))
				return;

			if(dio->versionNumber > rpl.versionNumber)// cuidados com o version
			{
				rpl.rank = INFRANK;
				createRPL(dio,macaddress);
				resetDAO();
				resetTimer();
				return;
			}

			if(dio->rplinstanceid != rpl.rplinstanceid)//
				return;

			refreshParentRank();
			volatile uint16_t possiblenwerank = calculateRank(dio,macaddress);
			if(rpl.minrank > possiblenwerank)
				rpl.minrank = possiblenwerank;

			// inf rank
			if(possiblenwerank == INFRANK){
					BOOL prfparentRemoved = removeFromParents(macaddress);
					if(prfparentRemoved == TRUE){
						  if(rpl.rank == INFRANK){
							  sendDIOInfRank = NUMINFDIO;
							  sendDIS_ = DISTRIES;
						  }
						  else{
							  resetDAO();
						  }
						  resetTimer();
					}
					return;
				}

			if((possiblenwerank>>3) == (rpl.rank>>3)){ // this may be in the parents set

			  if(dio->DTSN > rpl.DTSN){
				  rpl.DTSN = dio->DTSN;
				  resetDAO();
				  resetTimer();
			  }
			}
			uint16_t oldAddress = rpl.parent[0].macaddress;
			uint16_t diorank;
			COPYARRAY2INT16(diorank,dio->rank);
			selectRplParents(diorank,possiblenwerank,macaddress);

			if((rpl.rank>>3) != (possiblenwerank>>3))
			{
				 resetTimer();
			}

			if(oldAddress != rpl.parent[0].macaddress){
				 resetDAO();
			}
		}
}

/**
 * This function receives a DAO message and retransmit it
 * @param DAO instance
 * @param macaddress
 * @return
 */
char recieveDAO(DAO* dao,uint16_t *addr){
	//
	uint16_t checksum = calculateDAOChecksum(dao);
	if(!COMPARE(checksum,dao->checksum)) return DONOTHING;

	if(dao->rplinstanceid != rpl.rplinstanceid) return DONOTHING; // decide if will or not send DAO
	if(rpl.initialized == FALSE)  return DONOTHING; // decide if will or not send DAO
	if(rpl.rank == INFRANK) return DONOTHING; // decide if will or not send DAO
	*addr = rpl.parent[0].macaddress; // send DAO to parent
	return RETRANSMIT;
}

/**
 * This function receives a DAOACK and stops sending DAO`s
 * @param DAOACK instance
 * @return
 */
void recieveDAOACK(DAO_ACK* daoack){
	uint16_t checksum = calculateDAOACKChecksum(daoack);
	if(!COMPARE(checksum,daoack->checksum)) return;

	if(daoack->rplinstanceid != rpl.rplinstanceid) return;
	if(!(COMPARE(rpl.dodagid,daoack->dodagid))) return;
	if(daoack->daosequence != DAOSEQ) return ;

	stopDAO();
}


/********************** FRAGMEENT*/
void removeAddress(char* id){
	char i,j;
	for(i = 0; i < NUMMSGADDR; i++){
		if((id[0] == rpl.msgaddress[i].id[0]) && (id[1] == rpl.msgaddress[i].id[1])){

			for(j = i; j < NUMMSGADDR-1; j++){
				rpl.msgaddress[j].id[0] = rpl.msgaddress[j+1].id[0];
				rpl.msgaddress[j].id[1] = rpl.msgaddress[j+1].id[1];
			}

			rpl.msgaddress[NUMMSGADDR-1].id[0] = NULLADDR;
			rpl.msgaddress[NUMMSGADDR-1].id[1] = NULLADDR;

			return;
		}
	}
}

BOOL takeAddress(char* id,uint16_t* address){
	char i;
	for(i = 0; i < NUMMSGADDR; i++){
		if((id[0] == rpl.msgaddress[i].id[0]) && (id[1] == rpl.msgaddress[i].id[1])){
			*address =  rpl.msgaddress[i].address;
			return TRUE;
		}
	}
	return FALSE;
}
/******************************************************************/


/**
 * This function receives a message(application)
 * @param message
 * @return
 */
void recievedData(const char *data){
	static uint8_t counter = 0;
	static uint8_t outData[2];
	uint8_t index;
	uint16_t ret;

	switch(data[0]){
		case 'e': // etx
	    index = etxindex(rpl.parent[0].macaddress);
	    if(index != 255){
	    	ret = getETX(index);
	    }else{
	    	ret = 0xffff;
	    }
	    break;

		case 'f': // etx
	    index = etxindex(rpl.parent[0].macaddress);
	    if(index != 255){
	    	ret =  ETXARRAY[index];
	    }else{
	    	ret = 0xffff;
	    }

		break;
		case 's' : // self rank
		ret = rpl.rank;
		break;
		case 'p' : // parent rank
		ret = rpl.parent[0].rank;
		break;
		case 'c' :
		counter++;
		ret = counter;
		break;
	}
	outData[0] = ret&0xff;
	outData[1] = ((0xff00&ret)>>8)&0xff;
	send(0,outData,2);
}

/**
 * This function receives LowPan2 message and process it
 * @return
 */
void reciveMessage(char *data,LowPan2* lowpan){

	char *dataux = data;
	char next = lowpan->next;
	ROUTING* rout;
	FRAG* frag;
	ECHOREQUEST *request;
	icmp6_hdr* hdr;
	uint16_t checksum;
	uint16_t aux;
	while((next != 0) && (next != 255)){ // TODO: 255 out of memory space
		switch (next) {
			case ICMPv6:{
				hdr = (icmp6_hdr*) dataux;

			    // ping request
				if(hdr->icmp6_type == PINGREQUEST_TYPE){


							request = (ECHOREQUEST*) dataux;

							COPYARRAY2INT16(checksum,request->checksum);
							COPYARRAY2INT16(aux,request->sequence);
							checksum -= aux;
							COPYARRAY2INT16(aux,request->identifier);
							checksum -= aux;
							checksum-=PINGREQUEST_TYPE;
							if(checksum != 0) return;

							sendPingReply(request,lowpan->addr.ip6_src);
							next = 0;
				}
				//dao ack
				if((hdr->icmp6_code == DOGAG_DAO_ACK_CODE) && (hdr->icmp6_type == RPL_TYPE)){
					recieveDAOACK((DAO_ACK*)dataux);
				}

				next = 0;
			}
				break;
			case ROUT_TYPE:
				rout = (ROUTING*) dataux;
				next = rout->next;
				dataux = dataux + ROUTINGSIZE + (rout->hdrextlen)*2;
				break;
			case FRAG_TYPE:
				frag = (FRAG*) dataux;
				next = frag->next;
				dataux = dataux + sizeof(FRAG);
				break;
			case DATA_TYPE:
				recievedData(dataux);
				next = 0;
				break;
			default:
				next = 0;
				break;
		}
	}
}

/**
 * This function receives routing message and take and refresh the next address
 * @return Restransmit message
 */
char recieveRouting(char *data,uint16_t* address){
	 ROUTING *rout = (ROUTING*) data;

	 if(rout->segmentsLeft == 0) return DONOTHING;

	 char* dataaux = data;
	 dataaux+=ROUTINGSIZE;
	 char position = ((rout->hdrextlen - rout->segmentsLeft) << 1);
	 dataaux+=position;
	 COPYARRAY2INT16(*address,dataaux)
	 COPYUINT16(dataaux,rpl.selfaddr)
	 rout->segmentsLeft--;

	 return RETRANSMIT;

}

char recieveFrag(char *data,uint16_t* address){
	 /*FRAG *frag = (FRAG*) data;
	 char* dataaux = data + sizeof(FRAG);

	 if(takeAddress(frag->id,address) == TRUE){
		 uint16_t fragoffset;
		 COPYARRAY2INT16(fragoffset,frag->fragoffset);
		 if(fragoffset&LASTFRAGMENT){
			 removeAddress(frag->id);
		 }
		 ROUTING *rout = (ROUTING*) data;
		 rout->addrIdx++;
		 return RETRANSMIT;
	 }


	 if(frag->next == ROUT_TYPE){
		 //return recieveRouting(frag->id,dataaux,address);
	 }*/
	return DONOTHING;
}

/**
 * This function receives lowpan2 messages
 * @param address
 * @param message
 * @return the next behavior
 */
char recieveHC2HDR(uint16_t* address, char *data){
	LowPan2* lowpan = (LowPan2*) data;
	char* dataaux = data + LOWPAN2SIZE;
	icmp6_hdr* hdr;
	//this message is for me
	if(COMPARE(rpl.selfaddr,lowpan->addr.ip6_dst)){
		reciveMessage(dataaux,lowpan);
		return DONOTHING;
	}

	switch(lowpan->next){
	case FRAG_TYPE:
		return recieveFrag(dataaux,address);

	case ROUT_TYPE:
		return recieveRouting(dataaux,address);

	case DATA_TYPE: //em caso de dados enviar para sink
		*address = rpl.parent[0].macaddress;
		return TRANSMITTOPARENT;

	case ICMPv6:
		hdr = (icmp6_hdr*) dataaux;
		if(hdr->icmp6_type == PINGREQUEST_TYPE){
	        //em caso de ping request enviar para sink
			*address = rpl.parent[0].macaddress;
			return TRANSMITTOPARENT;
		}

		if(hdr->icmp6_type == PINGREPLY_TYPE){
	        //em caso de ping request enviar para sink
			*address = rpl.parent[0].macaddress;
			return TRANSMITTOPARENT;
		}

	default:
		break; // this message does not is treated

	}
	return DONOTHING;
}


/**
 * This function initializes the rpl instance
 *
 * @param message
 *  @param Queue
 *  @param timerTosend_
 * @return
 */
void rplInit(char* array,Queue* mac2netw_,uint8_t** timerTosend_){
	RplTimerConfig();
	rpl.initialized = FALSE; // FALSE
	sendDIO_ = FALSE;
	memArray = array;
	mac2netwPointer  = mac2netw_;
	rpl.rank = INFRANK;
	rpl.minrank = INFRANK;
	//*timerTosend_ = &timerToSend;
	rpl.selfaddr = getZigbeeAddress();

	char i;
	for(i = 1; i < NUMMSGADDR; i++){
		rpl.msgaddress[i].id[0] = 0xff;
		rpl.msgaddress[i].id[1] = 0xff;
	}
}

/**
 * This function send DIO`s and DAO`s messages (this depends the timer values)
 *
 */
void executeRpl(){
	//recieved data
	//dio to send
	static char diodao = 1;
	if(sendDIS_ > 0){

		if(DISTimer == 0){
			sendDIS_--;
			sendDIS();
			DISTimer = DISTIME;
		}
		return;
	}

	if(diodao == 1)
	{
		if(diocounter > 0){
			sendDIO(); // never use this inside interruption (may cause errors)
			sendDIO_ = FALSE;
			diocounter--;
		}
	}else
	{
		if(sendDAO_ == TRUE){

			sendDAO(); // never use this inside interruption (may cause errors)
			sendDAO_ = FALSE;
		}
	}
	diodao ^= 1;
}
