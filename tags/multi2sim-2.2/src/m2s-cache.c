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
#include <m2s.h>


/* Options */
static char *sim_title = "";
static char *configfile = "";
static char *ctxconfig = "";

static char *syscall_debug_file = "";
static char *loader_debug_file = "";
static char *isa_call_debug_file = "";
static char *isa_inst_debug_file = "";
static char *cache_debug_file = "";

static char *inst_stat_file = "";
static char *inst_stop = "";
static uint64_t inst_reset = 0;
static uint64_t max_cycles = 0;
static uint32_t break_point = 0;
uint64_t sim_cycle = 0;


/* Variables */
static int sigint_received = 0;



static void sim_reg_options()
{
	opt_reg_string("-title", "simulation title", &sim_title);
	opt_reg_string("-config", "m2s-fast configuration file", &configfile);
	opt_reg_string("-ctxconfig", "context configuration file", &ctxconfig);

	opt_reg_string("-syscall_debug", "debug information for system calls", &syscall_debug_file);	
	opt_reg_string("-loader_debug", "debug information from program loader", &loader_debug_file);	
	opt_reg_string("-call_debug", "debug information about procedure calls", &isa_call_debug_file);	
	opt_reg_string("-inst_debug", "debug information about executed instructions", &isa_inst_debug_file);
	opt_reg_string("-cache_debug", "debug information about cache system", &cache_debug_file);

	opt_reg_string("-inst_stat", "file to dump instruction stats", &inst_stat_file);
	opt_reg_string("-inst_stop", "stop when executing inst", &inst_stop);
	opt_reg_uint64("-inst_reset", "cycle to reset instruction stats", &inst_reset);
	opt_reg_uint64("-max_cycles", "cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint32("-break_point", "value for eip to stop", &break_point);
}


/* Signal handlers */
void sim_signal_handler(int signum)
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
	/* Initial information */
	fprintf(stderr, "\nMulti2Sim. Functional Simulator (m2s-fast)\n");
	fprintf(stderr, "Last compilation: %s %s\n\n", __DATE__, __TIME__);

	/* Options */
	opt_init();
	sim_reg_options();
	cache_system_reg_options();
	opt_check_options(&argc, argv);
	opt_check_config(configfile);
	opt_print_options(stderr);

	/* Initialize */
	esim_init();
	ke_init();
	net_init();
	cache_system_init(8, 1);

	/* Debug */
	debug_init();
	debug_assign_file(syscall_debug_category, syscall_debug_file);
	debug_assign_file(ld_debug_category, loader_debug_file);
	debug_assign_file(isa_call_debug_category, isa_call_debug_file);
	debug_assign_file(isa_inst_debug_category, isa_inst_debug_file);
	debug_assign_file(cache_debug_category, cache_debug_file);

	/* Load programs from configuration file and command line. */
	if (*ctxconfig)
		ld_load_prog_from_ctxconfig(ctxconfig);
	if (argc > 1)
		ld_load_prog_from_cmdline(argc - 1, argv + 1);
	if (!ke->context_list)
		fatal("no context loaded");
	

	{
		uint32_t list[10], core[10], thread[10], cycle[10], read[10];
		uint32_t addr = 0;
		int curr = 0, i;
		int count = 3;

		for (i = 0; i < 10000; i++) {
			mmu_translate(0, addr);
			addr += mmu_page_size;
		}

		list[0] = 0x84322;
		cycle[0] = 0;
		core[0] = 0;
		thread[0] = 0;
		read[0] = 1;

		list[1] = 0x84333;
		cycle[1] = 1000;
		core[1] = 1;
		thread[1] = 0;
		read[1] = 1;

		list[2] = 0x84333;
		cycle[2] = 2000;
		core[2] = 2;
		thread[2] = 0;
		read[2] = 0;

		do {
			while (curr < count && cache_system_can_access(core[curr], thread[curr],
				cache_kind_data, list[curr]) && cycle[curr] <= sim_cycle)
			{
				if (read[curr])
					cache_system_read(core[curr], thread[curr], cache_kind_data, list[curr], NULL);
				else
					cache_system_write(core[curr], thread[curr], cache_kind_data, list[curr], NULL);
				curr++;
			}
			esim_process_events();
			sim_cycle++;
		} while (curr < count || esim_pending());
	}
	
#if 0
	
	/* Simulation loop */
	signal(SIGINT, &sim_signal_handler);
	signal(SIGABRT, &sim_signal_handler);
	while (!sigint_received) {
		
		/* Break point */
		if (break_point && ke->context_list->regs->eip == break_point) {
			regs_dump(ke->context_list->regs, stdout);
			break;
		}
		
		/* Run an instruction from each context */
		ke_run();

		/* Stop conditions */
		esim_process_events();
		sim_cycle++;
		if (!ke->context_list)
			break;
		if (sim_cycle >= max_cycles && max_cycles) {
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
#endif

	/* Instruction stats */
	if (*inst_stat_file) {
		FILE *f = open_write(inst_stat_file);
		isa_inst_stat_dump(f);
		fclose(f);
	}

	/* Stats */
	fprintf(stderr, "\n");
	fprintf(stderr, "cycles  %lld  # Simulation cycles\n",
		(long long) sim_cycle);
	fprintf(stderr, "time  %.1f  # Simulation time\n",
		(double) clock() / CLOCKS_PER_SEC);
	fprintf(stderr, "cps  %.0f  # Cycles simulated per second\n",
		clock() ? (double) sim_cycle / clock() * CLOCKS_PER_SEC : 0.0);

	/* Finalization */
	cache_system_done();
	net_done();
	ke_done();
	esim_done();
	opt_done();
	debug_done();
	mhandle_done();

	fprintf(stderr, "\n");
	return 0;
}