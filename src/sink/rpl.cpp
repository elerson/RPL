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
#include <iostream>
#include <map>


#define CRC16_POLY			0x8005
#define CRC16_FINAL_XOR     0x0
#define CRC16_INIT_REM      0x0
#define CRC16_CHECK         0xFEE8
#define COPYCHAR2(val,var) { val = var[1]; val = val << 8 ; val |= var[0];}
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

rpl::rpl(Zigbee* zigbee)
{
    diotimer_.resetTimer();
    rpldata_.initialized  = false;
    connect(&timer_,SIGNAL(timeout()),this,SLOT(timerDIO()));
    timer_.start(30); // 30ms
    zigbee_ = zigbee;
    connect(zigbee_,SIGNAL(recievedAPIData(QByteArray,uint16_t)),this,SLOT(recieveData(QByteArray,uint16_t)));
    fragid_ = 0;
    this->selfaddress = 0;
    pingsequence = 0;
}

void rpl::sendData(QByteArray data, uint16_t addr,uint8_t type)
{
    std::vector<uint16_t> path;
    std::map<uint16_t,uint16_t>::iterator it;
    uint16_t dst = addr;
    bool sent = false;

    it = routingTable.find(dst);
    while(it != routingTable.end()){

         if(it->second == it->first){
            routingTable.erase(it);
            return;
         }

         if(it->second == selfaddress){
             sendData(selfaddress,addr,it->first,path,data,type);
             sent = true;
         }
         path.push_back(it->first);
         dst = it->second;
         it = routingTable.find(dst);
    }

    if(sent == false){
        rpldata_.DTSN++;
        diotimer_.resetTimer();
    }

}

void rpl::sendPing(uint16_t addr)
{
    QByteArray data;
    ECHOREQUEST request;
    request.code = 0;
    request.type = PINGREQUEST_TYPE;
    request.sequence = pingsequence;
    request.checksum = pingsequence + PINGREQUEST_TYPE;
    request.identifier = 0;
    data.append((char*)&request,ECHOREQUESTSIZE);
    pingtime_[pingsequence] = new QTime();
    pingtime_[pingsequence]->start();
    sendData(data,addr,ICMPv6);
    pingsequence++;
}

void rpl::timerDIO()
{
    if(rpldata_.initialized == true)
    {
        diotimer_.counter++;
    }
    if(diotimer_.counter == diotimer_.t){
        sendDIO();
        std::cout << "saida" << std::endl;
    }else if(diotimer_.counter == diotimer_.I){
        diotimer_.incrementTimer();
    }
}

void rpl::recieveData(QByteArray byteArray, uint16_t address)
{
    char* data = (char*) byteArray.data();
    LowPan1* lowpan = (LowPan1*) data;

    if(lowpan->IPHC == HC1_HDR){
        recivedHC1packet(byteArray,address);
    }else if(lowpan->IPHC  == HC2_HDR){
        recivedHC2packet(byteArray,address);
    }
}
void rpl::create6LowpanHeader1(QByteArray& data){

    LowPan1 lowpan;
    lowpan.IPHC = HC1_HDR;//(uint16_t) DIO_HC1;
    lowpan.next = (uint8_t) ICMPv6 ;//(uint8_t) ICMP6;
    data.append((char*)&lowpan,LowPan1size);

}

void rpl::create6LowpanHeader2(QByteArray &data)
{

}

