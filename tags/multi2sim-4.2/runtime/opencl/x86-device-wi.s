.text
.global opencl_x86_work_item_entry_point
.global opencl_x86_device_switch_fiber
.equ X86_STACK_SIZE, 0x2000
.equ X86_WORKGROUP_OFFSET, -0x60
.equ X86_STACK_MASK, ~(X86_STACK_SIZE - 1)

# takes no args, not a real function
opencl_x86_work_item_entry_point:
	lea X86_STACK_SIZE(%esp), %eax
	and $X86_STACK_MASK, %eax
	mov X86_WORKGROUP_OFFSET(%eax), %eax # now eax has core data structure

	mov 0x0(%eax), %ecx # ecx gets register params
	mov 0x4(%eax), %eax # eax gets meta-function address

	movaps 0x00(%ecx), %xmm0 # AMD uses xmm0-xmm3 to pass in parameters.
	movaps 0x10(%ecx), %xmm1
	movaps 0x20(%ecx), %xmm2
	movaps 0x30(%ecx), %xmm3
	jmp *%eax

# void opencl_x86_device_switch_fiber(
#	volatile struct opencl_x86_device_fiber_t *current,
#	volatile struct opencl_x86_device_fiber_t *dest)

opencl_x86_device_switch_fiber:
	push %eax		/* Push registers onto sp + 24 */
	push %ebx		/* sp + 20 */
	push %ecx		/* sp + 16 */
	push %edx		/* sp + 12 */
	push %esi		/* sp + 8 */
	push %edi		/* sp + 4 */
	push %ebp		/* sp */

	mov 32(%esp), %eax	/* eax <= current */
	mov 36(%esp), %edx	/* edx <= dest */

	sub $0x80, %esp		/* Make room for SSE registers */
	movups %xmm0, 0x0(%esp)
	movups %xmm1, 0x10(%esp)
	movups %xmm2, 0x20(%esp)
	movups %xmm3, 0x30(%esp)
	movups %xmm4, 0x40(%esp)
	movups %xmm5, 0x50(%esp)
	movups %xmm6, 0x60(%esp)
	movups %xmm7, 0x70(%esp)

	mov %esp, (%eax)	/* current->esp <= esp */
	movl $fiber_return, 0x4(%eax)	/* current->eip <= label 'switch_fiber_return' */

	mov (%edx), %esp	/* esp <= dest->esp */
	jmp *0x4(%edx)		/* eip <= dest->eip */

fiber_return:
	movups 0x0(%esp), %xmm0
	movups 0x10(%esp), %xmm1
	movups 0x20(%esp), %xmm2
	movups 0x30(%esp), %xmm3
	movups 0x40(%esp), %xmm4
	movups 0x50(%esp), %xmm5
	movups 0x60(%esp), %xmm6
	movups 0x70(%esp), %xmm7
	add $0x80, %esp

	pop %ebp
	pop %edi
	pop %esi
	pop %edx
	pop %ecx
	pop %ebx
	pop %eax
	ret
