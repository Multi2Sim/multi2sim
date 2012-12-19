#ifndef __FIBERS_H__
#define __FIBERS_H__

typedef void (*fiber_proc_t)(void);
typedef void (*return_address_t)(void);

struct fiber_t
{
	void *esp;
	void *eip;
	void *stack_bottom;
	size_t stack_size;
};

void make_fiber(struct fiber_t *fiber, fiber_proc_t fiber_proc, int num_args, ...);
void make_fiber_ex(struct fiber_t *fiber, fiber_proc_t fiber_proc, return_address_t return_proc, int arg_size, void *args);
void switch_fiber(struct fiber_t *current, struct fiber_t *dest, void *reg_values);

#endif
