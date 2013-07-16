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
/**
 * This function calculates the checksum for DIO messages
 * @param  DIO message
 * @return A 16bit number representing the checksum
 */
 uint16_t calculateDIOChecksum(DIO* dio){
	 char i;
	 uint16_t checksum = 0;
	 char* data = (char*) dio;
	 for(i = 4 ; i < sizeof(DIO);i++){ //iterates in the messages
		 checksum += data[i];
	 }
	 return checksum;
 }
 /**
  * This function calculates the checksum for DAO messages
  * @param  DAO message
  * @return A 16bit number representing the checksum
  */
 uint16_t calculateDAOChecksum(DAO* dio){
	 char i;
	 uint16_t checksum = 0;
	 char* data = (char*) dio;
	 for(i = 4 ; i < sizeof(DAO);i++){ //iterates in the messages
		 checksum += data[i];
	 }
	 return checksum;
 }
 /**
  * This function calculates the checksum for DAOACK messages
  * @param  DAOACK message
  * @return A 16bit number representing the checksum
  */
 uint16_t calculateDAOACKChecksum(DAO_ACK* dio){
	 char i;
	 uint16_t checksum = 0;
	 char* data = (char*) dio;
	 for(i = 4 ; i < sizeof(DAO_ACK);i++){ //iterates in the messages
		 checksum += data[i];
	 }
	 return checksum;
 }

 /**
  * This function compares the checksum represented in 16bits with a checksum represented in 2 bytes
  * @param  Checksum represented in 16bits
  * @param  Checksum represented in two number of 8bits
  * @return True if the checksums represent the same number. False otherwise.
  */
 BOOL compareChecksum(uint16_t checksum1, char checksum2[2]){
	 if(COMPARE(checksum1,checksum2)) return TRUE;
	 return FALSE;
 }
