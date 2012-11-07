/*****************************************************************************
* Copyright (c) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Carnegie Mellon University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Contributing Authors:
* Nuno Pereira - Polytechnic Institute of Porto; nap@isep.ipp.pt
*******************************************************************************/

/* ========================================================================== */
/*                                                                            */
/*   wd_timer.h                                                               */
/*                                                                            */
/*   USE_TIMER_COUNTER defines which timer/counter from the atmega128 is used */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#ifndef _WD_TIMER_CPU_CLK_H
#define _WD_TIMER_CPU_CLK_H

#include <hal.h>

// This defines which 16-bit timer/counter is used (1 or 3)
#define USE_TIMER_COUNTER 3

#define TCLK_CPU_DIV TIMER_CLK_DIV8

#if TCLK_CPU_DIV == TIMER_CLK_DIV1
// Clock resolution in seconds (125 ns)
#define CPU_CLOCK_TICK_TIME	0.000000125
#endif

#if TCLK_CPU_DIV == TIMER_CLK_DIV8
// Clock resolution in seconds (1 us)
#define CPU_CLOCK_TICK_TIME	0.000001
#endif

#if TCLK_CPU_DIV == TIMER_CLK_DIV64
// Clock resolution in seconds (8 us)
#define CPU_CLOCK_TICK_TIME	0.000008
#endif

#if TCLK_CPU_DIV == TIMER_CLK_DIV256
// Clock resolution in seconds (32 us)
#define CPU_CLOCK_TICK_TIME	0.000032
#endif

#endif

