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

#include <southern-islands-asm.h>
#include <evergreen-timing.h>
#include <x86-timing.h>
#include <fermi-timing.h>
#include <visual-common.h>
#include <southern-islands-asm.h>


/* Multi2Sim version */
#ifndef VERSION
#define VERSION ""
#endif

static char *ctx_debug_file_name = "";
static char *syscall_debug_file_name = "";
static char *opencl_debug_file_name = "";
static char *cpu_disasm_file_name = "";
static char *gpu_disasm_file_name = "";
static char *si_disasm_file_name = "";
static char *si_isa_debug_file_name = "";
static char *opengl_disasm_file_name = "";
static char *fermi_disasm_file_name = "";
static char *gpu_stack_debug_file_name = "";
static char *gpu_isa_debug_file_name = "";
static char *visual_file_name = "";
static char *mem_debug_file_name = "";
static char *loader_debug_file_name = "";
static char *isa_call_debug_file_name = "";
static char *isa_inst_debug_file_name = "";
static char *error_debug_file_name = "";
static char *ctxconfig_file_name = "";
static char *elf_debug_file_name = "";
static char *net_debug_file_name = "";
static char *trace_file_name = "";
static char *x86_glut_debug_file_name = "";
static char *x86_opengl_debug_file_name = "";
static char *x86_clrt_debug_file_name = "";

static int opengl_disasm_shader_index = 1;

/* FIXME We need to fix the initialization and selection of devices */
static int si_emulator = 0;

/* Error debug */
int x86_cpu_error_debug_category;


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
	"        --debug-x86-glut: trace of GLUT runtime calls (x86).\n"
	"        --debug-x86-clrt: trace of OpenCL runtime calls (x86).\n"
	"        --debug-x86-opengl: trace of OpenGL runtime calls (x86).\n"
	"        --debug-mem: trace of event-driven simulation for memory system\n"
	"            hierarchy. Must be used with '--gpu-sim detailed'.\n"
	"        --debug-gpu-isa: during the emulation of an OpenCL device kernel, trace\n"
	"            of executed AMD Evergreen ISA instructions.\n"
	"        --debug-gpu-stack: trace of operations on GPU active mask stacks.\n"
	"        --debug-loader: information for the x86 ELF binary analysis performed\n"
	"            by the program loader.\n"
	"        --debug-network: trace of interconnection networks activity.\n"
	"        --debug-call: trace of function calls, based on emulated x86 instr.\n"
	"        --debug-cpu-isa: trace of emulated x86 ISA instructions.\n"
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
	"  --report-mem-access <file>\n"
	"      File to dump a report of memory access. The file contains a list of allocated\n"
	"      memory pages, ordered as per number of accesses. It lists read, write, and\n"
	"      execution accesses to each physical memory page.\n"
	"\n"
	"  --report-net <file>\n"
	"      File to dump detailed statistics for each network defined in the network\n"
	"      configuration file (option '--net-config'). The report includes statistics\n"
	"      on bandwidth utilization, network traffic, etc.\n"
	"\n"
	"  --trace <file>.gz\n"
	"      Generate a trace file with debug information on the configuration of the\n"
	"      modeled CPUs, GPUs, and memory system, as well as their dynamic simulation.\n"
	"      The trace is a compressed plain-text file in format.\n"
	"      The user should watch the size of the generated trace as simulation runs,\n"
	"      since the trace file can quickly become extremely large.\n"
	"\n"
	"  --visual <file>.gz\n"
	"      Run the Multi2Sim Visualization Tool. This option consumes a file generated\n"
	"      with the '--trace' option in a previous simulation. This option is only\n"
	"      available on systems with support for GTK 3.0 or higher.\n"
	"\n";


static char *err_help_note =
	"Please type 'm2s --help' for a list of valid Multi2Sim command-line options.\n";


static void m2s_need_argument(int argc, char **argv, int argi)
{
	if (argi == argc - 1)
		fatal("option '%s' required one argument.\n%s",
			argv[argi], err_help_note);
}


