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

#ifndef BYTEARRAY_H
#define BYTEARRAY_H
#include <vector>
class ByteArray{
public:
    void append(char* data,uint8_t size){
        data_.insert(data_.end(),data,data + size);
    }

    void append(char data){
        data_.push_back(data);
    }

    void append(ByteArray& bytearray){
        data_.insert(data_.end(),bytearray.data_.begin(),bytearray.data_.end());
    }

    char* data(){
        return data_.data();
    }

   void remove(uint16_t initial,uint16_t final){

       std::vector<char>::iterator intial_it = data_.begin();
       for(int i =0;i<initial;i++)
           intial_it++;

       std::vector<char>::iterator final_it = data_.begin();
       for(int i =0;i<final;i++)
           final_it++;

       data_.erase(intial_it,final_it);
   }

   unsigned int size(){return data_.size();}
   char at(unsigned int i){return data_.at(i);}
   void clear(){ data_.clear();   }

private:
    std::vector<char> data_;
};

#endif // BYTEARRAY_H
