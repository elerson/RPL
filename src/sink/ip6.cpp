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

#include "ip6.h"
#include <iostream>
 void DIO::calculateDIOChecksum(){
	 char i;
	 uint16_t checksum = 0;
     char* data = (char*) this;
	 for(i = 4 ; i < sizeof(DIO);i++){
         checksum +=  (uint8_t) data[i];
	 }
     this->checksum = checksum;
     std::cout <<" cs "<< checksum << std::endl;

 }

 void DAO::calculateDAOChecksum()
 {
     char i;
     uint16_t checksum = 0;
     char* data = (char*) this;
     for(i = 4 ; i < sizeof(DAO);i++){
         checksum += (uint8_t) data[i];
     }
     this->checksum = checksum;
     std::cout <<" cs "<< checksum << std::endl;
 }

 void DAO_ACK::calculateDAOACKChecksum()
 {
     char i;
     uint16_t checksum = 0;
     char* data = (char*) this;
     for(i = 4 ; i < sizeof(DAO_ACK);i++){
         checksum +=  (uint8_t) data[i];
     }
     this->checksum = checksum;
     std::cout <<" cs "<< checksum << std::endl;
 }
