section .text
global exit_fiber
global switch_fiber
global make_fiber
global switch_fiber_cl

; void exit_fiber(struct fiber_t *dest)
exit_fiber:
	mov eax, [esp + 4]		; move pointer to destination context into eax
	mov esp, [eax]			; move stack pointer into esp
	jmp [eax + 4]			; jump to address

; void switch_fiber_cl(struct fiber_t *current, struct fiber_t *dest, int32_t *reg_values)
switch_fiber_cl:
	push eax			; push registers onto stack sp + 24
	push ebx			; sp + 20
	push ecx			; sp + 16
	push edx			; sp + 12
	push esi			; sp + 8
	push edi			; sp + 4
	push ebp			; sp

	mov eax, [esp + 32]		; move pointer to current context into eax
	mov ebx, [esp + 36]		; move pointer to destitnation context into ebx
	mov ecx, [esp + 40]		; move reg_values into ecx

	sub esp, 0x80			; make room for SSE registers
	movups [esp], xmm0		; back up SSE registers
	movups [esp + 0x10], xmm1	
	movups [esp + 0x20], xmm2
	movups [esp + 0x30], xmm3
	movups [esp + 0x40], xmm4
	movups [esp + 0x50], xmm5
	movups [esp + 0x60], xmm6
	movups [esp + 0x70], xmm7


	movaps xmm0, [ecx]		; AMD uses xmm0-xmm3 to pass in parameters.
	movaps xmm1, [ecx + 0x10]
	movaps xmm2, [ecx + 0x20]
	movaps xmm3, [ecx + 0x30]	

	mov [eax], esp			; save stack pointer in current context.  (struct fiber_t *)eax->esp = esp
	jmp done_backup


; void switch_fiber(struct fiber_t *current, struct fiber_t *dest)
switch_fiber:
	push eax			; push registers onto stack sp + 24
	push ebx			; sp + 20
	push ecx			; sp + 16
	push edx			; sp + 12
	push esi			; sp + 8
	push edi			; sp + 4
	push ebp			; sp

	mov eax, [esp + 32]		; move pointer to current context into eax
	mov ebx, [esp + 36]		; move pointer to destitnation context into ebx

	sub esp, 0x80			; make room for SSE registers
	movups [esp], xmm0		; back up SSE registers
	movups [esp + 0x10], xmm1	
	movups [esp + 0x20], xmm2
	movups [esp + 0x30], xmm3
	movups [esp + 0x40], xmm4
	movups [esp + 0x50], xmm5
	movups [esp + 0x60], xmm6
	movups [esp + 0x70], xmm7

	mov [eax], esp			; save stack pointer in current context.  (struct fiber_t *)eax->esp = esp
done_backup:
	call get_pc
get_pc:
	pop ecx				; now ecx has the absolute value of get_pc
	add ecx, (return_place - get_pc); now ecx has the absolute value of return_place
	mov [eax + 4], ecx		; save destination pc into current context.  (struct fiber_t *)eax->epc = ecx
	mov esp, [ebx]			; esp = (struct fiber_t *)ebx->esp
	jmp [ebx + 4]			; jump to (struct fiber_t *)ebx->epc

return_place:
	movups xmm0, [esp]		; restore SSE registers
	movups xmm1, [esp + 0x10]	
	movups xmm2, [esp + 0x20]
	movups xmm3, [esp + 0x30]
	movups xmm4, [esp + 0x40]
	movups xmm5, [esp + 0x50]
	movups xmm6, [esp + 0x60]
	movups xmm7, [esp + 0x70]
	add esp, 0x80

	pop ebp				; pop registers off of stack.
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret

