/*

MIT License

Copyright (c) 2025 Oliver Schmidt (https://a2retro.de/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <pico/multicore.h>
#include <a2pico.h>

#include "board.h"

extern const __attribute__((aligned(4))) uint8_t firmware[];

static volatile bool active;

static volatile uint32_t ser_command;
static volatile uint32_t ser_control;

static uint8_t bits[] = {
    0b11111111,
    0b01111111,
    0b00111111,
    0b00011111};

static volatile uint8_t mask;

static void __time_critical_func(reset)(bool asserted) {
    if (asserted) {
        active = false;

        multicore_fifo_drain();
        ser_command = 0b00000000;
        ser_control = 0b00000000;
        mask = bits[0b00];
    }
}

static void __time_critical_func(nop_get)(void) {
}

static void __time_critical_func(ser_dipsw1_get)(void) {
                        // 0001     9600 baud
                        //     00   <zero>
                        //       01 printer mode
    a2pico_putdata(pio0, 0b11101110);
}

static void __time_critical_func(ser_dipsw2_get)(void) {
                        // 1        1 stop bit
                        //  0       <zero>
                        //   0      no delay
                        //    0     <zero>
                        //     11   40 cols
                        //       1  auto-lf
                        //        0 cts line (not dipsw2)   
    a2pico_putdata(pio0, 0b01110000);
}

static void __time_critical_func(ser_data_get)(void) {
    a2pico_putdata(pio0, sio_hw->fifo_rd & mask);
}

static void __time_critical_func(ser_status_get)(void) {
    // SIO_FIFO_ST_VLD_BITS _u(0x00000001)
    // SIO_FIFO_ST_RDY_BITS _u(0x00000002)
    a2pico_putdata(pio0, (sio_hw->fifo_st & 3) << 3);
}

static void __time_critical_func(ser_command_get)(void) {
    a2pico_putdata(pio0, ser_command);
}

static void __time_critical_func(ser_control_get)(void) {
    a2pico_putdata(pio0, ser_control);
}

void (*devsel_get[])(void) = {
    nop_get,      ser_dipsw1_get, ser_dipsw2_get,  nop_get,
    nop_get,      nop_get,        nop_get,         nop_get,
    ser_data_get, ser_status_get, ser_command_get, ser_control_get,
    nop_get,      nop_get,        nop_get,         nop_get
};

static void __time_critical_func(nop_put)(uint32_t data) {
}

static void __time_critical_func(ser_data_put)(uint32_t data) {
    sio_hw->fifo_wr = data & mask;
}

static void __time_critical_func(ser_reset_put)(uint32_t data) {
    ser_command &= 0b11100000;
}

static void __time_critical_func(ser_command_put)(uint32_t data) {
    ser_command = data;
}

static void __time_critical_func(ser_control_put)(uint32_t data) {
    ser_control = data;
    mask = bits[(data >> 5) & 0b11];
}

void (*devsel_put[])(uint32_t) = {
    nop_put,      nop_put,       nop_put,         nop_put,
    nop_put,      nop_put,       nop_put,         nop_put,
    ser_data_put, ser_reset_put, ser_command_put, ser_control_put,
    nop_put,      nop_put,       nop_put,         nop_put
};

void __time_critical_func(board)(void) {

    a2pico_init(pio0);

    a2pico_resethandler(&reset);

    while (true) {
        uint32_t pico = a2pico_getaddr(pio0);
        uint32_t addr = pico & 0x0FFF;
        uint32_t io   = pico & 0x0F00;      // IOSTRB or IOSEL
        uint32_t strb = pico & 0x0800;      // IOSTRB
        uint32_t read = pico & 0x1000;      // R/W

        if (read) {
            if (!io) {  // DEVSEL
                devsel_get[addr & 0xF]();
            } else if (!strb) {  // IOSEL
                a2pico_putdata(pio0, firmware[addr | 0x0700]);
            } else if (active && (addr != 0x0FFF)) {  // IOSTRB
                a2pico_putdata(pio0, firmware[addr & 0x07FF]);
            }
        } else {
            uint32_t data = a2pico_getdata(pio0);
            if (!io) {  // DEVSEL
                devsel_put[addr & 0xF](data);
            }
        }

        if (io && !strb) {
            active = true;
        } else if (addr == 0x0FFF) {
            active = false;
        }
    }
}
