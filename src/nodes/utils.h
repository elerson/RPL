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

#ifndef UTILS_H_
#define UTILS_H_

#define COPYUINT16(var,val) {var[1] = (val>>8); var[0] = val&0xff;}
#define COPYINADDR(var,val) {var.addr[1] = (val>>8); var.addr[0] = val&0xff;}
#define COPYARRAY2INT16(val,var) { val = var[1]; val = val << 8 ; val |= var[0];}
#define COPYCHAR2INR(val,var) { val = var.addr[1]; val = val << 8 ; val |= var.addr[0];}
#define COMPARE(val16bits,data)  ((data[0] == (val16bits&0xff))&&(data[1] == (val16bits>>8)))

typedef char BOOL;

#define TRUE 1
#define FALSE 0

#endif /* UTILS_H_ */