void rpl::recivedHC1packet(QByteArray byteArray, uint16_t address)
{
    char* data = (char*) byteArray.data();
    LowPan1* lowpan = (LowPan1*) data;
    icmp6_hdr* hdr ;
    if((lowpan->next == ICMPv6)){
        data += LowPan1size;
        hdr = (icmp6_hdr*) data;
        if((hdr->icmp6_code == DOGAG_DAO_CODE)&&(hdr->icmp6_type == RPL_TYPE)){
            uint16_t crc16 = crc16MakeBitwise2(CRC16_INIT_REM, CRC16_POLY,(unsigned char*) byteArray.data(), byteArray.size() -3);
            uint16_t crcmsg;
            char* crcaux = byteArray.data();
            crcaux += byteArray.size() -2; // the last bit is checksum

            crcmsg = *crcaux;
            crcmsg = (crcmsg << 8)&0xFF00;
            crcaux-=1;
            crcmsg |= *crcaux&0x00FF;

            //COPYCHAR2(crcmsg,crcaux);
            if(crcmsg != crc16) {
                std::cout << "DAO Descartado" << crcmsg << " " <<crc16 << std::endl  ;
                return;
            }
            byteArray.remove(0,LowPan1size);
            recivedDAO(byteArray,address);
        }
    }

    std::cout << "recieved HC1" << std::endl;
}

void rpl::recivedHC2packet(QByteArray byteArray, uint16_t address)
{
    LowPan2* lowpan = (LowPan2* ) byteArray.data();

    uint16_t src = lowpan->addr.ip6_src;
    uint16_t dst = lowpan->addr.ip6_dst;
    uint8_t type = lowpan->next;
    byteArray.remove(0,LowPan2size);

    recievedDATA(src,dst,byteArray,type);

    std::cout << "recieved HC2" << std::endl;


}

void rpl::recivedDAO(QByteArray bytearray, uint16_t address)
{
     std::cout << "recieved DAO" << std::endl;
     char* data = (char*) bytearray.data();
     DAO* dao = (DAO*) data;
     uint16_t checksum = dao->checksum;
     dao->calculateDAOChecksum();
     if(checksum != dao->checksum){
         std::cout << "errado" <<checksum <<" "<< dao->checksum << std::endl;
         return;}
     if(dao->code != DOGAG_DAO_CODE) return;
     if(dao->rplinstanceid != rpldata_.rplinstanceid) return;


     //TODO: send dao ack

     data+= sizeof(DAO);
     RPLTARGET *rpltarget = (RPLTARGET*) data;
      data+= sizeof(RPLTARGET);
     TRANSINFO *transinfo = (TRANSINFO*) data;

     std::cout << "DAO address" << rpltarget->prefix;
     std::map<uint16_t,uint8_t>::iterator it;


     it = rplinfo.find(rpltarget->prefix);
     if(it == rplinfo.end()){
        rplinfo[rpltarget->prefix] = dao->dao_sequence;
        routingTable[rpltarget->prefix] = transinfo->parentaddress;
     }else{
         if((it->second < dao->dao_sequence) || ((it->second > 240) && (dao->dao_sequence < 5))){
             rplinfo[rpltarget->prefix] = dao->dao_sequence;
             routingTable[rpltarget->prefix] = transinfo->parentaddress;
         }
     }
     emit newroutingTable();

     DAO_ACK daoack;
     QByteArray daodata;
     daoack.type = RPL_TYPE;
     daoack.code = DOGAG_DAO_ACK_CODE;
     daoack.daosequence = dao->dao_sequence;
     daoack.dodagid = dao->dodagid;
     daoack.rplinstanceid = dao->rplinstanceid;
     daoack.calculateDAOACKChecksum();
     daodata.append((char*) &daoack,DAO_ACKSIZE);
     sendData(daodata,rpltarget->prefix,ICMPv6);

}

void rpl::recivedDIS(QByteArray byteArray)
{
    char* data = (char*) byteArray.data();
    DIS* dis = (DIS*) data;
    if(dis->code == 0 && dis->flags == RPL_TYPE){
        diotimer_.resetTimer();
    }
}

