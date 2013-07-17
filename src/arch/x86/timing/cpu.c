/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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


#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "bpred.h"
#include "commit.h"
#include "core.h"
#include "cpu.h"
#include "decode.h"
#include "dispatch.h"
#include "fetch.h"
#include "fetch-queue.h"
#include "fu.h"
#include "inst-queue.h"
#include "issue.h"
#include "load-store-queue.h"
#include "mem-config.h"
#include "reg-file.h"
#include "rob.h"
#include "sched.h"
#include "thread.h"
#include "trace-cache.h"
#include "uop-queue.h"
#include "writeback.h"


/*
 * Global variables
 */


/* Help message */

char *x86_config_help =
	"The x86 CPU configuration file is a plain text INI file, defining\n"
	"the parameters of the CPU model used for a detailed (architectural) simulation.\n"
	"This configuration file is passed to Multi2Sim with option '--x86-config <file>,\n"
	"which must be accompanied by option '--x86-sim detailed'.\n"
	"\n"
	"The following is a list of the sections allowed in the CPU configuration file,\n"
	"along with the list of variables for each section.\n"
	"\n"
	"Section '[ General ]':\n"
	"\n"
	"  Frequency = <freq> (Default = 1000 MHz)\n"
	"      Frequency in MHz for the x86 CPU. Value between 1 and 10K.\n"
	"  Cores = <num_cores> (Default = 1)\n"
	"      Number of cores.\n"
	"  Threads = <num_threads> (Default = 1)\n"
	"      Number of hardware threads per core. The total number of computing nodes\n"
	"      in the CPU model is equals to Cores * Threads.\n"
	"  FastForward = <num_inst> (Default = 0)\n"
	"      Number of x86 instructions to run with a fast functional simulation before\n"
	"      the architectural simulation starts.\n"
	"  ContextQuantum = <cycles> (Default = 100k)\n"
	"      If ContextSwitch is true, maximum number of cycles that a context can occupy\n"
	"      a CPU hardware thread before it is replaced by other pending context.\n"
	"  ThreadQuantum = <cycles> (Default = 1k)\n"
	"      For multithreaded processors (Threads > 1) configured as coarse-grain multi-\n"
	"      threading (FetchKind = SwitchOnEvent), number of cycles in which instructions\n"
	"      are fetched from the same thread before switching.\n"
	"  ThreadSwitchPenalty = <cycles> (Default = 0)\n"
	"      For coarse-grain multithreaded processors (FetchKind = SwitchOnEvent), number\n"
	"      of cycles that the fetch stage stalls after a thread switch.\n"
	"  RecoverKind = {Writeback|Commit} (Default = Writeback)\n"
	"      On branch misprediction, stage in the execution of the mispredicted branch\n"
	"      when processor recovery is triggered.\n"
	"  RecoverPenalty = <cycles> (Default = 0)\n"
	"      Number of cycles that the fetch stage gets stalled after a branch\n"
	"      misprediction.\n"
	"  PageSize = <size> (Default = 4kB)\n"
	"      Memory page size in bytes.\n"
	"  DataCachePerfect = {t|f} (Default = False)\n"
	"  ProcessPrefetchHints = {t|f} (Default = True)\n"
	"      If specified as false, the cpu will ignore any prefetch hints/instructions.\n"
	"  PrefetchHistorySize = <size> (Default = 10)\n"
	"      Number of past prefetches to keep track of, so as to avoid redundant prefetches\n"
	"      from being issued from the cpu to the cache module.\n"
	"  InstructionCachePerfect = {t|f} (Default = False)\n"
	"      Set these options to true to simulate a perfect data/instruction caches,\n"
	"      respectively, where every access results in a hit. If set to false, the\n"
	"      parameters of the caches are given in the memory configuration file\n"
	"\n"
	"Section '[ Pipeline ]':\n"
	"\n"
	"  FetchKind = {Shared|TimeSlice|SwitchOnEvent} (Default = TimeSlice)\n"
	"      Policy for fetching instruction from different threads. A shared fetch stage\n"
	"      fetches instructions from different threads in the same cycle; a time-slice\n"
	"      fetch switches between threads in a round-robin fashion; option SwitchOnEvent\n"
	"      switches thread fetch on long-latency operations or thread quantum expiration.\n"
	"  DecodeWidth = <num_inst> (Default = 4)\n"
	"      Number of x86 instructions decoded per cycle.\n"
	"  DispatchKind = {Shared|TimeSlice} (Default = TimeSlice)\n"
	"      Policy for dispatching instructions from different threads. If shared,\n"
	"      instructions from different threads are dispatched in the same cycle. Otherwise,\n"
	"      instruction dispatching is done in a round-robin fashion at a cycle granularity.\n"
	"  DispatchWidth = <num_inst> (Default = 4)\n"
	"      Number of microinstructions dispatched per cycle.\n"
	"  IssueKind = {Shared|TimeSlice} (Default = TimeSlice)\n"
	"      Policy for issuing instructions from different threads. If shared, instructions\n"
	"      from different threads are issued in the same cycle; otherwise, instruction issue\n"
	"      is done round-robin at a cycle granularity.\n"
	"  IssueWidth = <num_inst> (Default = 4)\n"
	"      Number of microinstructions issued per cycle.\n"
	"  CommitKind = {Shared|TimeSlice} (Default = Shared)\n"
	"      Policy for committing instructions from different threads. If shared,\n"
	"      instructions from different threads are committed in the same cycle; otherwise,\n"
	"      they commit in a round-robin fashion.\n"
	"  CommitWidth = <num_inst> (Default = 4)\n"
	"      Number of microinstructions committed per cycle.\n"
	"  OccupancyStats = {t|f} (Default = False)\n"
	"      Calculate structures occupancy statistics. Since this computation requires\n"
	"      additional overhead, the option needs to be enabled explicitly. These statistics\n"
	"      will be attached to the CPU report.\n"
	"\n"
	"Section '[ Queues ]':\n"
	"\n"
	"  FetchQueueSize = <bytes> (Default = 64)\n"
	"      Size of the fetch queue given in bytes.\n"
	"  UopQueueSize = <num_uops> (Default = 32)\n"
	"      Size of the uop queue size, given in number of uops.\n"
	"  RobKind = {Private|Shared} (Default = Private)\n"
	"      Reorder buffer sharing among hardware threads.\n"
	"  RobSize = <num_uops> (Default = 64)\n"
	"      Reorder buffer size in number of microinstructions (if private, per-thread size).\n"
	"  IqKind = {Private|Shared} (Default = Private)\n"
	"      Instruction queue sharing among threads.\n"
	"  IqSize = <num_uops> (Default = 40)\n"
	"      Instruction queue size in number of uops (if private, per-thread IQ size).\n"
	"  LsqKind = {Private|Shared} (Default = 20)\n"
	"      Load-store queue sharing among threads.\n"
	"  LsqSize = <num_uops> (Default = 20)\n"
	"      Load-store queue size in number of uops (if private, per-thread LSQ size).\n"
	"  RfKind = {Private|Shared} (Default = Private)\n"
	"      Register file sharing among threads.\n"
	"  RfIntSize = <entries> (Default = 80)\n"
	"      Number of integer physical register (if private, per-thread).\n"
	"  RfFpSize = <entries> (Default = 40)\n"
	"      Number of floating-point physical registers (if private, per-thread).\n"
	"  RfXmmSize = <entries> (Default = 40)\n"
	"      Number of XMM physical registers (if private, per-thread).\n"
	"\n"
	"Section '[ TraceCache ]':\n"
	"\n"
	"  Present = {t|f} (Default = False)\n"
	"      If true, a trace cache is included in the model. If false, the rest of the\n"
	"      options in this section are ignored.\n"
	"  Sets = <num_sets> (Default = 64)\n"
	"      Number of sets in the trace cache.\n"
	"  Assoc = <num_ways> (Default = 4)\n"
	"      Associativity of the trace cache. The product Sets * Assoc is the total\n"
	"      number of traces that can be stored in the trace cache.\n"
	"  TraceSize = <num_uops> (Default = 16)\n"
	"      Maximum size of a trace of uops.\n"
	"  BranchMax = <num_branches> (Default = 3)\n"
	"      Maximum number of branches contained in a trace.\n"
	"  QueueSize = <num_uops> (Default = 32)\n"
	"      Size of the trace queue size in uops.\n"
	"\n"
	"Section '[ FunctionalUnits ]':\n"
	"\n"
	"  The possible variables in this section follow the format\n"
	"      <func_unit>.<field> = <value>\n"
	"  where <func_unit> refers to a functional unit type, and <field> refers to a\n"
	"  property of it. Possible values for <func_unit> are:\n"
	"\n"
	"      IntAdd      Integer adder\n"
	"      IntMult     Integer multiplier\n"
	"      IntDiv      Integer divider\n"
	"\n"
	"      EffAddr     Operator for effective address computations\n"
	"      Logic       Operator for logic operations\n"
	"\n"
	"      FloatSimple    Simple floating-point operations\n"
	"      FloatAdd       Floating-point adder\n"
	"      FloatComp      Floating-point comparator\n"
	"      FloatMult      Floating-point multiplier\n"
	"      FloatDiv       Floating-point divider\n"
	"      FloatComplex   Operator for complex floating-point computations\n"
	"\n"
	"      XMMIntAdd      XMM integer adder\n"
	"      XMMIntMult     XMM integer multiplier\n"
	"      XMMIntDiv      XMM integer Divider\n"
	"\n"
	"      XMMLogic       XMM logic operations\n"
	"\n"
	"      XMMFloatAdd       XMM floating-point adder\n"
	"      XMMFloatComp      XMM floating-point comparator\n"
	"      XMMFloatMult      XMM floating-point multiplier\n"
	"      XMMFloatDiv       XMM floating-point divider\n"
	"      XMMFloatConv      XMM floating-point converter\n"
	"      XMMFloatComplex   Complex XMM floating-point operations\n"
	"\n"
	"  Possible values for <field> are:\n"
	"      Count       Number of functional units of a given kind.\n"
	"      OpLat       Latency of the operator.\n"
	"      IssueLat    Latency since an instruction was issued until the functional\n"
	"                  unit is available for the next use. For pipelined operators,\n"
	"                  IssueLat is smaller than OpLat.\n"
	"\n"
	"Section '[ BranchPredictor ]':\n"
	"\n"
	"  Kind = {Perfect|Taken|NotTaken|Bimodal|TwoLevel|Combined} (Default = TwoLevel)\n"
	"      Branch predictor type.\n"
	"  BTB.Sets = <num_sets> (Default = 256)\n"
	"      Number of sets in the BTB.\n"
	"  BTB.Assoc = <num_ways) (Default = 4)\n"
	"      BTB associativity.\n"
	"  Bimod.Size = <entries> (Default = 1024)\n"
	"      Number of entries of the bimodal branch predictor.\n"
	"  Choice.Size = <entries> (Default = 1024)\n"
	"      Number of entries for the choice predictor.\n"
	"  RAS.Size = <entries> (Default = 32)\n"
	"      Number of entries of the return address stack (RAS).\n"
	"  TwoLevel.L1Size = <entries> (Default = 1)\n"
	"      For the two-level adaptive predictor, level 1 size.\n"
	"  TwoLevel.L2Size = <entries> (Default = 1024)\n"
	"      For the two-level adaptive predictor, level 2 size.\n"
	"  TwoLevel.HistorySize = <size> (Default = 8)\n"
	"      For the two-level adaptive predictor, level 2 history size.\n"
	"\n";


