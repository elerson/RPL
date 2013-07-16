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

#ifndef RPL_H
#define RPL_H
#include <inttypes.h>
#include <ctime>
#include <QTimer>
#include <QTime>
#include <QObject>
#include <QByteArray>
#include "ip6.h"
#include "zigbee.h"
#include <map>
#include <vector>
#include "SinkPlatform.h"
#define IMAX 65200
#define IMIN 4
//HEADER
#define HC1_HDR 31027//01111001 00110011 without address
#define HC2_HDR 26658//01101000 00100010 16bits address

#define NUMPARENTS 4
#define NUMMSGADDR 4
#define PARENTNULL 0xffff
#define INFRANK 0xffff
#define NUMINFDIO 8
#define DONOTHING 0
#define RETRANSMIT 1
#define TRANSMITTOPARENT 2
#define NULLADDR 0xff
#define MAXDATASIZE 100

struct rpldata{

        uint16_t dodagid;
        uint8_t rplinstanceid;
        uint8_t versionNumber;
        uint8_t GO_MOP_Prf;
        uint8_t DTSN;
        uint16_t rank;
        bool initialized;

};

/* This Class implements the DIO timer
*/

struct Diotimer
{
    uint16_t t;
    uint16_t I;
    uint16_t counter;
    /* This function resets the dio timer
    */
    void resetTimer(){
        this->I = IMIN;
        srand();
        this->t = IMIN/2 + rand()%IMIN/2;
        this->counter = 0;
    }
    /* This function incremets the timer
    */
    void incrementTimer(){
        if((this->I << 1) < IMAX){
            this->I = this->I << 1;
        }
        srand();
        this->t =this->I/2 + rand()%(this->I>>1);
        this->counter = 0;
    }
private:

    uint16_t nextrand;
    int rand(void){
        nextrand = nextrand * 13 + 13; //pseudo rand
        return nextrand;
    }

    void srand(){
        nextrand = time(0);
    }
};

/* This Class implements the rpl routing protocol(sink version)
*/
class rpl : public QObject
{
    Q_OBJECT
public:
    rpl(Zigbee* zigbee);

    void setDodagid( uint16_t dodagid){rpldata_.dodagid = dodagid;}
    void setRplinstanceid(uint8_t rplinstanceid){rpldata_.rplinstanceid = rplinstanceid;}
    void setVersionNumber(uint8_t versionNumber){rpldata_.versionNumber = versionNumber;}
    void setGOMOPPrf(uint8_t GO_MOP_Prf){rpldata_.GO_MOP_Prf = GO_MOP_Prf;}
    void setDTSN(uint8_t DTSN){rpldata_.DTSN = DTSN;}
    void start(){rpldata_.initialized = true;}
    int DTSN(){return rpldata_.DTSN;}
    /* This sends data to a especific address
    * @param data
    * @param address
    * @param message type
    */
    void sendData(QByteArray data, uint16_t addr,uint8_t type);
    /* This function sends ping messages to especific address
    */
    void sendPing(uint16_t addr);
     std::map<uint16_t,uint16_t>& getRoutingTable(){ return routingTable; }

public slots:
    void reset(){diotimer_.resetTimer();}
    void incrementDTSN(){rpldata_.DTSN++;}

private slots:
    void timerDIO();
    void recieveData(QByteArray byteArray,uint16_t address);

signals:
    void recievedRPLData(QByteArray,uint16_t src);
    void recievedPingTime(int,int);
    void newroutingTable();

private:
    /* This function creates DIO messages
    * @param DIO message pointer(return)
    */
    void createDIO(QByteArray &byteArray);
    void sendDIO();
    /* This functions create Headers for lowpan messages
    * @param message pointer(return)
    */
    void create6LowpanHeader1(QByteArray& data);
    void create6LowpanHeader2(QByteArray& data);
    /* This functions threat receiced lowpan messages
    * @param message
    * @param sender address
    */
    void recivedHC1packet(QByteArray byteArray,uint16_t address);
    void recivedHC2packet(QByteArray byteArray,uint16_t address);

    /* This functions deal with DAO DIS and DATA messages
    */
    void recivedDAO(QByteArray byteArray,uint16_t address);
    void recivedDIS(QByteArray byteArray);
    void recievedDATA(uint16_t src,uint16_t dst, QByteArray data,uint8_t type);

    /* This functions send data to the network
    */
    void sendData(uint16_t src,uint16_t dst,uint16_t nexthop,std::vector<uint16_t>&, QByteArray data,uint8_t type);
    void sendDAOACK(DAO* dao,uint16_t dst, uint16_t nexthop,std::vector<uint16_t> &path);

    rpldata rpldata_;
    Diotimer diotimer_;
    QTimer timer_;
    Zigbee* zigbee_ ;
    uint16_t selfaddress;
    uint16_t fragid_;
    uint16_t pingsequence;

    std::map<uint16_t,uint16_t> routingTable;
    std::map<uint16_t,uint8_t> rplinfo;
    std::map<uint16_t,QTime*> pingtime_;

};

#endif // RPL_H
