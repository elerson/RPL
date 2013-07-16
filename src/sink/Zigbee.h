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

#ifndef Zigbee_H
#define Zigbee_H
#include <QObject>
#include <QtAddOnSerialPort/serialport.h>
#include "QByteArray.h"
#include <QString>
#include <inttypes.h>
#include <iostream>
#include <queue>
#include <QTimer>
#include "SinkPlatform.h"

/*needed by serial port class*/
QT_BEGIN_NAMESPACE_SERIALPORT
class SerialPort;
QT_END_NAMESPACE_SERIALPORT

QT_USE_NAMESPACE_SERIALPORT

#define BROADCASTMAC 0xffff


typedef enum OPTION_ {NONE = 0x00,DISBLEACK = 0x01,BROADCASTPANID = 0x04} OPTION;
#define APIIDINDEX 3

struct zigbeeOutdata{
    uint16_t address;
    OPTION option;
    QByteArray array;
    zigbeeOutdata(uint16_t addr,OPTION opt,QByteArray& ar ){
        address = addr;
        option = opt;
        array.append(ar);
    }
};


class Zigbee : public QObject
{
    Q_OBJECT
public:
    Zigbee();
    /* This function connects to a zigbee
    *
    */
    bool connect(QString port);
    /* This function sends data to the plugged zigbee
    * @return void
    */
    void sendData(uint16_t address,OPTION opt,QByteArray array);
private slots:
    void recieveData();
    void send();
    void sendData_(uint16_t address,OPTION option,QByteArray array);

signals:
    /* This functions receive a messages from mac layer
    */
    void recievedData(QByteArray array);
    void recievedAPIData(QByteArray array,uint16_t address);
    /* This function shows the sent message
       */
    void sent(QByteArray array);
private:
    SerialPort serial_port_;
    bool is_connected_;
    char frameID;
    std::queue<zigbeeOutdata*> outdataqueue;
    QTimer timer;
    uint32_t timetosend;
    QByteArray databuffer_;
    uint16_t databuffersize_;
    uint16_t dataindex_;
};

#endif // Zigbee_H