/* Trace */
int x86_trace_category;


/* Configuration file and parameters */

char *x86_config_file_name = "";
char *x86_cpu_report_file_name = "";

int x86_cpu_frequency = 1000;
int x86_cpu_num_cores = 1;
int x86_cpu_num_threads = 1;

long long x86_cpu_fast_forward_count;

int x86_cpu_context_quantum;
int x86_cpu_thread_quantum;
int x86_cpu_thread_switch_penalty;

char *x86_cpu_recover_kind_map[] = { "Writeback", "Commit" };
enum x86_cpu_recover_kind_t x86_cpu_recover_kind;
int x86_cpu_recover_penalty;

char *x86_cpu_fetch_kind_map[] = { "Shared", "TimeSlice", "SwitchOnEvent" };
enum x86_cpu_fetch_kind_t x86_cpu_fetch_kind;

int x86_cpu_decode_width;

char *x86_cpu_dispatch_kind_map[] = { "Shared", "TimeSlice" };
enum x86_cpu_dispatch_kind_t x86_cpu_dispatch_kind;
int x86_cpu_dispatch_width;

char *x86_cpu_issue_kind_map[] = { "Shared", "TimeSlice" };
enum x86_cpu_issue_kind_t x86_cpu_issue_kind;
int x86_cpu_issue_width;

