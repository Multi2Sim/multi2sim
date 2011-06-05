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
static char *gpu_pipeline_debug_file_name = "";
static char *loader_debug_file_name = "";
static char *isa_call_debug_file_name = "";
static char *isa_inst_debug_file_name = "";
static char *cache_debug_file_name = "";
static char *esim_debug_file_name = "";
static char *error_debug_file_name = "";
static char *ctxconfig_file_name = "";


/* Error debug */
int error_debug_category;


static char *sim_help =
	"Syntax:\n"
	"\n"
	"        m2s [<options>] [<x86_binary> [<arg_list>]]\n"
	"\n"
	"The user command-line supports a sequence of command-line options, and can\n"
	"include an x86 ELF binary (executable x86 program) followed by its arguments.\n"
	"The execution of this program will be simulated by Multi2Sim, together with\n"
	"the rest of the x86 programs specified in the context configuration file\n"
	"(option '--ctx-config <file>').\n"
	"\n"
	"List of supported options:\n"
	"\n"
	"  --cache-config <file>\n"
	"      Configuration file for the cache memory hierarchy, including cache levels,\n"
	"      cache geometry, latencies, and interconnection networks. Type\n"
	"      'm2s --help-cache-config' for details on the file format.\n"
	"\n"
	"  --cpu-config <file>\n"
	"      Configuration file for the CPU model, including parameters describing the\n"
	"      stages bandwidth, structures size, and other parameters of processor cores\n"
	"      and threads. Type 'm2s --help-cpu-config' for details on the file format.\n"
	"\n"
	"  --cpu-sim {functional|detailed}\n"
	"      Choose a functional simulation (emulation) of an x86 program, versus\n"
	"      a detailed (architectural) simulation. Simulation is function by default.\n"
	"\n"
	"  --ctx-config <file>, -c <file>\n"
	"      Use <file> as the context configuration file. This file describes the\n"
	"      initial set of running applications, their arguments, and environment\n"
	"      variables. Type 'm2s --help-ctx-config' for a description of the file\n"
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
	"        --debug-gpu-pipeline: trace of AMD Evergreen instructions in the GPU\n"
	"            pipeline. This option requires '--gpu-sim detailed' option.\n"
	"        --debug-loader: information for the x86 ELF binary analysis performed\n"
	"            by the program loader.\n"
	"        --debug-cache: event trace of block transfers and requests in the\n"
	"            cache memory hierarchy.\n"
	"        --debug-call: trace of function calls, based on emulated x86 instr.\n"
	"        --debug-cpu-isa: trace of emulated x86 ISA instructions.\n"
	"        --debug-cpu-pipeline: trace of x86 microinstructions in the CPU pipeline.\n"
	"            The output file can be used as an input for the 'm2s-pipeline' tool\n"
	"            to obtain graphical timing diagrams.\n"
	"        --debug-error: on simulation crashes, dump of the modeled CPU state.\n"
	"\n"
	"  --gpu-sim {functional|detailed}\n"
	"      Functional simulation (emulation) of the AMD Evergreen GPU kernel, versus\n"
	"      detailed (architectural) simulation. Functional simulation is default.\n"
	"\n"
	"  --help-<xxx>\n"
	"      Show help on the format of configuration files for Multi2Sim. <xxx> stands\n"
	"      for one of the following options:\n"
	"        --help-cache-config: format of the configuration file for the cache system.\n"
	"        --help-cpu-config: format of the CPU model configuration file.\n"
	"        --help-ctx-config: format of the context configuration file.\n"
	"        --help-gpu-config: format of the GPU model configuration file.\n"
	"\n"
	"  --max-cycles <num_cycles>\n"
	"      Specify a limit in the number of cycles. In the functional simulation, one\n"
	"      instruction from each active context is executed in a cycle. In the detailed\n"
	"      simulation, this option refers to CPU cycles. Use a value of 0 (default) for\n"
	"      unlimited.\n"
	"\n"
	"  --max-inst <num_inst>\n"
	"      Specify the limit in the number of executed instructions. In the detailed\n"
	"      simulation, this limit refers to the number of committed instructions. Use\n"
	"      a value of 0 (default) for unlimited.\n"
	"\n"
	"  --max-time <seconds>\n"
	"      Maximum simulation time in seconds. The simulator will stop after this time\n"
	"      is exceeded. Use 0 (default) for no time limit.\n"
	"\n"
	"  --opencl-binary <file>\n"
	"      Specify OpenCL kernel binary to be loaded when the OpenCL host program\n"
	"      performs a call to 'clCreateProgramWithSource'. Since on-line compilation\n"
	"      of OpenCL kernels is not supported, this is a possible way to load them.\n"
	"\n"
	"  --report-cpu <file>\n"
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

		/* Cache system configuration file */
		if (!strcmp(argv[argi], "--cache-config")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a cache configuration file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			cache_system_config_file_name = argv[argi];
			continue;
		}

		/* CPU configuration file */
		if (!strcmp(argv[argi], "--cpu-config")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a CPU configuration file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			cpu_config_file_name = argv[argi];
			continue;
		}

		/* CPU simulation accuracy */
		if (!strcmp(argv[argi], "--cpu-sim")) {
			if (argi == argc - 1)
				fatal("option '%s' requires a simulation accuracy argument.\n%s",
					argv[argi], err_help_note);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				cpu_sim_kind = cpu_sim_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				cpu_sim_kind = cpu_sim_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], err_help_note);
			continue;
		}

		/* Context configuration file */
		if (!strcmp(argv[argi], "--ctx-config") || !strcmp(argv[argi], "-c")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			ctxconfig_file_name = argv[argi];
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

		/* CPU pipeline debug */
		if (!strcmp(argv[argi], "--debug-cpu-pipeline")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for the CPU pipeline debug info.\n%s",
					argv[argi], err_help_note);
			argi++;
			esim_debug_file_name = argv[argi];
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

		/* Error debug */
		if (!strcmp(argv[argi], "--debug-error")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for error debug info.\n%s",
					argv[argi], err_help_note);
			argi++;
			error_debug_file_name = argv[argi];
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

		/* GPU pipeline debug file */
		if (!strcmp(argv[argi], "--debug-gpu-pipeline")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by the debug file name for the GPU pipeline.\n%s",
					argv[argi], err_help_note);
			argi++;
			gpu_pipeline_debug_file_name = argv[argi];
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

		/* OpenCL debug file */
		if (!strcmp(argv[argi], "--debug-opencl")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a file name for OpenCL calls debugging.\n%s",
					argv[argi], err_help_note);
			argi++;
			opencl_debug_file_name = argv[argi];
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

		/* GPU simulation accuracy */
		if (!strcmp(argv[argi], "--gpu-sim")) {
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

		/* Help for cache configuration file */
		if (!strcmp(argv[argi], "--help-cache-config")) {
			fprintf(stderr, "%s", cache_system_config_help);
			continue;
		}

		/* Help for CPU configuration file */
		if (!strcmp(argv[argi], "--help-cpu-config")) {
			fprintf(stderr, "%s", cpu_config_help);
			continue;
		}

		/* Help for context configuration file format */
		if (!strcmp(argv[argi], "--help-ctx-config")) {
			fprintf(stderr, "%s", ld_help_ctxconfig);
			continue;
		}

		/* Help for GPU configuration file */
		if (!strcmp(argv[argi], "--help-gpu-config")) {
			fprintf(stderr, "%s", gpu_config_help);
			continue;
		}

		/* Maximum number of cycles */
		if (!strcmp(argv[argi], "--max-cycles")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by an integer number.\n%s",
					argv[argi], err_help_note);
			argi++;
			ke_max_cycles = atoll(argv[argi]);
			continue;
		}

		/* Maximum number of instructions */
		if (!strcmp(argv[argi], "--max-inst")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by an integer number.\n%s",
					argv[argi], err_help_note);
			argi++;
			ke_max_inst = atoll(argv[argi]);
			continue;
		}

		/* Simulation time limit */
		if (!strcmp(argv[argi], "--max-time")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by an integer number.\n%s",
					argv[argi], err_help_note);
			argi++;
			ke_max_time = atoll(argv[argi]);
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
			cache_system_report_file_name = argv[argi];
			continue;
		}

		/* Pipeline report */
		if (!strcmp(argv[argi], "--report-cpu")) {
			if (argi == argc - 1)
				fatal("option '%s' must be followed by a pipeline report file name.\n%s",
					argv[argi], err_help_note);
			argi++;
			cpu_report_file_name = argv[argi];
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

	/* Check configuration consistency */
	if (cpu_sim_kind == cpu_sim_kind_functional) {
		if (*cpu_config_file_name || *cpu_report_file_name)
			fatal("A CPU configuration file or statistics report cannot be specified for functional CPU simulation.\n"
				"If you want to run an architectural simulation, add option '--cpu-sim detailed'.\n");
		if (*cache_system_config_file_name || *cache_system_report_file_name)
			fatal("A configuration file or statistics report for the cache system cannot be specified\n"
				"for a functional simulation. If you want to run an architectural simulation,\n"
				"add command-line option '--cpu-sim detailed'.\n");
	}

	/* Discard arguments used as options */
	arg_discard = argi - 1;
	for (argi = 1; argi < argc - arg_discard; argi++)
		argv[argi] = argv[argi + arg_discard];
	*argc_ptr = argc - arg_discard;
}


