/*
   Copyright (c) 2015 Bernhard Schneider <bernhard@neaptide.org> 

   All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   */


#include <MCP3911.h>

using namespace MCP3911;

void C::reset(void) {

    const uint8_t r = 0b11000000;

    SPI_write(REG_CONFIG2, (uint8_t *)&r, 1); // 6.7.2 p. 47

}

bool C::reg_read(uint8_t addr, Looping g, uint8_t count) {
    uint8_t *data = ((uint8_t *)_c)+addr;

    if (count == 0) {
        switch (g) {
            case REGISTER: count = 1; 
                           break;
            case GROUP: switch (addr) {
                            case REG_CHANNEL_0: 
                            case REG_CHANNEL_1:  count = 3; break;
                            case REG_MOD: 
                            case REG_STATUSCOM:  count = 4; break;
                            case REG_OFFCAL_CH0:
                            case REG_OFFCAL_CH1: count = 6; break;
                            case REG_VREFCAL:    count = 1; break;
                        }
                        break;
            case TYPE: switch (addr) {
                           case REG_CHANNEL_0: count = 6; break;
                           case REG_MOD: count = 21; break;
                       }
                       break;
            case ALL: switch (addr) {
                          case REG_CHANNEL_0: count = 0x1b; break;
                      }
                      break;
            default: return false;
        }
    }

    SPI_read(addr, data, count);

    return true;
}

bool C::reg_write(uint8_t addr, Looping g, uint8_t count) {
    uint8_t *data = ((uint8_t *)_c)+addr;

    if (count == 0) {
        switch (g) {
            case REGISTER: count = 1; 
                           break;
            case TYPE: switch (addr) {
                           case REG_MOD: count = 21; break;
                       }
                       break;
            case GROUP: break;
            case ALL: break;
            default: return false;
        }}

    SPI_write(addr, data, count);

    return true;
}

void C::get_value(double *result, uint8_t channel) {

    _ChVal& val = (*_c).ch[channel];

    int32_t data_ch = msb2l((uint8_t *)&val, 3);

    if (data_ch & 0x00800000L)
        data_ch |= 0xff000000L;

    uint8_t gain = (channel == 0)?(*_c).gain.ch0:(*_c).gain.ch1;

    *result = Vref * data_ch / ( (3*32768*256/2) * (1 << gain) );
}

// msb array to ulong
uint32_t C::msb2l(void *src, uint8_t count) {
    uint32_t value = 0L;
    uint8_t *b = (uint8_t *)src;

    while (count--) {
        value <<= 8;
        value |= *b++;
    }
    return value;
}

// ulong to msb array
void C::l2msb(uint32_t value, void *tgt, uint8_t count) {
    uint32_t v = value;
    uint8_t *b = (uint8_t *)tgt;

    while (1) {
        count--;
        *(b+count) = (uint8_t)(v & 0xff);
        if (count == 0)
            break;
        v >>= 8;
    }
}
