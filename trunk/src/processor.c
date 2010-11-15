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
int p_occupancy_stats = 0;
uint32_t p_cores = 1;
uint32_t p_threads = 1;
uint32_t p_cpus = 1;
uint32_t p_context_quantum = 100000;
int p_context_switch = 1;
uint32_t p_thread_quantum = 1000;
uint32_t p_thread_switch_penalty = 0;
char *p_report_file = "";

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

	opt_reg_bool("-context_switch", "allow context switches and scheduling",
		&p_context_switch);
	opt_reg_uint32("-context_quantum", "quantum for a context before context switch",
		&p_context_quantum);

	opt_reg_bool("-stage_time_stats", "measure time for stages",
		&p_stage_time_stats);
	opt_reg_bool("-occupancy_stats", "include occupancy stats in the pipeline report",
		&p_occupancy_stats);
	
	opt_reg_enum("-recover_kind", "when to recover {writeback|commit}",
		(int *) &p_recover_kind, p_recover_kind_map, 2);
	opt_reg_uint32("-recover_penalty", "cycles to stall fetch after recover",
		&p_recover_penalty);
	
	opt_reg_uint32("-thread_quantum", "thread quantum in cycles for switch-on-event fetch",
		&p_thread_quantum);
	opt_reg_uint32("-thread_switch_penalty", "for switch-on-event fetch",
		&p_thread_switch_penalty);
	opt_reg_enum("-fetch_kind", "fetch policy {shared|timeslice|switchonevent}",
		(int *) &p_fetch_kind, p_fetch_kind_map, 3);
	
	opt_reg_uint32("-decode_width", "decode width",
		&p_decode_width);
	
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
	fetchq_reg_options();
	uopq_reg_options();
	rob_reg_options();
	rf_reg_options();
	iq_reg_options();
	lsq_reg_options();
	fu_reg_options();
}


