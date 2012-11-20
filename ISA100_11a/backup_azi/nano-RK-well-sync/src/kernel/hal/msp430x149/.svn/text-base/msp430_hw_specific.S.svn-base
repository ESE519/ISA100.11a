.global _nrk_timer_suspend_task 

_nrk_timer_suspend_task:
	dint
	nop

	add.w #0x000a, r1 ;Remove what gcc pushed.

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

	ret

.global nrk_start_high_ready_task 

nrk_start_high_ready_task:
	mov.w nrk_high_ready_TCB, r12
	mov.w @r12, r1

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

	;Now the last two things on the stack are the status reg and the PC.
	;The status reg contains the interrupt enable and low power mode settings.
	;We reset these so that interrupts are enabled and the CPU does not reti
	;to a low power mode.
	bis #0x0008,@r1
	bic #0x00f0,@r1

  reti

