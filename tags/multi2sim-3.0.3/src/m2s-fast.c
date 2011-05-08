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

#include <time.h>
#include <options.h>
#include <signal.h>
#include <m2skernel.h>


/* Multi2Sim version */
#ifndef VERSION
#define VERSION ""
#endif


/* Options */
static char *sim_title = "";
static char *configfile = "";
static char *ctxconfig = "";

static char *ctx_debug_file = "";
static char *syscall_debug_file = "";
static char *opencl_debug_file = "";
static char *gpu_isa_debug_file = "";
static char *elf_debug_file = "";
static char *loader_debug_file = "";
static char *isa_call_debug_file = "";
static char *isa_inst_debug_file = "";

static char *inst_stat_file = "";
static char *inst_stop = "";
static uint64_t inst_reset = 0;
static uint64_t max_cycles = 0;
static uint64_t max_inst = 0;
static uint32_t break_point = 0;
static uint64_t sim_cycle = 0;
static uint64_t sim_inst = 0;


/* Variables */
static int sigint_received = 0;



static void sim_reg_options()
{
	opt_reg_string("-title", "Simulation title", &sim_title);
	opt_reg_string("-config", "m2s-fast configuration file", &configfile);
	opt_reg_string("-ctxconfig", "Context configuration file", &ctxconfig);

	opt_reg_string("-debug:ctx", "Debug information for context creation & status updates", &ctx_debug_file);
	opt_reg_string("-debug:syscall", "Debug information for system calls", &syscall_debug_file);	
	opt_reg_string("-debug:opencl", "Debug information for OpenCL programs", &opencl_debug_file);
	opt_reg_string("-debug:gpu_isa", "Debug information for GPU instructions", &gpu_isa_debug_file);
	opt_reg_string("-debug:elf", "Debug information from elf file analyzer", &elf_debug_file);	
	opt_reg_string("-debug:loader", "Debug information from program loader", &loader_debug_file);	
	opt_reg_string("-debug:call", "Debug information about procedure calls", &isa_call_debug_file);	
	opt_reg_string("-debug:inst", "Debug information about executed instructions", &isa_inst_debug_file);

	opt_reg_string("-inst_stat", "File to dump instruction stats", &inst_stat_file);
	opt_reg_string("-inst_stop", "Stop when executing inst", &inst_stop);
	opt_reg_uint64("-inst_reset", "Cycle to reset instruction stats", &inst_reset);
	opt_reg_uint64("-max_cycles", "Cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint64("-max_inst", "Maximum number of instructions", &max_inst);
	opt_reg_uint32("-break_point", "Value for eip to stop", &break_point);
	opt_reg_bool("-mem_safe_mode", "Safe accesses to memory", &mem_safe_mode);

	gk_reg_options();
}


/* Signal handlers */
static void sim_signal_handler(int signum)
{
	FILE *f;

	switch (signum) {
	
	case SIGINT:
		if (sigint_received)
			abort();
		sigint_received = 1;
		fprintf(stderr, "SIGINT received\n");
		break;
	
	case SIGABRT:
		signal(SIGABRT, SIG_DFL);
		fprintf(stderr, "cycle %lld: aborted\n", (long long) sim_cycle);
		f = open_write(inst_stat_file);
		if (f)
			isa_inst_stat_dump(f);
		isa_dump(stderr);
		ke_dump(stderr);
		exit(1);
		break;
	}
}


int main(int argc, char **argv)
{
	uint64_t t;

	/* Initial information */
	fprintf(stderr, "\nMulti2Sim %s - Fast Functional Simulator (m2s-fast)\n",
		VERSION);
	fprintf(stderr, "Last compilation: %s %s\n\n", __DATE__, __TIME__);

	/* Options */
	opt_init();
	sim_reg_options();
	opt_check_options(&argc, argv);
	opt_check_config(configfile);
	opt_print_options(stderr);

	/* Initialize */
	ke_init();

	/* Debug */
	debug_init();
	debug_assign_file(ctx_debug_category, ctx_debug_file);
	debug_assign_file(syscall_debug_category, syscall_debug_file);
	debug_assign_file(opencl_debug_category, opencl_debug_file);
	debug_assign_file(gpu_isa_debug_category, gpu_isa_debug_file);
	debug_assign_file(elf_debug_category, elf_debug_file);
	debug_assign_file(ld_debug_category, loader_debug_file);
	debug_assign_file(isa_call_debug_category, isa_call_debug_file);
	debug_assign_file(isa_inst_debug_category, isa_inst_debug_file);

	/* Load programs from configuration file and command line. */
	if (*ctxconfig)
		ld_load_prog_from_ctxconfig(ctxconfig);
	if (argc > 1)
		ld_load_prog_from_cmdline(argc - 1, argv + 1);
	
	/* Simulation loop */
	signal(SIGINT, &sim_signal_handler);
	signal(SIGABRT, &sim_signal_handler);
	while (!sigint_received) {
		
		/* Break point */
		if (break_point && ke->context_list_head->regs->eip == break_point) {
			regs_dump(ke->context_list_head->regs, stdout);
			break;
		}
		
		/* Run an instruction from each running context */
		ke_run();
		sim_inst += ke->running_count;
		if (!ke->context_list_head)
			break;

		/* Stop conditions */
		sim_cycle++;
		if ((sim_cycle >= max_cycles && max_cycles) ||
			(sim_inst >= max_inst && max_inst))
		{
			regs_dump(isa_regs, stdout);
			ke_dump(stdout);
			break;
		}
		if (sim_cycle == inst_reset && inst_reset)
			isa_inst_stat_reset();

		if (*inst_stop && !strcmp(isa_inst.format, inst_stop))
			panic("instruction '%s' found", inst_stop);

		if ((long long) sim_cycle >= (long long) max_cycles - 100
			&& max_cycles && !debug_status(isa_inst_debug_category))
			debug_assign_file(isa_inst_debug_category, "stdout");
	}

	/* Instruction stats */
	if (*inst_stat_file) {
		FILE *f = open_write(inst_stat_file);
		isa_inst_stat_dump(f);
		fclose(f);
	}

	/* Stats */
	t = ke_timer();
	fprintf(stderr, "\n");
	fprintf(stderr, "sim.cycles  %lld  # Simulation cycles\n",
		(long long) sim_cycle);
	fprintf(stderr, "sim.inst  %lld  # Number of instructions executed\n",
		(long long) sim_inst);
	fprintf(stderr, "sim.time  %.1f  # Simulation time in seconds\n",
		(double) t / 1000000);
	fprintf(stderr, "sim.cps  %.0f  # Cycles simulated per second\n",
		t ? (double) sim_cycle / t * 1e6 : 0.0);
	fprintf(stderr, "sim.contexts  %d  # Maximum number of contexts running concurrently\n",
		ke->running_max);
	fprintf(stderr, "sim.memory  %lu  # Physical memory used by benchmarks\n",
		mem_mapped_space);
	fprintf(stderr, "sim.memory_max  %lu  # Maximum physical memory used by benchmarks\n",
		mem_max_mapped_space);
	fprintf(stderr, "\n");

	/* Finalization */
	ke_done();
	opt_done();
	debug_done();
	mhandle_done();
	return 0;
}
