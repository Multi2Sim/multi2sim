/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpuarch.h>
#include <repos.h>
#include <network.h>
#include <esim.h>
#include <cachesystem.h>


/*
 * Global variables
 */

struct repos_t *gpu_mem_access_repos;
int gpu_cache_debug_category;



/*
 * Public functions
 */

void gpu_cache_init(void)
{
	struct gpu_cache_t *gpu_cache;

	struct gpu_cache_t *gpu_cache_l2;
	struct net_t *net_l1_l2;
	struct net_t *net_l2_gm;

	char buf[MAX_STRING_SIZE];
	int curr, i;
	int sw_id;

	/* Debug */
	gpu_cache_debug_category = debug_new_category();

	/* Initialize cache array and caches */
	gpu->gpu_cache_count = gpu_num_compute_units + 1 + 1;  /* L1 (one per compute unit) + L2 + global mem */
	gpu->gpu_caches = calloc(gpu->gpu_cache_count, sizeof(void *));

	/* Initialize L1 caches */
	for (i = 0; i < gpu_num_compute_units; i++) {
		gpu->gpu_caches[i] = gpu_cache_create();
		gpu_cache = gpu->gpu_caches[i];
		gpu->compute_units[i]->gpu_cache = gpu_cache;
		snprintf(gpu_cache->name, sizeof(gpu_cache->name), "L1[%d]", i);
		gpu_cache->block_size = 256;  /* FIXME */
		gpu_cache->log_block_size = log_base2(gpu_cache->block_size);
		gpu_cache->latency = 1;  /* FIXME */
		gpu_cache->cache = cache_create(16, gpu_cache->block_size, 2, cache_policy_lru);  /* FIXME */
	}

	/* L2 cache */
	curr = gpu_num_compute_units;
	gpu_cache = gpu_cache_create();
	gpu->gpu_caches[curr] = gpu_cache;
	gpu_cache_l2 = gpu_cache;
	snprintf(gpu_cache->name, sizeof(gpu_cache->name), "L2");
	gpu_cache->block_size = 256;  /* FIXME */
	gpu_cache->log_block_size = log_base2(gpu_cache->block_size);
	gpu_cache->latency = 10;  /* FIXME */
	gpu_cache->cache = cache_create(64, gpu_cache->block_size, 4, cache_policy_lru);  /* FIXME */

	/* Global memory */
	curr = gpu_num_compute_units + 1;
	gpu_cache = gpu_cache_create();
	gpu->gpu_caches[curr] = gpu_cache;
	gpu->global_memory = gpu_cache;
	snprintf(gpu_cache->name, sizeof(gpu_cache->name), "GlobalMem");
	gpu_cache->block_size = 256;  /* FIXME */
	gpu_cache->log_block_size = log_base2(gpu_cache->block_size);
	gpu_cache->latency = 100;  /* FIXME */

	/* Assign 'gpu_cache_next' */
	for (i = 0; i < gpu_num_compute_units; i++)
		gpu->gpu_caches[i]->gpu_cache_next = gpu->gpu_caches[gpu_num_compute_units];
	gpu->gpu_caches[gpu_num_compute_units]->gpu_cache_next = gpu->global_memory;
	gpu->global_memory->gpu_cache_next = NULL;

	/* Networks */
	gpu->network_count = 2;  /* L1-L2 and L2-global_memory */
	gpu->networks = calloc(gpu->network_count, sizeof(void *));
	net_l1_l2 = net_create("net_11_l2");
	gpu->networks[0] = net_l1_l2;
	net_l2_gm = net_create("net_l2_gm");
	gpu->networks[1] = net_l2_gm;

	/* Interconnect nodes */
	for (i = 0; i < gpu_num_compute_units; i++) {
		gpu_cache = gpu->gpu_caches[i];
		sprintf(buf, "L1[%d]", i);
		gpu_cache->id_lo = net_new_node(net_l1_l2, buf, gpu_cache);
	}
	gpu_cache_l2->id_hi = net_new_node(net_l1_l2, "L2", gpu_cache_l2);
	gpu_cache_l2->id_lo = net_new_node(net_l2_gm, "L2", gpu_cache_l2);
	gpu->global_memory->id_hi = net_new_node(net_l2_gm, "GlobalMemory", gpu->global_memory);

	/* Network 'net_l1_l2' switch and paths */
	sw_id = net_new_switch(net_l1_l2,
		gpu_num_compute_units + 1, 512, gpu_num_compute_units + 1, 512,  /* FIXME */
		512, "sw", NULL);
	for (i = 0; i < gpu_num_compute_units; i++) {
		gpu_cache = gpu->gpu_caches[i];
		net_new_bidirectional_link(net_l1_l2, gpu_cache->id_lo, sw_id, 128);  /* FIXME */
	}
	net_new_bidirectional_link(net_l1_l2, sw_id, gpu_cache_l2->id_hi, 128);  /* FIXME */
	net_calculate_routes(net_l1_l2);

	/* Network 'net_l2_gm' switch and paths */
	sw_id = net_new_switch(net_l2_gm,
		2, 512, 2, 512,  /* FIXME */
		64, "sw", NULL);
	net_new_bidirectional_link(net_l2_gm, gpu_cache_l2->id_lo, sw_id, 64);  /* FIXME */
	net_new_bidirectional_link(net_l2_gm, sw_id, gpu->global_memory->id_hi, 64);  /* FIXME */
	net_calculate_routes(net_l2_gm);

	/* Assign 'net_hi', 'net_lo' */
	for (i = 0; i < gpu_num_compute_units; i++) {
		gpu->gpu_caches[i]->net_hi = NULL;
		gpu->gpu_caches[i]->net_lo = gpu->networks[0];
	}
	gpu_cache_l2->net_hi = gpu->networks[0];
	gpu_cache_l2->net_lo = gpu->networks[1];
	gpu->global_memory->net_hi = gpu->networks[1];
	gpu->global_memory->net_lo = NULL;

	/* GPU memory access repository */
	gpu_mem_access_repos = repos_create(sizeof(struct gpu_mem_access_t), "gpu_mem_access_repos");

	/* Events */
	EV_GPU_CACHE_READ = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_RECEIVE = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_REPLY = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_FINISH = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_UNLOCK = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_FINISH = esim_register_event(gpu_cache_handler_read);

	EV_GPU_CACHE_WRITE = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_REQUEST_RECEIVE = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_UNLOCK = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_FINISH = esim_register_event(gpu_cache_handler_write);

	/* Repository of GPU cache stacks */
	gpu_cache_stack_repos = repos_create(sizeof(struct gpu_cache_stack_t), "gpu_cache_stack_repos");
}