void rpl::recievedDATA(uint16_t src, uint16_t dst, QByteArray data,uint8_t type)
{
     std::cout << "recieved DATA" << type << std::endl;

    if(dst == selfaddress){

        icmp6_hdr* hdr = (icmp6_hdr*) data.data();
        if((type == ICMPv6) && (hdr->icmp6_type == PINGREPLY_TYPE)){

            ECHOREPLY* echo = (ECHOREPLY*) data.data();

            std::map<uint16_t,QTime*>::iterator it;
            it = pingtime_.find(echo->sequence);

            if(it != pingtime_.end()){
                int time = it->second->elapsed();
                emit recievedPingTime(time,echo->sequence);
                pingtime_.erase(echo->sequence);
            }

            return;
        }

        emit recievedRPLData(data,src);
    }else{ // retrasmit data

        std::vector<uint16_t> path;
        std::map<uint16_t,uint16_t>::iterator it;

        it = routingTable.find(dst);
        while(it != routingTable.end()){

             if(it->second == selfaddress){
                 sendData(src,dst,it->first,path,data,type);
             }
             path.push_back(it->first);
             dst = it->second;
             it = routingTable.find(dst);
        }

     }
}


void rpl::sendData(uint16_t src, uint16_t dst, uint16_t nexthop,std::vector<uint16_t> &path, QByteArray data,uint8_t type)
{
    QByteArray outData;
    //QByteArray outData2;
    LowPan2 lowpan;

    lowpan.IPHC = HC2_HDR;
    lowpan.next = type;
    lowpan.addr.ip6_dst = dst;
    lowpan.addr.ip6_src = src;
    outData.append((char*) &lowpan,LowPan2size);

    if(path.size() == 0){
        outData.append(data);
        zigbee_->sendData(nexthop,NONE,outData);
        return;
    }
    outData.clear();
    lowpan.next = ROUT_TYPE; //next is routing type
    outData.append((char*) &lowpan,LowPan2size);

    //create the routing header
    ROUTING routing;
    routing.cmprICmprE = 0; //TODO
    routing.hdrextlen = path.size();
    routing.next = type;
    routing.routingtype = 3; //routing
    routing.segmentsLeft = path.size();
    outData.append((char*) &routing,ROUTINGSIZE);

    for(int i = 0 ; i < outData.size(); i++)
        std::cout <<(int)(outData.at(i)&0xff) << " ";
    std::cout << std::endl;

    //add the addresses
    for(int i = 0 ; i < routing.hdrextlen;i++){
       uint16_t addr = path.back();
       path.pop_back();
       outData.append((char*)&addr,sizeof(uint16_t));
    }

    for(int i = 0 ; i < outData.size(); i++)
        std::cout <<(int)(outData.at(i)&0xff) << " ";
    std::cout << std::endl;

    outData.append(data);
    zigbee_->sendData(nexthop,NONE,outData);

    for(int i = 0 ; i < outData.size(); i++)
        std::cout <<(int)(outData.at(i)&0xff) << " ";
    std::cout << std::endl;  
}


void rpl::sendDAOACK(DAO* dao,uint16_t dst, uint16_t nexthop,std::vector<uint16_t> &path)
{

}

void rpl::createDIO(QByteArray &byteArray){
    DIO dio;
    dio.type = RPL_TYPE; // ICMP type
    dio.code = DOGAG_DIO_CODE ;
    dio.dodagid = rpldata_.dodagid;
    dio.DTSN =  rpldata_.DTSN;
    dio.flags = 5;
    dio.GO_MOP_Prf = rpldata_.GO_MOP_Prf;
    dio.rank = 8;
    dio.reserved = 0;
    dio.rplinstanceid = rpldata_.rplinstanceid;
    dio.versionNumber = rpldata_.versionNumber;
    //dio.checksum = 0x0508;
    dio.calculateDIOChecksum();
    byteArray.append((char*)&dio,sizeof(dio));
}

void rpl::sendDIO()
{
    QByteArray byteArray;
    create6LowpanHeader1(byteArray);
    createDIO(byteArray);
    std::cout << byteArray.size() << std::endl;
    //QByteArray.append('a');
    zigbee_->sendData(BROADCASTMAC,DISBLEACK,byteArray);
}
