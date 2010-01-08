/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2s.h>


/* Main Processor Global Variable */
struct processor_t *p;


/* processor parameters */
int p_stage_time_stats = 0;
uint32_t p_cores = 1;
uint32_t p_threads = 1;
uint32_t p_quantum = 1000;
uint32_t p_switch_penalty = 0;
int p_occupancy_stats = 0;

enum p_recover_kind_enum p_recover_kind = p_recover_kind_writeback;
uint32_t p_recover_penalty = 0;

enum p_fetch_kind_enum p_fetch_kind = p_fetch_kind_timeslice;

uint32_t p_decode_width = 4;

enum p_dispatch_kind_enum p_dispatch_kind = p_dispatch_kind_timeslice;
uint32_t p_dispatch_width = 4;

enum p_issue_kind_enum p_issue_kind = p_issue_kind_timeslice;
uint32_t p_issue_width = 4;

enum p_commit_kind_enum p_commit_kind = p_commit_kind_shared;
uint32_t p_commit_width = 4;





/* Options & stats */
void p_reg_options()
{
	static char *p_recover_kind_map[] = { "writeback", "commit" };
	static char *p_fetch_kind_map[] = { "shared", "timeslice", "switchonevent" };
	static char *p_dispatch_kind_map[] = { "shared", "timeslice" };
	static char *p_issue_kind_map[] = { "shared", "timeslice" };
	static char *p_commit_kind_map[] = { "shared", "timeslice" };
	
	opt_reg_uint32("-cores", "number of processor cores", &p_cores);
	opt_reg_uint32("-threads", "number of threads per core", &p_threads);

	opt_reg_bool("-stage_time_stats", "measure time for stages",
		&p_stage_time_stats);
	opt_reg_bool("-occupancy_stats", "measure iq, lq, sq, rob, rf occupancy stats",
		&p_occupancy_stats);
	
	opt_reg_enum("-recover_kind", "when to recover {writeback|commit}",
		(int *) &p_recover_kind, p_recover_kind_map, 2);
	opt_reg_uint32("-recover_penalty", "cycles to stall fetch after recover",
		&p_recover_penalty);
	
	opt_reg_uint32("-quantum", "time quantum in cycles for switch-on-event fetch",
		&p_quantum);
	opt_reg_uint32("-switch_penalty", "in switchonevent, thread switch penalty",
		&p_switch_penalty);
	opt_reg_enum("-fetch_kind", "fetch policy {shared|timeslice|switchonevent}",
		(int *) &p_fetch_kind, p_fetch_kind_map, 3);
	
	opt_reg_enum("-dispatch_kind", "dispatch stage sharing {shared|timeslice}",
		(int *) &p_dispatch_kind, p_dispatch_kind_map, 2);
	opt_reg_uint32("-dispatch_width", "dispatch width (for shared/timeslice dispatch)",
		&p_dispatch_width);
	
	opt_reg_enum("-issue_kind", "issue stage sharing {shared|timeslice}",
		(int *) &p_issue_kind, p_issue_kind_map, 2);
	opt_reg_uint32("-issue_width", "issue width (for shared/timeslice issue)", &p_issue_width);
	
	opt_reg_enum("-commit_kind", "commit stage sharing {shared|timeslice}",
		(int *) &p_commit_kind, p_commit_kind_map, 2);
	opt_reg_uint32("-commit_width", "commit depth (in instr/thread/cycle)", &p_commit_width);

	/* other options */
	bpred_reg_options();
	tcache_reg_options();
	phregs_reg_options();
	fetchq_reg_options();
	rob_reg_options();
	iq_reg_options();
	lsq_reg_options();
	fu_reg_options();
}