void gpu_cache_done(void)
{
	int i;
	
	/* Free caches and cache array */
	for (i = 0; i < gpu->gpu_cache_count; i++)
		gpu_cache_free(gpu->gpu_caches[i]);
	free(gpu->gpu_caches);

	/* Free networks */
	for (i = 0; i < gpu->network_count; i++)
		net_free(gpu->networks[i]);
	free(gpu->networks);

	/* GPU memory access repository */
	repos_free(gpu_mem_access_repos);

	/* GPU cache stack repository */
	repos_free(gpu_cache_stack_repos);
}


struct gpu_cache_t *gpu_cache_create(void)
{
	struct gpu_cache_t *gpu_cache;
	
	gpu_cache = calloc(1, sizeof(struct gpu_cache_t));
	gpu_cache->bank_count = 4;  /* FIXME */
	gpu_cache->read_port_count = 4;  /* FIXME */
	gpu_cache->write_port_count = 2;  /* FIXME */
	gpu_cache->banks = calloc(1, gpu_cache->bank_count + SIZEOF_GPU_CACHE_BANK(gpu_cache));
	return gpu_cache;
}


void gpu_cache_free(struct gpu_cache_t *gpu_cache)
{
	if (gpu_cache->cache)
		cache_free(gpu_cache->cache);
	free(gpu_cache->banks);
	free(gpu_cache);
}


void gpu_cache_read(int compute_unit_id, uint32_t addr, uint32_t size)
{
	struct gpu_cache_stack_t *stack;

	gpu_cache_stack_id++;
	assert(IN_RANGE(compute_unit_id, 0, gpu_num_compute_units));
	stack = gpu_cache_stack_create(gpu_cache_stack_id,
		gpu->gpu_caches[compute_unit_id], addr,
		ESIM_EV_NONE, NULL);
	esim_schedule_event(EV_GPU_CACHE_READ, stack, 0);
}


void gpu_cache_write(int compute_unit_id, uint32_t addr, uint32_t size)
{
	struct gpu_cache_stack_t *stack;

	gpu_cache_stack_id++;
	assert(IN_RANGE(compute_unit_id, 0, gpu_num_compute_units));
	stack = gpu_cache_stack_create(gpu_cache_stack_id,
		gpu->gpu_caches[compute_unit_id], addr,
		ESIM_EV_NONE, NULL);
	esim_schedule_event(EV_GPU_CACHE_WRITE, stack, 0);
}




/*
 * Event-driven simulation
 */


/* Events */

