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

static char *ctx_debug_file_name = "";
static char *syscall_debug_file_name = "";
static char *opencl_debug_file_name = "";
static char *gpu_isa_debug_file = "";
static char *loader_debug_file_name = "";
static char *isa_call_debug_file_name = "";
static char *isa_inst_debug_file_name = "";
static char *cache_debug_file_name = "";
static char *esim_debug_file_name = "";
static char *error_debug_file_name = "";


/* Simulation cycle and total committed inst */
uint64_t sim_cycle;
uint64_t sim_inst;


/* Error debug */
int error_debug_category;




static void sim_reg_options()
{
	opt_reg_string("-title", "Simulation title", &sim_title);
	opt_reg_string("-config", "Processor configuration file", &configfile);
	opt_reg_string("-ctxconfig", "Context configuration file", &ctxfile);

	opt_reg_uint64("-max_cycles", "Cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint64("-max_inst", "Max number of retireed instructions (0=no max)", &max_inst);
	opt_reg_uint64("-max_time", "Max running time (in seconds)", &max_time);
	opt_reg_uint64("-fastfwd", "Cycles to run with fast simulation", &fastfwd);

	opt_reg_string("-debug:ctx", "Debug information for context creation & status updates", &ctx_debug_file_name);
	opt_reg_string("-debug:syscall", "Debug information for system calls", &syscall_debug_file_name);
	opt_reg_string("-debug:opencl", "Debug information for OpenCL programs", &opencl_debug_file_name);
	opt_reg_string("-debug:gpu_isa", "Debug information for GPU instructions", &gpu_isa_debug_file);
	opt_reg_string("-debug:loader", "Debug information from program loader", &loader_debug_file_name);
	opt_reg_string("-debug:call", "Debug information about procedure calls", &isa_call_debug_file_name);
	opt_reg_string("-debug:inst", "Debug information about executed instructions", &isa_inst_debug_file_name);
	opt_reg_string("-debug:cache", "Debug information for cache system", &cache_debug_file_name);
	opt_reg_string("-debug:pipeline", "Debug information for pipeline", &esim_debug_file_name);
	opt_reg_string("-debug:error", "Debug information after errors", &error_debug_file_name);

	opt_reg_string("-report:pipeline", "Report for pipeline statistics", &p_report_file);
	opt_reg_string("-report:cache", "Report for cache system", &cache_system_report_file);

	gk_reg_options();
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
	debug_assign_file(ctx_debug_category, ctx_debug_file_name);
	debug_assign_file(syscall_debug_category, syscall_debug_file_name);
	debug_assign_file(opencl_debug_category, opencl_debug_file_name);
	debug_assign_file(gpu_isa_debug_category, gpu_isa_debug_file);
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

		/* Process host threads generating events */
		ke_process_events();

		/* Event-driven module */
		esim_process_events();
		
		/* Halt execution. Avoid costly calls to ke_timer by checking
		 * time only every 10k cycles */
		if (sigint_received)
			break;
		if (max_cycles && sim_cycle >= max_cycles)
			break;
		if (max_inst && sim_inst >= max_inst)
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
