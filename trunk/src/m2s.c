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


/* Multi2Sim version */
#ifndef VERSION
#define VERSION ""
#endif


/* Signals */
static int sigint_received = 0;
static int sigusr_received = 0;
static int sigalrm_interval = 30;
static uint64_t last_sigalrm_cycle = 0;


/* Stats */
static uint64_t max_cycles = 0;
static uint64_t max_inst = 0;
static uint64_t max_time = 0;
static uint64_t fastfwd;
static char *ctxfile = "";
static char *configfile = "";
static char *sim_title = "";

static char *syscall_debug_file_name = "";
static char *loader_debug_file_name = "";
static char *isa_call_debug_file_name = "";
static char *isa_inst_debug_file_name = "";
static char *cache_debug_file_name = "";
static char *esim_debug_file_name = "";
static char *error_debug_file_name = "";


/* Simulation cycle */
uint64_t sim_cycle;


/* Error debug */
int error_debug_category;




static void sim_reg_options()
{
	opt_reg_string("-title", "simulation title", &sim_title);
	opt_reg_string("-config", "processor configuration file", &configfile);
	opt_reg_string("-ctxconfig", "context configuration file", &ctxfile);

	opt_reg_uint64("-max_cycles", "cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint64("-max_inst", "max number of retireed instructions (0=no max)", &max_inst);
	opt_reg_uint64("-max_time", "max running time (in seconds)", &max_time);
	opt_reg_uint64("-fastfwd", "cycles to run with fast simulation", &fastfwd);

	opt_reg_string("-debug:syscall", "debug information for system calls", &syscall_debug_file_name);
	opt_reg_string("-debug:loader", "debug information from program loader", &loader_debug_file_name);
	opt_reg_string("-debug:call", "debug information about procedure calls", &isa_call_debug_file_name);
	opt_reg_string("-debug:inst", "debug information about executed instructions", &isa_inst_debug_file_name);
	opt_reg_string("-debug:cache", "debug information for cache system", &cache_debug_file_name);
	opt_reg_string("-debug:pipeline", "debug information for pipeline", &esim_debug_file_name);
	opt_reg_string("-debug:error", "debug information after errors", &error_debug_file_name);
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
		p_print_stats(f);
		p_dump(f);
		fclose(f);
	}

	/* Ready to receive new SIGUSR signals */
	sigusr_received = 0;
}


/* Signal handlers */
static void sim_signal_handler(int signum)
{
	switch (signum) {
	
	case SIGINT:
		if (sigint_received)
			abort();
		sigint_received = 1;
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
		if (debug_status(error_debug_category)) {
			p_print_stats(debug_file(error_debug_category));
			p_dump(debug_file(error_debug_category));
		}
		exit(1);
		break;
	
	case SIGUSR2:
		sigusr_received = 1;
	
	}
}


int main(int argc, char **argv)
{
	/* Options & stats */
	opt_init();
	sim_reg_options();
	p_reg_options();
	cache_system_reg_options();
	opt_check_options(&argc, argv);
	if (*configfile)
		opt_check_config(configfile);

	/* Initial information */
	fprintf(stderr, "\nMulti2Sim %s - Multicore Multithreaded Processor Simulator\n",
		VERSION);
	fprintf(stderr, "Last compilation: %s %s\n\n", __DATE__, __TIME__);
	opt_print_options(stderr);

	/* Initialization */
	uop_init();
	esim_init();
	net_init();
	cache_system_init(p_cores, p_threads);
	ke_init();

	/* Debug */
	debug_init();
	error_debug_category = debug_new_category();
	debug_assign_file(syscall_debug_category, syscall_debug_file_name);
	debug_assign_file(ld_debug_category, loader_debug_file_name);
	debug_assign_file(isa_call_debug_category, isa_call_debug_file_name);
	debug_assign_file(isa_inst_debug_category, isa_inst_debug_file_name);
	debug_assign_file(cache_debug_category, cache_debug_file_name);
	debug_assign_file(error_debug_category, error_debug_file_name);
	esim_debug_init(esim_debug_file_name);

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

	/* Finalize esim */
	while (esim_pending() && esim_cycle < sim_cycle + (1<<20))
		esim_process_events();
	esim_debug_done();
	
	/* Finalization */
	fprintf(stderr, "\n");
	opt_done();
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
