/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <cpuarch.h>



/*
 * Global variables
 */

/* Main Processor Global Variable */
struct processor_t *p;

/* Configuration file and parameters */

enum p_sim_kind_enum p_sim_kind = p_sim_kind_functional;

char *p_config_file_name = "";
char *p_report_file_name = "";

int p_occupancy_stats;

int p_cores;
int p_threads;

int p_context_quantum;
int p_context_switch;

int p_thread_quantum;
int p_thread_switch_penalty;

char *p_recover_kind_map[] = { "Writeback", "Commit" };
enum p_recover_kind_enum p_recover_kind;
int p_recover_penalty;

char *p_fetch_kind_map[] = { "Shared", "TimeSlice", "SwitchOnEvent" };
enum p_fetch_kind_enum p_fetch_kind;

int p_decode_width;

char *p_dispatch_kind_map[] = { "Shared", "TimeSlice" };
enum p_dispatch_kind_enum p_dispatch_kind;
int p_dispatch_width;

char *p_issue_kind_map[] = { "Shared", "TimeSlice" };
enum p_issue_kind_enum p_issue_kind;
int p_issue_width;

char *p_commit_kind_map[] = { "Shared", "TimeSlice" };
enum p_commit_kind_enum p_commit_kind;
int p_commit_width;





/*
 * Public Functions
 */


/* Options */
void p_reg_options()
{
	opt_reg_string("-cpuconfig", "Configuration file for the CPU model",
		&p_config_file_name);
}