int EV_GPU_CACHE_READ;
int EV_GPU_CACHE_READ_REQUEST;
int EV_GPU_CACHE_READ_REQUEST_RECEIVE;
int EV_GPU_CACHE_READ_REQUEST_REPLY;
int EV_GPU_CACHE_READ_REQUEST_FINISH;
int EV_GPU_CACHE_READ_UNLOCK;
int EV_GPU_CACHE_READ_FINISH;

int EV_GPU_CACHE_WRITE;
int EV_GPU_CACHE_WRITE_REQUEST_RECEIVE;
int EV_GPU_CACHE_WRITE_UNLOCK;
int EV_GPU_CACHE_WRITE_FINISH;

struct repos_t *gpu_cache_stack_repos;
uint64_t gpu_cache_stack_id;


struct gpu_cache_stack_t *gpu_cache_stack_create(uint64_t id, struct gpu_cache_t *gpu_cache,
	uint32_t addr, int ret_event, void *ret_stack)
{
	struct gpu_cache_stack_t *stack;
	stack = repos_create_object(gpu_cache_stack_repos);
	stack->id = id;
	stack->gpu_cache = gpu_cache;
	stack->addr = addr;
	stack->ret_event = ret_event;
	stack->ret_stack = ret_stack;
	return stack;
}


void gpu_cache_stack_return(struct gpu_cache_stack_t *stack)
{
	int ret_event = stack->ret_event;
	void *ret_stack = stack->ret_stack;

	repos_free_object(gpu_cache_stack_repos, stack);
	esim_schedule_event(ret_event, ret_stack, 0);
}