char *x86_cpu_commit_kind_map[] = { "Shared", "TimeSlice" };
enum x86_cpu_commit_kind_t x86_cpu_commit_kind;
int x86_cpu_commit_width;

int x86_cpu_occupancy_stats;




/*
 * Private Functions
 */

static char *x86_cpu_err_fast_forward =
	"\tThe number of instructions specified in the x86 CPU configuration file\n"
	"\tfor fast-forward (functional) execution has caused all contexts to end\n"
	"\tbefore the timing simulation could start. Please decrease the number\n"
	"\tof fast-forward instructions and retry.\n";




/*
 * Public Functions
 */

/* Version of x86 trace producer.
 * See 'src/visual/x86/cpu.c' for x86 trace consumer. */

#define X86_TRACE_VERSION_MAJOR		1
#define X86_TRACE_VERSION_MINOR		671


void X86CpuReadConfig(void)
{
	struct config_t *config;
	char *section;

	/* Open file */
	config = config_create(x86_config_file_name);
	if (*x86_config_file_name)
		config_load(config);


	/* General configuration */

	section = "General";

	x86_cpu_frequency = config_read_int(config, section, "Frequency", x86_cpu_frequency);
	if (!IN_RANGE(x86_cpu_frequency, 1, ESIM_MAX_FREQUENCY))
		fatal("%s: invalid value for 'Frequency'.", x86_config_file_name);

	x86_cpu_num_cores = config_read_int(config, section, "Cores", x86_cpu_num_cores);
	x86_cpu_num_threads = config_read_int(config, section, "Threads", x86_cpu_num_threads);

	x86_cpu_fast_forward_count = config_read_llint(config, section, "FastForward", 0);

	x86_cpu_context_quantum = config_read_int(config, section, "ContextQuantum", 100000);
	x86_cpu_thread_quantum = config_read_int(config, section, "ThreadQuantum", 1000);
	x86_cpu_thread_switch_penalty = config_read_int(config, section, "ThreadSwitchPenalty", 0);

	x86_cpu_recover_kind = config_read_enum(config, section, "RecoverKind", x86_cpu_recover_kind_writeback, x86_cpu_recover_kind_map, 2);
	x86_cpu_recover_penalty = config_read_int(config, section, "RecoverPenalty", 0);

	x86_emu_process_prefetch_hints = config_read_bool(config, section, "ProcessPrefetchHints", 1);
	prefetch_history_size = config_read_int(config, section, "PrefetchHistorySize", 10);


	/* Section '[ Pipeline ]' */

	section = "Pipeline";

	x86_cpu_fetch_kind = config_read_enum(config, section, "FetchKind", x86_cpu_fetch_kind_timeslice, x86_cpu_fetch_kind_map, 3);

	x86_cpu_decode_width = config_read_int(config, section, "DecodeWidth", 4);

	x86_cpu_dispatch_kind = config_read_enum(config, section, "DispatchKind", x86_cpu_dispatch_kind_timeslice, x86_cpu_dispatch_kind_map, 2);
	x86_cpu_dispatch_width = config_read_int(config, section, "DispatchWidth", 4);

	x86_cpu_issue_kind = config_read_enum(config, section, "IssueKind", x86_cpu_issue_kind_timeslice, x86_cpu_issue_kind_map, 2);
	x86_cpu_issue_width = config_read_int(config, section, "IssueWidth", 4);

	x86_cpu_commit_kind = config_read_enum(config, section, "CommitKind", x86_cpu_commit_kind_shared, x86_cpu_commit_kind_map, 2);
	x86_cpu_commit_width = config_read_int(config, section, "CommitWidth", 4);

	x86_cpu_occupancy_stats = config_read_bool(config, section, "OccupancyStats", 0);


	/* Section '[ Queues ]' */

	section = "Queues";

	x86_fetch_queue_size = config_read_int(config, section, "FetchQueueSize", 64);

	x86_uop_queue_size = config_read_int(config, section, "UopQueueSize", 32);

	x86_rob_kind = config_read_enum(config, section, "RobKind",
			x86_rob_kind_private, x86_rob_kind_map, 2);
	x86_rob_size = config_read_int(config, section, "RobSize", 64);

	x86_iq_kind = config_read_enum(config, section, "IqKind",
			x86_iq_kind_private, x86_iq_kind_map, 2);
	x86_iq_size = config_read_int(config, section, "IqSize", 40);

	x86_lsq_kind = config_read_enum(config, section, "LsqKind",
			x86_lsq_kind_private, x86_lsq_kind_map, 2);
	x86_lsq_size = config_read_int(config, section, "LsqSize", 20);

	/* Register file */
	X86ReadRegFileConfig(config);

	/* Functional Units */
	X86ReadFunctionalUnitsConfig(config);

	/* Branch predictor */
	X86ReadBranchPredConfig(config);

	/* Trace Cache */
	X86ReadTraceCacheConfig(config);

	/* Close file */
	config_check(config);
	config_free(config);
}


void X86CpuInit(void)
{
	/* Classes */
	CLASS_REGISTER(X86Cpu);
	CLASS_REGISTER(X86Core);
	CLASS_REGISTER(X86Thread);

	/* Trace */
	x86_trace_category = trace_new_category();
}


/* Finalization */
void X86CpuDone(void)
{
}





/*
 * Class 'X86Cpu'
 */

CLASS_IMPLEMENTATION(X86Cpu);


