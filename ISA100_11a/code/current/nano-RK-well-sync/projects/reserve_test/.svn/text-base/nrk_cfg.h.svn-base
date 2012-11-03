/***************************************************************
*                            NanoRK CONFIG                     *
***************************************************************/

#ifndef __nrk_cfg_h	
#define __nrk_cfg_h
// it is easier to see debugging messages.
//#define NRK_HALT_ON_ERROR

// NRK_REPORT_ERRORS will cause the kernel to print out information about
// missed deadlines or reserve violations
#define NRK_REPORT_ERRORS

// NRK_STACK_CHECK adds a little check to see if the bottom of the stack
// has been over written on all suspend calls
#define NRK_STACK_CHECK

// Leave NRK_NO_POWER_DOWN define in if the target can not wake up from sleep 
// because it has no asynchronously clocked
#define NRK_NO_POWER_DOWN

#define NRK_MAX_TASKS       		2    // Max number of tasks in your application
#define NRK_N_SYS_TASKS    		1    // you need at least the idle task
#define	NRK_N_RES			1	
                           

#define NRK_MAX_DRIVER_CNT	       2
#define NRK_DRIVER_FREQ	 	       250 
#define NRK_MAX_RESERVES	       5 

#define NRK_TASK_IDLE_STK_SIZE         128   // Idle task stack size min=32 
#define NRK_APP_STACKSIZE              90 
#define NRK_KERNEL_STACKSIZE           128 
#define NRK_MAX_RESOURCE_CNT           5
#define NRK_MBOX_EN                	0    // Include code for MAILBOXES 
#define NRK_Q_EN                   	0    // Include code for QUEUES  
#define NRK_SEM_EN                 	0    // Include code for SEMAPHORES 
#define NRK_TASK_CHANGE_PRIO_EN    	0    // Include code for OSTaskChangePrio()  


#define NRK_STAT_EXT			0    // Extended Status for OS 

#endif
