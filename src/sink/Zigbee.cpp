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

#include "Zigbee.h"
#include <iostream>
#include <queue>

Zigbee::Zigbee()
{
    is_connected_ = false;
    serial_port_.setRate(SerialPort::Rate9600);
    serial_port_.setFlowControl(SerialPort::NoFlowControl);
    serial_port_.setParity(SerialPort::NoParity);
    serial_port_.setDataBits(SerialPort::Data8);
    serial_port_.setStopBits(SerialPort::OneStop);
    frameID = 0;
    timetosend = 100; //100ms
}

bool Zigbee::connect(QString port)
{
    serial_port_.setRate(SerialPort::Rate9600);
    serial_port_.setFlowControl(SerialPort::NoFlowControl);
    serial_port_.setParity(SerialPort::NoParity);
    serial_port_.setDataBits(SerialPort::Data8);
    serial_port_.setStopBits(SerialPort::OneStop);

    serial_port_.setPort(port);
    serial_port_.open(QIODevice::ReadWrite);

    if(!serial_port_.isOpen()){
        std::cout << "unable to open port: " << std::endl;
        is_connected_ = false;
    }

    QObject::connect(&serial_port_,SIGNAL(readyRead()),this,SLOT(recieveData()));
    is_connected_ = true;
    QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(send()));
    timer.start(timetosend);
    return is_connected_;
}

void Zigbee::sendData(uint16_t address,OPTION option,QByteArray array){
    zigbeeOutdata *out = new zigbeeOutdata(address,option,array);
    outdataqueue.push(out);

}

void Zigbee::sendData_(uint16_t address,OPTION option,QByteArray array)
{
    if(!is_connected_) return;
    char size = array.size();
    QByteArray data2send;
    QByteArray datafinal;

    //create API data

    //frame mark
    datafinal.append(0x7E);
    //size

    data2send.append((char)(((size+5)&0xFF00)>>8));
    data2send.append((char)(size+5)&0xFF);
    data2send.append(0x01); // 16bits address
    frameID = frameID + 1 == 0 ? 1 : frameID + 1; // new frameid
    data2send.append(frameID); //frameID
    //destination address
    data2send.append((char)(address>>8)&0xFF);
    data2send.append((char)address&0xFF);
    //option
    data2send.append((option));
    char i;
    char checksum = 0;
    //checksum on frame data
    checksum+=0x01;
    checksum+=frameID;
    checksum+=(char)(address>>8)&0xFF;
    checksum+=(char)address&0xFF;
    checksum+=option;
    //data

    for(i = 0 ; i < size ; i++)
    {
        data2send.append(array.at(i));
        checksum += array.at(i)&0xFF;
    }
    data2send.append((0xFF - checksum)); //checksum*

    for(i = 0 ; i < data2send.size() ; i++)
    {
        if( (data2send.at(i) == 0x7E) || (data2send.at(i)  == 0x7D) ||  (data2send.at(i)  == 0x11) || (data2send.at(i)  == 0x13))
        {
            datafinal.append(0x7D);
            datafinal.append(data2send.at(i)^0x20);
        }else
        {
             datafinal.append(data2send.at(i));
        }
    }
    QByteArray dataarray;
    dataarray.append(datafinal.data(),datafinal.size());
    serial_port_.write(dataarray);
    emit sent(datafinal);

}

void Zigbee::recieveData()
{

    QByteArray data = serial_port_.readAll();

    int i = 0;

    if(data.at(0) == 126)
    {
       dataindex_ = 0;
       i = 1;
       databuffer_.clear();
       databuffer_.append(126);

     }

    for(; i < data.size();i++){
        dataindex_++;
        if(data.at(i) == 125){
            i++;
            if(i >= data.size() ) return;
            databuffer_.append((data.at(i)^0x20));
        }else{
            databuffer_.append(data.at(i));
        }

        if(dataindex_ == 1){
            if(databuffer_.size() > 1){
                databuffersize_ = databuffer_.at(1);
                databuffersize_ <<= 8;
            }
        }
        if(dataindex_ == 2){
            if(databuffer_.size() > 2){
                databuffersize_ |= databuffer_.at(2);
            }
        }

        if((databuffersize_ + 4) == databuffer_.size() ){
            emit recievedData(databuffer_);
            if(databuffer_.size() > 10)
            if((int)(databuffer_.at(APIIDINDEX)&0xff) == 129){
                std::cout << (int)(databuffer_.at(APIIDINDEX)&0xff)<< std::endl;
                uint16_t address = databuffer_.at(APIIDINDEX+1);
                address = address << 8;
                address |=databuffer_.at(APIIDINDEX+2);
                databuffer_.remove(0,8);
                emit recievedAPIData(databuffer_,address);
                databuffer_.clear();
            }
        }
    }
}

void Zigbee::send()
{
    if(outdataqueue.size() > 0){
        zigbeeOutdata *out = outdataqueue.front();
        outdataqueue.pop();
        sendData_(out->address,out->option,out->array);
    }
}