void X86CpuCreate(X86Cpu *self, X86Emu *emu)
{
	X86Core *core;
	X86Thread *thread;

	char name[MAX_STRING_SIZE];

	int i;
	int j;

	/* Parent */
	TimingCreate(asTiming(self));
	
	/* Frequency */
	asTiming(self)->frequency = x86_cpu_frequency;
	asTiming(self)->frequency_domain = esim_new_domain(x86_cpu_frequency);

	/* Initialize */
	self->emu = emu;
	self->uop_trace_list = linked_list_create();

	/* Create cores */
	self->cores = xcalloc(x86_cpu_num_cores, sizeof(X86Core *));
	for (i = 0; i < x86_cpu_num_cores; i++)
		self->cores[i] = new(X86Core, self);

	/* Assign names and IDs to cores and threads */
	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		core = self->cores[i];
		snprintf(name, sizeof name, "c%d", i);
		X86CoreSetName(core, name);
		core->id = i;
		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			thread = core->threads[j];
			snprintf(name, sizeof name, "c%dt%d", i, j);
			X86ThreadSetName(thread, name);
			thread->id_in_core = j;
			thread->id_in_cpu = i * x86_cpu_num_threads + j;
		}
	}

	/* Virtual functions */
	asObject(self)->Dump = X86CpuDump;
	asTiming(self)->DumpSummary = X86CpuDumpSummary;
	asTiming(self)->Run = X86CpuRun;
	asTiming(self)->MemConfigCheck = X86CpuMemConfigCheck;
	asTiming(self)->MemConfigDefault = X86CpuMemConfigDefault;
	asTiming(self)->MemConfigParseEntry = X86CpuMemConfigParseEntry;

	/* Trace */
	x86_trace_header("x86.init version=\"%d.%d\" num_cores=%d num_threads=%d\n",
		X86_TRACE_VERSION_MAJOR, X86_TRACE_VERSION_MINOR,
		x86_cpu_num_cores, x86_cpu_num_threads);
}


void X86CpuDestroy(X86Cpu *self)
{
	int i;
	FILE *f;

	/* Dump report */
	f = file_open_for_write(x86_cpu_report_file_name);
	if (f)
	{
		X86CpuDumpReport(self, f);
		fclose(f);
	}

	/* Uop trace list */
	X86CpuEmptyTraceList(self);
	linked_list_free(self->uop_trace_list);

	/* Free cores */
	for (i = 0; i < x86_cpu_num_cores; i++)
		delete(self->cores[i]);
	free(self->cores);
}


void X86CpuDump(Object *self, FILE *f)
{
	X86Cpu *cpu = asX86Cpu(self);
	X86Core *core;
	X86Thread *thread;

	int i;
	int j;
	
	/* General information */
	fprintf(f, "\n");
	fprintf(f, "LastDump = %lld   ; Cycle of last dump\n", cpu->last_dump);
	fprintf(f, "IPCLastDump = %.4g   ; IPC since last dump\n",
			asTiming(cpu)->cycle - cpu->last_dump > 0 ?
			(double) (cpu->num_committed_uinst - cpu->last_committed)
			/ (asTiming(cpu)->cycle - cpu->last_dump) : 0);
	fprintf(f, "\n");

	/* Cores */
	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		core = cpu->cores[i];
		fprintf(f, "-------\n");
		fprintf(f, "Core %d\n", core->id);
		fprintf(f, "-------\n\n");
		
		fprintf(f, "Event Queue:\n");
		x86_uop_linked_list_dump(core->event_queue, f);

		fprintf(f, "Reorder Buffer:\n");
		X86CoreDumpROB(core, f);

		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			thread = core->threads[j];

			fprintf(f, "----------------------\n");
			fprintf(f, "Thread %d (in core %d)\n", j, i);
			fprintf(f, "----------------------\n\n");
			
			fprintf(f, "Fetch queue:\n");
			x86_uop_list_dump(thread->fetch_queue, f);

			fprintf(f, "Uop queue:\n");
			x86_uop_list_dump(thread->uop_queue, f);

			fprintf(f, "Instruction Queue:\n");
			x86_uop_linked_list_dump(thread->iq, f);

			fprintf(f, "Load Queue:\n");
			x86_uop_linked_list_dump(thread->lq, f);

			fprintf(f, "Store Queue:\n");
			x86_uop_linked_list_dump(thread->sq, f);

			X86ThreadDumpRegFile(thread, f);
			if (thread->ctx)
				fprintf(f, "MappedContext = %d\n", thread->ctx->pid);
			
			fprintf(f, "\n");
		}
	}

	/* Register last dump */
	cpu->last_dump = asTiming(cpu)->cycle;
	cpu->last_committed = cpu->num_committed_uinst;

	/* End */
	fprintf(f, "\n\n");
}


void X86CpuDumpSummary(Timing *self, FILE *f)
{
	X86Cpu *cpu = asX86Cpu(self);

	double inst_per_cycle;
	double uinst_per_cycle;
	double branch_acc;

	/* Calculate statistics */
	inst_per_cycle = asTiming(cpu)->cycle ? (double) cpu->num_committed_inst
			/ asTiming(cpu)->cycle : 0.0;
	uinst_per_cycle = asTiming(cpu)->cycle ? (double) cpu->num_committed_uinst
			/ asTiming(cpu)->cycle : 0.0;
	branch_acc = cpu->num_branch_uinst ? (double) (cpu->num_branch_uinst -
			cpu->num_mispred_branch_uinst) / cpu->num_branch_uinst : 0.0;

	/* Print statistics */
	fprintf(f, "FastForwardInstructions = %lld\n", cpu->num_fast_forward_inst);
	fprintf(f, "CommittedInstructions = %lld\n", cpu->num_committed_inst);
	fprintf(f, "CommittedInstructionsPerCycle = %.4g\n", inst_per_cycle);
	fprintf(f, "CommittedMicroInstructions = %lld\n", cpu->num_committed_uinst);
	fprintf(f, "CommittedMicroInstructionsPerCycle = %.4g\n", uinst_per_cycle);
	fprintf(f, "BranchPredictionAccuracy = %.4g\n", branch_acc);

	/* Call parent */
	TimingDumpSummary(self, f);
}


