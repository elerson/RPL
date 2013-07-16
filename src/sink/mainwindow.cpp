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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnConnect,SIGNAL(clicked()),this,SLOT(connectZigbee()));
    connect(&zigbee_,SIGNAL(recievedData(QByteArray)),this,SLOT(recieveData(QByteArray)));
    connect(&zigbee_,SIGNAL(sent(QByteArray)),this,SLOT(sentData(QByteArray)));

    rpl_ = new rpl(&zigbee_);
    connect(ui->btnreset,SIGNAL(clicked()),this,SLOT(reset()));
    connect(ui->btnincrement,SIGNAL(clicked()),rpl_,SLOT(incrementDTSN()));
    connect(ui->btnincrement,SIGNAL(clicked()),this,SLOT(refresh()));
    connect(rpl_,SIGNAL(recievedRPLData(QByteArray,uint16_t)),this,SLOT(recieveRPLData(QByteArray,uint16_t)));
    connect(ui->btnsendData,SIGNAL(clicked()),this,SLOT(sendData()));
    connect(ui->btnPing,SIGNAL(clicked()),this,SLOT(sendPing()));
    connect(rpl_,SIGNAL(recievedPingTime(int,int)),this,SLOT(recievePing(int,int)));
    connect(rpl_,SIGNAL(newroutingTable()),this,SLOT(routingTable()));
    connected_ = false;
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::connectZigbee(){

    if(connected_) return;
    rpl_->setDodagid(ui->ledodaid->text().toInt());
    rpl_->setDTSN(ui->ledstn->text().toInt());
    rpl_->setGOMOPPrf(0);
    rpl_->setRplinstanceid(ui->leinstance->text().toInt());
    rpl_->setVersionNumber(ui->leversion->text().toInt());
    rpl_->start();
    connected_ = zigbee_.connect(ui->lePort->text());

}

void MainWindow::recieveData(QByteArray data){
    QString rvddata;
    for(int i = 0 ; i < data.size() ; i++){
       rvddata.append(QString::number((int)data.at(i)&0xff));
       rvddata.append(" ");
    }
    ui->teRcvdata->append(rvddata);

}

void MainWindow::sentData(QByteArray data)
{
    QString rvddata;
    for(int i = 0 ; i < data.size() ; i++){
       rvddata.append(QString::number((int)data.at(i)&0xff));
       rvddata.append(" ");
    }
    ui->tesent->append(rvddata);
}

void MainWindow::reset()
{
    rpl_->setDodagid(ui->ledodaid->text().toInt());
    rpl_->setDTSN(ui->ledstn->text().toInt());
    rpl_->setGOMOPPrf(0);
    rpl_->setRplinstanceid(ui->leinstance->text().toInt());
    rpl_->setVersionNumber(ui->leversion->text().toInt());
    rpl_->reset();
}

void MainWindow::sendData()
{
    QByteArray DataAux = ui->teDatatosend->toPlainText().toAscii();
    QByteArray Data;
    Data.append(DataAux.data(),DataAux.size());
    uint16_t addr = ui->leAddr->text().toInt();

    rpl_->sendData(Data,addr,DATA_TYPE);

}

void MainWindow::sendPing()
{
    QByteArray DataAux =ui->teDatatosend->toPlainText().toAscii();
    QByteArray Data;
    Data.append(DataAux.data(),DataAux.size());
    uint16_t addr = ui->leAddr->text().toInt();
    rpl_->sendPing(addr);
}

void MainWindow::recievePing(int pingtime,int seq)
{   QString rvddata("Ping ");

    rvddata += QString::number(seq) + " time: " + QString::number(pingtime);
    ui->teDatarecieved->append(rvddata);
}

void MainWindow::recieveRPLData(QByteArray data,uint16_t addr)
{
    QString rvddata;
    for(int i = 0 ; i < data.size() ; i++){
       rvddata.append(QString::number((int)data.at(i)&0xff));
       rvddata.append(" ");
    }
    ui->teDatarecieved->append(rvddata);
}

void MainWindow::refresh()
{
    ui->ledstn->setText(QString::number(rpl_->DTSN()));
    rpl_->reset();
}

void MainWindow::routingTable()
{
    ui->teRoutingtable->clear();
    std::map<uint16_t,uint16_t> table = rpl_->getRoutingTable();
    std::map<uint16_t,uint16_t>::iterator it;
    for(it = table.begin(); it != table.end() ; it++){
        ui->teRoutingtable->append(QString::number(it->first) +QString(" -> ")+ QString::number(it->second));
    }
}
