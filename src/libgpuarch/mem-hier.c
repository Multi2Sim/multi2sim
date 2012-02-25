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

#include <gpuarch.h>
#include <network.h>
#include <esim.h>
#include <cachesystem.h>


/*
 * Global variables
 */

int gpu_cache_debug_category;

char *gpu_cache_report_file_name = "";




/*
 * Public Functions
 */

void gpu_cache_init(void)
{
	/* Try to open report file */
	if (gpu_cache_report_file_name[0] && !can_open_write(gpu_cache_report_file_name))
		fatal("%s: cannot open GPU cache report file",
			gpu_cache_report_file_name);

	/* Events */
	EV_GPU_CACHE_READ = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_RECEIVE = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_REPLY = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_REQUEST_FINISH = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_UNLOCK = esim_register_event(gpu_cache_handler_read);
	EV_GPU_CACHE_READ_FINISH = esim_register_event(gpu_cache_handler_read);

	EV_GPU_CACHE_WRITE = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_REQUEST_SEND = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_REQUEST_RECEIVE = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_REQUEST_REPLY = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_REQUEST_REPLY_RECEIVE = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_UNLOCK = esim_register_event(gpu_cache_handler_write);
	EV_GPU_CACHE_WRITE_FINISH = esim_register_event(gpu_cache_handler_write);

	/* Repository of GPU cache stacks */
	gpu_cache_stack_repos = repos_create(sizeof(struct gpu_cache_stack_t), "gpu_cache_stack_repos");
	
	/* Read cache configuration file */
	gpu_cache_config_read();
}


void gpu_cache_done(void)
{
	int i;
	
	/* Dump report */
	gpu_cache_dump_report();

	/* Free caches and cache array */
	for (i = 0; i < gpu->gpu_cache_count; i++)
		gpu_cache_free(gpu->gpu_caches[i]);
	free(gpu->gpu_caches);

	/* Free networks */
	for (i = 0; i < gpu->network_count; i++)
		net_free(gpu->networks[i]);
	if (gpu->networks)
		free(gpu->networks);

	/* GPU cache stack repository */
	repos_free(gpu_cache_stack_repos);
}


