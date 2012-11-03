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
*******************************************************************************/

#include <include.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <nrk_timer.h>
#include <avr/eeprom.h>
#include "flash.h"
#include "readflash.h"

void ws_flash_read_page(uint32_t addr, uint8_t *buf);
void ws_flash_read_byte(uint32_t addr, uint8_t *buf);

// Read 256 bytes(1 Page) from flash
void ws_flash_read_page(uint32_t addr, uint8_t *buf)
{
  uint8_t sreg;
  uint32_t i;

  sreg = SREG;
  cli();
  eeprom_busy_wait();

  for (i=0;i<PAGESIZE;i++)
  {
    _WAIT_FOR_SPM();
    eeprom_busy_wait();
    boot_spm_busy_wait();
    _ENABLE_RWW_SECTION();
    buf[i] = pgm_read_byte_far(addr+i);
  }

  boot_rww_enable();
  SREG = sreg;
  return;
}

// Read 1 byte from flash
void ws_flash_read_byte(uint32_t addr, uint8_t *buf)
{
  uint8_t sreg;

  sreg = SREG;
  cli();
  eeprom_busy_wait();

  _WAIT_FOR_SPM();
  eeprom_busy_wait();
  boot_spm_busy_wait();
  _ENABLE_RWW_SECTION();
  *buf = pgm_read_byte_far(addr);

  boot_rww_enable();
  SREG = sreg;
  return;
}
