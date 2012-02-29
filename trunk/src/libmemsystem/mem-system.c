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


/*
 * Global Variables
 */

int mem_debug_category;

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
	EV_MOD_FIND_AND_LOCK = esim_register_event(mod_handler_find_and_lock);
	EV_MOD_FIND_AND_LOCK_ACTION = esim_register_event(mod_handler_find_and_lock);
	EV_MOD_FIND_AND_LOCK_FINISH = esim_register_event(mod_handler_find_and_lock);

	EV_MOD_LOAD = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_ACTION = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_MISS = esim_register_event(mod_handler_load);
	EV_MOD_LOAD_FINISH = esim_register_event(mod_handler_load);

	EV_MOD_STORE = esim_register_event(mod_handler_store);
	EV_MOD_STORE_ACTION = esim_register_event(mod_handler_store);
	EV_MOD_STORE_FINISH = esim_register_event(mod_handler_store);

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
	//mem_system_config_read();  /* FIXME: not ready for this yet */

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


void mem_system_dump_report(void)
{
	FILE *f;
	int i;

	struct mod_t *mod;
	struct cache_t *cache;

	/* Open file */
	f = open_write(mem_report_file_name);
	if (!f)
		return;

	/* Intro */
	fprintf(f, "; Report for memory hierarchy.\n");
	fprintf(f, ";    Accesses - Total number of accesses requested\n");
	fprintf(f, ";    Reads - Number of read requests\n");
	fprintf(f, ";    Writes - Number of write requests\n");
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
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		/* Get cache */
		mod = list_get(mem_system->mod_list, i);
		cache = mod->cache;
		fprintf(f, "[ %s ]\n\n", mod->name);

		/* Configuration */
		if (cache)
		{
			fprintf(f, "Sets = %d\n", cache->num_sets);
			fprintf(f, "Assoc = %d\n", cache->assoc);
			fprintf(f, "Policy = %s\n", map_value(&cache_policy_map, cache->policy));
		}
		fprintf(f, "BlockSize = %d\n", mod->block_size);
		fprintf(f, "Latency = %d\n", mod->latency);
		fprintf(f, "Banks = %d\n", mod->bank_count);
		fprintf(f, "ReadPorts = %d\n", mod->read_port_count);
		fprintf(f, "WritePorts = %d\n", mod->write_port_count);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "Accesses = %lld\n", (long long) (mod->reads + mod->writes));
		fprintf(f, "Reads = %lld\n", (long long) mod->reads);
		fprintf(f, "Writes = %lld\n", (long long) mod->writes);
		fprintf(f, "CoalescedReads = %lld\n", (long long) (mod->reads
			- mod->effective_reads));
		fprintf(f, "CoalescedWrites = %lld\n", (long long) (mod->writes
			- mod->effective_writes));
		fprintf(f, "EffectiveReads = %lld\n", (long long) mod->effective_reads);
		fprintf(f, "EffectiveReadHits = %lld\n", (long long) mod->effective_read_hits);
		fprintf(f, "EffectiveReadMisses = %lld\n", (long long) (mod->effective_reads
			- mod->effective_read_hits));
		fprintf(f, "EffectiveWrites = %lld\n", (long long) mod->effective_writes);
		fprintf(f, "EffectiveWriteHits = %lld\n", (long long) mod->effective_write_hits);
		fprintf(f, "EffectiveWriteMisses = %lld\n", (long long) (mod->effective_writes
			- mod->effective_write_hits));
		fprintf(f, "Evictions = %lld\n", (long long) mod->evictions);
		fprintf(f, "\n\n");
	}


	/* Dump report for networks */
	for (i = 0; i < list_count(mem_system->net_list); i++)
		net_dump_report(list_get(mem_system->net_list, i), f);

	/* Close */
	fclose(f);
}