int main(int argc, char **argv)
{
	/* Initial information */
	fprintf(stderr, "\n");
	fprintf(stderr, "; Multi2Sim %s - A Simulation Framework for CPU-GPU Heterogeneous Computing\n",
		VERSION);
	fprintf(stderr, "; Please use command 'm2s --help' for a list of command-line options.\n");
	fprintf(stderr, "; Last compilation: %s %s\n", __DATE__, __TIME__);
	fprintf(stderr, "\n");
	opt_print_options(stderr);

	/* Read command line */
	sim_read_command_line(&argc, argv);

	/* Initialization for functional simulation */
	ke_init();

	/* Initialization for detailed simulation */
	if (cpu_sim_kind == cpu_sim_kind_detailed) {
		uop_init();
		esim_init();
		net_init();
		cpu_init();
	}
	if (gpu_sim_kind == gpu_sim_kind_detailed)
		gpu_init();

	/* Debug */
	debug_init();
	error_debug_category = debug_new_category();
	debug_assign_file(ctx_debug_category, ctx_debug_file_name);
	debug_assign_file(syscall_debug_category, syscall_debug_file_name);
	debug_assign_file(opencl_debug_category, opencl_debug_file_name);
	debug_assign_file(gpu_isa_debug_category, gpu_isa_debug_file_name);
	debug_assign_file(gpu_pipeline_debug_category, gpu_pipeline_debug_file_name);
	debug_assign_file(ld_debug_category, loader_debug_file_name);
	debug_assign_file(isa_call_debug_category, isa_call_debug_file_name);
	debug_assign_file(isa_inst_debug_category, isa_inst_debug_file_name);
	debug_assign_file(cache_debug_category, cache_debug_file_name);
	debug_assign_file(error_debug_category, error_debug_file_name);
	esim_debug_init(esim_debug_file_name);

	/* Load programs */
	cpu_load_progs(argc, argv, ctxconfig_file_name);

	/* Simulation loop */
	if (ke->running_list_head) {
		if (cpu_sim_kind == cpu_sim_kind_detailed)
			cpu_run();
		else
			ke_run();
	}

	/* Finalization of detailed CPU simulation */
	if (cpu_sim_kind == cpu_sim_kind_detailed) {
		
		/* Finalize event-driven simulations */
		while (esim_pending() && esim_cycle < cpu->cycle + (1 << 20))
			esim_process_events();
		esim_debug_done();

		/* Rest */
		cpu_done();
		uop_done();
		net_done();
		esim_done();
	}

	/* Finalization of detailed GPU simulation */
	if (gpu_sim_kind == gpu_sim_kind_detailed)
		gpu_done();
	
	/* Finalization of functional simulation */
	ke_done();
	debug_done();
	mhandle_done();

	/* End */
	return 0;
}