static void m2s_read_command_line(int *argc_ptr, char **argv)
{
	int argc = *argc_ptr;
	int argi;
	int arg_discard = 0;

	char *net_sim_last_option = NULL;

	for (argi = 1; argi < argc; argi++)
	{
		/* CPU configuration file */
		if (!strcmp(argv[argi], "--cpu-config"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_cpu_config_file_name = argv[++argi];
			continue;
		}

		/* CPU disassembler */
		if (!strcmp(argv[argi], "--cpu-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			cpu_disasm_file_name = argv[++argi];
			continue;
		}


		/* CPU simulation accuracy */
		if (!strcmp(argv[argi], "--cpu-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				x86_emu_kind = x86_emu_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				x86_emu_kind = x86_emu_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], err_help_note);
			continue;
		}

		/* Context configuration file */
		if (!strcmp(argv[argi], "--ctx-config") || !strcmp(argv[argi], "-c"))
		{
			m2s_need_argument(argc, argv, argi);
			ctxconfig_file_name = argv[++argi];
			continue;
		}

		/* Function calls debug file */
		if (!strcmp(argv[argi], "--debug-call"))
		{
			m2s_need_argument(argc, argv, argi);
			isa_call_debug_file_name = argv[++argi];
			continue;
		}
		
		/* CPU ISA debug file */
		if (!strcmp(argv[argi], "--debug-cpu-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			isa_inst_debug_file_name = argv[++argi];
			continue;
		}

		/* Context debug file */
		if (!strcmp(argv[argi], "--debug-ctx"))
		{
			m2s_need_argument(argc, argv, argi);
			ctx_debug_file_name = argv[++argi];
			continue;
		}

		/* ELF debug file */
		if (!strcmp(argv[argi], "--debug-elf"))
		{
			m2s_need_argument(argc, argv, argi);
			elf_debug_file_name = argv[++argi];
			continue;
		}

		/* Error debug */
		if (!strcmp(argv[argi], "--debug-error"))
		{
			m2s_need_argument(argc, argv, argi);
			error_debug_file_name = argv[++argi];
			continue;
		}

		/* Evergreen ISA debug file */
		if (!strcmp(argv[argi], "--debug-gpu-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			gpu_isa_debug_file_name = argv[++argi];
			continue;
		}

		/* Southern Islands ISA debug file */
		if (!strcmp(argv[argi], "--debug-si-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			si_isa_debug_file_name = argv[++argi];
			continue;
		}

		/* GPU-REL: debug file for stack pushes/pops */
		if (!strcmp(argv[argi], "--debug-gpu-stack"))
		{
			m2s_need_argument(argc, argv, argi);
			gpu_stack_debug_file_name = argv[++argi];
			continue;
		}

		/* GPU-REL: debug file for faults */
		if (!strcmp(argv[argi], "--debug-gpu-faults"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_faults_debug_file_name = argv[++argi];
			continue;
		}

		/* Loader debug file */
		if (!strcmp(argv[argi], "--debug-loader"))
		{
			m2s_need_argument(argc, argv, argi);
			loader_debug_file_name = argv[++argi];
			continue;
		}

		/* Memory hierarchy debug file */
		if (!strcmp(argv[argi], "--debug-mem"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_debug_file_name = argv[++argi];
			continue;
		}

		/* Interconnect debug file */
		if (!strcmp(argv[argi], "--debug-network"))
		{
			m2s_need_argument(argc, argv, argi);
			net_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenCL debug file */
		if (!strcmp(argv[argi], "--debug-opencl"))
		{
			m2s_need_argument(argc, argv, argi);
			opencl_debug_file_name = argv[++argi];
			continue;
		}

		/* System call debug file */
		if (!strcmp(argv[argi], "--debug-syscall"))
		{
			m2s_need_argument(argc, argv, argi);
			syscall_debug_file_name = argv[++argi];
			continue;
		}

		/* GLUT debug file */
		if (!strcmp(argv[argi], "--debug-x86-glut"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_glut_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenCL runtime debug file */
		if (!strcmp(argv[argi], "--debug-x86-clrt"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_clrt_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenGL debug file */
		if (!strcmp(argv[argi], "--debug-x86-opengl"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_opengl_debug_file_name = argv[++argi];
			continue;
		}
		/* GPU occupancy calculation plots */
		if (!strcmp(argv[argi], "--gpu-calc"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_gpu_calc_file_name = argv[++argi];
			continue;
		}

		/* GPU configuration file */
		if (!strcmp(argv[argi], "--gpu-config"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_gpu_config_file_name = argv[++argi];
			continue;
		}

		/* GPU disassembler */
		if (!strcmp(argv[argi], "--gpu-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			gpu_disasm_file_name = argv[++argi];
			continue;
		}

		/* SI disassembler */
		if (!strcmp(argv[argi], "--si-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			si_disasm_file_name = argv[++argi];
			continue;
		}

		/* OpenGL shader binary disassembler */
		if (!strcmp(argv[argi], "--opengl-disasm"))
		{
			if (argc != 4)
				fatal("option '%s' requires two argument.\n%s",
					argv[argi], err_help_note);			
			opengl_disasm_file_name = argv[++argi];
			opengl_disasm_shader_index = atoi(argv[++argi]);
			continue;
		}

		/* Fermi disassembler */
		if (!strcmp(argv[argi], "--fermi-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			fermi_disasm_file_name = argv[++argi];
			continue;
		}


		/* GPU-REL: file to introduce faults  */
		if (!strcmp(argv[argi], "--gpu-faults"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_faults_file_name = argv[++argi];
			continue;
		}

		/* GPU simulation accuracy */
		if (!strcmp(argv[argi], "--gpu-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				evg_emu_kind = evg_emu_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				evg_emu_kind = evg_emu_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], err_help_note);
			continue;
		}

		/* Southern Islands simulation accuracy */
		if (!strcmp(argv[argi], "--si-sim"))
		{
			si_emulator = 1;
			m2s_need_argument(argc, argv, argi);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				si_emu_kind = evg_emu_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				si_emu_kind = evg_emu_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], err_help_note);
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
			fprintf(stderr, "%s", x86_cpu_config_help);
			continue;
		}

		/* Help for context configuration file format */
		if (!strcmp(argv[argi], "--help-ctx-config"))
		{
			fprintf(stderr, "%s", x86_loader_help_ctxconfig);
			continue;
		}

		/* Help for GPU configuration file */
		if (!strcmp(argv[argi], "--help-gpu-config"))
		{
			fprintf(stderr, "%s", evg_gpu_config_help);
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
			m2s_need_argument(argc, argv, argi);
			x86_emu_max_cycles = atoll(argv[++argi]);
			continue;
		}

		/* Maximum number of CPU instructions */
		if (!strcmp(argv[argi], "--max-cpu-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_max_inst = atoll(argv[++argi]);
			continue;
		}

		/* Maximum number of GPU cycles */
		if (!strcmp(argv[argi], "--max-gpu-cycles"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_max_cycles = atoll(argv[++argi]);
			continue;
		}

		/* Maximum number of GPU instructions */
		if (!strcmp(argv[argi], "--max-gpu-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_max_inst = atoll(argv[++argi]);
			continue;
		}

		/* Maximum number of GPU kernels */
		if (!strcmp(argv[argi], "--max-gpu-kernels"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_max_kernels = atoi(argv[++argi]);
			continue;
		}

		/* Simulation time limit */
		if (!strcmp(argv[argi], "--max-time"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_max_time = atoll(argv[++argi]);
			continue;
		}
		
		/* Memory hierarchy configuration file */
		if (!strcmp(argv[argi], "--mem-config"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_config_file_name = argv[++argi];
			continue;
		}

		/* Network configuration file */
		if (!strcmp(argv[argi], "--net-config"))
		{
			m2s_need_argument(argc, argv, argi);
			net_config_file_name = argv[++argi];
			continue;
		}

		/* Injection rate for network simulation */
		if (!strcmp(argv[argi], "--net-injection-rate"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_injection_rate = atof(argv[argi]);
			continue;
		}

		/* Cycles for network simulation */
		if (!strcmp(argv[argi], "--net-max-cycles"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_max_cycles = atoll(argv[argi]);
			continue;
		}

		/* Network message size */
		if (!strcmp(argv[argi], "--net-msg-size"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_last_option = argv[argi];
			argi++;
			net_msg_size = atoi(argv[argi]);
			continue;
		}

		/* Network simulation */
		if (!strcmp(argv[argi], "--net-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_network_name = argv[++argi];
			continue;
		}

		/* Evergreen OpenCL binary */
		if (!strcmp(argv[argi], "--opencl-binary"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_opencl_binary_name = argv[++argi];
			continue;
		}

		/* Southern Islands OpenCL binary */
		if (!strcmp(argv[argi], "--si-opencl-binary"))
		{
			m2s_need_argument(argc, argv, argi);
			si_emu_opencl_binary_name = argv[++argi];
			continue;
		}

		/* CPU pipeline report */
		if (!strcmp(argv[argi], "--report-cpu-pipeline"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_cpu_report_file_name = argv[++argi];
			continue;
		}

		/* GPU emulation report */
		if (!strcmp(argv[argi], "--report-gpu-kernel"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_report_file_name = argv[++argi];
			continue;
		}

		/* GPU pipeline report */
		if (!strcmp(argv[argi], "--report-gpu-pipeline"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_gpu_report_file_name = argv[++argi];
			continue;
		}

		/* Memory hierarchy report */
		if (!strcmp(argv[argi], "--report-mem"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_report_file_name = argv[++argi];
			continue;
		}

		/* Memory accesses report */
		if (!strcmp(argv[argi], "--report-mem-access"))
		{
			m2s_need_argument(argc, argv, argi);
			mmu_report_file_name = argv[++argi];
			continue;
		}

		/* Network report file */
		if (!strcmp(argv[argi], "--report-net"))
		{
			m2s_need_argument(argc, argv, argi);
			net_report_file_name = argv[++argi];
			continue;
		}

		/* Simulation trace */
		if (!strcmp(argv[argi], "--trace"))
		{
			m2s_need_argument(argc, argv, argi);
			trace_file_name = argv[++argi];
			continue;
		}

		/* Visualization tool */
		if (!strcmp(argv[argi], "--visual"))
		{
			m2s_need_argument(argc, argv, argi);
			visual_file_name = argv[++argi];
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

	/* Options that only make sense for CPU detailed simulation */
	if (x86_emu_kind == x86_emu_kind_functional)
	{
		char *msg = "option '%s' not valid for functional CPU simulation.\n"
			"\tPlease use option '--cpu-sim detailed' as well.\n";

		if (*x86_cpu_config_file_name)
			fatal(msg, "--cpu-config");
		if (*x86_cpu_report_file_name)
			fatal(msg, "--report-cpu-pipeline");
	}

	/* Options that only make sense for GPU detailed simulation */
	if (evg_emu_kind == evg_emu_kind_functional)
	{
		char *msg = "option '%s' not valid for functional GPU simulation.\n"
			"\tPlease use option '--gpu-sim detailed' as well.\n";

		if (*gpu_stack_debug_file_name)
			fatal(msg, "--debug-gpu-stack");
		if (*evg_faults_debug_file_name)  /* GPU-REL */
			fatal(msg, "--debug-gpu-faults");
		if (*evg_gpu_config_file_name)
			fatal(msg, "--gpu-config");
		if (*evg_gpu_report_file_name)
			fatal(msg, "--report-gpu-pipeline");
	}

	/* Options that only make sense one there is either CPU or GPU
	 * detailed simulation. */
	if (evg_emu_kind == evg_emu_kind_functional && x86_emu_kind == x86_emu_kind_functional)
	{
		char *msg = "option '%s' needs architectural CPU or GPU simulation.\n"
			"\tPlease use option '--cpu-sim detailed' or '--gpu-sim detailed' as well.\n";

		if (*mmu_report_file_name)
			fatal(msg, "--report-mem-access");
		if (*mem_config_file_name)
			fatal(msg, "--mem-config");
		if (*mem_report_file_name)
			fatal(msg, "--report-mem");
	}

	/* Other checks */
	if (*gpu_disasm_file_name && argc > 3)
		fatal("option '--gpu-disasm' is incompatible with any other options.");
	if (*si_disasm_file_name && argc > 3)
		fatal("option '--si-disasm' is incompatible with any other options.");
	if (*opengl_disasm_file_name && argc != 4)
		fatal("option '--opengl-disasm' is incompatible with any other options.");	
	if (*fermi_disasm_file_name && argc > 3)
		fatal("option '--fermi-disasm' is incompatible with any other options.");
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


void m2s_stats_summary(void)
{
	long long now = esim_real_time();

	long long evg_now;
	long long si_now;

	long long inst_count;
	long long fast_forward_inst_count;

	double sec_count;
	double inst_per_sec;
	double inst_per_cycle;
	double branch_acc;
	double cycles_per_sec;

	/* Get x86 CPU instruction count */
	if (x86_emu_kind == x86_emu_kind_functional)
	{
		inst_count = x86_emu->inst_count;
		fast_forward_inst_count = 0;
	}
	else
	{
		inst_count = x86_cpu->inst;
		fast_forward_inst_count = x86_cpu->fast_forward_inst_count;
	}

	/* No statistic dump if no x86 instruction was executed (i.e., no simulation) */
	if (!inst_count && !fast_forward_inst_count)
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
	fprintf(stderr, "Instructions = %lld\n", inst_count);
	fprintf(stderr, "FastForwardInstructions = %lld\n", fast_forward_inst_count);
	fprintf(stderr, "InstructionsPerSecond = %.0f\n", inst_per_sec);
	fprintf(stderr, "Contexts = %d\n", x86_emu->running_list_max);
	fprintf(stderr, "Memory = %lu\n", mem_max_mapped_space);
	fprintf(stderr, "SimEnd = %s\n", map_value(&x86_emu_finish_map, x86_emu_finish));

	/* CPU detailed simulation */
	if (x86_emu_kind == x86_emu_kind_detailed)
	{
		inst_per_cycle = x86_cpu->cycle ? (double) x86_cpu->inst / x86_cpu->cycle : 0.0;
		branch_acc = x86_cpu->branches ? (double) (x86_cpu->branches - x86_cpu->mispred) / x86_cpu->branches : 0.0;
		cycles_per_sec = sec_count > 0.0 ? (double) x86_cpu->cycle / sec_count : 0.0;
		fprintf(stderr, "Cycles = %lld\n", x86_cpu->cycle);
		fprintf(stderr, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
		fprintf(stderr, "BranchPredictionAccuracy = %.4g\n", branch_acc);
		fprintf(stderr, "CyclesPerSecond = %.0f\n", cycles_per_sec);
	}
	fprintf(stderr, "\n");

	/* Evergreen functional simulation */
	if (evg_emu->ndrange_count)
	{
		evg_now = m2s_timer_get_value(evg_emu->timer);
		sec_count = (double) evg_now / 1e6;
		inst_per_sec = sec_count > 0.0 ? (double) evg_emu->inst_count / sec_count : 0.0;
		fprintf(stderr, "[ GPU ]\n");
		fprintf(stderr, "Time = %.2f\n", sec_count);
		fprintf(stderr, "NDRangeCount = %d\n", evg_emu->ndrange_count);
		fprintf(stderr, "Instructions = %lld\n", evg_emu->inst_count);
		fprintf(stderr, "InstructionsPerSecond = %.0f\n", inst_per_sec);
	
		/* Evergreen detailed simulation */
		if (evg_emu_kind == evg_emu_kind_detailed)
		{
			inst_per_cycle = evg_gpu->cycle ? (double) evg_emu->inst_count / evg_gpu->cycle : 0.0;
			cycles_per_sec = sec_count > 0.0 ? (double) evg_gpu->cycle / sec_count : 0.0;
			fprintf(stderr, "Cycles = %lld\n", evg_gpu->cycle);
			fprintf(stderr, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
			fprintf(stderr, "CyclesPerSecond = %.0f\n", cycles_per_sec);
		}
		fprintf(stderr, "\n");
	}

	/* Southern Islands functional simulation */
	if (si_emu->ndrange_count)
	{
		si_now = m2s_timer_get_value(si_emu->timer);
		sec_count = (double) si_now / 1e6;
		inst_per_sec = sec_count > 0.0 ? (double) si_emu->inst_count / sec_count : 0.0;
		fprintf(stderr, "[ GPU ]\n");
		fprintf(stderr, "Time = %.2f\n", sec_count);
		fprintf(stderr, "NDRangeCount = %d\n", si_emu->ndrange_count);
		fprintf(stderr, "Scalar ALU Instructions = %lld\n", si_emu->scalar_alu_inst_count);
		fprintf(stderr, "Scalar Memory Instructions = %lld\n", si_emu->scalar_mem_inst_count);
		fprintf(stderr, "Vector ALU Instructions = %lld\n", si_emu->vector_alu_inst_count);
		fprintf(stderr, "Vector Memory Instructions = %lld\n", si_emu->vector_mem_inst_count);
		fprintf(stderr, "Total Instructions = %lld\n", si_emu->inst_count);
		fprintf(stderr, "InstructionsPerSecond = %.0f\n", inst_per_sec);
	
		/* Southern Islands detailed simulation */
		if (si_emu_kind == si_emu_detailed)
		{
			assert(0);
			inst_per_cycle = evg_gpu->cycle ? (double) evg_emu->inst_count / evg_gpu->cycle : 0.0;
			cycles_per_sec = sec_count > 0.0 ? (double) evg_gpu->cycle / sec_count : 0.0;
			fprintf(stderr, "Cycles = %lld\n", evg_gpu->cycle);
			fprintf(stderr, "InstructionsPerCycle = %.4g\n", inst_per_cycle);
			fprintf(stderr, "CyclesPerSecond = %.0f\n", cycles_per_sec);
		}
		fprintf(stderr, "\n");
	}
}


/* Signal handler while functional simulation loop is running */
void m2s_signal_handler(int signum)
{
	switch (signum)
	{

	case SIGINT:

		/* Second time signal was received, abort. */
		if (x86_emu_finish)
			abort();

		/* Try to normally finish simulation */
		x86_emu_finish = x86_emu_finish_signal;
		fprintf(stderr, "SIGINT received\n");
		break;

	default:

		fprintf(stderr, "Signal %d received\n", signum);
		exit(1);
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
	m2s_read_command_line(&argc, argv);

	/* CPU disassembler tool */
	if (*cpu_disasm_file_name)
		x86_emu_disasm(cpu_disasm_file_name);

	/* Evergreen disassembler tool */
	if (*gpu_disasm_file_name)
		evg_emu_disasm(gpu_disasm_file_name);

	/* SI disassembler tool */
	if (*si_disasm_file_name)
		si_disasm(si_disasm_file_name);

	/* OpenGL disassembler tool */
	if (*opengl_disasm_file_name)
		evg_emu_opengl_disasm(opengl_disasm_file_name, opengl_disasm_shader_index);	

	/* Fermi disassembler tool */
	if (*fermi_disasm_file_name)
		frm_emu_disasm(fermi_disasm_file_name);

	/* Memory hierarchy visualization tool */
	if (*visual_file_name)
		visual_run(visual_file_name);

	/* Network simulation tool */
	if (*net_sim_network_name)
		net_sim(net_debug_file_name);

	/* Debug */
	debug_init();
	x86_isa_inst_debug_category = debug_new_category(isa_inst_debug_file_name);
	x86_isa_call_debug_category = debug_new_category(isa_call_debug_file_name);
	elf_debug_category = debug_new_category(elf_debug_file_name);
	net_debug_category = debug_new_category(net_debug_file_name);
	x86_loader_debug_category = debug_new_category(loader_debug_file_name);
	x86_sys_debug_category = debug_new_category(syscall_debug_file_name);
	x86_ctx_debug_category = debug_new_category(ctx_debug_file_name);
	mem_debug_category = debug_new_category(mem_debug_file_name);
	evg_opencl_debug_category = debug_new_category(opencl_debug_file_name);
	evg_isa_debug_category = debug_new_category(gpu_isa_debug_file_name);
	evg_stack_debug_category = debug_new_category(gpu_stack_debug_file_name);  /* GPU-REL */
	evg_faults_debug_category = debug_new_category(evg_faults_debug_file_name);  /* GPU-REL */
	si_opencl_debug_category = debug_new_category(opencl_debug_file_name);
	si_isa_debug_category = debug_new_category(si_isa_debug_file_name);
	x86_cpu_error_debug_category = debug_new_category(error_debug_file_name);
	x86_glut_debug_category = debug_new_category(x86_glut_debug_file_name);
	x86_clrt_debug_category = debug_new_category(x86_clrt_debug_file_name);
	x86_opengl_debug_category = debug_new_category(x86_opengl_debug_file_name);

	/* Trace */
	trace_init(trace_file_name);
	mem_trace_category = trace_new_category();
	evg_trace_category = trace_new_category();
	x86_trace_category = trace_new_category();

	/* Initialization for functional simulation */
	esim_init();
	x86_emu_init();
	net_init();

	/* Select the GPU emulator */
	if (si_emulator)
		x86_emu->gpu_emulator = gpu_emulator_si;

	/* Initialization for detailed simulation */
	if (x86_emu_kind == x86_emu_kind_detailed)
		x86_cpu_init();
	if (evg_emu_kind == evg_emu_kind_detailed)
		evg_gpu_init();

	/* Memory hierarchy initialization, done after we initialized CPU cores
	 * and GPU compute units. */
	mem_system_init();

	/* Load programs */
	x86_cpu_load_progs(argc, argv, ctxconfig_file_name);

	/* Install signal handlers */
	signal(SIGINT, &m2s_signal_handler);
	signal(SIGABRT, &m2s_signal_handler);

	/* Simulation loop */
	while (!x86_emu_finish)
	{
		/* x86 CPU simulation */
		if (x86_emu_kind == x86_emu_kind_detailed)
			x86_cpu_run();
		else
			x86_emu_run();

		/* Evergreen GPU simulation */
		if (evg_emu_kind == evg_emu_kind_detailed)
			evg_gpu_run();
		else
			evg_emu_run();

		/* Event-driven simulation */
		esim_process_events();
	}

	/* Restore default signal handlers */
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);

	/* Flush event-driven simulation */
	esim_process_all_events();

	/* Dump statistics summary */
	m2s_stats_summary();

	/* Finalization of memory system */
	mem_system_done();

	/* Finalization of detailed CPU simulation */
	if (x86_emu_kind == x86_emu_kind_detailed)
		x86_cpu_done();

	/* Finalization of detailed GPU simulation */
	if (evg_emu_kind == evg_emu_kind_detailed)
		evg_gpu_done();

	/* Finalization */
	net_done();
	esim_done();
	trace_done();
	x86_emu_done();
	debug_done();
	mhandle_done();

	/* End */
	return 0;
}
