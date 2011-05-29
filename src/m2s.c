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
#include <gpuarch.h>


/* Multi2Sim version */
#ifndef VERSION
#define VERSION ""
#endif

static char *ctx_debug_file_name = "";
static char *syscall_debug_file_name = "";
static char *opencl_debug_file_name = "";
static char *gpu_isa_debug_file_name = "";
static char *loader_debug_file_name = "";
static char *isa_call_debug_file_name = "";
static char *isa_inst_debug_file_name = "";
static char *cache_debug_file_name = "";
static char *esim_debug_file_name = "";
static char *error_debug_file_name = "";


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
static char *ctxconfig_file_name = "";
static char *sim_title = "";

/* Simulation cycle and total committed inst */
uint64_t sim_cycle;
uint64_t sim_inst;


/* Error debug */
int error_debug_category;




static void sim_reg_options()
{
	opt_reg_string("-title", "Simulation title", &sim_title);

	opt_reg_uint64("-max_cycles", "Cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint64("-max_inst", "Max number of retired instructions (0=no max)", &max_inst);
	opt_reg_uint64("-max_time", "Max running time (in seconds)", &max_time);
	opt_reg_uint64("-fastfwd", "Cycles to run with fast simulation", &fastfwd);

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


/*	"  --debug-ctx <file>, --debug-syscall <file>, --debug-opencl <file>,\n"
	"  --debug-gpu-isa <file>, --debug-loader <file>, --debug-call <file>,\n"
	"  --debug-cpu-isa <file>, --debug-cache <file>, --debug-cpu-pipeline <file>,\n"
	"  --debug-error <file>\n"*/
		


static char *sim_help =
	"  --cpu-simulation {functional|detailed}\n"
	"      Choose a functional simulation (emulation) of an x86 program, versus\n"
	"      a detailed (architectural) simulation. Simulation is function by default.\n"
	"\n"
	"  --ctxconfig <file>, -c <file>\n"
	"      Use <file> as the context configuration file. This file describes the\n"
	"      initial set of running applications, their arguments, and environment\n"
	"      variables. Type 'm2s --help-ctxconfig' for a description of the file\n"
	"      format.\n"
	"\n"
	"  --debug-<xxx> <file>\n"
	"      Dump detailed debug information about the program simulation. <xxx> stands\n"
	"      for one of the following options:\n"
	"        --debug-ctx: emulated CPU context status updates.\n"
	"        --debug-syscall: detailed system calls trace and arguments.\n"
	"        --debug-opencl: trace of OpenCL calls and their arguments.\n"
	"        --debug-gpu-isa: during the emulation of an OpenCL device kernel, trace\n"
	"            of executed AMD Evergreen ISA instructions.\n"
	"        --debug-loader: information for the x86 ELF binary analysis performed\n"
	"            by the program loader.\n"
	"        --debug-call: trace of function calls, based on emulated x86 instr.\n"
	"        --debug-cpu-isa: trace of emulated x86 ISA instructions.\n"
	"        --debug-cache: event trace of block transfers and requests in the\n"
	"            cache memory hierarchy.\n"
	"        --debug-cpu-pipeline: trace of x86 microinstructions in the CPU pipeline.\n"
	"            The output file can be used as an input for the 'm2s-pipeline' tool\n"
	"            to obtain graphical timing diagrams.\n"
	"        --debug-error: on simulation crashes, dump of the modeled CPU state.\n"
	"\n"
	"  --gpu-simulation {functional|detailed}\n"
	"      Functional simulation (emulation) of the AMD Evergreen GPU kernel, versus\n"
	"      detailed (architectural) simulation. Functional simulation is default.\n"
	"\n"
	"  --opencl-binary <file>\n"
	"      Specify OpenCL kernel binary to be loaded when the OpenCL host program\n"
	"      performs a call to 'clCreateProgramWithSource'. Since on-line compilation\n"
	"      of OpenCL kernels is not supported, this is a possible way to load them.\n"
	"\n"
	"  --report-cpu-pipeline <file>\n"
	"      File to dump a report of the CPU pipeline, such as number of instructions\n"
	"      handled in every pipeline stage, or read/write accesses performed to\n"
	"      pipeline queues (ROB, IQ, etc.). Use only when CPU simulation accuracy\n"
	"      is set to 'detailed'.\n"
	"\n"
	"  --report-gpu <file>\n"
	"      File to dump report of a GPU device kernel emulation. The report includes\n"
	"      statistics about type of instructions, VLIW packing, thread divergence, etc.\n"
	"\n";

static char *err_help_note =
	"Please type 'm2s --help' for a list of valid Multi2Sim command-line options.\n";


static void sim_read_command_line(int *argc_ptr, char **argv)
{
	int argc = *argc_ptr;
	int argi;
	int arg_discard = 0;

	for (argi = 1; argi < argc; argi++) {

		/* CPU simulation accuracy */
		if (!strcmp(argv[argi], "--cpu-simulation")) {
			if (argi == argc - 1)
				fatal("option '%s' requires a simulation accuracy argument.\n%s",
					argv[argi], err_help_note);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				p_sim_kind = p_sim_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				p_sim_kind = p_sim_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], err_help_note);
			continue;
		}

		/* Context configuration file */
		if (!strcmp(argv[argi], "--ctxconfig") || !strcmp(argv[argi], "-c")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			ctxconfig_file_name = argv[argi];
			continue;
		}

		/* Context debug file */
		if (!strcmp(argv[argi], "--debug-ctx")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a context debug file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			ctx_debug_file_name = argv[argi];
			continue;
		}

		/* System call debug file */
		if (!strcmp(argv[argi], "--debug-syscall")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for system call debugging.\n%s",
					argv[argi], err_help_note);
			argi++;
			syscall_debug_file_name = argv[argi];
			continue;
		}

		/* OpenCL debug file */
		if (!strcmp(argv[argi], "--debug-opencl")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for OpenCL calls debugging.\n%s",
					argv[argi], err_help_note);
			argi++;
			opencl_debug_file_name = argv[argi];
			continue;
		}

		/* GPU ISA debug file */
		if (!strcmp(argv[argi], "--debug-gpu-isa")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by the debug file name for GPU instructions.\n%s",
					argv[argi], err_help_note);
			argi++;
			gpu_isa_debug_file_name = argv[argi];
			continue;
		}

		/* Loader debug file */
		if (!strcmp(argv[argi], "--debug-loader")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for program loading debug info.\n%s",
					argv[argi], err_help_note);
			argi++;
			loader_debug_file_name = argv[argi];
			continue;
		}

		/* Function calls debug file */
		if (!strcmp(argv[argi], "--debug-call")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for function calls debug info.\n%s",
					argv[argi], err_help_note);
			argi++;
			isa_call_debug_file_name = argv[argi];
			continue;
		}
		
		/* CPU ISA debug file */
		if (!strcmp(argv[argi], "--debug-cpu-isa")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by the debug file name for CPU x86 instructions.\n%s",
					argv[argi], err_help_note);
			argi++;
			isa_inst_debug_file_name = argv[argi];
			continue;
		}

		/* Cache system debug */
		if (!strcmp(argv[argi], "--debug-cache")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for the cache system debug info.\n%s",
					argv[argi], err_help_note);
			argi++;
			cache_debug_file_name = argv[argi];
			continue;
		}

		/* CPU pipeline debug */
		if (!strcmp(argv[argi], "--debug-pipeline")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for the CPU pipeline debug info.\n%s",
					argv[argi], err_help_note);
			argi++;
			esim_debug_file_name = argv[argi];
			continue;
		}

		/* Error debug */
		if (!strcmp(argv[argi], "--debug-error")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for error debug info.\n%s",
					argv[argi], err_help_note);
			argi++;
			error_debug_file_name = argv[argi];
			continue;
		}
		
		/* GPU simulation accuracy */
		if (!strcmp(argv[argi], "--gpu-simulation")) {
			if (argi == argc - 1)
				fatal("option '%s' requires a simulation accuracy argument.\n%s",
					argv[argi], err_help_note);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				gpu_sim_kind = gpu_sim_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				gpu_sim_kind = gpu_sim_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], err_help_note);
			continue;
		}

		/* Show help */
		if (!strcmp(argv[argi], "--help") || !strcmp(argv[argi], "-h")) {
			fprintf(stderr, "%s", sim_help);
			continue;
		}

		/* OpenCL binary */
		if (!strcmp(argv[argi], "--opencl-binary")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by an OpenCL binary file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			gk_opencl_binary_name = argv[argi];
			continue;
		}

		/* Cache system report */
		if (!strcmp(argv[argi], "--report-cache")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a cache report file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			cache_system_report_file = argv[argi];
			continue;
		}

		/* Pipeline report */
		if (!strcmp(argv[argi], "--report-cpu-pipeline")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a pipeline report file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			p_report_file_name = argv[argi];
			continue;
		}

		/* GPU emulation report */
		if (!strcmp(argv[argi], "--report-gpu")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a GPU report file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			gk_report_file_name = argv[argi];
			continue;
		}

		/* Invalid option */
		if (argv[argi][0] == '-') {
			fatal("'%s' is not a valid command-line option.\n%s",
				argv[argi], err_help_note);
		}

		/* End of options */
		break;
	}

	/* Discard arguments used as options */
	arg_discard = argi - 1;
	for (argi = 1; argi < argc - arg_discard; argi++)
		argv[argi] = argv[argi + arg_discard];
	*argc_ptr = argc - arg_discard;
}


