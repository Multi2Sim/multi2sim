/*
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
#include <cpuarch.h>
#include <gpuarch.h>


/*
 * Global Variables
 */

int mem_debug_category;
int mem_trace_category;

struct mem_system_t *mem_system;




/*
 * Public Functions
 */

void mem_system_init(void)
{
	/* Try to open report file */
	if (*mem_report_file_name && !can_open_write(mem_report_file_name))
		fatal("%s: cannot open GPU cache report file",
			mem_report_file_name);

	/* Create memory system */
	mem_system = calloc(1, sizeof(struct mem_system_t));
	if (!mem_system)
		fatal("%s: out of memory", __FUNCTION__);

	/* Create network and module list */
	mem_system->net_list = list_create();
	mem_system->mod_list = list_create();

	/* GPU memory event-driven simulation */
	EV_MOD_GPU_LOAD = esim_register_event(mod_handler_gpu_load);
	EV_MOD_GPU_LOAD_FINISH = esim_register_event(mod_handler_gpu_load);

	EV_MOD_GPU_STORE = esim_register_event(mod_handler_gpu_store);
	EV_MOD_GPU_STORE_FINISH = esim_register_event(mod_handler_gpu_store);

	EV_MOD_GPU_READ = esim_register_event(mod_handler_gpu_read);
	EV_MOD_GPU_READ_REQUEST = esim_register_event(mod_handler_gpu_read);
	EV_MOD_GPU_READ_REQUEST_RECEIVE = esim_register_event(mod_handler_gpu_read);
	EV_MOD_GPU_READ_REQUEST_REPLY = esim_register_event(mod_handler_gpu_read);
	EV_MOD_GPU_READ_REQUEST_FINISH = esim_register_event(mod_handler_gpu_read);
	EV_MOD_GPU_READ_UNLOCK = esim_register_event(mod_handler_gpu_read);
	EV_MOD_GPU_READ_FINISH = esim_register_event(mod_handler_gpu_read);

	EV_MOD_GPU_WRITE = esim_register_event(mod_handler_gpu_write);
	EV_MOD_GPU_WRITE_REQUEST_SEND = esim_register_event(mod_handler_gpu_write);
	EV_MOD_GPU_WRITE_REQUEST_RECEIVE = esim_register_event(mod_handler_gpu_write);
	EV_MOD_GPU_WRITE_REQUEST_REPLY = esim_register_event(mod_handler_gpu_write);
	EV_MOD_GPU_WRITE_REQUEST_REPLY_RECEIVE = esim_register_event(mod_handler_gpu_write);
	EV_MOD_GPU_WRITE_UNLOCK = esim_register_event(mod_handler_gpu_write);
	EV_MOD_GPU_WRITE_FINISH = esim_register_event(mod_handler_gpu_write);

	/* CPU memory event-driven simulation */
	EV_MOD_LOAD = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_LOCK = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_ACTION = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_MISS = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_UNLOCK = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_FINISH = esim_register_event(mod_handler_load);

	EV_MOD_STORE = esim_register_event(mod_handler_store);
	EV_MOD_STORE_LOCK = esim_register_event(mod_handler_store);
	EV_MOD_STORE_ACTION = esim_register_event(mod_handler_store);
	EV_MOD_STORE_UNLOCK = esim_register_event(mod_handler_store);
	EV_MOD_STORE_FINISH = esim_register_event(mod_handler_store);

	EV_MOD_FIND_AND_LOCK = esim_register_event(mod_handler_find_and_lock);
	EV_MOD_FIND_AND_LOCK_PORT = esim_register_event(mod_handler_find_and_lock);
	EV_MOD_FIND_AND_LOCK_ACTION = esim_register_event(mod_handler_find_and_lock);
	EV_MOD_FIND_AND_LOCK_FINISH = esim_register_event(mod_handler_find_and_lock);

	EV_MOD_EVICT = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_INVALID = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_ACTION = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_RECEIVE = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_WRITEBACK = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_WRITEBACK_EXCLUSIVE = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_WRITEBACK_FINISH = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_PROCESS = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_REPLY = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_REPLY_RECEIVE = esim_register_event(mod_handler_evict);
	EV_MOD_EVICT_FINISH = esim_register_event(mod_handler_evict);

	EV_MOD_WRITE_REQUEST = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_RECEIVE = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_ACTION = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_EXCLUSIVE = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_UPDOWN = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_UPDOWN_FINISH = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_DOWNUP = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_REPLY = esim_register_event(mod_handler_write_request);
	EV_MOD_WRITE_REQUEST_FINISH = esim_register_event(mod_handler_write_request);

	EV_MOD_READ_REQUEST = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_RECEIVE = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_ACTION = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_UPDOWN = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_UPDOWN_MISS = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_UPDOWN_FINISH = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_DOWNUP = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_DOWNUP_FINISH = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_REPLY = esim_register_event(mod_handler_read_request);
	EV_MOD_READ_REQUEST_FINISH = esim_register_event(mod_handler_read_request);

	EV_MOD_INVALIDATE = esim_register_event(mod_handler_invalidate);
	EV_MOD_INVALIDATE_FINISH = esim_register_event(mod_handler_invalidate);

	/* Read cache configuration file */
	mem_system_config_read();

	/* Initialize MMU */
	mmu_init();
}


