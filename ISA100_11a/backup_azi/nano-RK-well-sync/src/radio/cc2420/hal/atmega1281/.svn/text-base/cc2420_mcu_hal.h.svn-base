/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Chipcon Development Team 
*  Anthony Rowe
*******************************************************************************/


#ifndef _CC2420_MCU_HAL_H_
#define _CC2420_MCU_HAL_H_

// This file contains CC2420 processor specific attributes


//
//  FAST SPI: Register access
//      s = command strobe
//      a = register address
//      v = register value

#define FASTSPI_STROBE(s) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(s); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_SETREG(a,v) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(a); \
        FASTSPI_TX((uint8_t) ((v) >> 8)); \
        FASTSPI_TX((uint8_t) (v)); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_GETREG(a,v) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(a); \
        FASTSPI_RX_WORD(v); \
        SPI_DISABLE(); \
    } while (0)

// Updates the SPI status byte
#define FASTSPI_UPD_STATUS(s) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(CC2420_SNOP); \
        s = SPDR; \
        SPI_DISABLE(); \
    } while (0)
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
//  FAST SPI: FIFO access
//      p = pointer to the byte array to be read/written
//      c = the number of bytes to read/write
//      b = single data byte

#define FASTSPI_WRITE_FIFO(p,c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(CC2420_TXFIFO); \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            FASTSPI_TX(((uint8_t*)(p))[spiCnt]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO(p,c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            while (!FIFO_IS_1); \
            FASTSPI_RX(((uint8_t*)(p))[spiCnt]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO_BYTE(b) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        FASTSPI_RX(b); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO_NO_WAIT(p,c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            FASTSPI_RX(((uint8_t*)(p))[spiCnt]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO_GARBAGE(c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        for (uint8_t spiCnt = 0; ((spiCnt < (c)) && (FIFO_IS_1)); spiCnt++) { \
            FASTSPI_RX_GARBAGE(); \
        } \
        SPI_DISABLE(); \
    } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
//  FAST SPI: CC2420 RAM access (big or little-endian order)
//      p = pointer to the variable to be written
//      a = the CC2420 RAM address
//      c = the number of bytes to write
//      n = counter variable which is used in for/while loops (uint8_t)
//
//  Example of usage:
//      uint8_t n;
//      uint16_t shortAddress = 0xBEEF;
//      FASTSPI_WRITE_RAM_LE(&shortAddress, CC2420RAM_SHORTADDR, 2);

#define FASTSPI_WRITE_RAM_LE(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX((a >> 1) & 0xC0); \
        for (n = 0; n < (c); n++) { \
            FASTSPI_TX(((uint8_t*)(p))[n]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_RAM_LE(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX(((a >> 1) & 0xC0) | 0x20); \
        for (n = 0; n < (c); n++) { \
            FASTSPI_RX(((uint8_t*)(p))[n]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_WRITE_RAM(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX((a >> 1) & 0xC0); \
        n = c; \
        do { \
            FASTSPI_TX(((uint8_t*)(p))[--n]); \
        } while (n); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_RAM(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX(((a >> 1) & 0xC0) | 0x20); \
        n = c; \
        do { \
            FASTSPI_RX(((uint8_t*)(p))[--n]); \
        } while (n); \
        SPI_DISABLE(); \
    } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Other useful SPI macros
#define FASTSPI_RESET_CC2420() \
    do { \
        FASTSPI_SETREG(CC2420_MAIN, 0x0000); \
        FASTSPI_SETREG(CC2420_MAIN, 0xF800); \
    } while (0)
//-------------------------------------------------------------------------------------------------------

#endif