void p_print_stats(FILE *f)
{
	uint64_t now = ke_timer();

	/* Global stats */
	fprintf(f, "sim.cycles  %lld  # Simulation cycles\n",
		(long long) sim_cycle);
	fprintf(f, "sim.inst  %lld  # Total committed instructions\n",
		(long long) p->committed);
	fprintf(f, "sim.ipc  %.4f  # Global IPC\n",
		sim_cycle ? (double) p->committed / sim_cycle : 0);
	fprintf(f, "sim.squashed  %lld  # Number of uops squashed in the ROB\n",
		(long long) p->squashed);
	fprintf(f, "sim.time  %.1f  # Simulation time in seconds\n",
		(double) now / 1000000);
	fprintf(f, "sim.cps  %.0f  # Cycles simulated per second\n",
		now ? (double) sim_cycle / now * 1000000 : 0.0);
	fprintf(f, "sim.memory  %lu  # Physical memory used by benchmarks\n",
		mem_mapped_space);
	fprintf(f, "sim.memory_max  %lu  # Maximum physical memory used by benchmarks\n",
		mem_max_mapped_space);
	fprintf(f, "sim.branches  %lld  # Committed branches\n",
		(long long) p->branches);
	fprintf(f, "sim.mispred  %lld  # Mispredicted branches in correct path\n",
		(long long) p->mispred);
	fprintf(f, "sim.predacc  %.4f  # Branch prediction accuracy\n",
		p->branches ? (double) (p->branches - p->mispred) / p->branches : 0.0);
	
	/* Dispatch stats */
	if (p_dispatch_kind == p_dispatch_kind_timeslice) {
		fprintf(f, "di.stall[used]  %lld  # Dispatched slots used with committed inst\n",
			(long long) p->di_stall[di_stall_used]);
		fprintf(f, "di.stall[spec]  %lld  # Dispatched slots used in spec mode\n",
			(long long) p->di_stall[di_stall_spec]);
		fprintf(f, "di.stall[uopq]  %lld  # Wasted dispatch slots due to empty uop queue\n",
			(long long) p->di_stall[di_stall_uopq]);
		fprintf(f, "di.stall[rob]  %lld  # Wasted dispatch slots due to full rob\n",
			(long long) p->di_stall[di_stall_rob]);
		fprintf(f, "di.stall[iq]  %lld  # Wasted dispatch slots due to full iq\n",
			(long long) p->di_stall[di_stall_iq]);
		fprintf(f, "di.stall[lsq]  %lld  # Wasted dispatch slots due to full lsq\n",
			(long long) p->di_stall[di_stall_lsq]);
		fprintf(f, "di.stall[rename]  %lld  # No physical register free\n",
			(long long) p->di_stall[di_stall_rename]);
		fprintf(f, "di.stall[ctx]  %lld  # Dispatch stalled due to absence of running context\n",
			(long long) p->di_stall[di_stall_ctx]);
	}

	/* Stage time stats */
	if (p_stage_time_stats && sim_cycle) {
		fprintf(f, "stage_time.fetch  %.3f  # Time for stage in us/cycle\n",
			(double) stage_time_fetch / sim_cycle);
		fprintf(f, "stage_time.dispatch  %.3f\n",
			(double) stage_time_dispatch / sim_cycle);
		fprintf(f, "stage_time.issue  %.3f\n",
			(double) stage_time_issue / sim_cycle);
		fprintf(f, "stage_time.writeback  %.3f\n",
			(double) stage_time_writeback / sim_cycle);
		fprintf(f, "stage_time.commit  %.3f\n",
			(double) stage_time_commit / sim_cycle);
		fprintf(f, "stage_time.rest  %.3f\n",
			(double) stage_time_rest / sim_cycle);
	}

	/* Occupancy stats */
	if (p_occupancy_stats) {
		fprintf(f, "occupancy.iq  %.2f  # Instruction queue occupancy\n",
			p->occupancy_count ? (double) p->occupancy_iq_acc / p->occupancy_count : 0);
		fprintf(f, "occupancy.lsq  %.2f  # Load/Store queue occupancy\n",
			p->occupancy_count ? (double) p->occupancy_lsq_acc / p->occupancy_count : 0);
		fprintf(f, "occupancy.rf  %.2f  # Register file occupancy\n",
			p->occupancy_count ? (double) p->occupancy_rf_acc / p->occupancy_count : 0);
		fprintf(f, "occupancy.rob  %.2f  # ROB occupancy\n",
			p->occupancy_count ? (double) p->occupancy_rob_acc / p->occupancy_count : 0);
	}
	
}


