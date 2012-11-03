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


#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_reserve.h>
#include <nrk_stack_check.h>


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1 (void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);


void nrk_create_taskset ();

int main ()
{
  uint8_t t;
  nrk_setup_ports ();
  nrk_setup_uart (UART_BAUDRATE_115K2);

  printf ("Starting up...\r\n");

  nrk_init ();

  nrk_led_clr (ORANGE_LED);
  nrk_led_clr (BLUE_LED);
  nrk_led_set (GREEN_LED);
  nrk_led_clr (RED_LED);

  nrk_time_set (0, 0);
  nrk_create_taskset ();
  nrk_start ();

  return 0;
}

void my_error ()
{
  nrk_kprintf (PSTR ("DOH, Reserve error handler called...\r\n"));
}

void Task1 ()
{
  uint16_t cnt;
  int8_t v;
  int8_t my_rsv;
  nrk_time_t t;

  my_rsv = nrk_reserve_create ();
  if (my_rsv == NRK_ERROR)
    nrk_kprintf (PSTR ("ERROR creating reserve\r\n"));
  t.secs = 10;
  t.nano_secs = 0;
// create a 10 second reserve with 10 accesses
// my_error is called when the reserve is violated
  v = nrk_reserve_set (my_rsv, &t, 10, &my_error);
  if (v == NRK_ERROR)
    nrk_kprintf (PSTR ("ERROR setting reserve\r\n"));

  printf ("My node's address is %d\r\n", NODE_ADDR);

  printf ("Task1 PID=%d\r\n", nrk_get_pid ());
  cnt = 0;
  while (1) {
    nrk_led_toggle (ORANGE_LED);
    printf ("Task1 cnt=%d\r\n", cnt);
    v = nrk_reserve_consume (my_rsv);
    if (v == NRK_ERROR)
      nrk_kprintf (PSTR ("Task 1: ERROR consuming reserve\r\n"));
    v = nrk_reserve_get (my_rsv);
    printf ("Reserve value: %d\r\n", v);
    nrk_wait_until_next_period ();
    cnt++;
  }
}

void Task2 ()
{
  uint8_t cnt;
  int8_t my_other_rsv, v;
  nrk_time_t t;

  my_other_rsv = nrk_reserve_create ();
  if (my_other_rsv == NRK_ERROR)
    nrk_kprintf (PSTR ("ERROR creating reserve\r\n"));
  t.secs = 10;
  t.nano_secs = 0;
  // create a 10 second reserve with 10 accesses
  // No function is called when the reserve is violated
  v = nrk_reserve_set (my_other_rsv, &t, 10, NULL);
  if (v == NRK_ERROR)
    nrk_kprintf (PSTR ("ERROR setting reserve\r\n"));

  printf ("Task2 PID=%d\r\n", nrk_get_pid ());
  cnt = 0;
  while (1) {
    // This will silently fail half of the time
    if (nrk_reserve_consume (my_other_rsv) != NRK_ERROR) {
      nrk_led_toggle (BLUE_LED);
      printf ("Task2 cnt=%d\r\n", cnt);
      cnt++;
    }
    nrk_wait_until_next_period ();
  }
}


void nrk_create_taskset ()
{
  TaskOne.task = Task1;
  TaskOne.Ptos = (void *) &Stack1[NRK_APP_STACKSIZE];
  TaskOne.Pbos = (void *) &Stack1[0];
  TaskOne.prio = 1;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 0;
  TaskOne.period.nano_secs = 500 * NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 50 * NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs = 0;
  nrk_activate_task (&TaskOne);

  TaskTwo.task = Task2;
  TaskTwo.Ptos = (void *) &Stack2[NRK_APP_STACKSIZE];
  TaskTwo.Pbos = (void *) &Stack2[0];
  TaskTwo.prio = 2;
  TaskTwo.FirstActivation = TRUE;
  TaskTwo.Type = BASIC_TASK;
  TaskTwo.SchType = PREEMPTIVE;
  TaskTwo.period.secs = 0;
  TaskTwo.period.nano_secs = 500 * NANOS_PER_MS;
  TaskTwo.cpu_reserve.secs = 0;
  TaskTwo.cpu_reserve.nano_secs = 100 * NANOS_PER_MS;
  TaskTwo.offset.secs = 0;
  TaskTwo.offset.nano_secs = 0;
  nrk_activate_task (&TaskTwo);


}
