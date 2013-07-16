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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <zigbee.h>
#include "rpl.h"
#include "QByteArray.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void connectZigbee();
    void recieveData(QByteArray data);
    void sentData(QByteArray data);
    void reset();
    void sendData();
    void sendPing();
    void recievePing(int,int);
    void recieveRPLData(QByteArray data,uint16_t addr);
    void refresh();
    void routingTable();
private:
    bool connected_;
    Ui::MainWindow *ui;
    Zigbee zigbee_ ;
    rpl* rpl_;

};

#endif // MAINWINDOW_H