int main(int argc, char **argv)
{
	/* Options & stats */
	opt_init();
	sim_reg_options();
	p_reg_options();
	cache_system_reg_options();
	//opt_check_options(&argc, argv);

	/* Initial information */
	fprintf(stderr, "\n");
	fprintf(stderr, "Multi2Sim %s - A Simulation Framework for CPU-GPU Heterogeneous Computing\n",
		VERSION);
	fprintf(stderr, "Please use command 'm2s --help' for a list of command-line options.\n");
	fprintf(stderr, "Last compilation: %s %s\n", __DATE__, __TIME__);
	fprintf(stderr, "\n");
	opt_print_options(stderr);

	/* Read command line */
	sim_read_command_line(&argc, argv);

	/* Initialization */
	uop_init();
	esim_init();
	net_init();
	ke_init();
	p_init();

	/* Debug */
	debug_init();
	error_debug_category = debug_new_category();
	debug_assign_file(ctx_debug_category, ctx_debug_file_name);
	debug_assign_file(syscall_debug_category, syscall_debug_file_name);
	debug_assign_file(opencl_debug_category, opencl_debug_file_name);
	debug_assign_file(gpu_isa_debug_category, gpu_isa_debug_file_name);
	debug_assign_file(ld_debug_category, loader_debug_file_name);
	debug_assign_file(isa_call_debug_category, isa_call_debug_file_name);
	debug_assign_file(isa_inst_debug_category, isa_inst_debug_file_name);
	debug_assign_file(cache_debug_category, cache_debug_file_name);
	debug_assign_file(error_debug_category, error_debug_file_name);
	esim_debug_init(esim_debug_file_name);

	/* Load programs */
	p_load_progs(argc, argv, ctxconfig_file_name);

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
	net_done();
	esim_done();
	debug_done();
	fprintf(stderr, "\n");
	mhandle_done();
	return 0;
}
