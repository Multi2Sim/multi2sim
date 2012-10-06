#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <m2s-clrt.h>

void exit_work_item(void);

void make_fiber(struct fiber_t *fiber, fiber_proc_t fiber_proc, int num_args, ...)
{
	int i;
	int *params;
	int stack_words;
	int *stack_bottom;

	params = &num_args + 1;

	assert((int) params % sizeof (int) == 0);
	assert((int) fiber->stack_bottom % sizeof (int) == 0);
	assert(fiber->stack_size % sizeof (int) == 0);

	stack_words = fiber->stack_size / sizeof (int);
	stack_bottom = (int *) fiber->stack_bottom;

	for (i = 0; i < num_args; i++)
		stack_bottom[stack_words - num_args + i] = params[i];
	/* return address of fake 'calling function' */
	stack_bottom[stack_words - num_args - 1] = 0;

	fiber->eip = fiber_proc;
	fiber->esp = stack_bottom + stack_words - num_args - 1;
}

void make_fiber_ex(struct fiber_t *fiber, fiber_proc_t fiber_proc, return_address_t return_proc, int arg_size, void *args)
{
	arg_size *= sizeof (size_t);
	assert(arg_size % sizeof (size_t) == 0);
	assert((size_t) fiber->stack_bottom % sizeof (size_t) == 0);
	assert(fiber->stack_size % sizeof (size_t) == 0);

	memcpy((char *) fiber->stack_bottom + fiber->stack_size - arg_size, args, arg_size);

	
	fiber->eip = fiber_proc;
	fiber->esp = (char *)fiber->stack_bottom + fiber->stack_size - arg_size - sizeof (size_t);
	*(size_t *) fiber->esp = (size_t)return_proc;
}

