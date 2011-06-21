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


struct repos_t *gpu_mem_access_repos;


void gpu_cache_init(void)
{
	struct gpu_cache_t *gpu_cache;

	struct gpu_cache_t *gpu_cache_l2;
	struct net_t *net_l1_l2;
	struct net_t *net_l2_gm;

	char buf[MAX_STRING_SIZE];
	int curr, i;
	int sw_id;

	/* Initialize cache array and caches */
	gpu->gpu_cache_count = gpu_num_compute_units + 1 + 1;  /* L1 (one per compute unit) + L2 + global mem */
	gpu->gpu_caches = calloc(gpu->gpu_cache_count, sizeof(void *));

	/* Initialize L1 caches */
	for (i = 0; i < gpu_num_compute_units; i++) {
		gpu->gpu_caches[i] = gpu_cache_create();
		gpu_cache = gpu->gpu_caches[i];
		gpu->compute_units[i]->gpu_cache = gpu_cache;
	}

	/* L2 cache */
	curr = gpu_num_compute_units;
	gpu_cache = gpu_cache_create();
	gpu->gpu_caches[curr] = gpu_cache;
	gpu_cache_l2 = gpu_cache;

	/* Global memory */
	curr = gpu_num_compute_units + 1;
	gpu_cache = gpu_cache_create();
	gpu->gpu_caches[curr] = gpu_cache;
	gpu->global_memory = gpu_cache;

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
		gpu_num_compute_units + 1, 128, gpu_num_compute_units + 1, 128,  /* FIXME */
		128, "sw", NULL);
	for (i = 0; i < gpu_num_compute_units; i++) {
		gpu_cache = gpu->gpu_caches[i];
		net_new_bidirectional_link(net_l1_l2, gpu_cache->id_lo, sw_id, 128);  /* FIXME */
	}
	net_new_bidirectional_link(net_l1_l2, sw_id, gpu_cache_l2->id_hi, 128);  /* FIXME */
	net_calculate_routes(net_l1_l2);

	/* Network 'net_l2_gm' switch and paths */
	sw_id = net_new_switch(net_l2_gm,
		2, 128, 2, 128,  /* FIXME */
		128, "sw", NULL);
	net_new_bidirectional_link(net_l2_gm, gpu_cache_l2->id_lo, sw_id, 128);  /* FIXME */
	net_new_bidirectional_link(net_l2_gm, sw_id, gpu->global_memory->id_hi, 128);  /* FIXME */
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
	free(gpu_cache->banks);
	free(gpu_cache);
}


void gpu_cache_stack_wait_in_cache(struct gpu_cache_stack_t *stack, int event)
{
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;

	assert(!DOUBLE_LINKED_LIST_MEMBER(gpu_cache, waiting, stack));
	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(gpu_cache, waiting, stack);
}


void gpu_cache_stack_wait_in_read_port(struct gpu_cache_stack_t *stack, int bank_index, int read_port_index, int event)
{
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;
	struct gpu_cache_bank_t *bank;
	struct gpu_cache_port_t *port;

	bank = GPU_CACHE_BANK_INDEX(gpu_cache, bank_index);
	port = GPU_CACHE_READ_PORT_INDEX(gpu_cache, bank, read_port_index);
	assert(!DOUBLE_LINKED_LIST_MEMBER(port, waiting, stack));

	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(port, waiting, stack);
}


void gpu_cache_stack_wait_in_write_port(struct gpu_cache_stack_t *stack, int bank_index, int write_port_index, int event)
{
	struct gpu_cache_t *gpu_cache = stack->gpu_cache;
	struct gpu_cache_bank_t *bank;
	struct gpu_cache_port_t *port;

	bank = GPU_CACHE_BANK_INDEX(gpu_cache, bank_index);
	port = GPU_CACHE_WRITE_PORT_INDEX(gpu_cache, bank, write_port_index);
	assert(!DOUBLE_LINKED_LIST_MEMBER(port, waiting, stack));

	stack->waiting_list_event = event;
	DOUBLE_LINKED_LIST_INSERT_TAIL(port, waiting, stack);
}

