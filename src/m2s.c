/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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
#include <gpuvisual.h>


/* Multi2Sim version */
#ifndef VERSION
#define VERSION ""
#endif

static char *ctx_debug_file_name = "";
static char *syscall_debug_file_name = "";
static char *opencl_debug_file_name = "";
static char *cpu_disasm_file_name = "";
static char *gpu_disasm_file_name = "";
static char *opengl_disasm_file_name = "";
static char *gpu_stack_debug_file_name = "";
static char *gpu_isa_debug_file_name = "";
static char *gpu_pipeline_debug_file_name = "";
static char *gpu_visual_file_name = "";
static char *mem_debug_file_name = "";
static char *loader_debug_file_name = "";
static char *isa_call_debug_file_name = "";
static char *isa_inst_debug_file_name = "";
static char *esim_debug_file_name = "";
static char *error_debug_file_name = "";
static char *ctxconfig_file_name = "";
static char *elf_debug_file_name = "";
static char *net_debug_file_name = "";

static int opengl_disasm_shader_index = 1;

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
	"  --cpu-config <file>\n"
	"      Configuration file for the CPU model, including parameters describing the\n"
	"      stages bandwidth, structures size, and other parameters of processor cores\n"
	"      and threads. Type 'm2s --help-cpu-config' for details on the file format.\n"
	"\n"
	"  --cpu-disasm <file>\n"
	"      Disassemble the x86 ELF file provided in <file>, using the internal x86\n"
	"      disassembler. This option is incompatible with any other option.\n"
	"\n"
	"  --cpu-sim {functional|detailed}\n"
	"      Choose a functional simulation (emulation) of an x86 program, versus\n"
	"      a detailed (architectural) simulation. Simulation is functional by default.\n"
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
	"        --debug-elf: debug information related to ELF files processing.\n"
	"        --debug-syscall: detailed system calls trace and arguments.\n"
	"        --debug-opencl: trace of OpenCL calls and their arguments.\n"
	"        --debug-mem: trace of event-driven simulation for memory system\n"
	"            hierarchy. Must be used with '--gpu-sim detailed'.\n"
	"        --debug-gpu-isa: during the emulation of an OpenCL device kernel, trace\n"
	"            of executed AMD Evergreen ISA instructions.\n"
	"        --debug-gpu-pipeline: trace of AMD Evergreen instructions in the GPU\n"
	"            pipeline. This option requires '--gpu-sim detailed' option.\n"
	"        --debug-gpu-stack: trace of operations on GPU active mask stacks.\n"
	"        --debug-loader: information for the x86 ELF binary analysis performed\n"
	"            by the program loader.\n"
	"        --debug-network: trace of interconnection networks activity.\n"
	"        --debug-call: trace of function calls, based on emulated x86 instr.\n"
	"        --debug-cpu-isa: trace of emulated x86 ISA instructions.\n"
	"        --debug-cpu-pipeline: trace of x86 microinstructions in the CPU pipeline.\n"
	"            The output file can be used as an input for the 'm2s-pipeline' tool\n"
	"            to obtain graphical timing diagrams.\n"
	"        --debug-error: on simulation crashes, dump of the modeled CPU state.\n"\
	"\n"
	"  --gpu-calc <file_prefix>\n"
	"      If this option is set, a kernel execution will cause three GPU occupancy plots\n"
	"      to be dumped in files '<file_prefix>.<ndrange_id>.<plot>.eps', where\n"
	"      <ndrange_id> is the identifier of the current ND-Range, and <plot> is\n"
	"      {work_items|registers|local_mem}. This options requires 'gnuplot' to be\n"
	"      installed in the system.\n"
	"\n"
	"  --gpu-config <file>\n"
	"      Configuration file for the GPU model, including parameters such as number of\n"
	"      compute units, stream cores, or wavefront size. Type 'm2s --help-gpu-config'\n"
	"      for details on the file format.\n"
	"\n"
	"  --gpu-disasm <file>\n"
	"      Disassemble OpenCL kernel binary provided in <file>. This option must be\n"
	"      used with no other options.\n"
	"\n"
	"  --opengl-disasm <file> <index>\n"
	"      Disassemble OpenGL shader binary provided in <file>. The shader is specified by <index>\n"
	"      This option must be used with no other options.\n"
	"\n"
	"  --gpu-sim {functional|detailed}\n"
	"      Functional simulation (emulation) of the AMD Evergreen GPU kernel, versus\n"
	"      detailed (architectural) simulation. Functional simulation is default.\n"
	"\n"
	"  --gpu-visual <file>\n"
	"      Run GPU visualization tool. After running a GPU detailed simulation with a\n"
	"      pipeline trace (option --debug-gpu-pipeline <file>), the generated file can\n"
	"      be used as an input for the visual tool. To enable this option, the GTK\n"
	"      library must be installed in your system.\n"
	"\n"
	"  --help-<xxx>\n"
	"      Show help on the format of configuration files for Multi2Sim. <xxx> stands\n"
	"      for one of the following options:\n"
	"        --help-cpu-config: format of the CPU model configuration file.\n"
	"        --help-ctx-config: format of the context configuration file.\n"
	"        --help-gpu-config: format of the GPU model configuration file.\n"
	"        --help-mem-config: format of the memory system configuration file.\n"
	"\n"
	"  --max-cpu-cycles <num_cycles>\n"
	"      Maximum number of CPU cycles. For functional CPU simulation, one instruction\n"
	"      from each active context is executed every cycle. Use 0 (default) for\n"
	"      unlimited.\n"
	"\n"
	"  --max-cpu-inst <num_inst>\n"
	"      Maximum number of CPU x86 instructions executed. For detailed simulation\n"
	"      this is the number of committed instructions. Use 0 (default) for unlimited.\n"
	"\n"
	"  --max-gpu-cycles <num_cycles>\n"
	"      Maximum number of GPU cycles. For functional GPU simulation, an instruction\n"
	"      from every work-item in every work-group is executed every cycle. Use 0\n"
	"      (default) for no limit.\n"
	"\n"
	"  --max-gpu-inst <num_inst>\n"
	"      Maximum number of GPU instructions. An instruction executed in common for a\n"
	"      whole wavefront counts as 1 toward this limit. Use 0 (default) for no limit.\n"
	"\n"
	"  --max-gpu-kernels <num_kernels>\n"
	"      Maximum number of GPU kernels (0 for no maximum). After kernel <num_kernels>\n"
	"      finishes execution, the simulation will stop.\n"
	"\n"
	"  --max-time <seconds>\n"
	"      Maximum simulation time in seconds. The simulator will stop after this time\n"
	"      is exceeded. Use 0 (default) for no time limit.\n"
	"\n"
	"  --mem-config <file>\n"
	"      Configuration file for memory hierarchy. Run 'm2s --help-mem-config' for a\n"
	"      description of the file format.\n"
	"\n"
	"  --net-injection-rate <rate>\n"
	"      For network simulation, packet injection rate for nodes (e.g. 0.01 means one\n"
	"      packet every 100 cycles on average. Nodes will injects packets into the\n"
	"      network using random delays with exponential distribution with lambda = <rate>.\n"
	"      This option must be used together with '--net-sim'.\n"
	"\n"
	"  --net-config <file>\n"
	"      Network configuration file. All networks are defined here, and referenced from\n"
	"      other configuration files. For a description of the format, please run\n"
	"      'm2s --help-net-config'.\n"
	"\n"
	"  --net-max-cycles <cycles>\n"
	"      Maximum number of cycles for network simulation. This option must be used\n"
	"      together with option '--net-sim'.\n"
	"\n"
	"  --net-msg-size <size>\n"
	"      For network simulation, packet size in bytes. An entire packet is assumed to\n"
	"      fit in a node's buffer, but its transfer latency through a link will depend\n"
	"      on the message size and the link bandwidth. This option must be used together\n"
	"      with '--net-sim'.\n"
	"\n"
	"  --net-sim <network>\n"
	"      Runs a network simulation, where <network> is the name of a network\n"
	"      specified in the network configuration file (option '--net-config').\n"
	"\n"
	"  --opencl-binary <file>\n"
	"      Specify OpenCL kernel binary to be loaded when the OpenCL host program\n"
	"      performs a call to 'clCreateProgramWithSource'. Since on-line compilation\n"
	"      of OpenCL kernels is not supported, this is a possible way to load them.\n"
	"\n"
	"  --report-cpu-pipeline <file>\n"
	"      File to dump a report of the CPU pipeline, such as number of instructions\n"
	"      handled in every pipeline stage, or read/write accesses performed to\n"
	"      pipeline queues (ROB, IQ, etc.). Use only together with a detailed CPU\n"
	"      simulation (option '--cpu-sim detailed').\n"
	"\n"
	"  --report-gpu-kernel <file>\n"
	"      File to dump report of a GPU device kernel emulation. The report includes\n"
	"      statistics about type of instructions, VLIW packing, thread divergence, etc.\n"
	"\n"
	"  --report-gpu-pipeline <file>\n"
	"      File to dump a report of the GPU pipeline, such as active execution engines,\n"
	"      compute units occupancy, stream cores utilization, etc. Use together with a\n"
	"      detailed GPU simulation (option '--gpu-sim detailed').\n"
	"\n"
	"  --report-mem <file>\n"
	"      File for a report on the memory hierarchy, including cache hits, misses,\n"
	"      evictions, etc. Use together with detailed CPU or GPU simulation.\n"
	"\n"
	"  --report-net <file>\n"
	"      File to dump detailed statistics for each network defined in the network\n"
	"      configuration file (option '--net-config'). The report includes statistics\n"
	"      on bandwidth utilization, network traffic, etc.\n"
	"\n";


