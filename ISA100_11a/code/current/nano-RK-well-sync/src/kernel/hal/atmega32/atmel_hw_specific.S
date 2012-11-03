#define __SREG__ 0x3f
#define __SP_H__ 0x3e
#define __SP_L__ 0x3d






.global _nrk_timer_suspend_task 

_nrk_timer_suspend_task:

	cli
        pop r31
        pop r31

	
    	push    r1
	push	r1
   	push	r0
    	in	r0, __SREG__	; 63
  	push	r0
    	eor 	r1, r1
		
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
    	push r18
    	push r19
    	push r20
    	push r21
    	push r22
    	push r23
    	push r24
    	push r25
    	push r26
    	push r27
	push r28
    	push r29
   	push r30
	push r31

    	pop r16

    	lds r30,nrk_cur_task_TCB		
    	lds r31,nrk_cur_task_TCB+1
  	
	;We now have OSTCBcur in Z register

	;move SP into registers
  

    	in r1,__SP_H__
    	in r2,__SP_L__

	;write out

    	st z+,r2
    	st z,r1
	
	; load r26 and r27 with return address based on this 16 bit pointer
	lds r26,nrk_kernel_stk_ptr		
	lds r27,nrk_kernel_stk_ptr+1
    	
	ld r1,-x

	out __SP_H__, r27
	out __SP_L__, r26  

	ret

.global nrk_start_high_ready_task 

nrk_start_high_ready_task:


	lds r30,nrk_high_ready_TCB		
	lds r31,nrk_high_ready_TCB+1

    	;z points to &OSTCB[x]
    
	ld r26,z+ 		
	ld r27,z
  
    	;x points to OSTCB[x].StkPointer

    	ld r1,-x 

	;We now have &OSTCBHigh->StkPointer in X register
	
	out __SP_H__, r27
	out __SP_L__, r26
  
	pop	r31	
	pop	r30
	
	
	
	pop r29
	pop r28
			
	pop	r27
	pop	r26
	pop	r25
	pop	r24			
	pop	r23
	pop	r22
	pop	r21
	pop	r20	
	pop	r19
	pop	r18	
	
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
		
	
	pop	r0
	out	__SREG__, r0	; 63	
	pop	r0	
    	pop	r1

	
	pop r1
	   


	sei
    	reti 



;.global _nrk_suspend_task
;
;_nrk_suspend_task:
;    	cli
;    	push    r1
;	push	r1
;   	push	r0
;    	in	r0, __SREG__	; 63
;  	push	r0
;    	eor 	r1, r1
		
;	push r2
;	push r3
;	push r4
;	push r5
;	push r6
;	push r7
;	push r8
;	push r9
;	push r10
;	push r11
;	push r12
;	push r13
;	push r14
;	push r15
;	push r16
;	push r17
		
;    	push	r18
;    	push	r19
;    	push	r20
;    	push	r21
;    	push	r22
;    	push	r23
;    	push	r24
;    	push	r25
;    	push	r26
;    	push	r27
	
;	push    r28
;    	push    r29

;   	push	r30
;	push	r31

;    	pop r16

;    	lds r30,nrk_cur_task_TCB		
;    	lds r31,nrk_cur_task_TCB+1
  	
	;We now have OSTCBcur in Z register
	;move SP into registers
  

;    	in r1,__SP_H__
;    	in r2,__SP_L__

	;write out

;    	st z+,r2
;    	st z,r1
	
	; sp <- (OSTCB[current])
  
;	lds r26,nrk_kernel_stk_ptr		
;	lds r27,nrk_kernel_stk_ptr+1

;    	ld r1,-x
;	out __SP_H__, r27
;	out __SP_L__, r26  

;	ret	

