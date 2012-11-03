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
*  Anthony Rowe
*******************************************************************************/


#include <include.h>
#include <ulib.h>
#include <audio.h>
#include <avr/interrupt.h>
#include<nrk_cfg.h>

void
audio_init()
{
uint8_t i;
// Initialize values here
  DDRA = 0x80;
  ADC_INIT ();
  ADC_ENABLE ();
  ADC_SET_CHANNEL (MIC_PIN);
  audio_index=0;  // set index to 0
  for(i=0; i<AUDIO_BUFS; i++ )
  {
	audio_cnt[i]=0;  // clear buffer counts
  }
  printf( "Audio init\r" );
}


void audio_start()
{
    //TIFR = TIFR | BM (OCF0);      // Clear interrupt flag
    //TIMSK = TIMSK | BM (OCIE0);       
    //TCCR0 = BM (WGM01) | BM (CS01);    
    //OCR0 = 248;   //4Khz
    //OCR0 = 124;   // 8Khz
    //TIMSK = TIMSK | BM (OCIE0);  
    
    ADC_SET_CHANNEL (MIC_PIN);  // could move into start_audio later...
    ADC_SAMPLE_SINGLE ();
    printf( "Audio Started\r" );
}

uint8_t audio_switch_buffers()
{
uint8_t prev_index;
DISABLE_GLOBAL_INT();
prev_index=audio_index;
audio_index++;
if(audio_index>=AUDIO_BUFS) audio_index=0;
audio_cnt[audio_index]=0;
ENABLE_GLOBAL_INT();
return prev_index;
}


void audio_sample(uint8_t state,uint8_t opt,uint8_t *buff,uint8_t size)
{
  uint16_t sample;
  //ff_set_led(1);
  //DISABLE_GLOBAL_INT();
    ADC_SET_CHANNEL (MIC_PIN);  // could move into start_audio later...
    ADC_SAMPLE_SINGLE ();
  ADC_GET_SAMPLE_10 (sample);
  //sample = sample &0xFF;
  sample=sample>>2;
  sample=sample&0xFF;
  if(audio_cnt[audio_index]<MAX_AUDIO_BUF)
  {
	audio_buffers[audio_index][audio_cnt[audio_index]]=sample;
	audio_cnt[audio_index]++;
  }
  //nrk_event_signal(AUDIO_SIGNAL);
  //ENABLE_GLOBAL_INT();
  //ff_clr_led(1);
}

/*SIGNAL (SIG_OUTPUT_COMPARE0)
{
audio_sample();
}*/