static char *err_help_note =
	"Please type 'm2s --help' for a list of valid Multi2Sim command-line options.\n";


static void sim_need_argument(int argc, char **argv, int argi)
{
	if (argi == argc - 1)
		fatal("option '%s' required one argument.\n%s",
			argv[argi], err_help_note);
}


static void sim_read_command_line(int *argc_ptr, char **argv)
{
	int argc = *argc_ptr;
	int argi;
	int arg_discard = 0;

	char *net_sim_last_option = NULL;

	for (argi = 1; argi < argc; argi++)
	{
		/* CPU cache configuration - FIXME: remove */
		if (!strcmp(argv[argi], "--cpu-cache-config"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			cache_system_config_file_name = argv[argi];
			continue;
		}

		/* CPU configuration file */
		if (!strcmp(argv[argi], "--cpu-config"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			cpu_config_file_name = argv[argi];
			continue;
		}

		/* CPU disassembler */
		if (!strcmp(argv[argi], "--cpu-disasm"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			cpu_disasm_file_name = argv[argi];
			continue;
		}


		/* CPU simulation accuracy */
		if (!strcmp(argv[argi], "--cpu-sim"))
		{
			sim_need_argument(argc, argv, argi);
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
		if (!strcmp(argv[argi], "--ctx-config") || !strcmp(argv[argi], "-c"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			ctxconfig_file_name = argv[argi];
			continue;
		}

		/* Function calls debug file */
		if (!strcmp(argv[argi], "--debug-call"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			isa_call_debug_file_name = argv[argi];
			continue;
		}
		
		/* CPU ISA debug file */
		if (!strcmp(argv[argi], "--debug-cpu-isa"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			isa_inst_debug_file_name = argv[argi];
			continue;
		}

		/* CPU pipeline debug */
		if (!strcmp(argv[argi], "--debug-cpu-pipeline"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			esim_debug_file_name = argv[argi];
			continue;
		}

		/* Context debug file */
		if (!strcmp(argv[argi], "--debug-ctx"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			ctx_debug_file_name = argv[argi];
			continue;
		}

		/* ELF debug file */
		if (!strcmp(argv[argi], "--debug-elf"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			elf_debug_file_name = argv[argi];
			continue;
		}

		/* Error debug */
		if (!strcmp(argv[argi], "--debug-error"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			error_debug_file_name = argv[argi];
			continue;
		}

		/* GPU ISA debug file */
		if (!strcmp(argv[argi], "--debug-gpu-isa"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_isa_debug_file_name = argv[argi];
			continue;
		}

		/* Interconnect debug file */
		if (!strcmp(argv[argi], "--debug-network"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			net_debug_file_name = argv[argi];
			continue;
		}

		/* GPU pipeline debug file */
		if (!strcmp(argv[argi], "--debug-gpu-pipeline"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_pipeline_debug_file_name = argv[argi];
			continue;
		}

		/* GPU-REL: debug file for stack pushes/pops */
		if (!strcmp(argv[argi], "--debug-gpu-stack"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_stack_debug_file_name = argv[argi];
			continue;
		}

		/* GPU-REL: debug file for faults */
		if (!strcmp(argv[argi], "--debug-gpu-faults"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_faults_debug_file_name = argv[argi];
			continue;
		}

		/* Loader debug file */
		if (!strcmp(argv[argi], "--debug-loader"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			loader_debug_file_name = argv[argi];
			continue;
		}

		/* Memory hierarchy debug file */
		if (!strcmp(argv[argi], "--debug-mem"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			mem_debug_file_name = argv[argi];
			continue;
		}

		/* OpenCL debug file */
		if (!strcmp(argv[argi], "--debug-opencl"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			opencl_debug_file_name = argv[argi];
			continue;
		}

		/* System call debug file */
		if (!strcmp(argv[argi], "--debug-syscall"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			syscall_debug_file_name = argv[argi];
			continue;
		}

		/* GPU occupancy calculation plots */
		if (!strcmp(argv[argi], "--gpu-calc"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_calc_file_name = argv[argi];
			continue;
		}

		/* GPU configuration file */
		if (!strcmp(argv[argi], "--gpu-config"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_config_file_name = argv[argi];
			continue;
		}

		/* GPU disassembler */
		if (!strcmp(argv[argi], "--gpu-disasm"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_disasm_file_name = argv[argi];
			continue;
		}

		/* OpenGL shader binary disassembler */
		if (!strcmp(argv[argi], "--opengl-disasm"))
		{
			if ( argc != 4 )
				fatal("option '%s' required two argument.\n%s",
					argv[argi], err_help_note);			
			argi++;
			opengl_disasm_file_name = argv[argi];
			argi++;
			opengl_disasm_shader_index = atoi(argv[argi]);
			continue;
		}


		/* GPU-REL: file to introduce faults  */
		if (!strcmp(argv[argi], "--gpu-faults"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_faults_file_name = argv[argi];
			continue;
		}

		/* GPU simulation accuracy */
		if (!strcmp(argv[argi], "--gpu-sim"))
		{
			sim_need_argument(argc, argv, argi);
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

		/* GPU visualization */
		if (!strcmp(argv[argi], "--gpu-visual"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_visual_file_name = argv[argi];
			continue;
		}

		/* Show help */
		if (!strcmp(argv[argi], "--help") || !strcmp(argv[argi], "-h"))
		{
			fprintf(stderr, "%s", sim_help);
			continue;
		}

		/* Help for CPU configuration file */
		if (!strcmp(argv[argi], "--help-cpu-config"))
		{
			fprintf(stderr, "%s", cpu_config_help);
			continue;
		}

		/* Help for context configuration file format */
		if (!strcmp(argv[argi], "--help-ctx-config"))
		{
			fprintf(stderr, "%s", ld_help_ctxconfig);
			continue;
		}

		/* Help for GPU configuration file */
		if (!strcmp(argv[argi], "--help-gpu-config"))
		{
			fprintf(stderr, "%s", gpu_config_help);
			continue;
		}

		/* Help for memory hierarchy configuration file */
		if (!strcmp(argv[argi], "--help-mem-config"))
		{
			fprintf(stderr, "%s", mem_config_help);
			continue;
		}

		/* Help for network configuration file */
		if (!strcmp(argv[argi], "--help-net-config"))
		{
			fprintf(stderr, "%s", net_config_help);
			continue;
		}

		/* Maximum number of CPU cycles */
		if (!strcmp(argv[argi], "--max-cpu-cycles"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			ke_max_cycles = atoll(argv[argi]);
			continue;
		}

		/* Maximum number of CPU instructions */
		if (!strcmp(argv[argi], "--max-cpu-inst"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			ke_max_inst = atoll(argv[argi]);
			continue;
		}

		/* Maximum number of GPU cycles */
		if (!strcmp(argv[argi], "--max-gpu-cycles"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_max_cycles = atoll(argv[argi]);
			continue;
		}

		/* Maximum number of GPU instructions */
		if (!strcmp(argv[argi], "--max-gpu-inst"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_max_inst = atoll(argv[argi]);
			continue;
		}

		/* Maximum number of GPU kernels */
		if (!strcmp(argv[argi], "--max-gpu-kernels"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_max_kernels = atoi(argv[argi]);
			continue;
		}

		/* Simulation time limit */
		if (!strcmp(argv[argi], "--max-time"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			ke_max_time = atoll(argv[argi]);
			continue;
		}
		
		/* Memory hierarchy configuration file */
		if (!strcmp(argv[argi], "--mem-config"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			mem_config_file_name = argv[argi];
			continue;
		}

		/* Network configuration file */
		if (!strcmp(argv[argi], "--net-config"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			net_config_file_name = argv[argi];
			continue;
		}

		/* Injection rate for network simulation */
		if (!strcmp(argv[argi], "--net-injection-rate"))
		{
			sim_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_injection_rate = atof(argv[argi]);
			continue;
		}

		/* Cycles for network simulation */
		if (!strcmp(argv[argi], "--net-max-cycles"))
		{
			sim_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_max_cycles = atoll(argv[argi]);
			continue;
		}

		/* Network message size */
		if (!strcmp(argv[argi], "--net-msg-size"))
		{
			sim_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_msg_size = atoi(argv[argi]);
			continue;
		}

		/* Network simulation */
		if (!strcmp(argv[argi], "--net-sim"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			net_sim_network_name = argv[argi];
			continue;
		}

		/* OpenCL binary */
		if (!strcmp(argv[argi], "--opencl-binary"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_opencl_binary_name = argv[argi];
			continue;
		}

		/* CPU pipeline report */
		if (!strcmp(argv[argi], "--report-cpu-pipeline"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			cpu_report_file_name = argv[argi];
			continue;
		}

		/* GPU emulation report */
		if (!strcmp(argv[argi], "--report-gpu-kernel"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_kernel_report_file_name = argv[argi];
			continue;
		}

		/* GPU pipeline report */
		if (!strcmp(argv[argi], "--report-gpu-pipeline"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			gpu_report_file_name = argv[argi];
			continue;
		}

		/* Memory hierarchy report */
		if (!strcmp(argv[argi], "--report-mem"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			mem_report_file_name = argv[argi];
			continue;
		}

		/* Network report file */
		if (!strcmp(argv[argi], "--report-net"))
		{
			sim_need_argument(argc, argv, argi);
			argi++;
			net_report_file_name = argv[argi];
			continue;
		}

		/* Invalid option */
		if (argv[argi][0] == '-')
		{
			fatal("'%s' is not a valid command-line option.\n%s",
				argv[argi], err_help_note);
		}

		/* End of options */
		break;
	}

	/* Check configuration consistency */
	if (cpu_sim_kind == cpu_sim_kind_functional)
	{
		char *msg = "option '%s' not valid for functional CPU simulation.\n"
			"Please use option '--cpu-sim detailed' as well.\n";
		if (*cpu_config_file_name)
			fatal(msg, "--cpu-config");
		if (*esim_debug_file_name)
			fatal(msg, "--debug-cpu-pipeline");
		if (*cpu_report_file_name)
			fatal(msg, "--report-cpu-pipeline");
	}
	if (gpu_sim_kind == gpu_sim_kind_functional)
	{
		char *msg = "option '%s' not valid for functional GPU simulation.\n"
			"Please use option '--gpu-sim detailed' as well.\n";
		if (*gpu_pipeline_debug_file_name)
			fatal(msg, "--debug-gpu-pipeline");
		if (*gpu_stack_debug_file_name)
			fatal(msg, "--debug-gpu-stack");
		if (*gpu_faults_debug_file_name)  /* GPU-REL */
			fatal(msg, "--debug-gpu-faults");
		if (*gpu_config_file_name)
			fatal(msg, "--gpu-config");
		if (*gpu_report_file_name)
			fatal(msg, "--report-gpu-pipeline");
	}
	if (*gpu_visual_file_name && argc > 3)
		fatal("option '--gpu-visual' is incompatible with any other options.");
	if (*gpu_disasm_file_name && argc > 3)
		fatal("option '--gpu-disasm' is incompatible with any other options.");
	if (*opengl_disasm_file_name && argc != 4)
		fatal("option '--opengl-disasm' is incompatible with any other options.");	
	if (*cpu_disasm_file_name && argc > 3)
		fatal("option '--cpu-disasm' is incompatible with other options.");
	if (!*net_sim_network_name && net_sim_last_option)
		fatal("option '%s' requires '--net-sim'", net_sim_last_option);
	if (*net_sim_network_name && !*net_config_file_name)
		fatal("option '--net-sim' requires '--net-config'");

	/* Discard arguments used as options */
	arg_discard = argi - 1;
	for (argi = 1; argi < argc - arg_discard; argi++)
		argv[argi] = argv[argi + arg_discard];
	*argc_ptr = argc - arg_discard;
}


void sim_stats_summary(void)
{
	uint64_t now = ke_timer();
	uint64_t gpu_now = gk_timer();
	uint64_t inst_count;

	double sec_count;
	double inst_per_sec;
	double inst_per_cycle;
	double branch_acc;
	double cycles_per_sec;

	/* Check if any simulation was actually performed */
	inst_count = cpu_sim_kind == cpu_sim_kind_functional ? ke->inst_count : cpu->inst;
	if (!inst_count)
		return;

	/* Statistics */
	fprintf(stderr, "\n");
	fprintf(stderr, ";\n");
	fprintf(stderr, "; Simulation Statistics Summary\n");
	fprintf(stderr, ";\n");
	fprintf(stderr, "\n");

	/* CPU functional simulation */
	sec_count = (double) now / 1e6;
	inst_per_sec = sec_count > 0.0 ? (double) inst_count / sec_count : 0.0;
	fprintf(stderr, "[ CPU ]\n");
	fprintf(stderr, "Time = %.2f\n", sec_count);
	fprintf(stderr, "Instructions = %lld\n", (long long) inst_count);
	fprintf(stderr, "InstructionsPerSecond = %.0f\n", inst_per_sec);
	fprintf(stderr, "Contexts = %d\n", ke->running_list_max);
	fprintf(stderr, "Memory = %lu\n", mem_max_mapped_space);
	fprintf(stderr, "SimEnd = %s\n", map_value(&ke_sim_finish_map, ke_sim_finish));

	/* CPU detailed simulation */
	if (cpu_sim_kind == cpu_sim_kind_detailed) {
		inst_per_cycle = cpu->cycle ? (double) cpu->inst / cpu->cycle : 0.0;
		branch_acc = cpu->branches ? (double) (cpu->branches - cpu->mispred) / cpu->branches : 0.0;
		cycles_per_sec = sec_count > 0.0 ? (double) cpu->cycle / sec_count : 0.0;
		fprintf(stderr, "Cycles = %lld\n", (long long) cpu->cycle);
		fprintf(stderr, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
		fprintf(stderr, "BranchPredictionAccuracy = %.4g\n", branch_acc);
		fprintf(stderr, "CyclesPerSecond = %.0f\n", cycles_per_sec);
	}
	fprintf(stderr, "\n");

	/* GPU functional simulation */
	if (gk->ndrange_count) {
		
		sec_count = (double) gpu_now / 1e6;
		inst_per_sec = sec_count > 0.0 ? (double) gk->inst_count / sec_count : 0.0;
		fprintf(stderr, "[ GPU ]\n");
		fprintf(stderr, "Time = %.2f\n", sec_count);
		fprintf(stderr, "NDRangeCount = %d\n", gk->ndrange_count);
		fprintf(stderr, "Instructions = %lld\n", (long long) gk->inst_count);
		fprintf(stderr, "InstructionsPerSecond = %.0f\n", inst_per_sec);
	
		/* GPU detailed simulation */
		if (gpu_sim_kind == gpu_sim_kind_detailed) {
			inst_per_cycle = gpu->cycle ? (double) gk->inst_count / gpu->cycle : 0.0;
			cycles_per_sec = sec_count > 0.0 ? (double) gpu->cycle / sec_count : 0.0;
			fprintf(stderr, "Cycles = %lld\n", (long long) gpu->cycle);
			fprintf(stderr, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
			fprintf(stderr, "CyclesPerSecond = %.0f\n", cycles_per_sec);
		}
		fprintf(stderr, "\n");
	}
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

	/* Read command line */
	sim_read_command_line(&argc, argv);

	/* CPU disassembler tool */
	if (*cpu_disasm_file_name)
		ke_disasm(cpu_disasm_file_name);

	/* GPU disassembler tool */
	if (*gpu_disasm_file_name)
		gk_disasm(gpu_disasm_file_name);

	/* OpenGL disassembler tool */
	if (*opengl_disasm_file_name)
		gl_disasm(opengl_disasm_file_name, opengl_disasm_shader_index);	

	/* GPU visualization tool */
	if (*gpu_visual_file_name)
		vgpu_run(gpu_visual_file_name);
	
	/* Network simulation tool */
	if (*net_sim_network_name)
	{
		/* Initialize */
		debug_init();
		esim_init();
		net_init();
		net_debug_category = debug_new_category(net_debug_file_name);

		/* Run */
		net_sim();

		/* Finalize */
		net_done();
		esim_done();
		debug_done();
		mhandle_done();
		return 0;
	}

	/* Debug */
	debug_init();
	isa_inst_debug_category = debug_new_category(isa_inst_debug_file_name);
	isa_call_debug_category = debug_new_category(isa_call_debug_file_name);
	elf_debug_category = debug_new_category(elf_debug_file_name);
	net_debug_category = debug_new_category(net_debug_file_name);
	ld_debug_category = debug_new_category(loader_debug_file_name);
	syscall_debug_category = debug_new_category(syscall_debug_file_name);
	ctx_debug_category = debug_new_category(ctx_debug_file_name);
	mem_debug_category = debug_new_category(mem_debug_file_name);
	opencl_debug_category = debug_new_category(opencl_debug_file_name);
	gpu_isa_debug_category = debug_new_category(gpu_isa_debug_file_name);
	gpu_stack_debug_category = debug_new_category(gpu_stack_debug_file_name);  /* GPU-REL */
	gpu_faults_debug_category = debug_new_category(gpu_faults_debug_file_name);  /* GPU-REL */
	gpu_pipeline_debug_category = debug_new_category(gpu_pipeline_debug_file_name);
	error_debug_category = debug_new_category(error_debug_file_name);
	esim_debug_init(esim_debug_file_name);

	/* Initialization for functional simulation */
	ke_init();
	esim_init();
	net_init();

	/* Initialization for detailed simulation */
	if (cpu_sim_kind == cpu_sim_kind_detailed)
		cpu_init();
	if (gpu_sim_kind == gpu_sim_kind_detailed)
		gpu_init();

	/* Memory hierarchy initialization, done after we initialized CPU cores
	 * and GPU compute units. */
	mem_system_init();

	/* Load programs */
	cpu_load_progs(argc, argv, ctxconfig_file_name);

	/* Simulation loop */
	if (ke->running_list_head)
	{
		if (cpu_sim_kind == cpu_sim_kind_detailed)
			cpu_run();
		else
			ke_run();
	}

	/* Flush event-driven simulation */
	esim_process_all_events(0);

	/* Dump statistics summary */
	sim_stats_summary();

	/* Finalization of memory system */
	mem_system_done();

	/* Finalization of detailed CPU simulation */
	if (cpu_sim_kind == cpu_sim_kind_detailed)
	{
		esim_debug_done();
		cpu_done();
	}

	/* Finalization of detailed GPU simulation */
	if (gpu_sim_kind == gpu_sim_kind_detailed)
		gpu_done();

	/* Finalization */
	net_done();
	esim_done();
	ke_done();
	debug_done();
	mhandle_done();

	/* End */
	return 0;
}
