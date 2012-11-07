/***************************************************************
*                            NanoRK CONFIG                     *
***************************************************************/

#ifndef __nrk_cfg_h	
#define __nrk_cfg_h

// NRK_REPORT_ERRORS will cause the kernel to print out information about
// missed deadlines or reserve violations
#define NRK_REPORT_ERRORS
// NRK_HALT_ON_ERRORS will cause the kernel to freeze on errors so that
// it is easier to see debugging messages.
#define NRK_HALT_ON_ERROR

// NRK_STACK_CHECK adds a little check to see if the bottom of the stack
// has been over written on all suspend calls
#define NRK_STACK_CHECK

// Leave NRK_NO_POWER_DOWN define in if the target can not wake up from sleep 
// because it has no asynchronously clocked
//  #define NRK_NO_POWER_DOWN

#define NRK_MAX_TASKS       		4    // Max number of tasks in your application
                           

#define NRK_TASK_IDLE_STK_SIZE         128   // Idle task stack size min=32 
#define NRK_APP_STACKSIZE              128 
#define NRK_KERNEL_STACKSIZE           128 
#define NRK_MAX_RESOURCE_CNT           1 


#endif
