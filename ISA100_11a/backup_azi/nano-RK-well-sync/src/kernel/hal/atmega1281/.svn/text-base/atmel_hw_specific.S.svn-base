#define __SREG__ 0x3f
#define __SP_H__ 0x3e
#define __SP_L__ 0x3d


.global nrk_start_high_ready_task 

nrk_start_high_ready_task:

	lds r26,nrk_high_ready_TCB		
	lds r27,nrk_high_ready_TCB+1

    	;x points to &OSTCB[x]
    
	ld r28,x+
	out __SP_L__, r28
	ld r29,x+
	out __SP_H__, r29
  
	pop r31	
	pop r30
	pop r29
	pop r28
	pop r27
	pop r26
	pop r25
	pop r24			
	pop r23
	pop r22
	pop r21
	pop r20	
	pop r19
	pop r18	
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	pop r2
	pop r1
	pop r0
	out __SREG__, r0
	pop r0	
	   
    	reti 