void p_dump_uop_report(FILE *f, uint64_t *uop_stats, char *prefix, int peak_ipc)
{
	uint64_t icomp = 0;
	uint64_t lcomp = 0;
	uint64_t fcomp = 0;
	uint64_t mem = 0;
	uint64_t ctrl = 0;
	uint64_t total = 0;

#define UOP(_uop, _fu, _flags) \
	fprintf(f, "%s.Uop." #_uop " = %lld\n", prefix, (long long) uop_stats[uop_##_uop]); \
	if ((_flags) & FICOMP) icomp += uop_stats[uop_##_uop]; \
	if ((_flags) & FLCOMP) lcomp += uop_stats[uop_##_uop]; \
	if ((_flags) & FFCOMP) fcomp += uop_stats[uop_##_uop]; \
	if ((_flags) & FMEM) mem += uop_stats[uop_##_uop]; \
	if ((_flags) & FCTRL) ctrl += uop_stats[uop_##_uop]; \
	total += uop_stats[uop_##_uop];
#include "uop1.dat"
#undef UOP

	fprintf(f, "%s.SimpleInteger = %lld\n", prefix,
		(long long) (icomp - uop_stats[uop_mult] - uop_stats[uop_div]));
	fprintf(f, "%s.ComplexInteger = %lld\n", prefix,
		(long long) (uop_stats[uop_mult] + uop_stats[uop_div]));
	fprintf(f, "%s.Integer = %lld\n", prefix, (long long) icomp);
	fprintf(f, "%s.Logical = %lld\n", prefix, (long long) lcomp);
	fprintf(f, "%s.FloatingPoint = %lld\n", prefix, (long long) fcomp);
	fprintf(f, "%s.Memory = %lld\n", prefix, (long long) mem);
	fprintf(f, "%s.Ctrl = %lld\n", prefix, (long long) ctrl);
	fprintf(f, "%s.WndSwitch = %lld\n", prefix, (long long)
		(uop_stats[uop_call] + uop_stats[uop_ret]));
	fprintf(f, "%s.Total = %lld\n", prefix, (long long) total);
	fprintf(f, "%s.IPC = %.4g\n", prefix, sim_cycle ? (double) total / sim_cycle : 0.0);
	fprintf(f, "%s.DutyCycle = %.4g\n", prefix, sim_cycle && peak_ipc ?
		(double) total / sim_cycle / peak_ipc : 0.0);
	fprintf(f, "\n");
}


#define DUMP_FU_STAT(NAME, ITEM) { \
	fprintf(f, "fu." #NAME ".Accesses = %lld\n", (long long) CORE.fu->accesses[ITEM]); \
	fprintf(f, "fu." #NAME ".Denied = %lld\n", (long long) CORE.fu->denied[ITEM]); \
	fprintf(f, "fu." #NAME ".WaitingTime = %.4g\n", CORE.fu->accesses[ITEM] ? \
		(double) CORE.fu->waiting_time[ITEM] / CORE.fu->accesses[ITEM] : 0.0); \
}

#define DUMP_DISPATCH_STAT(NAME) { \
	fprintf(f, "Dispatch.Stall." #NAME " = %lld\n", (long long) CORE.di_stall[di_stall_##NAME]); \
}

#define DUMP_CORE_STRUCT_STATS(NAME, ITEM) { \
	fprintf(f, #NAME ".Size = %d\n", (int) ITEM##_size * p_threads); \
	if (p_occupancy_stats) \
		fprintf(f, #NAME ".Occupancy = %.2f\n", sim_cycle ? (double) CORE.ITEM##_occupancy / sim_cycle : 0.0); \
	fprintf(f, #NAME ".Full = %lld\n", (long long) CORE.ITEM##_full); \
	fprintf(f, #NAME ".Reads = %lld\n", (long long) CORE.ITEM##_reads); \
	fprintf(f, #NAME ".Writes = %lld\n", (long long) CORE.ITEM##_writes); \
}

#define DUMP_THREAD_STRUCT_STATS(NAME, ITEM) { \
	fprintf(f, #NAME ".Size = %d\n", (int) ITEM##_size); \
	if (p_occupancy_stats) \
		fprintf(f, #NAME ".Occupancy = %.2f\n", sim_cycle ? (double) THREAD.ITEM##_occupancy / sim_cycle : 0.0); \
	fprintf(f, #NAME ".Full = %lld\n", (long long) THREAD.ITEM##_full); \
	fprintf(f, #NAME ".Reads = %lld\n", (long long) THREAD.ITEM##_reads); \
	fprintf(f, #NAME ".Writes = %lld\n", (long long) THREAD.ITEM##_writes); \
}

void p_dump_report()
{
	FILE *f;
	int core, thread;
	uint64_t now = ke_timer();

	/* Open file */
	f = open_write(p_report_file);
	if (!f)
		return;
	
	/* Report for the complete processor */
	fprintf(f, "; Global statistics\n");
	fprintf(f, "[ global ]\n\n");
	fprintf(f, "Cycles = %lld\n", (long long) sim_cycle);
	fprintf(f, "Time = %.1f\n", (double) now / 1000000);
	fprintf(f, "CyclesPerSecond = %.0f\n", now ? (double) sim_cycle / now * 1000000 : 0.0);
	fprintf(f, "MemoryUsed = %lu\n", (long) mem_mapped_space);
	fprintf(f, "MemoryUsedMax = %lu\n", (long) mem_max_mapped_space);
	fprintf(f, "\n");

	/* Dispatch stage */
	fprintf(f, "; Dispatch stage\n");
	p_dump_uop_report(f, p->dispatched, "Dispatch", p_dispatch_width);

	/* Issue stage */
	fprintf(f, "; Issue stage\n");
	p_dump_uop_report(f, p->issued, "Issue", p_issue_width);

	/* Commit stage */
	fprintf(f, "; Commit stage\n");
	p_dump_uop_report(f, p->committed, "Commit", p_commit_width);

	/* Committed branches */
	fprintf(f, "; Committed branches\n");
	fprintf(f, ";    Branches - Number of committed control uops\n");
	fprintf(f, ";    Squashed - Number of mispredicted uops squashed from the ROB\n");
	fprintf(f, ";    Mispred - Number of mispredicted branches in the correct path\n");
	fprintf(f, ";    PredAcc - Prediction accuracy\n");
	fprintf(f, "Commit.Branches = %lld\n", (long long) p->branches);
	fprintf(f, "Commit.Squashed = %lld\n", (long long) p->squashed);
	fprintf(f, "Commit.Mispred = %lld\n", (long long) p->mispred);
	fprintf(f, "Commit.PredAcc = %.4g\n", p->branches ?
		(double) (p->branches - p->mispred) / p->branches : 0.0);
	fprintf(f, "\n");
	
	/* Report for each core */
	FOREACH_CORE {
		
		/* Core */
		fprintf(f, "\n; Statistics for core %d\n", core);
		fprintf(f, "[ c%d ]\n\n", core);

		/* Functional units */
		fprintf(f, "; Functional unit pool\n");
		fprintf(f, ";    Accesses - Number of uops issued to a f.u.\n");
		fprintf(f, ";    Denied - Number of requests denied due to busy f.u.\n");
		fprintf(f, ";    WaitingTime - Average number of waiting cycles to reserve f.u.\n");
		DUMP_FU_STAT(IntAdd, fu_intadd);
		DUMP_FU_STAT(IntSub, fu_intsub);
		DUMP_FU_STAT(IntMult, fu_intmult);
		DUMP_FU_STAT(IntDiv, fu_intdiv);
		DUMP_FU_STAT(Effaddr, fu_effaddr);
		DUMP_FU_STAT(Logical, fu_logical);
		DUMP_FU_STAT(FPSimple, fu_fpsimple);
		DUMP_FU_STAT(FPAdd, fu_fpadd);
		DUMP_FU_STAT(FPComp, fu_fpcomp);
		DUMP_FU_STAT(FPMult, fu_fpmult);
		DUMP_FU_STAT(FPDiv, fu_fpdiv);
		DUMP_FU_STAT(FPComplex, fu_fpcomplex);
		fprintf(f, "\n");

		/* Dispatch slots */
		if (p_dispatch_kind == p_dispatch_kind_timeslice) {
			fprintf(f, "; Dispatch slots usage (sum = cycles * dispatch width)\n");
			fprintf(f, ";    used - dispatch slot was used by a non-spec uop\n");
			fprintf(f, ";    spec - used by a mispeculated uop\n");
			fprintf(f, ";    ctx - no context allocated to thread\n");
			fprintf(f, ";    uopq,rob,iq,lsq,rename - no space in structure\n");
			DUMP_DISPATCH_STAT(used);
			DUMP_DISPATCH_STAT(spec);
			DUMP_DISPATCH_STAT(uopq);
			DUMP_DISPATCH_STAT(rob);
			DUMP_DISPATCH_STAT(iq);
			DUMP_DISPATCH_STAT(lsq);
			DUMP_DISPATCH_STAT(rename);
			DUMP_DISPATCH_STAT(ctx);
			fprintf(f, "\n");
		}

		/* Dispatch stage */
		fprintf(f, "; Dispatch stage\n");
		p_dump_uop_report(f, CORE.dispatched, "Dispatch", p_dispatch_width);

		/* Issue stage */
		fprintf(f, "; Issue stage\n");
		p_dump_uop_report(f, CORE.issued, "Issue", p_issue_width);

		/* Commit stage */
		fprintf(f, "; Commit stage\n");
		p_dump_uop_report(f, CORE.committed, "Commit", p_commit_width);

		/* Committed branches */
		fprintf(f, "; Committed branches\n");
		fprintf(f, "Commit.Branches = %lld\n", (long long) CORE.branches);
		fprintf(f, "Commit.Squashed = %lld\n", (long long) CORE.squashed);
		fprintf(f, "Commit.Mispred = %lld\n", (long long) CORE.mispred);
		fprintf(f, "Commit.PredAcc = %.4g\n", CORE.branches ?
			(double) (CORE.branches - CORE.mispred) / CORE.branches : 0.0);
		fprintf(f, "\n");

		/* Occupancy stats */
		fprintf(f, "; Structure statistics (reorder buffer, instruction queue,\n");
		fprintf(f, "; load-store queue, and integer/floating-point register file)\n");
		fprintf(f, ";    Size - Available size\n");
		fprintf(f, ";    Occupancy - Average number of occupied entries\n");
		fprintf(f, ";    Full - Number of cycles when the structure was full\n");
		fprintf(f, ";    Reads, Writes - Accesses to the structure\n");
		if (rob_kind == rob_kind_shared)
			DUMP_CORE_STRUCT_STATS(ROB, rob);
		if (iq_kind == iq_kind_shared) {
			DUMP_CORE_STRUCT_STATS(IQ, iq);
			fprintf(f, "IQ.WakeupAccesses = %lld\n", (long long) CORE.iq_wakeup_accesses);
		}
		if (lsq_kind == lsq_kind_shared)
			DUMP_CORE_STRUCT_STATS(LSQ, lsq);
		if (rf_kind == rf_kind_shared) {
			DUMP_CORE_STRUCT_STATS(RF_Int, rf_int);
			DUMP_CORE_STRUCT_STATS(RF_Fp, rf_fp);
		}
		fprintf(f, "\n");

		/* Report for each thread */
		FOREACH_THREAD {
			fprintf(f, "\n; Statistics for core %d - thread %d\n", core, thread);
			fprintf(f, "[ c%dt%d ]\n\n", core, thread);

			/* Dispatch stage */
			fprintf(f, "; Dispatch stage\n");
			p_dump_uop_report(f, THREAD.dispatched, "Dispatch", p_dispatch_width);

			/* Issue stage */
			fprintf(f, "; Issue stage\n");
			p_dump_uop_report(f, THREAD.issued, "Issue", p_issue_width);

			/* Commit stage */
			fprintf(f, "; Commit stage\n");
			p_dump_uop_report(f, THREAD.committed, "Commit", p_commit_width);

			/* Committed branches */
			fprintf(f, "; Committed branches\n");
			fprintf(f, "Commit.Branches = %lld\n", (long long) THREAD.branches);
			fprintf(f, "Commit.Squashed = %lld\n", (long long) THREAD.squashed);
			fprintf(f, "Commit.Mispred = %lld\n", (long long) THREAD.mispred);
			fprintf(f, "Commit.PredAcc = %.4g\n", THREAD.branches ?
				(double) (THREAD.branches - THREAD.mispred) / THREAD.branches : 0.0);
			fprintf(f, "\n");

			/* Occupancy stats */
			fprintf(f, "; Structure statistics (reorder buffer, instruction queue, load-store queue,\n");
			fprintf(f, "; integer/floating-point register file, and renaming table)\n");
			if (rob_kind == rob_kind_private)
				DUMP_THREAD_STRUCT_STATS(ROB, rob);
			if (iq_kind == iq_kind_private) {
				DUMP_THREAD_STRUCT_STATS(IQ, iq);
				fprintf(f, "IQ.WakeupAccesses = %lld\n", (long long) THREAD.iq_wakeup_accesses);
			}
			if (lsq_kind == lsq_kind_private)
				DUMP_THREAD_STRUCT_STATS(LSQ, lsq);
			if (rf_kind == rf_kind_private) {
				DUMP_THREAD_STRUCT_STATS(RF_Int, rf_int);
				DUMP_THREAD_STRUCT_STATS(RF_Fp, rf_fp);
			}
			fprintf(f, "RAT.IntReads = %lld\n", (long long) THREAD.rat_int_reads);
			fprintf(f, "RAT.IntWrites = %lld\n", (long long) THREAD.rat_int_writes);
			fprintf(f, "RAT.FpReads = %lld\n", (long long) THREAD.rat_fp_reads);
			fprintf(f, "RAT.FpWrites = %lld\n", (long long) THREAD.rat_fp_writes);
			fprintf(f, "BTB.Reads = %lld\n", (long long) THREAD.btb_reads);
			fprintf(f, "BTB.Writes = %lld\n", (long long) THREAD.btb_writes);
			fprintf(f, "\n");

			/* Trace cache stats */
			if (THREAD.tcache)
				tcache_dump_report(THREAD.tcache, f);
		}
	}

	/* Close */
	fclose(f);
}


void p_print_stats(FILE *f)
{
	uint64_t now = ke_timer();

	/* Global stats */
	fprintf(f, "sim.cycles  %lld  # Simulation cycles\n",
		(long long) sim_cycle);
	fprintf(f, "sim.inst  %lld  # Total committed instructions\n",
		(long long) sim_inst);
	fprintf(f, "sim.ipc  %.4g  # Global IPC\n",
		sim_cycle ? (double) sim_inst / sim_cycle : 0);
	fprintf(f, "sim.predacc  %.4g  # Branch prediction accuracy\n",
		p->branches ? (double) (p->branches - p->mispred) / p->branches : 0.0);
	fprintf(f, "sim.time  %.1f  # Simulation time in seconds\n",
		(double) now / 1000000);
	fprintf(f, "sim.cps  %.0f  # Cycles simulated per second\n",
		now ? (double) sim_cycle / now * 1000000 : 0.0);
	fprintf(f, "sim.contexts  %d  # Maximum number of contexts running concurrently\n",
		ke->running_max);
	fprintf(f, "sim.memory  %lu  # Physical memory used by benchmarks\n",
		mem_mapped_space);
	fprintf(f, "sim.memory_max  %lu  # Maximum physical memory used by benchmarks\n",
		mem_max_mapped_space);
	
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

	/* Report */
	p_dump_report();
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
	p_cpus = p_cores * p_threads;
	p = calloc(1, sizeof(struct processor_t));
	p->core = calloc(p_cores, sizeof(struct processor_core_t));
	FOREACH_CORE
		p_core_init(core);

	rf_init();
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
	rf_done();
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
}


void p_dump(FILE *f)
{
	int core, thread;
	
	/* General information */
	fprintf(f, "\n");
	fprintf(f, "sim.last_dump  %lld  # Cycle of last dump\n", (long long) p->last_dump);
	fprintf(f, "sim.ipc_last_dump  %.4g  # IPC since last dump\n", sim_cycle - p->last_dump > 0 ?
		(double) (sim_inst - p->last_committed) / (sim_cycle - p->last_dump) : 0);
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
			fprintf(f, "uop queue:\n");
			uop_list_dump(THREAD.uopq, f);
			fprintf(f, "iq:\n");
			uop_lnlist_dump(THREAD.iq, f);
			fprintf(f, "lq:\n");
			uop_lnlist_dump(THREAD.lq, f);
			fprintf(f, "sq:\n");
			uop_lnlist_dump(THREAD.sq, f);
			rf_dump(core, thread, f);
			if (THREAD.ctx) {
				fprintf(f, "mapped context: %d\n", THREAD.ctx->pid);
				ctx_dump(THREAD.ctx, f);
			}
			
			fprintf(f, "\n");
		}
	}

	/* Register last dump */
	p->last_dump = sim_cycle;
	p->last_committed = sim_inst;
}


#define UPDATE_THREAD_OCCUPANCY_STATS(ITEM) { \
	THREAD.ITEM##_occupancy += THREAD.ITEM##_count; \
	if (THREAD.ITEM##_count == ITEM##_size) \
		THREAD.ITEM##_full++; \
}


#define UPDATE_CORE_OCCUPANCY_STATS(ITEM) { \
	CORE.ITEM##_occupancy += CORE.ITEM##_count; \
	if (CORE.ITEM##_count == ITEM##_size * p_threads) \
		CORE.ITEM##_full++; \
}


void p_update_occupancy_stats()
{
	int core, thread;

	FOREACH_CORE {

		/* Update occupancy stats for shared structures */
		if (rob_kind == rob_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(rob);
		if (iq_kind == iq_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(iq);
		if (lsq_kind == lsq_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(lsq);
		if (rf_kind == rf_kind_shared) {
			UPDATE_CORE_OCCUPANCY_STATS(rf_int);
			UPDATE_CORE_OCCUPANCY_STATS(rf_fp);
		}

		/* Occupancy stats for private structures */
		FOREACH_THREAD {
			if (rob_kind == rob_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(rob);
			if (iq_kind == iq_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(iq);
			if (lsq_kind == lsq_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(lsq);
			if (rf_kind == rf_kind_private) {
				UPDATE_THREAD_OCCUPANCY_STATS(rf_int);
				UPDATE_THREAD_OCCUPANCY_STATS(rf_fp);
			}
		}
	}
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
	/* Static scheduler called after any context changed status other than 'sepcmode' */
	if (!p_context_switch && ke->context_reschedule) {
		p_static_schedule();
		ke->context_reschedule = 0;
	}

	/* Dynamic scheduler called after any context changed status other than 'specmode',
	 * or quantum of the oldest context expired, and no context is being evicted. */
	if (p_context_switch && !p->ctx_dealloc_signals &&
		(ke->context_reschedule || p->ctx_alloc_oldest + p_context_quantum <= sim_cycle))
	{
		p_dynamic_schedule();
		ke->context_reschedule = 0;
	}

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

	/* Update stats for structures occupancy */
	if (p_occupancy_stats)
		p_update_occupancy_stats();
}
#undef STAGE


/* Fast forward simulation */
void p_fast_forward(uint64_t cycles)
{
	int core, thread;

	/* Functional simulation */
	while (cycles && ke->context_list_head) {
		ke_run();
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
}


/* return an address that combines the context and the virtual address page;
 * this address will be univocal for all generated
 * addresses and is a kind of hash function to index tlbs */
uint32_t p_tlb_address(int ctx, uint32_t vaddr)
{
	assert(ctx >= 0 && ctx < p_cores * p_threads);
	return (vaddr >> MEM_LOGPAGESIZE) * p_cores * p_threads + ctx;
}

