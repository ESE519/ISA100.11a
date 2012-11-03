	;TODO: implement these for msp

.global _nrk_timer_suspend_task 

_nrk_timer_suspend_task:
	dint
	nop
	;undo what gcc did
	;pop r12 ;undo function call?
	;pop r13
	;pop r12
	add.w #0x000a, r1
	;pop r12
	;pop r13
	;pop r14
	;pop r15

	;pop r12
	;pop r13
	;pop r14
	;pop r15

	;save state
	;push r2
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
	mov.w nrk_cur_task_TCB, r12
	mov.w r1, @r12

	mov.w nrk_kernel_stk_ptr, r12
	mov.w r12, r1

	;bis.b   #llo(64), &0x0035

	ret

.global nrk_start_high_ready_task 

nrk_start_high_ready_task:

	;bis.b   #llo(64), &0x0035
	;nop

	mov.w nrk_high_ready_TCB, r12
	mov.w @r12, r1

	;mov.w @r15, r1

	;pop r12 ;undo function call?
	;pop r13
	;pop r14
	;pop r15
	;add.w #0x0008, r1

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
	;pop r2

	;bis.b   #llo(1), &0x0035

	;fix interrupt enable
	bis #0x0008,@r1

	;eint

    	reti