void mem_system_done(void)
{
	int i;

	/* Dump report */
	mem_system_dump_report();

	/* Finalize MMU */
	mmu_done();

	/* Free memory modules */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
		mod_free(list_get(mem_system->mod_list, i));
	list_free(mem_system->mod_list);

	/* Free networks */
	for (i = 0; i < list_count(mem_system->net_list); i++)
		net_free(list_get(mem_system->net_list, i));
	list_free(mem_system->net_list);

	/* Free memory system */
	free(mem_system);
}


void mem_system_dump_report()
{
	struct net_t *net;
	struct mod_t *mod;
	struct cache_t *cache;
	FILE *f;

	int i;

	/* Open file */
	f = open_write(mem_report_file_name);
	if (!f)
		return;
	
	/* Intro */
	fprintf(f, "; Report for caches, TLBs, and main memory\n");
	fprintf(f, ";    Accesses - Total number of accesses\n");
	fprintf(f, ";    Hits, Misses - Accesses resulting in hits/misses\n");
	fprintf(f, ";    HitRatio - Hits divided by accesses\n");
	fprintf(f, ";    Evictions - Invalidated or replaced cache blocks\n");
	fprintf(f, ";    Retries - For L1 caches, accesses that were retried\n");
	fprintf(f, ";    ReadRetries, WriteRetries - Read/Write retried accesses\n");
	fprintf(f, ";    NoRetryAccesses - Number of accesses that were not retried\n");
	fprintf(f, ";    NoRetryHits, NoRetryMisses - Hits and misses for not retried accesses\n");
	fprintf(f, ";    NoRetryHitRatio - NoRetryHits divided by NoRetryAccesses\n");
	fprintf(f, ";    NoRetryReads, NoRetryWrites - Not retried reads and writes\n");
	fprintf(f, ";    Reads, Writes - Total read/write accesses\n");
	fprintf(f, ";    BlockingReads, BlockingWrites - Reads/writes coming from lower-level cache\n");
	fprintf(f, ";    NonBlockingReads, NonBlockingWrites - Coming from upper-level cache\n");
	fprintf(f, "\n\n");
	
	/* Report for each cache */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		cache = mod->cache;
		fprintf(f, "[ %s ]\n", mod->name);
		fprintf(f, "\n");

		/* Configuration */
		if (cache) {
			fprintf(f, "Sets = %d\n", cache->num_sets);
			fprintf(f, "Assoc = %d\n", cache->assoc);
			fprintf(f, "Policy = %s\n", map_value(&cache_policy_map, cache->policy));
		}
		fprintf(f, "BlockSize = %d\n", mod->block_size);
		fprintf(f, "Latency = %d\n", mod->latency);
		fprintf(f, "Ports = %d\n", mod->num_ports);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "Accesses = %lld\n", mod->accesses);
		fprintf(f, "Hits = %lld\n", mod->hits);
		fprintf(f, "Misses = %lld\n", mod->accesses - mod->hits);
		fprintf(f, "HitRatio = %.4g\n", mod->accesses ?
			(double) mod->hits / mod->accesses : 0.0);
		fprintf(f, "Evictions = %lld\n", mod->evictions);
		fprintf(f, "Retries = %lld\n", mod->read_retries + mod->write_retries);
		fprintf(f, "ReadRetries = %lld\n", mod->read_retries);
		fprintf(f, "WriteRetries = %lld\n", mod->write_retries);
		fprintf(f, "\n");
		fprintf(f, "NoRetryAccesses = %lld\n", mod->no_retry_accesses);
		fprintf(f, "NoRetryHits = %lld\n", mod->no_retry_hits);
		fprintf(f, "NoRetryMisses = %lld\n", mod->no_retry_accesses - mod->no_retry_hits);
		fprintf(f, "NoRetryHitRatio = %.4g\n", mod->no_retry_accesses ?
			(double) mod->no_retry_hits / mod->no_retry_accesses : 0.0);
		fprintf(f, "NoRetryReads = %lld\n", mod->no_retry_reads);
		fprintf(f, "NoRetryReadHits = %lld\n", mod->no_retry_read_hits);
		fprintf(f, "NoRetryReadMisses = %lld\n", (mod->no_retry_reads -
			mod->no_retry_read_hits));
		fprintf(f, "NoRetryWrites = %lld\n", mod->no_retry_writes);
		fprintf(f, "NoRetryWriteHits = %lld\n", mod->no_retry_write_hits);
		fprintf(f, "NoRetryWriteMisses = %lld\n", mod->no_retry_writes
			- mod->no_retry_write_hits);
		fprintf(f, "\n");
		fprintf(f, "Reads = %lld\n", mod->reads);
		fprintf(f, "BlockingReads = %lld\n", mod->blocking_reads);
		fprintf(f, "NonBlockingReads = %lld\n", mod->non_blocking_reads);
		fprintf(f, "ReadHits = %lld\n", mod->read_hits);
		fprintf(f, "ReadMisses = %lld\n", mod->reads - mod->read_hits);
		fprintf(f, "\n");
		fprintf(f, "Writes = %lld\n", mod->writes);
		fprintf(f, "BlockingWrites = %lld\n", mod->blocking_writes);
		fprintf(f, "NonBlockingWrites = %lld\n", mod->non_blocking_writes);
		fprintf(f, "WriteHits = %lld\n", mod->write_hits);
		fprintf(f, "WriteMisses = %lld\n", mod->writes - mod->write_hits);
		fprintf(f, "\n\n");
	}

	/* Dump report for networks */
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		net = list_get(mem_system->net_list, i);
		net_dump_report(net, f);
	}
	
	/* Done */
	fclose(f);
}


