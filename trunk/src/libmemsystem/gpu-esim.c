/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <mem-system.h>
#include <gpuarch.h>


/*
 * Event stack
 */


long long mod_stack_id;

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
	uint32_t addr, int ret_event, void *ret_stack)
{
	struct mod_stack_t *stack;

	/* Create stack */
	stack = calloc(1, sizeof(struct mod_stack_t));
	if (!stack)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	stack->id = id;
	stack->mod = mod;
	stack->addr = addr;
	stack->ret_event = ret_event;
	stack->ret_stack = ret_stack;

	/* Return */
	return stack;
}


void mod_stack_return(struct mod_stack_t *stack)
{
	int ret_event = stack->ret_event;
	void *ret_stack = stack->ret_stack;

	free(stack);
	esim_schedule_event(ret_event, ret_stack, 0);
}


void mod_stack_wait_in_cache(struct mod_stack_t *stack, int event)
{
	struct mod_t *mod = stack->mod;

	assert(!DOUBLE_LINKED_LIST_MEMBER(mod, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(mod, waiting, stack);
}


/* Enqueue stack in waiting list of 'stack->port' */
void mod_stack_wait_in_port(struct mod_stack_t *stack, int event)
{
	struct mod_port_t *port = stack->port;

	assert(!DOUBLE_LINKED_LIST_MEMBER(port, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(port, waiting, stack);
}


/* Wake up accesses in 'mod->waiting_list' */
void mod_wakeup(struct mod_t *mod)
{
	struct mod_stack_t *stack;
	int event;

	while (mod->waiting_list_head) {
		stack = mod->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(mod, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


/* Wake up accesses in 'port->waiting_list' */
void mod_port_wakeup(struct mod_port_t *port)
{
	struct mod_stack_t *stack;
	int event;

	while (port->waiting_list_head) {
		stack = port->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(port, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


/*
 * Event-Driven Simulation
 */

int EV_GPU_MEM_READ;
int EV_GPU_MEM_READ_REQUEST;
int EV_GPU_MEM_READ_REQUEST_RECEIVE;
int EV_GPU_MEM_READ_REQUEST_REPLY;
int EV_GPU_MEM_READ_REQUEST_FINISH;
int EV_GPU_MEM_READ_UNLOCK;
int EV_GPU_MEM_READ_FINISH;

int EV_GPU_MEM_WRITE;
int EV_GPU_MEM_WRITE_REQUEST_SEND;
int EV_GPU_MEM_WRITE_REQUEST_RECEIVE;
int EV_GPU_MEM_WRITE_REQUEST_REPLY;
int EV_GPU_MEM_WRITE_REQUEST_REPLY_RECEIVE;
int EV_GPU_MEM_WRITE_UNLOCK;
int EV_GPU_MEM_WRITE_FINISH;

#define CYCLE ((long long) esim_cycle)
#define ID ((long long) stack->id)


void mod_handler_read(int event, void *data)
{
	struct mod_stack_t *stack = data, *newstack;
	struct mod_t *mod = stack->mod;

	if (event == EV_GPU_MEM_READ)
	{
		struct mod_port_t *port;
		int i;

		/* If there is any pending access in the cache, this access should
		 * be enqueued in the waiting list, since all accesses need to be
		 * done in order. */
		if (mod->waiting_list_head) {
			gpu_mem_debug("%lld %lld read cache=\"%s\" addr=%u\n",
				CYCLE, ID, mod->name, stack->addr);
			gpu_mem_debug("%lld %lld wait why=\"order\"\n",
				CYCLE, ID);
			mod_stack_wait_in_cache(stack, EV_GPU_MEM_READ);
			return;
		}

		/* Get bank, set, way, tag, status */
		stack->tag = stack->addr & ~(mod->block_size - 1);
		stack->block_index = stack->tag >> mod->log_block_size;
		stack->bank_index = stack->block_index % mod->bank_count;
		stack->bank = MOD_BANK_INDEX(mod, stack->bank_index);

		/* If any read port in bank is processing the same tag, there are two options:
		 *   1) If the previous access started in the same cycle, it will be coalesced with the
		 *      current access, assuming that they were issued simultaneously.
		 *   2) If the previous access started in a previous cycle, the new access will
		 *      wait until the previous access finishes, because there might be writes in
		 *      between. */
		for (i = 0; i < mod->read_port_count; i++)
		{
			/* Do something if the port is locked and it is handling the same tag. */
			port = MOD_READ_PORT_INDEX(mod, stack->bank, i);
			if (!port->locked || port->stack->tag != stack->tag)
				continue;

			/* If current and previous access are in the same cycle, coalesce. */
			if (port->lock_when == esim_cycle)
			{
				gpu_mem_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, mod->name, stack->addr, stack->bank_index);
				stack->read_port_index = i;
				stack->port = port;
				gpu_mem_debug("  %lld %lld coalesce id=%lld bank=%d read_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index, stack->read_port_index);
				mod_stack_wait_in_port(stack, EV_GPU_MEM_READ_FINISH);

				/* Stats */
				mod->reads++;
				return;
			}

			/* Current block is handled by an in-flight access, wait for it. */
			gpu_mem_debug("%lld %lld read cache=\"%s\" addr=%u\n",
				CYCLE, ID, mod->name, stack->addr);
			gpu_mem_debug("%lld %lld wait why=\"in_flight\"\n",
				CYCLE, ID);
			mod_stack_wait_in_cache(stack, EV_GPU_MEM_READ);
			return;
		}

		/* Look for a free read port */
		for (i = 0; i < mod->read_port_count; i++) {
			port = MOD_READ_PORT_INDEX(mod, stack->bank, i);
			if (!port->locked) {
				stack->read_port_index = i;
				stack->port = port;
				break;
			}
		}
		
		/* If there is no free read port, enqueue in cache waiting list. */
		if (!stack->port) {
			gpu_mem_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d\n",
				CYCLE, ID, mod->name, stack->addr, stack->bank_index);
			gpu_mem_debug("  %lld %lld wait why=\"no_read_port\"\n", CYCLE, ID);
			mod_stack_wait_in_cache(stack, EV_GPU_MEM_READ);
			return;
		}

		/* Lock read port */
		port = stack->port;
		assert(!port->locked);
		assert(mod->locked_read_port_count < mod->read_port_count * mod->bank_count);
		port->locked = 1;
		port->lock_when = esim_cycle;
		port->stack = stack;
		mod->locked_read_port_count++;
	
		/* Stats */
		mod->reads++;
		mod->effective_reads++;

		/* If there is no cache, assume hit */
		if (!mod->cache)
		{
			esim_schedule_event(EV_GPU_MEM_READ_UNLOCK, stack, mod->latency);

			/* Stats */
			mod->effective_read_hits++;
			gpu_mem_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d read_port=%d\n",
				CYCLE, ID, mod->name, stack->addr, stack->bank_index, stack->read_port_index);
			return;
		}

		/* Get block from cache, consuming 'latency' cycles. */
		stack->hit = cache_find_block(mod->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		if (stack->hit)
		{
			mod->effective_read_hits++;
			esim_schedule_event(EV_GPU_MEM_READ_UNLOCK, stack, mod->latency);
		}
		else
		{
			stack->way = cache_replace_block(mod->cache, stack->set);
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->status);
			if (stack->status)
				mod->evictions++;
			esim_schedule_event(EV_GPU_MEM_READ_REQUEST, stack, mod->latency);
		}

		/* Debug */
		gpu_mem_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d read_port=%d set=%d way=%d\n",
			CYCLE, ID, mod->name, stack->addr, stack->bank_index, stack->read_port_index,
			stack->set, stack->way);
		return;
	}

	if (event == EV_GPU_MEM_READ_REQUEST)
	{
		struct net_t *net;

		/* Get low network and module */
		net = mod->low_net;
		assert(net);
		stack->target_mod = mod_get_low_mod(mod, stack->addr);
		gpu_mem_debug("  %lld %lld read_request src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, mod->name, stack->target_mod->name, net->name);

		/* Send message */
		stack->msg = net_try_send_ev(net, mod->low_net_node, stack->target_mod->high_net_node,
			8, EV_GPU_MEM_READ_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_GPU_MEM_READ_REQUEST_RECEIVE)
	{
		struct mod_t *target_mod = stack->target_mod;

		assert(target_mod);
		gpu_mem_debug("  %lld %lld read_request_receive cache=\"%s\"\n",
			CYCLE, ID, target_mod->name);

		/* Receive element */
		net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);

		/* Function call to 'EV_GPU_MEM_READ' */
		newstack = mod_stack_create(stack->id,
			target_mod, stack->tag,
			EV_GPU_MEM_READ_REQUEST_REPLY, stack);
		esim_schedule_event(EV_GPU_MEM_READ, newstack, 0);
		return;
	}

	if (event == EV_GPU_MEM_READ_REQUEST_REPLY)
	{
		struct net_t *net = mod->low_net;
		struct mod_t *target_mod = stack->target_mod;

		assert(net && target_mod);
		gpu_mem_debug("  %lld %lld read_request_reply src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, target_mod->name, mod->name, net->name);

		/* Send message */
		stack->msg = net_try_send_ev(net, target_mod->high_net_node, mod->low_net_node,
			mod->block_size + 8, EV_GPU_MEM_READ_REQUEST_FINISH, stack,
			event, stack);
		return;
	}

	if (event == EV_GPU_MEM_READ_REQUEST_FINISH)
	{
		gpu_mem_debug("  %lld %lld read_request_finish\n", CYCLE, ID);
		assert(mod->cache);

		/* Receive message */
		net_receive(mod->low_net, mod->low_net_node, stack->msg);

		/* Set tag and state of the new block.
		 * A set other than 0 means that the block is valid. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag, 1);
		esim_schedule_event(EV_GPU_MEM_READ_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_GPU_MEM_READ_UNLOCK)
	{
		struct mod_port_t *port = stack->port;

		gpu_mem_debug("  %lld %lld read_unlock\n", CYCLE, ID);

		/* Update LRU counters */
		if (mod->cache)
			cache_access_block(mod->cache, stack->set, stack->way);

		/* Unlock port */
		assert(port->locked);
		assert(mod->locked_read_port_count > 0);
		port->locked = 0;
		port->stack = NULL;
		mod->locked_read_port_count--;

		/* Wake up accesses in waiting lists */
		mod_port_wakeup(port);
		mod_wakeup(mod);

		esim_schedule_event(EV_GPU_MEM_READ_FINISH, stack, 0);
		return;
	}

	if (event == EV_GPU_MEM_READ_FINISH)
	{
		gpu_mem_debug("  %lld %lld read_finish\n", CYCLE, ID);

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return */
		mod_stack_return(stack);
		return;

	}

	abort();
}


void mod_handler_write(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_t *mod = stack->mod;

	if (event == EV_GPU_MEM_WRITE)
	{
		struct mod_port_t *port;
		int i;

		/* If there is any pending access in the cache, access gets enqueued. */
		if (mod->waiting_list_head)
		{
			gpu_mem_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, mod->name, stack->addr);
			gpu_mem_debug("%lld %lld wait why=\"order\"\n",
				CYCLE, ID);
			mod_stack_wait_in_cache(stack, EV_GPU_MEM_WRITE);
			return;
		}

		/* If there is any locked read port in the cache, the write is stalled.
		 * The reason is that a write must wait for all reads to be complete, since
		 * writes could be faster than reads in the memory hierarchy. */
		if (mod->locked_read_port_count)
		{
			gpu_mem_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, mod->name, stack->addr);
			gpu_mem_debug("%lld %lld wait why=\"write_after_read\"\n",
				CYCLE, ID);
			mod_stack_wait_in_cache(stack, EV_GPU_MEM_WRITE);
			return;
		}

		/* Get bank, set, way, tag, status */
		stack->tag = stack->addr & ~(mod->block_size - 1);
		stack->block_index = stack->tag >> mod->log_block_size;
		stack->bank_index = stack->block_index % mod->bank_count;
		stack->bank = MOD_BANK_INDEX(mod, stack->bank_index);

		/* If any write port in bank is processing the same tag, there are two options:
		 *   1) If the previous access started in the same cycle, it will be coalesced with the
		 *      current access, assuming that they were issued simultaneously.
		 *   2) If the previous access started in a previous cycle, the new access will
		 *      wait until the previous access finishes, because there might be writes in
		 *      between. */
		for (i = 0; i < mod->write_port_count; i++)
		{
			/* Do what follows only if the port is locked and it is handling the same tag. */
			port = MOD_WRITE_PORT_INDEX(mod, stack->bank, i);
			if (!port->locked || port->stack->tag != stack->tag)
				continue;

			if (port->lock_when == esim_cycle)
			{
				gpu_mem_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, mod->name, stack->addr, stack->bank_index);
				stack->write_port_index = i;
				stack->port = port;
				gpu_mem_debug("  %lld %lld coalesce id=%lld bank=%d write_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index,
					stack->write_port_index);
				mod_stack_wait_in_port(stack, EV_GPU_MEM_WRITE_FINISH);

				/* Increment witness variable as soon as a port was secured */
				if (stack->witness_ptr)
					(*stack->witness_ptr)++;

				/* Stats */
				mod->writes++;
				return;
			}

			/* Current block is handled by an in-flight access, wait for it. */
			gpu_mem_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, mod->name, stack->addr);
			gpu_mem_debug("%lld %lld wait why=\"in_flight\"\n",
				CYCLE, ID);
			mod_stack_wait_in_cache(stack, EV_GPU_MEM_WRITE);
			return;
		}

		/* Look for a free write port */
		for (i = 0; i < mod->write_port_count; i++)
		{
			port = MOD_WRITE_PORT_INDEX(mod, stack->bank, i);
			if (!port->locked)
			{
				stack->write_port_index = i;
				stack->port = port;
				break;
			}
		}
		
		/* If there is no free write port, enqueue in cache waiting list. */
		if (!stack->port)
		{
			gpu_mem_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
				CYCLE, ID, mod->name, stack->addr, stack->bank_index);
			gpu_mem_debug("  %lld %lld wait why=\"no_write_port\"\n", CYCLE, ID);
			mod_stack_wait_in_cache(stack, EV_GPU_MEM_WRITE);
			return;
		}

		/* Lock write port */
		port = stack->port;
		assert(!port->locked);
		assert(mod->locked_write_port_count <
			mod->write_port_count * mod->bank_count);
		port->locked = 1;
		port->lock_when = esim_cycle;
		port->stack = stack;
		mod->locked_write_port_count++;
		gpu_mem_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d write_port=%d\n",
			CYCLE, ID, mod->name, stack->addr, stack->bank_index, stack->write_port_index);

		/* Increment witness variable as soon as a port was secured */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Stats */
		mod->writes++;
		mod->effective_writes++;
	
		/* If this is main memory, access block */
		if (!mod->cache)
		{
			stack->pending++;
			esim_schedule_event(EV_GPU_MEM_WRITE_UNLOCK, stack, mod->latency);
			mod->effective_write_hits++;
			return;
		}

		/* Access cache to write on block (write actually occurs only if block is present). */
		stack->hit = cache_find_block(mod->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		if (stack->hit)
			mod->effective_write_hits++;
		stack->pending++;
		esim_schedule_event(EV_GPU_MEM_WRITE_UNLOCK, stack, mod->latency);

		/* Access lower level cache */
		stack->pending++;
		esim_schedule_event(EV_GPU_MEM_WRITE_REQUEST_SEND, stack, 0);
		return;
	}

	if (event == EV_GPU_MEM_WRITE_REQUEST_SEND)
	{
		struct net_t *net;

		/* Get low network and module */
		net = mod->low_net;
		assert(net);
		stack->target_mod = mod_get_low_mod(mod, stack->addr);

		/* Debug */
		gpu_mem_debug("  %lld %lld write_request_send src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, mod->name, stack->target_mod->name, net->name);

		/* Send message */
		stack->msg = net_try_send_ev(net, mod->low_net_node, stack->target_mod->high_net_node, 8,
			EV_GPU_MEM_WRITE_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_GPU_MEM_WRITE_REQUEST_RECEIVE)
	{
		struct mod_t *target = stack->target_mod;
		struct mod_stack_t *newstack;

		assert(target);
		gpu_mem_debug("  %lld %lld write_request_receive cache=\"%s\"\n",
			CYCLE, ID, target->name);

		/* Receive message */
		net_receive(target->high_net, target->high_net_node, stack->msg);

		/* Function call to 'EV_GPU_MEM_WRITE' */
		newstack = mod_stack_create(stack->id, target, stack->tag,
			EV_GPU_MEM_WRITE_REQUEST_REPLY, stack);
		esim_schedule_event(EV_GPU_MEM_WRITE, newstack, 0);
		return;
	}
	
	if (event == EV_GPU_MEM_WRITE_REQUEST_REPLY)
	{
		struct mod_t *target = stack->target_mod;
		struct net_t *net = mod->low_net;

		assert(target);
		assert(net);
		gpu_mem_debug("  %lld %lld write_request_reply src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, mod->name, target->name, net->name);

		/* Send message */
		stack->msg = net_try_send_ev(net, target->high_net_node, mod->low_net_node, 8,
			EV_GPU_MEM_WRITE_REQUEST_REPLY_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_GPU_MEM_WRITE_REQUEST_REPLY_RECEIVE)
	{
		gpu_mem_debug("  %lld %lld write_request_reply_receive dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, mod->name, mod->low_net->name);

		/* Receive message */
		net_receive(mod->low_net, mod->low_net_node, stack->msg);

		/* Continue */
		esim_schedule_event(EV_GPU_MEM_WRITE_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_GPU_MEM_WRITE_UNLOCK)
	{
		struct mod_port_t *port = stack->port;

		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		/* Debug */
		gpu_mem_debug("  %lld %lld write_unlock\n", CYCLE, ID);

		/* Update LRU counters */
		if (stack->hit)
		{
			assert(mod->cache);
			cache_access_block(mod->cache, stack->set, stack->way);
		}

		/* Unlock port */
		assert(port->locked);
		assert(mod->locked_write_port_count > 0);
		port->locked = 0;
		port->stack = NULL;
		mod->locked_write_port_count--;

		/* Wake up accesses in waiting lists */
		mod_port_wakeup(port);
		mod_wakeup(mod);

		/* Finish */
		esim_schedule_event(EV_GPU_MEM_WRITE_FINISH, stack, 0);
		return;
	}

	if (event == EV_GPU_MEM_WRITE_FINISH)
	{
		/* Return */
		gpu_mem_debug("  %lld %lld write_finish\n", CYCLE, ID);
		mod_stack_return(stack);
		return;
	}

	abort();
}