void gpu_cache_dump_report(void)
{
	FILE *f;
	int i;

	struct gpu_cache_t *gpu_cache;
	struct cache_t *cache;

	/* Open file */
	f = open_write(gpu_cache_report_file_name);
	if (!f)
		return;

	/* Intro */
	fprintf(f, "; Report for the GPU global memory hierarchy.\n");
	fprintf(f, ";    Accesses - Total number of accesses requested from a compute unit or upper-level cache\n");
	fprintf(f, ";    Reads - Number of read requests received from a compute unit or upper-level cache\n");
	fprintf(f, ";    Writes - Number of write requests received from a compute unit or upper-level cache\n");
	fprintf(f, ";    CoalescedReads - Number of reads that were coalesced with previous accesses (discarded)\n");
	fprintf(f, ";    CoalescedWrites - Number of writes coalesced with previous accesses\n");
	fprintf(f, ";    EffectiveReads - Number of reads actually performed (= Reads - CoalescedReads)\n");
	fprintf(f, ";    EffectiveReadHits - Number of effective reads producing cache hit\n");
	fprintf(f, ";    EffectiveReadMisses - Number of effective reads missing in the cache\n");
	fprintf(f, ";    EffectiveWrites - Number of writes actually performed (= Writes - CoalescedWrites)\n");
	fprintf(f, ";    EffectiveWriteHits - Number of effective writes that found the block in the cache\n");
	fprintf(f, ";    EffectiveWriteMisses - Number of effective writes missing in the cache\n");
	fprintf(f, ";    Evictions - Number of valid blocks replaced in the cache\n");
	fprintf(f, "\n\n");

	/* Print cache statistics */
	for (i = 0; i < gpu->gpu_cache_count; i++)
	{
		/* Get cache */
		gpu_cache = gpu->gpu_caches[i];
		cache = gpu_cache->cache;
		fprintf(f, "[ %s ]\n\n", gpu_cache->name);

		/* Configuration */
		if (cache) {
			fprintf(f, "Sets = %d\n", cache->nsets);
			fprintf(f, "Assoc = %d\n", cache->assoc);
			fprintf(f, "Policy = %s\n", map_value(&cache_policy_map, cache->policy));
		}
		fprintf(f, "BlockSize = %d\n", gpu_cache->block_size);
		fprintf(f, "Latency = %d\n", gpu_cache->latency);
		fprintf(f, "Banks = %d\n", gpu_cache->bank_count);
		fprintf(f, "ReadPorts = %d\n", gpu_cache->read_port_count);
		fprintf(f, "WritePorts = %d\n", gpu_cache->write_port_count);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "Accesses = %lld\n", (long long) (gpu_cache->reads + gpu_cache->writes));
		fprintf(f, "Reads = %lld\n", (long long) gpu_cache->reads);
		fprintf(f, "Writes = %lld\n", (long long) gpu_cache->writes);
		fprintf(f, "CoalescedReads = %lld\n", (long long) (gpu_cache->reads
			- gpu_cache->effective_reads));
		fprintf(f, "CoalescedWrites = %lld\n", (long long) (gpu_cache->writes
			- gpu_cache->effective_writes));
		fprintf(f, "EffectiveReads = %lld\n", (long long) gpu_cache->effective_reads);
		fprintf(f, "EffectiveReadHits = %lld\n", (long long) gpu_cache->effective_read_hits);
		fprintf(f, "EffectiveReadMisses = %lld\n", (long long) (gpu_cache->effective_reads
			- gpu_cache->effective_read_hits));
		fprintf(f, "EffectiveWrites = %lld\n", (long long) gpu_cache->effective_writes);
		fprintf(f, "EffectiveWriteHits = %lld\n", (long long) gpu_cache->effective_write_hits);
		fprintf(f, "EffectiveWriteMisses = %lld\n", (long long) (gpu_cache->effective_writes
			- gpu_cache->effective_write_hits));
		fprintf(f, "Evictions = %lld\n", (long long) gpu_cache->evictions);
		fprintf(f, "\n\n");
	}
	
	
	/* Dump report for networks */
	for (i = 0; i < gpu->network_count; i++)
		net_dump_report(gpu->networks[i], f);

	/* Close */
	fclose(f);
}


struct gpu_cache_t *gpu_cache_create(int bank_count, int read_port_count, int write_port_count,
	int block_size, int latency)
{
	struct gpu_cache_t *gpu_cache;
	
	gpu_cache = calloc(1, sizeof(struct gpu_cache_t));
	gpu_cache->bank_count = bank_count;
	gpu_cache->read_port_count = read_port_count;
	gpu_cache->write_port_count = write_port_count;
	gpu_cache->banks = calloc(1, gpu_cache->bank_count * SIZEOF_GPU_CACHE_BANK(gpu_cache));
	gpu_cache->latency = latency;

	/* Block size */
	gpu_cache->block_size = block_size;
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	gpu_cache->log_block_size = log_base2(block_size);

	return gpu_cache;
}


void gpu_cache_free(struct gpu_cache_t *gpu_cache)
{
	if (gpu_cache->cache)
		cache_free(gpu_cache->cache);
	free(gpu_cache->banks);
	free(gpu_cache);
}


void gpu_cache_dump(struct gpu_cache_t *gpu_cache, FILE *f)
{
	struct gpu_cache_bank_t *bank;
	struct gpu_cache_port_t *port;
	struct gpu_cache_stack_t *stack;
	int i, j;

	/* Read ports */
	fprintf(f, "gpu_cache '%s'\n", gpu_cache->name);
	for (i = 0; i < gpu_cache->bank_count; i++) {
		fprintf(f, "  bank %d:\n", i);
		bank = GPU_CACHE_BANK_INDEX(gpu_cache, i);
		for (j = 0; j < gpu_cache->read_port_count; j++) {
			port = GPU_CACHE_READ_PORT_INDEX(gpu_cache, bank, j);
			fprintf(f, "  read port %d: ", j);

			/* Waiting list */
			fprintf(f, "waiting={");
			for (stack = port->waiting_list_head; stack; stack = stack->waiting_next)
				fprintf(f, " %lld", (long long) stack->id);
			fprintf(f, " }\n");
		}
	}
}


