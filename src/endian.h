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
