section .text
global get_workgroup_data
STACK_SIZE equ 0x2000
WORKGROUP_OFFSET equ -0x60
; struct clcpu_workgroup_t *get_workgroup_data()
;
; to be called from within a work-item.  Gets the workgroup pointer on the stack.
;

get_workgroup_data:
	lea eax, [esp + STACK_SIZE]
	and eax, ~(STACK_SIZE - 1)
	mov eax, [eax + WORKGROUP_OFFSET]
	ret