/* Access a gpu_cache.
 * Argument 'access' defines whether it is a read (1) or a write (2).
 * Variable 'witness', if specified, will be increased when the access completes. */
void gpu_cache_access(struct gpu_cache_t *gpu_cache, int access, uint32_t addr, uint32_t size, int *witness_ptr)
{
	struct gpu_cache_stack_t *stack;
	int event;

	gpu_cache_stack_id++;
	stack = gpu_cache_stack_create(gpu_cache_stack_id,
		gpu_cache, addr, ESIM_EV_NONE, NULL);
	stack->witness_ptr = witness_ptr;
	assert(access == 1 || access == 2);
	event = access == 1 ? EV_GPU_CACHE_READ : EV_GPU_CACHE_WRITE;
	esim_schedule_event(event, stack, 0);
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
int EV_GPU_CACHE_WRITE_REQUEST_SEND;
int EV_GPU_CACHE_WRITE_REQUEST_RECEIVE;
int EV_GPU_CACHE_WRITE_REQUEST_REPLY;
int EV_GPU_CACHE_WRITE_REQUEST_REPLY_RECEIVE;
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

		/* If any read port in bank is processing the same tag, there are two options:
		 *   1) If the previous access started in the same cycle, it will be coalesced with the
		 *      current access, assuming that they were issued simultaneously.
		 *   2) If the previous access started in a previous cycle, the new access will
		 *      wait until the previous access finishes, because there might be writes in
		 *      between. */
		for (i = 0; i < gpu_cache->read_port_count; i++)
		{
			/* Do something if the port is locked and it is handling the same tag. */
			port = GPU_CACHE_READ_PORT_INDEX(gpu_cache, stack->bank, i);
			if (!port->locked || port->stack->tag != stack->tag)
				continue;

			/* If current and previous access are in the same cycle, coalesce. */
			if (port->lock_when == esim_cycle)
			{
				gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
				stack->read_port_index = i;
				stack->port = port;
				gpu_cache_debug("  %lld %lld coalesce id=%lld bank=%d read_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index, stack->read_port_index);
				gpu_cache_stack_wait_in_port(stack, EV_GPU_CACHE_READ_FINISH);

				/* Stats */
				gpu_cache->reads++;
				return;
			}

			/* Current block is handled by an in-flight access, wait for it. */
			gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"in_flight\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_READ);
			return;
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
	
		/* Stats */
		gpu_cache->reads++;
		gpu_cache->effective_reads++;

		/* If there is no cache, assume hit */
		if (!gpu_cache->cache)
		{
			esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, gpu_cache->latency);

			/* Stats */
			gpu_cache->effective_read_hits++;
			gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d read_port=%d\n",
				CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index, stack->read_port_index);
			return;
		}

		/* Get block from cache, consuming 'latency' cycles. */
		stack->hit = cache_find_block(gpu_cache->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		if (stack->hit)
		{
			gpu_cache->effective_read_hits++;
			esim_schedule_event(EV_GPU_CACHE_READ_UNLOCK, stack, gpu_cache->latency);
		}
		else
		{
			stack->way = cache_replace_block(gpu_cache->cache, stack->set);
			cache_get_block(gpu_cache->cache, stack->set, stack->way, NULL, &stack->status);
			if (stack->status)
				gpu_cache->evictions++;
			esim_schedule_event(EV_GPU_CACHE_READ_REQUEST, stack, gpu_cache->latency);
		}

		/* Debug */
		gpu_cache_debug("%lld %lld read cache=\"%s\" addr=%u bank=%d read_port=%d set=%d way=%d\n",
			CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index, stack->read_port_index,
			stack->set, stack->way);
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

		/* Send message */
		stack->msg = net_try_send_ev(net, gpu_cache->net_node_lo, target->net_node_hi,
			8, EV_GPU_CACHE_READ_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_RECEIVE)
	{
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;

		gpu_cache_debug("  %lld %lld read_request_receive cache=\"%s\"\n",
			CYCLE, ID, target->name);

		/* Receive element */
		net_receive(target->net_hi, target->net_node_hi, stack->msg);

		/* Function call to 'EV_GPU_CACHE_READ' */
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

		/* Send message */
		stack->msg = net_try_send_ev(net, target->net_node_hi, gpu_cache->net_node_lo,
			gpu_cache->block_size + 8, EV_GPU_CACHE_READ_REQUEST_FINISH, stack,
			event, stack);
		return;
	}

	if (event == EV_GPU_CACHE_READ_REQUEST_FINISH)
	{
		gpu_cache_debug("  %lld %lld read_request_finish\n", CYCLE, ID);
		assert(gpu_cache->cache);

		/* Receive message */
		net_receive(gpu_cache->net_lo, gpu_cache->net_node_lo, stack->msg);

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

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

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
		struct gpu_cache_port_t *port;
		int i;

		/* If there is any pending access in the cache, access gets enqueued. */
		if (gpu_cache->waiting_list_head)
		{
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
		if (gpu_cache->locked_read_port_count)
		{
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

		/* If any write port in bank is processing the same tag, there are two options:
		 *   1) If the previous access started in the same cycle, it will be coalesced with the
		 *      current access, assuming that they were issued simultaneously.
		 *   2) If the previous access started in a previous cycle, the new access will
		 *      wait until the previous access finishes, because there might be writes in
		 *      between. */
		for (i = 0; i < gpu_cache->write_port_count; i++)
		{
			/* Do what follows only if the port is locked and it is handling the same tag. */
			port = GPU_CACHE_WRITE_PORT_INDEX(gpu_cache, stack->bank, i);
			if (!port->locked || port->stack->tag != stack->tag)
				continue;

			if (port->lock_when == esim_cycle)
			{
				gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
					CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
				stack->write_port_index = i;
				stack->port = port;
				gpu_cache_debug("  %lld %lld coalesce id=%lld bank=%d write_port=%d\n",
					CYCLE, ID, (long long) port->stack->id, stack->bank_index,
					stack->write_port_index);
				gpu_cache_stack_wait_in_port(stack, EV_GPU_CACHE_WRITE_FINISH);

				/* Increment witness variable as soon as a port was secured */
				if (stack->witness_ptr)
					(*stack->witness_ptr)++;

				/* Stats */
				gpu_cache->writes++;
				return;
			}

			/* Current block is handled by an in-flight access, wait for it. */
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u\n",
				CYCLE, ID, gpu_cache->name, stack->addr);
			gpu_cache_debug("%lld %lld wait why=\"in_flight\"\n",
				CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* Look for a free write port */
		for (i = 0; i < gpu_cache->write_port_count; i++)
		{
			port = GPU_CACHE_WRITE_PORT_INDEX(gpu_cache, stack->bank, i);
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
			gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d\n",
				CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index);
			gpu_cache_debug("  %lld %lld wait why=\"no_write_port\"\n", CYCLE, ID);
			gpu_cache_stack_wait_in_cache(stack, EV_GPU_CACHE_WRITE);
			return;
		}

		/* Lock write port */
		port = stack->port;
		assert(!port->locked);
		assert(gpu_cache->locked_write_port_count <
			gpu_cache->write_port_count * gpu_cache->bank_count);
		port->locked = 1;
		port->lock_when = esim_cycle;
		port->stack = stack;
		gpu_cache->locked_write_port_count++;
		gpu_cache_debug("%lld %lld write cache=\"%s\" addr=%u bank=%d write_port=%d\n",
			CYCLE, ID, gpu_cache->name, stack->addr, stack->bank_index, stack->write_port_index);

		/* Increment witness variable as soon as a port was secured */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Stats */
		gpu_cache->writes++;
		gpu_cache->effective_writes++;
	
		/* If this is main memory, access block */
		if (!gpu_cache->cache)
		{
			stack->pending++;
			esim_schedule_event(EV_GPU_CACHE_WRITE_UNLOCK, stack, gpu_cache->latency);
			gpu_cache->effective_write_hits++;
			return;
		}

		/* Access cache to write on block (write actually occurs only if block is present). */
		stack->hit = cache_find_block(gpu_cache->cache, stack->tag,
			&stack->set, &stack->way, &stack->status);
		if (stack->hit)
			gpu_cache->effective_write_hits++;
		stack->pending++;
		esim_schedule_event(EV_GPU_CACHE_WRITE_UNLOCK, stack, gpu_cache->latency);

		/* Access lower level cache */
		stack->pending++;
		esim_schedule_event(EV_GPU_CACHE_WRITE_REQUEST_SEND, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_REQUEST_SEND)
	{
		struct net_t *net;
		struct gpu_cache_t *target;

		net = gpu_cache->net_lo;
		target = gpu_cache->gpu_cache_next;
		assert(target);
		assert(net);

		/* Debug */
		gpu_cache_debug("  %lld %lld write_request_send src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, gpu_cache->name, target->name, net->name);

		/* Send message */
		stack->msg = net_try_send_ev(net, gpu_cache->net_node_lo, target->net_node_hi, 8,
			EV_GPU_CACHE_WRITE_REQUEST_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_REQUEST_RECEIVE)
	{
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;
		struct gpu_cache_stack_t *newstack;

		gpu_cache_debug("  %lld %lld write_request_receive cache=\"%s\"\n",
			CYCLE, ID, target->name);

		/* Receive message */
		net_receive(target->net_hi, target->net_node_hi, stack->msg);

		/* Function call to 'EV_GPU_CACHE_WRITE' */
		newstack = gpu_cache_stack_create(stack->id, target, stack->tag,
			EV_GPU_CACHE_WRITE_REQUEST_REPLY, stack);
		esim_schedule_event(EV_GPU_CACHE_WRITE, newstack, 0);
		return;
	}
	
	if (event == EV_GPU_CACHE_WRITE_REQUEST_REPLY)
	{
		struct gpu_cache_t *target = gpu_cache->gpu_cache_next;
		struct net_t *net = gpu_cache->net_lo;

		assert(target);
		assert(net);
		gpu_cache_debug("  %lld %lld write_request_reply src=\"%s\" dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, gpu_cache->name, target->name, net->name);

		/* Send message */
		stack->msg = net_try_send_ev(net, target->net_node_hi, gpu_cache->net_node_lo, 8,
			EV_GPU_CACHE_WRITE_REQUEST_REPLY_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_REQUEST_REPLY_RECEIVE)
	{
		gpu_cache_debug("  %lld %lld write_request_reply_receive dest=\"%s\" net=\"%s\"\n",
			CYCLE, ID, gpu_cache->name, gpu_cache->net_lo->name);

		/* Receive message */
		net_receive(gpu_cache->net_lo, gpu_cache->net_node_lo, stack->msg);

		/* Continue */
		esim_schedule_event(EV_GPU_CACHE_WRITE_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_UNLOCK)
	{
		struct gpu_cache_port_t *port = stack->port;

		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		/* Debug */
		gpu_cache_debug("  %lld %lld write_unlock\n", CYCLE, ID);

		/* Update LRU counters */
		if (stack->hit)
		{
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

		/* Finish */
		esim_schedule_event(EV_GPU_CACHE_WRITE_FINISH, stack, 0);
		return;
	}

	if (event == EV_GPU_CACHE_WRITE_FINISH)
	{
		/* Return */
		gpu_cache_debug("  %lld %lld write_finish\n", CYCLE, ID);
		gpu_cache_stack_return(stack);
		return;
	}

	abort();
}