#define DUMP_DISPATCH_STAT(NAME) { \
	fprintf(f, "Dispatch.Stall." #NAME " = %lld\n", \
			core->dispatch_stall[x86_dispatch_stall_##NAME]); \
}

#define DUMP_CORE_STRUCT_STATS(NAME, ITEM) { \
	fprintf(f, #NAME ".Size = %d\n", (int) x86_##ITEM##_size * x86_cpu_num_threads); \
	if (x86_cpu_occupancy_stats) \
		fprintf(f, #NAME ".Occupancy = %.2f\n", asTiming(self)->cycle ? \
				(double) core->ITEM##_occupancy / asTiming(self)->cycle : 0.0); \
	fprintf(f, #NAME ".Full = %lld\n", core->ITEM##_full); \
	fprintf(f, #NAME ".Reads = %lld\n", core->ITEM##_reads); \
	fprintf(f, #NAME ".Writes = %lld\n", core->ITEM##_writes); \
}

#define DUMP_THREAD_STRUCT_STATS(NAME, ITEM) { \
	fprintf(f, #NAME ".Size = %d\n", (int) x86_##ITEM##_size); \
	if (x86_cpu_occupancy_stats) \
		fprintf(f, #NAME ".Occupancy = %.2f\n", asTiming(self)->cycle ? \
				(double) thread->ITEM##_occupancy / asTiming(self)->cycle : 0.0); \
	fprintf(f, #NAME ".Full = %lld\n", thread->ITEM##_full); \
	fprintf(f, #NAME ".Reads = %lld\n", thread->ITEM##_reads); \
	fprintf(f, #NAME ".Writes = %lld\n", thread->ITEM##_writes); \
}


/* Dump the CPU configuration */
static void X86DumpCpuConfig(FILE *f)
{
	/* General configuration */
	fprintf(f, "[ Config.General ]\n");
	fprintf(f, "Frequency = %d\n", x86_cpu_frequency);
	fprintf(f, "Cores = %d\n", x86_cpu_num_cores);
	fprintf(f, "Threads = %d\n", x86_cpu_num_threads);
	fprintf(f, "FastForward = %lld\n", x86_cpu_fast_forward_count);
	fprintf(f, "ContextQuantum = %d\n", x86_cpu_context_quantum);
	fprintf(f, "ThreadQuantum = %d\n", x86_cpu_thread_quantum);
	fprintf(f, "ThreadSwitchPenalty = %d\n", x86_cpu_thread_switch_penalty);
	fprintf(f, "RecoverKind = %s\n", x86_cpu_recover_kind_map[x86_cpu_recover_kind]);
	fprintf(f, "RecoverPenalty = %d\n", x86_cpu_recover_penalty);
	fprintf(f, "ProcessPrefetchHints = %d\n", x86_emu_process_prefetch_hints);
	fprintf(f, "PrefetchHistorySize = %d\n", prefetch_history_size);
	fprintf(f, "\n");

	/* Pipeline */
	fprintf(f, "[ Config.Pipeline ]\n");
	fprintf(f, "FetchKind = %s\n", x86_cpu_fetch_kind_map[x86_cpu_fetch_kind]);
	fprintf(f, "DecodeWidth = %d\n", x86_cpu_decode_width);
	fprintf(f, "DispatchKind = %s\n", x86_cpu_dispatch_kind_map[x86_cpu_dispatch_kind]);
	fprintf(f, "DispatchWidth = %d\n", x86_cpu_dispatch_width);
	fprintf(f, "IssueKind = %s\n", x86_cpu_issue_kind_map[x86_cpu_issue_kind]);
	fprintf(f, "IssueWidth = %d\n", x86_cpu_issue_width);
	fprintf(f, "CommitKind = %s\n", x86_cpu_commit_kind_map[x86_cpu_commit_kind]);
	fprintf(f, "CommitWidth = %d\n", x86_cpu_commit_width);
	fprintf(f, "OccupancyStats = %s\n", x86_cpu_occupancy_stats ? "True" : "False");
	fprintf(f, "\n");

	/* Queues */
	fprintf(f, "[ Config.Queues ]\n");
	fprintf(f, "FetchQueueSize = %d\n", x86_fetch_queue_size);
	fprintf(f, "UopQueueSize = %d\n", x86_uop_queue_size);
	fprintf(f, "RobKind = %s\n", x86_rob_kind_map[x86_rob_kind]);
	fprintf(f, "RobSize = %d\n", x86_rob_size);
	fprintf(f, "IqKind = %s\n", x86_iq_kind_map[x86_iq_kind]);
	fprintf(f, "IqSize = %d\n", x86_iq_size);
	fprintf(f, "LsqKind = %s\n", x86_lsq_kind_map[x86_lsq_kind]);
	fprintf(f, "LsqSize = %d\n", x86_lsq_size);
	fprintf(f, "RfKind = %s\n", x86_reg_file_kind_map[x86_reg_file_kind]);
	fprintf(f, "RfIntSize = %d\n", x86_reg_file_int_size);
	fprintf(f, "RfFpSize = %d\n", x86_reg_file_fp_size);
	fprintf(f, "\n");

	/* Trace Cache */
	fprintf(f, "[ Config.TraceCache ]\n");
	fprintf(f, "Present = %s\n", x86_trace_cache_present ? "True" : "False");
	fprintf(f, "Sets = %d\n", x86_trace_cache_num_sets);
	fprintf(f, "Assoc = %d\n", x86_trace_cache_assoc);
	fprintf(f, "TraceSize = %d\n", x86_trace_cache_trace_size);
	fprintf(f, "BranchMax = %d\n", x86_trace_cache_branch_max);
	fprintf(f, "QueueSize = %d\n", x86_trace_cache_queue_size);
	fprintf(f, "\n");

	/* Functional units */
	X86DumpFunctionalUnitsConfig(f);

	/* Branch Predictor */
	fprintf(f, "[ Config.BranchPredictor ]\n");
	fprintf(f, "Kind = %s\n", x86_bpred_kind_map[x86_bpred_kind]);
	fprintf(f, "BTB.Sets = %d\n", x86_bpred_btb_sets);
	fprintf(f, "BTB.Assoc = %d\n", x86_bpred_btb_assoc);
	fprintf(f, "Bimod.Size = %d\n", x86_bpred_bimod_size);
	fprintf(f, "Choice.Size = %d\n", x86_bpred_choice_size);
	fprintf(f, "RAS.Size = %d\n", x86_bpred_ras_size);
	fprintf(f, "TwoLevel.L1Size = %d\n", x86_bpred_twolevel_l1size);
	fprintf(f, "TwoLevel.L2Size = %d\n", x86_bpred_twolevel_l2size);
	fprintf(f, "TwoLevel.HistorySize = %d\n", x86_bpred_twolevel_hist_size);
	fprintf(f, "\n");

	/* End of configuration */
	fprintf(f, "\n");

}


void X86CpuDumpReport(X86Cpu *self, FILE *f)
{
	X86Emu *emu = self->emu;
	X86Core *core;
	X86Thread *thread;

	long long now;

	int i;
	int j;

	/* Get CPU timer value */
	now = m2s_timer_get_value(asEmu(emu)->timer);

	/* Dump CPU configuration */
	fprintf(f, ";\n; CPU Configuration\n;\n\n");
	X86DumpCpuConfig(f);

	/* Report for the complete processor */
	fprintf(f, ";\n; Simulation Statistics\n;\n\n");
	fprintf(f, "; Global statistics\n");
	fprintf(f, "[ Global ]\n\n");
	fprintf(f, "Cycles = %lld\n", asTiming(self)->cycle);
	fprintf(f, "Time = %.2f\n", (double) now / 1000000);
	fprintf(f, "CyclesPerSecond = %.0f\n", now ?
			(double) asTiming(self)->cycle / now * 1000000 : 0.0);
	fprintf(f, "MemoryUsed = %lu\n", (long) mem_mapped_space);
	fprintf(f, "MemoryUsedMax = %lu\n", (long) mem_max_mapped_space);
	fprintf(f, "\n");

	/* Dispatch stage */
	fprintf(f, "; Dispatch stage\n");
	X86CpuDumpUopReport(self, f, self->num_dispatched_uinst_array,
			"Dispatch", x86_cpu_dispatch_width);

	/* Issue stage */
	fprintf(f, "; Issue stage\n");
	X86CpuDumpUopReport(self, f, self->num_issued_uinst_array,
			"Issue", x86_cpu_issue_width);

	/* Commit stage */
	fprintf(f, "; Commit stage\n");
	X86CpuDumpUopReport(self, f, self->num_committed_uinst_array,
			"Commit", x86_cpu_commit_width);

	/* Committed branches */
	fprintf(f, "; Committed branches\n");
	fprintf(f, ";    Branches - Number of committed control uops\n");
	fprintf(f, ";    Squashed - Number of mispredicted uops squashed from the ROB\n");
	fprintf(f, ";    Mispred - Number of mispredicted branches in the correct path\n");
	fprintf(f, ";    PredAcc - Prediction accuracy\n");
	fprintf(f, "Commit.Branches = %lld\n", self->num_branch_uinst);
	fprintf(f, "Commit.Squashed = %lld\n", self->num_squashed_uinst);
	fprintf(f, "Commit.Mispred = %lld\n", self->num_mispred_branch_uinst);
	fprintf(f, "Commit.PredAcc = %.4g\n", self->num_branch_uinst ?
		(double) (self->num_branch_uinst - self->num_mispred_branch_uinst) / self->num_branch_uinst : 0.0);
	fprintf(f, "\n");

	/* Report for each core */
	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		/* Core */
		core = self->cores[i];
		fprintf(f, "\n; Statistics for core %d\n", core->id);
		fprintf(f, "[ c%d ]\n\n", core->id);

		/* Functional units */
		X86CoreDumpFunctionalUnitsReport(core, f);

		/* Dispatch slots */
		if (x86_cpu_dispatch_kind == x86_cpu_dispatch_kind_timeslice)
		{
			fprintf(f, "; Dispatch slots usage (sum = cycles * dispatch width)\n");
			fprintf(f, ";    used - dispatch slot was used by a non-spec uop\n");
			fprintf(f, ";    spec - used by a mispeculated uop\n");
			fprintf(f, ";    ctx - no context allocated to thread\n");
			fprintf(f, ";    uopq,rob,iq,lsq,rename - no space in structure\n");
			DUMP_DISPATCH_STAT(used);
			DUMP_DISPATCH_STAT(spec);
			DUMP_DISPATCH_STAT(uop_queue);
			DUMP_DISPATCH_STAT(rob);
			DUMP_DISPATCH_STAT(iq);
			DUMP_DISPATCH_STAT(lsq);
			DUMP_DISPATCH_STAT(rename);
			DUMP_DISPATCH_STAT(ctx);
			fprintf(f, "\n");
		}

		/* Dispatch stage */
		fprintf(f, "; Dispatch stage\n");
		X86CpuDumpUopReport(self, f, core->num_dispatched_uinst_array,
				"Dispatch", x86_cpu_dispatch_width);

		/* Issue stage */
		fprintf(f, "; Issue stage\n");
		X86CpuDumpUopReport(self, f, core->num_issued_uinst_array,
				"Issue", x86_cpu_issue_width);

		/* Commit stage */
		fprintf(f, "; Commit stage\n");
		X86CpuDumpUopReport(self, f, core->num_committed_uinst_array,
				"Commit", x86_cpu_commit_width);

		/* Committed branches */
		fprintf(f, "; Committed branches\n");
		fprintf(f, "Commit.Branches = %lld\n", core->num_branch_uinst);
		fprintf(f, "Commit.Squashed = %lld\n", core->num_squashed_uinst);
		fprintf(f, "Commit.Mispred = %lld\n", core->num_mispred_branch_uinst);
		fprintf(f, "Commit.PredAcc = %.4g\n", core->num_branch_uinst ?
				(double) (core->num_branch_uinst -
				core->num_mispred_branch_uinst)
				/ core->num_branch_uinst : 0.0);
		fprintf(f, "\n");

		/* Occupancy stats */
		fprintf(f, "; Structure statistics (reorder buffer, instruction queue,\n");
		fprintf(f, "; load-store queue, and integer/floating-point register file)\n");
		fprintf(f, ";    Size - Available size\n");
		fprintf(f, ";    Occupancy - Average number of occupied entries\n");
		fprintf(f, ";    Full - Number of cycles when the structure was full\n");
		fprintf(f, ";    Reads, Writes - Accesses to the structure\n");
		if (x86_rob_kind == x86_rob_kind_shared)
			DUMP_CORE_STRUCT_STATS(ROB, rob);
		if (x86_iq_kind == x86_iq_kind_shared)
		{
			DUMP_CORE_STRUCT_STATS(IQ, iq);
			fprintf(f, "IQ.WakeupAccesses = %lld\n", core->iq_wakeup_accesses);
		}
		if (x86_lsq_kind == x86_lsq_kind_shared)
			DUMP_CORE_STRUCT_STATS(LSQ, lsq);
		if (x86_reg_file_kind == x86_reg_file_kind_shared)
		{
			DUMP_CORE_STRUCT_STATS(RF_Int, reg_file_int);
			DUMP_CORE_STRUCT_STATS(RF_Fp, reg_file_fp);
		}
		fprintf(f, "\n");

		/* Report for each thread */
		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			thread = core->threads[j];
			fprintf(f, "\n; Statistics for core %d - thread %d\n",
					core->id, thread->id_in_core);
			fprintf(f, "[ %s ]\n\n", thread->name);

			/* Dispatch stage */
			fprintf(f, "; Dispatch stage\n");
			X86CpuDumpUopReport(self, f, thread->num_dispatched_uinst_array,
					"Dispatch", x86_cpu_dispatch_width);

			/* Issue stage */
			fprintf(f, "; Issue stage\n");
			X86CpuDumpUopReport(self, f, thread->num_issued_uinst_array,
					"Issue", x86_cpu_issue_width);

			/* Commit stage */
			fprintf(f, "; Commit stage\n");
			X86CpuDumpUopReport(self, f, thread->num_committed_uinst_array,
					"Commit", x86_cpu_commit_width);

			/* Committed branches */
			fprintf(f, "; Committed branches\n");
			fprintf(f, "Commit.Branches = %lld\n", thread->num_branch_uinst);
			fprintf(f, "Commit.Squashed = %lld\n", thread->num_squashed_uinst);
			fprintf(f, "Commit.Mispred = %lld\n", thread->num_mispred_branch_uinst);
			fprintf(f, "Commit.PredAcc = %.4g\n", thread->num_branch_uinst ?
				(double) (thread->num_branch_uinst - thread->num_mispred_branch_uinst) / thread->num_branch_uinst : 0.0);
			fprintf(f, "\n");

			/* Occupancy stats */
			fprintf(f, "; Structure statistics (reorder buffer, instruction queue, load-store queue,\n");
			fprintf(f, "; integer/floating-point register file, and renaming table)\n");
			if (x86_rob_kind == x86_rob_kind_private)
				DUMP_THREAD_STRUCT_STATS(ROB, rob);
			if (x86_iq_kind == x86_iq_kind_private)
			{
				DUMP_THREAD_STRUCT_STATS(IQ, iq);
				fprintf(f, "IQ.WakeupAccesses = %lld\n", thread->iq_wakeup_accesses);
			}
			if (x86_lsq_kind == x86_lsq_kind_private)
				DUMP_THREAD_STRUCT_STATS(LSQ, lsq);
			if (x86_reg_file_kind == x86_reg_file_kind_private)
			{
				DUMP_THREAD_STRUCT_STATS(RF_Int, reg_file_int);
				DUMP_THREAD_STRUCT_STATS(RF_Fp, reg_file_fp);
			}
			fprintf(f, "RAT.IntReads = %lld\n", thread->rat_int_reads);
			fprintf(f, "RAT.IntWrites = %lld\n", thread->rat_int_writes);
			fprintf(f, "RAT.FpReads = %lld\n", thread->rat_fp_reads);
			fprintf(f, "RAT.FpWrites = %lld\n", thread->rat_fp_writes);
			fprintf(f, "BTB.Reads = %lld\n", thread->btb_reads);
			fprintf(f, "BTB.Writes = %lld\n", thread->btb_writes);
			fprintf(f, "\n");

			/* Trace cache stats */
			if (thread->trace_cache)
				X86ThreadDumpTraceCacheReport(thread, f);
		}
	}
}


void X86CpuDumpUopReport(X86Cpu *self, FILE *f, long long *uop_stats,
		char *prefix, int peak_ipc)
{
	long long uinst_int_count = 0;
	long long uinst_logic_count = 0;
	long long uinst_fp_count = 0;
	long long uinst_mem_count = 0;
	long long uinst_ctrl_count = 0;
	long long uinst_total = 0;

	char *name;
	enum x86_uinst_flag_t flags;
	int i;

	for (i = 0; i < x86_uinst_opcode_count; i++)
	{
		name = x86_uinst_info[i].name;
		flags = x86_uinst_info[i].flags;

		fprintf(f, "%s.Uop.%s = %lld\n", prefix, name, uop_stats[i]);
		if (flags & X86_UINST_INT)
			uinst_int_count += uop_stats[i];
		if (flags & X86_UINST_LOGIC)
			uinst_logic_count += uop_stats[i];
		if (flags & X86_UINST_FP)
			uinst_fp_count += uop_stats[i];
		if (flags & X86_UINST_MEM)
			uinst_mem_count += uop_stats[i];
		if (flags & X86_UINST_CTRL)
			uinst_ctrl_count += uop_stats[i];
		uinst_total += uop_stats[i];
	}
	fprintf(f, "%s.Integer = %lld\n", prefix, uinst_int_count);
	fprintf(f, "%s.Logic = %lld\n", prefix, uinst_logic_count);
	fprintf(f, "%s.FloatingPoint = %lld\n", prefix, uinst_fp_count);
	fprintf(f, "%s.Memory = %lld\n", prefix, uinst_mem_count);
	fprintf(f, "%s.Ctrl = %lld\n", prefix, uinst_ctrl_count);
	fprintf(f, "%s.WndSwitch = %lld\n", prefix,
			uop_stats[x86_uinst_call] + uop_stats[x86_uinst_ret]);
	fprintf(f, "%s.Total = %lld\n", prefix, uinst_total);
	fprintf(f, "%s.IPC = %.4g\n", prefix, asTiming(self)->cycle ?
			(double) uinst_total / asTiming(self)->cycle : 0.0);
	fprintf(f, "%s.DutyCycle = %.4g\n", prefix,
			asTiming(self)->cycle && peak_ipc ?
			(double) uinst_total / asTiming(self)->cycle / peak_ipc : 0.0);
	fprintf(f, "\n");
}


int X86CpuRun(Timing *self)
{
	X86Cpu *cpu = asX86Cpu(self);
	X86Emu *emu = cpu->emu;

	/* Stop if no context is running */
	if (emu->finished_list_count >= emu->context_list_count)
		return FALSE;

	/* Fast-forward simulation */
	if (x86_cpu_fast_forward_count && asEmu(emu)->instructions
			< x86_cpu_fast_forward_count)
		X86CpuFastForward(cpu);

	/* Stop if maximum number of CPU instructions exceeded */
	if (x86_emu_max_inst && cpu->num_committed_inst >=
			x86_emu_max_inst - x86_cpu_fast_forward_count)
		esim_finish = esim_finish_x86_max_inst;

	/* Stop if maximum number of cycles exceeded */
	if (x86_emu_max_cycles && self->cycle >= x86_emu_max_cycles)
		esim_finish = esim_finish_x86_max_cycles;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* One more cycle of x86 timing simulation */
	self->cycle++;

	/* Empty uop trace list. This dumps the last trace line for instructions
	 * that were freed in the previous simulation cycle. */
	X86CpuEmptyTraceList(cpu);

	/* Processor stages */
	X86CpuRunStages(cpu);

	/* Process host threads generating events */
	X86EmuProcessEvents(emu);

	/* Still simulating */
	return TRUE;
}


void X86CpuRunStages(X86Cpu *self)
{
	/* Context scheduler */
	X86CpuSchedule(self);

	/* Stages */
	X86CpuCommit(self);
	X86CpuWriteback(self);
	X86CpuIssue(self);
	X86CpuDispatch(self);
	X86CpuDecode(self);
	X86CpuFetch(self);

	/* Update stats for structures occupancy */
	if (x86_cpu_occupancy_stats)
		X86CpuUpdateOccupancyStats(self);
}


/* Run fast-forward simulation */
void X86CpuFastForward(X86Cpu *self)
{
	X86Emu *emu = self->emu;

	/* Fast-forward simulation. Run 'x86_cpu_fast_forward' iterations of the x86
	 * emulation loop until any simulation end reason is detected. */
	while (asEmu(emu)->instructions < x86_cpu_fast_forward_count && !esim_finish)
		X86EmuRun(asEmu(emu));

	/* Record number of instructions in fast-forward execution. */
	self->num_fast_forward_inst = asEmu(emu)->instructions;

	/* Output warning if simulation finished during fast-forward execution. */
	if (esim_finish)
		warning("x86 fast-forwarding finished simulation.\n%s",
				x86_cpu_err_fast_forward);
}


void X86CpuAddToTraceList(X86Cpu *self, struct x86_uop_t *uop)
{
	assert(x86_tracing());
	assert(!uop->in_uop_trace_list);

	uop->in_uop_trace_list = 1;
	linked_list_add(self->uop_trace_list, uop);
}


void X86CpuEmptyTraceList(X86Cpu *self)
{
	X86Thread *thread;
	X86Core *core;
	struct linked_list_t *uop_trace_list;
	struct x86_uop_t *uop;

	uop_trace_list = self->uop_trace_list;
	while (uop_trace_list->count)
	{
		/* Remove from list */
		linked_list_head(uop_trace_list);
		uop = linked_list_get(uop_trace_list);
		thread = uop->thread;
		core = thread->core;
		linked_list_remove(uop_trace_list);
		assert(uop->in_uop_trace_list);

		/* Trace */
		x86_trace("x86.end_inst id=%lld core=%d\n",
			uop->id_in_core, core->id);

		/* Free uop */
		uop->in_uop_trace_list = 0;
		x86_uop_free_if_not_queued(uop);
	}
}


#define UPDATE_THREAD_OCCUPANCY_STATS(ITEM) { \
	thread->ITEM##_occupancy += thread->ITEM##_count; \
	if (thread->ITEM##_count == x86_##ITEM##_size) \
		thread->ITEM##_full++; \
}


#define UPDATE_CORE_OCCUPANCY_STATS(ITEM) { \
	core->ITEM##_occupancy += core->ITEM##_count; \
	if (core->ITEM##_count == x86_##ITEM##_size * x86_cpu_num_threads) \
		core->ITEM##_full++; \
}


void X86CpuUpdateOccupancyStats(X86Cpu *self)
{
	X86Core *core;
	X86Thread *thread;

	int i;
	int j;

	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		core = self->cores[i];

		/* Update occupancy stats for shared structures */
		if (x86_rob_kind == x86_rob_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(rob);
		if (x86_iq_kind == x86_iq_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(iq);
		if (x86_lsq_kind == x86_lsq_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(lsq);
		if (x86_reg_file_kind == x86_reg_file_kind_shared)
		{
			UPDATE_CORE_OCCUPANCY_STATS(reg_file_int);
			UPDATE_CORE_OCCUPANCY_STATS(reg_file_fp);
		}

		/* Occupancy stats for private structures */
		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			thread = core->threads[j];
			if (x86_rob_kind == x86_rob_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(rob);
			if (x86_iq_kind == x86_iq_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(iq);
			if (x86_lsq_kind == x86_lsq_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(lsq);
			if (x86_reg_file_kind == x86_reg_file_kind_private)
			{
				UPDATE_THREAD_OCCUPANCY_STATS(reg_file_int);
				UPDATE_THREAD_OCCUPANCY_STATS(reg_file_fp);
			}
		}
	}
}