/* Check CPU configuration file */
void p_config_check(void)
{
	struct config_t *cfg;
	int err;
	char *section;

	/* Open file */
	cfg = config_create(p_config_file_name);
	err = config_load(cfg);
	if (!err && p_config_file_name[0])
		fatal("%s: cannot load CPU configuration file", p_config_file_name);

	
	/* General configuration */

	section = "General";

	p_cores = config_read_int(cfg, section, "Cores", 1);
	p_threads = config_read_int(cfg, section, "Threads", 1);

	p_context_switch = config_read_bool(cfg, section, "ContextSwitch", 1);
	p_context_quantum = config_read_int(cfg, section, "ContextQuantum", 100000);

	p_thread_quantum = config_read_int(cfg, section, "ThreadQuantum", 1000);
	p_thread_switch_penalty = config_read_int(cfg, section, "ThreadSwitchPenalty", 0);

	p_recover_kind = config_read_enum(cfg, section, "RecoverKind", p_recover_kind_writeback, p_recover_kind_map, 2);
	p_recover_penalty = config_read_int(cfg, section, "RecoverPenalty", 0);


	/* Section '[ Pipeline ]' */

	section = "Pipeline";

	p_fetch_kind = config_read_enum(cfg, section, "FetchKind", p_fetch_kind_timeslice, p_fetch_kind_map, 3);

	p_decode_width = config_read_int(cfg, section, "DecodeWidth", 4);

	p_dispatch_kind = config_read_enum(cfg, section, "DispatchKind", p_dispatch_kind_timeslice, p_dispatch_kind_map, 2);
	p_dispatch_width = config_read_int(cfg, section, "DispatchWidth", 4);

	p_issue_kind = config_read_enum(cfg, section, "IssueKind", p_issue_kind_timeslice, p_issue_kind_map, 2);
	p_issue_width = config_read_int(cfg, section, "IssueWidth", 4);

	p_commit_kind = config_read_enum(cfg, section, "CommitKind", p_commit_kind_shared, p_commit_kind_map, 2);
	p_commit_width = config_read_int(cfg, section, "CommitWidth", 4);

	p_occupancy_stats = config_read_bool(cfg, section, "OccupancyStats", 0);


	/* Section '[ Queues ]' */
	section = "Queues";

	fetchq_size = config_read_int(cfg, section, "FetchQueueSize", 64);

	uopq_size = config_read_int(cfg, section, "UopQueueSize", 32);

	rob_kind = config_read_enum(cfg, section, "RobKind", rob_kind_private, rob_kind_map, 2);
	rob_size = config_read_int(cfg, section, "RobSize", 64);

	iq_kind = config_read_enum(cfg, section, "IqKind", iq_kind_private, iq_kind_map, 2);
	iq_size = config_read_int(cfg, section, "IqSize", 40);

	lsq_kind = config_read_enum(cfg, section, "LsqKind", lsq_kind_private, lsq_kind_map, 2);
	lsq_size = config_read_int(cfg, section, "LsqSize", 20);

	
	/* Section '[ RegisterFile ]' */
	section = "RegisterFile";
	rf_kind = config_read_enum(cfg, section, "Kind", rf_kind_private, rf_kind_map, 2);
	rf_int_size = config_read_int(cfg, section, "IntSize", 80);
	rf_fp_size = config_read_int(cfg, section, "FpSize", 40);


	/* Section '[ TraceCache ]' */
	section = "TraceCache";
	tcache_present = config_read_bool(cfg, section, "Present", 0);
	tcache_sets = config_read_int(cfg, section, "Sets", 64);
	tcache_assoc = config_read_int(cfg, section, "Assoc", 4);
	tcache_trace_size = config_read_int(cfg, section, "TraceSize", 16);
	tcache_branch_max = config_read_int(cfg, section, "BranchMax", 3);
	tcache_queue_size = config_read_int(cfg, section, "QueueSize", 32);

	
	/* Functional Units */
	section = "FunctionalUnits";

	fu_res_pool[fu_intadd].count = config_read_int(cfg, section, "IntAdd.Count", 4);
	fu_res_pool[fu_intadd].oplat = config_read_int(cfg, section, "IntAdd.OpLat", 2);
	fu_res_pool[fu_intadd].issuelat = config_read_int(cfg, section, "IntAdd.IssueLat", 1);

	fu_res_pool[fu_intsub].count = config_read_int(cfg, section, "IntSub.Count", 4);
	fu_res_pool[fu_intsub].oplat = config_read_int(cfg, section, "IntSub.OpLat", 2);
	fu_res_pool[fu_intsub].issuelat = config_read_int(cfg, section, "IntSub.IssueLat", 1);

	fu_res_pool[fu_intmult].count = config_read_int(cfg, section, "IntMult.Count", 1);
	fu_res_pool[fu_intmult].oplat = config_read_int(cfg, section, "IntMult.OpLat", 3);
	fu_res_pool[fu_intmult].issuelat = config_read_int(cfg, section, "IntMult.IssueLat", 3);

	fu_res_pool[fu_intdiv].count = config_read_int(cfg, section, "IntDiv.Count", 1);
	fu_res_pool[fu_intdiv].oplat = config_read_int(cfg, section, "IntDiv.OpLat", 20);
	fu_res_pool[fu_intdiv].issuelat = config_read_int(cfg, section, "IntDiv.IssueLat", 20);

	fu_res_pool[fu_effaddr].count = config_read_int(cfg, section, "EffAddr.Count", 4);
	fu_res_pool[fu_effaddr].oplat = config_read_int(cfg, section, "EffAddr.OpLat", 2);
	fu_res_pool[fu_effaddr].issuelat = config_read_int(cfg, section, "EffAddr.IssueLat", 1);

	fu_res_pool[fu_logical].count = config_read_int(cfg, section, "Logical.Count", 4);
	fu_res_pool[fu_logical].oplat = config_read_int(cfg, section, "Logical.OpLat", 1);
	fu_res_pool[fu_logical].issuelat = config_read_int(cfg, section, "Logical.IssueLat", 1);

	fu_res_pool[fu_fpsimple].count = config_read_int(cfg, section, "FpSimple.Count", 2);
	fu_res_pool[fu_fpsimple].oplat = config_read_int(cfg, section, "FpSimple.OpLat", 2);
	fu_res_pool[fu_fpsimple].issuelat = config_read_int(cfg, section, "FpSimple.IssueLat", 2);

	fu_res_pool[fu_fpadd].count = config_read_int(cfg, section, "FpAdd.Count", 2);
	fu_res_pool[fu_fpadd].oplat = config_read_int(cfg, section, "FpAdd.OpLat", 5);
	fu_res_pool[fu_fpadd].issuelat = config_read_int(cfg, section, "FpAdd.IssueLat", 5);

	fu_res_pool[fu_fpcomp].count = config_read_int(cfg, section, "FpComp.Count", 2);
	fu_res_pool[fu_fpcomp].oplat = config_read_int(cfg, section, "FpComp.OpLat", 5);
	fu_res_pool[fu_fpcomp].issuelat = config_read_int(cfg, section, "FpComp.IssueLat", 5);

	fu_res_pool[fu_fpmult].count = config_read_int(cfg, section, "FpMult.Count", 1);
	fu_res_pool[fu_fpmult].oplat = config_read_int(cfg, section, "FpMult.OpLat", 10);
	fu_res_pool[fu_fpmult].issuelat = config_read_int(cfg, section, "FpMult.IssueLat", 10);

	fu_res_pool[fu_fpdiv].count = config_read_int(cfg, section, "FpDiv.Count", 1);
	fu_res_pool[fu_fpdiv].oplat = config_read_int(cfg, section, "FpDiv.OpLat", 20);
	fu_res_pool[fu_fpdiv].issuelat = config_read_int(cfg, section, "FpDiv.IssueLat", 20);

	fu_res_pool[fu_fpcomplex].count = config_read_int(cfg, section, "FpComplex.Count", 1);
	fu_res_pool[fu_fpcomplex].oplat = config_read_int(cfg, section, "FpComplex.OpLat", 40);
	fu_res_pool[fu_fpcomplex].issuelat = config_read_int(cfg, section, "FpComplex.IssueLat", 40);


	/* Branch Predictor */
	section = "BranchPredictor";
	bpred_kind = config_read_enum(cfg, section, "Kind", bpred_kind_twolevel, bpred_kind_map, 6);
	bpred_btb_sets = config_read_int(cfg, section, "BTB.Sets", 256);
	bpred_btb_assoc = config_read_int(cfg, section, "BTB.Assoc", 4);
	bpred_bimod_size = config_read_int(cfg, section, "Bimod.Size", 1024);
	bpred_choice_size = config_read_int(cfg, section, "Choice.Size", 1024);
	bpred_ras_size = config_read_int(cfg, section, "RAS.Size", 32);
	bpred_twolevel_l1size = config_read_int(cfg, section, "TwoLevel.L1Size", 1);
	bpred_twolevel_l2size = config_read_int(cfg, section, "TwoLevel.L2Size", 1024);
	bpred_twolevel_hist_size = config_read_int(cfg, section, "TwoLevel.HistorySize", 8);

	/* Close file */
	config_free(cfg);
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
	f = open_write(p_report_file_name);
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

	/* Analyze CPU configuration file */
	p_config_check();

	/* Initialize cache system */
	cache_system_init(p_cores, p_threads);
	
	/* Create processor structure and allocate cores/threads */
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
	cache_system_done();

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

	/* Stages */
	p_commit();
	p_writeback();
	p_issue();
	p_dispatch();
	p_decode();
	p_fetch();

	/* Update stats for structures occupancy */
	if (p_occupancy_stats)
		p_update_occupancy_stats();
}


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