void p_thread_init(int core, int thread)
{
	/* Save block size of corresponding instruction cache. */
	THREAD.fetch_bsize = cache_system_block_size(core, thread,
		cache_kind_inst);
}


void p_core_init(int core)
{
	int thread;
	CORE.thread = calloc(p_threads, sizeof(struct processor_thread_t));
	FOREACH_THREAD
		p_thread_init(core, thread);
}


/* Initialization */
void p_init()
{
	int core;
	
	/* Create processor structure and allocate cores/threads */
	p = calloc(1, sizeof(struct processor_t));
	p->core = calloc(p_cores, sizeof(struct processor_core_t));
	FOREACH_CORE
		p_core_init(core);

	phregs_init();
	bpred_init();
	tcache_init();
	fetchq_init();
	uopq_init();
	rob_init();
	iq_init();
	lsq_init();
	eventq_init();
	fu_init();
}


/* Finalization */
void p_done()
{
	int core;

	/* Stats */
	p_print_stats(stderr);

	/* Finalize structures */
	fetchq_done();
	uopq_done();
	rob_done();
	iq_done();
	lsq_done();
	eventq_done();
	bpred_done();
	tcache_done();
	phregs_done();
	fu_done();

	/* Free processor */
	FOREACH_CORE
		free(CORE.thread);
	free(p->core);
	free(p);
}


/* Load programs to different contexts from a configuration text file or
 * from arguments */
void p_load_progs(int argc, char **argv, char *ctxfile)
{
	if (argc > 1)
		ld_load_prog_from_cmdline(argc - 1, argv + 1);
	if (*ctxfile)
		ld_load_prog_from_ctxconfig(ctxfile);
	p_context_map_update();
	if (!ke->context_list)
		fatal("no executable loaded");
}


void p_dump(FILE *f)
{
	int core, thread;
	
	/* General information */
	fprintf(f, "\n");
	fprintf(f, "sim.last_dump  %lld  # Cycle of last dump\n", (long long) p->last_dump);
	fprintf(f, "sim.ipc_last_dump  %.4f  # IPC since last dump\n", sim_cycle - p->last_dump > 0 ?
		(double) (p->committed - p->last_committed) / (sim_cycle - p->last_dump) : 0);
	fprintf(f, "\n");

	/* Cores */
	FOREACH_CORE {
		fprintf(f, "Core %d:\n", core);
		
		fprintf(f, "eventq:\n");
		uop_lnlist_dump(CORE.eventq, f);
		fprintf(f, "rob:\n");
		rob_dump(core, f);

		FOREACH_THREAD {
			fprintf(f, "Thread %d:\n", thread);
			
			fprintf(f, "fetch queue:\n");
			uop_list_dump(THREAD.fetchq, f);
			fprintf(f, "iq:\n");
			uop_lnlist_dump(THREAD.iq, f);
			fprintf(f, "lq:\n");
			uop_lnlist_dump(THREAD.lq, f);
			fprintf(f, "sq:\n");
			uop_lnlist_dump(THREAD.sq, f);
			phregs_dump(core, thread, f);
			if (THREAD.ctx) {
				fprintf(f, "mapped context: %d\n", THREAD.ctx->pid);
				ctx_dump(THREAD.ctx, f);
			}
			
			fprintf(f, "\n");
		}
	}

	/* Register last dump */
	p->last_dump = sim_cycle;
	p->last_committed = p->committed;
}


/* Return the pair {core,thread} where a context is mapped. If
 * the context is not mapped, map it. If there are no free threads to
 * map, simulator error. */
void p_context_map(struct ctx_t *ctx, int *pcore, int *pthread)
{
	int core, thread;
	int free_core = -1, free_thread = -1;

	/* Look for context mapping */
	assert(pcore && pthread);
	FOREACH_CORE FOREACH_THREAD {
		if (!THREAD.ctx && free_thread == -1) {
			free_core = core;
			free_thread = thread;
		}
		if (THREAD.ctx == ctx) {
			*pcore = core;
			*pthread = thread;
			return;
		}
	}

	/* Context not found and no free space to map */
	if (free_core == -1)
		fatal("cannot allocate thread to context, increase number of cores/threads");

	/* Context not found - map it */
	*pcore = core = free_core;
	*pthread = thread = free_thread;
	THREAD.ctx = ctx;
	THREAD.fetch_neip = ctx->regs->eip;
	p->context_map_count++;
	CORE.context_map_count++;
}


