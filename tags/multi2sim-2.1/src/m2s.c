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


/* Signals */
static int sigint_received = FALSE;
static int sigusr_received = FALSE;
static int sigalrm_interval = 60;
static uint64_t last_sigalrm_cycle = 0;


/* Stats */
static uint64_t max_cycles = 0;
static uint64_t max_inst = 0;
static uint64_t max_time = 0;
static uint64_t fastfwd;
static char *ctxfile = "";
static char *configfile = "";
static char *sim_title = "";

static char *syscall_debug_file = "";
static char *loader_debug_file = "";
static char *isa_call_debug_file = "";
static char *isa_inst_debug_file = "";
static char *cache_debug_file = "";


/* Simulation cycle */
uint64_t sim_cycle;




static void sim_reg_options()
{
	opt_reg_string("-title", "simulation title", &sim_title);
	opt_reg_string("-config", "processor configuration file", &configfile);
	opt_reg_string("-ctxconfig", "context configuration file", &ctxfile);

	opt_reg_uint64("-max_cycles", "cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint64("-max_inst", "max number of retireed instructions (0=no max)", &max_inst);
	opt_reg_uint64("-max_time", "max running time (in seconds)", &max_time);
	opt_reg_uint64("-fastfwd", "cycles to run with fast simulation", &fastfwd);

	opt_reg_string("-syscall_debug", "debug information for system calls", &syscall_debug_file);	
	opt_reg_string("-loader_debug", "debug information from program loader", &loader_debug_file);	
	opt_reg_string("-call_debug", "debug information about procedure calls", &isa_call_debug_file);	
	opt_reg_string("-inst_debug", "debug information about executed instructions", &isa_inst_debug_file);
	opt_reg_string("-cache_debug", "debug information for cache system", &cache_debug_file);
}


/* Dump a log of current status in a file */
static void sim_dump_log()
{
	FILE *f;
	char name[100];
	
	/* Dump log into file */
	sprintf(name, "m2s.%d.%lld", (int) getpid(), (long long) sim_cycle);
	f = fopen(name, "wt");
	if (f) {
		p_dump(f);
		fclose(f);
	}

	/* Ready to receive new SIGUSR signals */
	sigusr_received = FALSE;
}


/* Signal handlers */
static void sim_signal_handler(int signum)
{
	switch (signum) {
	
	case SIGINT:
		if (sigint_received)
			abort();
		sigint_received = TRUE;
		p_dump(stderr);
		fprintf(stderr, "SIGINT received\n");
		break;
		
	case SIGALRM:
		if (sim_cycle - last_sigalrm_cycle == 0)
			panic("simulator stalled in stage %s", p->stage);
		last_sigalrm_cycle = sim_cycle;
		alarm(sigalrm_interval);
		break;
	
	case SIGABRT:
		signal(SIGABRT, SIG_DFL);
		p_dump(stderr);
		exit(1);
		break;
	
	case SIGUSR2:
		sigusr_received = TRUE;
	
	}
}


int main(int argc, char **argv)
{
	/* Options & stats */
	opt_init();
	sim_reg_options();
	ptrace_reg_options();
	p_reg_options();
	cache_system_reg_options();
	opt_check_options(&argc, argv);
	if (*configfile)
		opt_check_config(configfile);

	/* Initial information */
	fprintf(stderr, "\nMulti2Sim - Multicore Multithreaded Processor Simulator\n");
	fprintf(stderr, "Last compilation: %s %s\n\n", __DATE__, __TIME__);
	opt_print_options(stderr);

	/* Initialization */
	ptrace_init();
	uop_init();
	esim_init();
	net_init();
	cache_system_init(p_cores, p_threads);
	ke_init();

	/* Debug */
	debug_init();
	debug_assign_file(syscall_debug_category, syscall_debug_file);
	debug_assign_file(ld_debug_category, loader_debug_file);
	debug_assign_file(isa_call_debug_category, isa_call_debug_file);
	debug_assign_file(isa_inst_debug_category, isa_inst_debug_file);
	debug_assign_file(cache_debug_category, cache_debug_file);

	/* Load programs */
	p_init();
	p_load_progs(argc, argv, ctxfile);

	/* Fast forward simulation */
	p_fast_forward(fastfwd);

	/* Exhaustive simulation */
	signal(SIGINT, &sim_signal_handler);
	signal(SIGABRT, &sim_signal_handler);
	signal(SIGUSR1, &sim_signal_handler);
	signal(SIGUSR2, &sim_signal_handler);
	signal(SIGALRM, &sim_signal_handler);
	alarm(sigalrm_interval);
	
	while (ke->finished_count < ke->context_count) {

		/* Next cycle */
		sim_cycle++;
		ptrace_new_cycle();

		/* Processor stages */
		p_stages();

		/* Event-driven module */
		esim_process_events();
		
		/* Halt execution. Avoid costly calls to ke_timer by checking
		 * time only every 10k cycles */
		if (sigint_received)
			break;
		if (max_cycles && sim_cycle >= max_cycles)
			break;
		if (max_inst && p->committed >= max_inst)
			break;
		if (max_time && !(sim_cycle % 10000) && ke_timer() > max_time * 1000000)
			break;

		/* Dump log */
		if (sigusr_received)
			sim_dump_log();
	}
	signal(SIGALRM, SIG_IGN);
	signal(SIGABRT, SIG_DFL);

	/* Empty esim. Set a security maximum of 1M iterations.  */
	while (esim_pending() && esim_cycle < sim_cycle + (1<<20))
		esim_process_events();
	
	/* Finalization */
	fprintf(stderr, "\n");
	opt_done();
	ptrace_done();
	p_done();
	ke_done();
	uop_done();
	cache_system_done();
	net_done();
	esim_done();
	debug_done();
	fprintf(stderr, "\n");
	mhandle_done();
	return 0;
}