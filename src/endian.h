/*
 * endian.h
 * crucio
 *
 * Copyright 2007 Davide De Rosa
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __ENDIAN_H
#define __ENDIAN_H

// big endian (network byte order), unrolled loops
namespace crucio {
    inline void shortToBytes(const unsigned short num, char* const buf) {
        buf[0] = (char)((num & 0x0000FF00) >> 8);
        buf[1] = (char)(num & 0x000000FF);
    }

    inline void longToBytes(const unsigned long num, char* const buf) {
        buf[0] = (char)((num & 0xFF000000) >> 24);
        buf[1] = (char)((num & 0x00FF0000) >> 16);
        buf[2] = (char)((num & 0x0000FF00) >> 8);
        buf[3] = (char)(num & 0x000000FF);
    }

    inline unsigned short bytesToShort(const char* const buf) {
        unsigned short num = (unsigned char) buf[0];
        num <<= 8;
        num |= (unsigned char) buf[1];
        return num;
    }

    inline unsigned long bytesToLong(const char* const buf) {
        unsigned long num = (unsigned char) buf[0];
        num <<= 8;
        num |= (unsigned char) buf[1];
        num <<= 8;
        num |= (unsigned char) buf[2];
        num <<= 8;
        num |= (unsigned char) buf[3];
        return num;
    }
}

#endif