void p_context_unmap(int core, int thread)
{
	assert(p->context_map_count > 0 && CORE.context_map_count > 0);
	CORE.context_map_count--;
	p->context_map_count--;
	THREAD.ctx = NULL;
}


void p_context_map_update(void)
{
	struct ctx_t *ctx;
	int core, thread;
	
	for (ctx = ke->context_list; ctx; ctx = ctx->context_next)
		p_context_map(ctx, &core, &thread);
}


uint64_t stage_time_fetch;
uint64_t stage_time_decode;
uint64_t stage_time_dispatch;
uint64_t stage_time_issue;
uint64_t stage_time_writeback;
uint64_t stage_time_commit;
uint64_t stage_time_rest;
static uint64_t stage_time_start;

#define STAGE(name) \
	p_##name(); \
	if (p_stage_time_stats) { \
		uint64_t end = ke_timer(); \
		stage_time_##name += end - stage_time_start; \
		stage_time_start = end; }
void p_stages()
{
	/* Kernel is stuck if all contexts are suspended and
	 * there is no timer pending. */
	if (!ke->running_list && !ke->event_timer_next)
		fatal("all contexts suspended");

	/* Time ellapsed since last call to p_stages */
	if (p_stage_time_stats) {
		uint64_t end = ke_timer();
		if (stage_time_start)
			stage_time_rest += end - stage_time_start;
		stage_time_start = end;
	}

	/* Stages */
	STAGE(commit);
	STAGE(writeback);
	STAGE(issue);
	STAGE(dispatch);
	STAGE(decode);
	STAGE(fetch);
}
#undef STAGE


/* Fast forward simulation */
void p_fast_forward(uint64_t cycles)
{
	struct ctx_t *ctx;
	int core, thread;

	while (cycles && ke->context_list) {
		
		/* Kernel is stuck if all contexts are suspended and
		 * there is no timer pending. */
		if (!ke->running_list && !ke->event_timer_next)
			fatal("all contexts suspended");

		/* Run one instruction from each running context */
		for (ctx = ke->running_list; ctx; ctx = ctx->running_next)
			ctx_execute_inst(ctx);

		/* Check for timer events */
		if (ke->event_timer_next && ke->event_timer_next < ke_timer())
			ke_event_timer();

		/* Next fast forward cycle */
		cycles--;
	}
	
	/* Free finished contexts and update
	 * context mappings. */
	FOREACH_CORE FOREACH_THREAD {
		if (THREAD.ctx && ctx_get_status(THREAD.ctx, ctx_finished)) {
			ke_list_remove(ke_list_finished, THREAD.ctx);
			ctx_free(THREAD.ctx);
			THREAD.ctx = NULL;
		}
	}

	/* Update mappings [ctx]->[core,thread] */
	p_context_map_update();
	
	/* Fetch PCs */
	FOREACH_CORE FOREACH_THREAD
		if (THREAD.ctx)
			THREAD.fetch_neip = THREAD.ctx->regs->eip;
}


/* return an address that combines the context and the virtual address page;
 * this address will be univocal for all generated
 * addresses and is a kind of hash function to index tlbs */
uint32_t p_tlb_address(int ctx, uint32_t vaddr)
{
	assert(ctx >= 0 && ctx < p_cores * p_threads);
	return (vaddr >> MEM_LOGPAGESIZE) * p_cores * p_threads + ctx;
}


void p_update_occupancy_stats(int core)
{
	p->occupancy_count++;
	p->occupancy_iq_acc += iq_kind == iq_kind_private ? CORE.iq_count / p_cores : CORE.iq_count;
	p->occupancy_lsq_acc += lsq_kind == lsq_kind_private ? CORE.lsq_count / p_cores : CORE.lsq_count;
}