void gpu_cache_stack_wait_in_cache(struct gpu_cache_stack_t *stack, int event)
{
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;

	assert(!DOUBLE_LINKED_LIST_MEMBER(gpu_cache, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(gpu_cache, waiting, stack);
}


/* Enqueue stack in waiting list of 'stack->port' */
void gpu_cache_stack_wait_in_port(struct gpu_cache_stack_t *stack, int event)
{
	struct gpu_cache_port_t *port = stack->port;

	assert(!DOUBLE_LINKED_LIST_MEMBER(port, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(port, waiting, stack);
}


/* Wake up accesses in 'gpu_cache->waiting_list' */
void gpu_cache_wakeup(struct gpu_cache_t *gpu_cache)
{
	struct gpu_cache_stack_t *stack;
	int event;

	while (gpu_cache->waiting_list_head) {
		stack = gpu_cache->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(gpu_cache, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


/* Wake up accesses in 'port->waiting_list' */
void gpu_cache_port_wakeup(struct gpu_cache_port_t *port)
{
	struct gpu_cache_stack_t *stack;
	int event;

	while (port->waiting_list_head) {
		stack = port->waiting_list_head;
		event = stack->waiting_list_event;
		DOUBLE_LINKED_LIST_REMOVE(port, waiting, stack);
		esim_schedule_event(event, stack, 0);
	}
}


#define CYCLE ((long long) esim_cycle)
#define ID ((long long) stack->id)


void gpu_cache_handler_read(int event, void *data)
{
	struct gpu_cache_stack_t *stack = data, *newstack;
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;

	if (event == EV_GPU_CACHE_READ)
	{
		struct gpu_cache_port_t *port;
		int i;

		/* If there is any pending access in the cache, this access should
		 * be enqueued in the waiting list, since all accesses need to be
		 * done in order. */
		if (gpu_cache->waiting_list_head) {
			gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"order\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_READ);
			return;
		}

		/* Get bank, set, way, tag, status */
		stack->tag = stack->addr & ~(gpu_cache->block_size - 1);
		stack->block_index = stack->tag >> gpu_cache->log_block_size;
		stack->bank_index = stack->block_index % gpu_cache->bank_count;
		stack->bank = GPU_CACHE_BANK_INDEX(gpu_cache, stack->bank_index);

		/* If any read port in bank is processing the same tag starting
		 * in the current cycle, the accesses are coalesced. */
		for (i = 0; i < gpu_cache->read_port_count; i++) {
			port = GPU_CACHE_READ_PORT_INDEX(gpu_cache, stack->bank, i);
			if (port->locked && port->lock_when == esim_cycle && port->stack->tag == stack->tag) {
				gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
				stack->read_port_index = i;
				stack->port = port;
				gpu_cache_debug("  %lld %lld coalesce id=%lld bank=%d read_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index, stack->read_port_index);
				gpu_cache_stack_wait_in_port(stack, EV_GPU_CACHE_READ_FINISH);
				return;
			}
		}

		/* Look for a free read port */
		for (i = 0; i < gpu_cache->read_port_count; i++) {
			port = GPU_CACHE_READ_PORT_INDEX(gpu_cache, stack->bank, i);
			if (!port->locked) {
				stack->read_port_index = i;
				stack->port = port;
				break;
			}
		}
		
		/* If there is no free read port, enqueue in cache waiting list. */
		if (!stack->port) {
			gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d\n",
				CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
			gpu_cache_debug("  %lld %lld wait why=\"no_read_port\"\n", CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_READ);
			return;
		}

		/* Lock read port */
		port = stack->port;
		assert(!port->locked);
		assert(gpu_cache->locked_read_port_count < gpu_cache->read_port_count * gpu_cache->bank_count);
		port->locked = 1;
		port->lock_when = esim_cycle;
		port->stack = stack;
		gpu_cache->locked_read_port_count++;
		gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d read_port=%d\n",
			CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index, stack->read_port_index);
	
		/* If there is no cache, assume hit */
		if (!gpu_cache->cache) {
			esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, gpu_cache->latency);
			return;
		}

		/* Get block from cache, consuming 'latency' cycles. */
		stack->hit = cache_find_block(gpu_cache->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		if (stack->hit)
			esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, gpu_cache->latency);
		else {
			stack->way = cache_replace_block(gpu_cache->cache, stack->set);
			esim_schedule_event(EV_GPU_CACHE_READ_REQUEST, stack, gpu_cache->latency);
		}

		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST)
	{
		struct net_t *net = gpu_cache->net_lo;
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;

		assert(net);
		assert(target);
		gpu_cache_debug("  %lld %lld read_request src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, gpu_cache->name, target->name, net->name);
		net_send_ev(net, gpu_cache->id_lo, target->id_hi, 8, EV_GPU_CACHE_READ_REQUEST_RECEIVE, stack);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_RECEIVE)
	{
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;

		gpu_cache_debug("  %lld %lld read_request_receive cache=\"%s\"\n",
			CYCLE, ID, target->name);

		newstack = gpu_cache_stack_create(stack->id,
			gpu_cache->gpu_cache_next, stack->tag,
			EV_GPU_CACHE_READ_REQUEST_REPLY, stack);
		esim_schedule_event(EV_GPU_CACHE_READ, newstack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_REPLY)
	{
		struct net_t *net = gpu_cache->net_lo;
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;

		assert(net && target);
		gpu_cache_debug("  %lld %lld read_request_reply src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, target->name, gpu_cache->name, net->name);
		net_send_ev(net, target->id_hi, gpu_cache->id_lo, gpu_cache->block_size + 8,
			EV_GPU_CACHE_READ_REQUEST_FINISH, stack);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_FINISH)
	{
		gpu_cache_debug("  %lld %lld read_request_finish\n", CYCLE, ID);
		assert(gpu_cache->cache);

		/* Set tag and state of the new block.
		 * A set other than 0 means that the block is valid. */
		cache_set_block(gpu_cache->cache, stack->set, stack->way, stack->tag, 1);
		esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_READ_UNLOCK)
	{
		struct gpu_cache_port_t *port = stack->port;

		gpu_cache_debug("  %lld %lld read_unlock\n", CYCLE, ID);

		/* Update LRU counters */
		if (gpu_cache->cache)
			cache_access_block(gpu_cache->cache, stack->set, stack->way);

		/* Unlock port */
		assert(port->locked);
		assert(gpu_cache->locked_read_port_count > 0);
		port->locked = 0;
		port->stack = NULL;
		gpu_cache->locked_read_port_count--;

		/* Wake up accesses in waiting lists */
		gpu_cache_port_wakeup(port);
		gpu_cache_wakeup(gpu_cache);

		esim_schedule_event(EV_GPU_CACHE_READ_FINISH, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_READ_FINISH)
	{
		gpu_cache_debug("  %lld %lld read_finish\n", CYCLE, ID);

		/* Return */
		gpu_cache_stack_return(stack);
		return;

	}

	abort();
}


void gpu_cache_handler_write(int event, void *data)
{
	struct gpu_cache_stack_t *stack = data;
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;

	if (event == EV_GPU_CACHE_WRITE)
	{
		struct gpu_cache_t *target;
		struct gpu_cache_port_t *port;
		struct net_t *net;
		int i;

		/* If there is any pending access in the cache, access gets enqueued. */
		if (gpu_cache->waiting_list_head) {
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"order\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* If there is any locked read port in the cache, the write is stalled.
		 * The reason is that a write must wait for all reads to be complete, since
		 * writes could be faster than reads in the memory hierarchy. */
		if (gpu_cache->locked_read_port_count) {
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"write_after_read\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* Get bank, set, way, tag, status */
		stack->tag = stack->addr & ~(gpu_cache->block_size - 1);
		stack->block_index = stack->tag >> gpu_cache->log_block_size;
		stack->bank_index = stack->block_index % gpu_cache->bank_count;
		stack->bank = GPU_CACHE_BANK_INDEX(gpu_cache, stack->bank_index);

		/* If any write port in bank is processing the same tag starting
		 * in the current cycle, the accesses are coalesced. */
		for (i = 0; i < gpu_cache->write_port_count; i++) {
			port = GPU_CACHE_WRITE_PORT_INDEX(gpu_cache, stack->bank, i);
			if (port->locked && port->lock_when == esim_cycle && port->stack->tag == stack->tag) {
				gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
				stack->write_port_index = i;
				stack->port = port;
				stack->pending++;
				gpu_cache_debug("  %lld %lld coalesce id=%lld bank=%d write_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index, stack->write_port_index);
				gpu_cache_stack_wait_in_port(stack, EV_GPU_CACHE_WRITE_FINISH);
				return;
			}
		}

		/* Look for a free write port */
		for (i = 0; i < gpu_cache->write_port_count; i++) {
			port = GPU_CACHE_WRITE_PORT_INDEX(gpu_cache, stack->bank, i);
			if (!port->locked) {
				stack->write_port_index = i;
				stack->port = port;
				break;
			}
		}
		
		/* If there is no free write port, enqueue in cache waiting list. */
		if (!stack->port) {
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
				CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
			gpu_cache_debug("  %lld %lld wait why=\"no_write_port\"\n", CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* Lock write port */
		port = stack->port;
		assert(!port->locked);
		assert(gpu_cache->locked_write_port_count < gpu_cache->write_port_count * gpu_cache->bank_count);
		port->locked = 1;
		port->lock_when = esim_cycle;
		port->stack = stack;
		gpu_cache->locked_write_port_count++;
		gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d write_port=%d\n",
			CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index, stack->write_port_index);
	
		/* If this is main memory, access block */
		if (!gpu_cache->cache) {
			stack->pending++;
			esim_schedule_event(EV_GPU_CACHE_WRITE_UNLOCK, stack, gpu_cache->latency);
			return;
		}

		/* Access cache to write on block (write actually occurs only if block is present). */
		stack->hit = cache_find_block(gpu_cache->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		stack->pending++;
		esim_schedule_event(EV_GPU_CACHE_WRITE_UNLOCK, stack, gpu_cache->latency);

		/* Access lower level cache */
		net = gpu_cache->net_lo;
		target = gpu_cache->gpu_cache_next;
		assert(target);
		assert(net);
		stack->pending++;
		gpu_cache_debug("  %lld %lld write_request src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, target->name, gpu_cache->name, net->name);
		net_send_ev(net, gpu_cache->id_lo, target->id_hi, 8, EV_GPU_CACHE_WRITE_REQUEST_RECEIVE, stack);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_REQUEST_RECEIVE)
	{
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;
		struct gpu_cache_stack_t *newstack;

		gpu_cache_debug("  %lld %lld write_request_receive cache=\"%s\"\n",
			CYCLE, ID, target->name);
		newstack = gpu_cache_stack_create(stack->id, target, stack->tag,
			EV_GPU_CACHE_WRITE_FINISH, stack);
		esim_schedule_event(EV_GPU_CACHE_WRITE, newstack, 0);
		return;
	}
	
	if (event == EV_GPU_CACHE_WRITE_UNLOCK)
	{
		struct gpu_cache_port_t *port = stack->port;

		gpu_cache_debug("  %lld %lld write_unlock\n", CYCLE, ID);

		/* Update LRU counters */
		if (stack->hit) {
			assert(gpu_cache->cache);
			cache_access_block(gpu_cache->cache, stack->set, stack->way);
		}

		/* Unlock port */
		assert(port->locked);
		assert(gpu_cache->locked_write_port_count > 0);
		port->locked = 0;
		port->stack = NULL;
		gpu_cache->locked_write_port_count--;

		/* Wake up accesses in waiting lists */
		gpu_cache_port_wakeup(port);
		gpu_cache_wakeup(gpu_cache);

		esim_schedule_event(EV_GPU_CACHE_WRITE_FINISH, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_FINISH)
	{
		
		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		/* Return */
		gpu_cache_debug("  %lld %lld write_finish\n", CYCLE, ID);
		gpu_cache_stack_return(stack);
		return;
	}

	abort();
}

