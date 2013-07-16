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

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "platform.h"
#include "queue.h"
#include "zigbee.h"
#include "rpl.h"
#include "utils.h"

#define APIIDENT_INDEX 3
#define RECIEVED16BITS 0x81
#define AUXARRAYSIZE 130
#define TIMETOWAIT 20

/**
 * This function sends a message to MAC layer(zigbee)
 */
void toMac(uint16_t macaddress,BOOL up,OPTION opt,char* data,char size);
/**
 * This function executes the rpl protocol
 */
void execNetworkProtocol();
/**
 * This function initializes the all the protocol
 */
void NetworkProtocolConfigure();


#endif /* PROTOCOL_H_ */
