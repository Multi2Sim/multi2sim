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
#include <fermi-emu.h>
#include <fermi-timing.h>
#include <visual-common.h>
#include <southern-islands-emu.h>
#include <southern-islands-timing.h>
#include <arm-emu.h>
#include <arm-timing.h>
#include <esim.h>


static char *ctx_debug_file_name = "";
static char *visual_file_name = "";
static char *ctx_config_file_name = "";
static char *elf_debug_file_name = "";
static char *trace_file_name = "";

static char *x86_disasm_file_name = "";
static char *x86_sys_debug_file_name = "";
static char *x86_loader_debug_file_name = "";
static char *x86_call_debug_file_name = "";
static char *x86_isa_debug_file_name = "";
static char *x86_load_checkpoint_file_name = "";
static char *x86_save_checkpoint_file_name = "";
static char *x86_glut_debug_file_name = "";
static char *x86_opengl_debug_file_name = "";
static char *x86_clrt_debug_file_name = "";

static char *evg_disasm_file_name = "";
static char *evg_isa_debug_file_name = "";
static char *evg_opencl_debug_file_name = "";
static char *evg_opengl_disasm_file_name = "";
static int evg_opengl_disasm_shader_index = 1;
static char *evg_stack_debug_file_name = "";

static char *si_disasm_file_name = "";
static char *si_isa_debug_file_name = "";
static int si_emulator = 0; /* FIXME We need to fix the initialization and selection of devices */

static char *frm_disasm_file_name = "";
static char *frm_cuda_debug_file_name = "";
static int frm_emulator = 0; /* FIXME We need to fix the initialization and selection of devices */

static char *arm_disasm_file_name = "";
static char *arm_loader_debug_file_name = "";
static char *arm_isa_debug_file_name = "";
static char *arm_sys_debug_file_name = "";
static char *arm_call_debug_file_name = "";

static char *mem_debug_file_name = "";

static char *net_debug_file_name = "";

static long long m2s_max_time = 0;  /* Max. simulation time in seconds (0 = no limit) */
static long long m2s_loop_iter = 0;  /* Number of iterations in main simulation loop */

static char *m2s_help =
	"Syntax:\n"
	"\n"
	"        m2s [<options>] [<x86_binary> [<arg_list>]]\n"
	"\n"
	"The user command-line supports a sequence of command-line options, and can\n"
	"include an x86 ELF binary (executable x86 program) followed by its arguments.\n"
	"The execution of this program will be simulated by Multi2Sim, together with the\n"
	"rest of the x86 programs specified in the context configuration file (option\n"
	"'--ctx-config <file>'). The rest of the possible command-line options are\n"
	"classified in categories and listed next:\n"
	"\n"
	"\n"
	"================================================================================\n"
	"General Options\n"
	"================================================================================\n"
	"\n"
	"  --ctx-config <file>\n"
	"      Use <file> as the context configuration file. This file describes the\n"
	"      initial set of running applications, their arguments, and environment\n"
	"      variables. Type 'm2s --ctx-config-help' for a description of the file\n"
	"      format.\n"
	"\n"
	"  --ctx-config-help\n"
	"      Show a help message describing the format of the context configuration\n"
	"      file, passed to the simulator through option '--ctx-config <file>'.\n"
	"\n"
	"  --ctx-debug <file>\n"
	"      Dump debug information related with context creation, destruction,\n"
	"      allocation, or state change.\n"
	"\n"
	"  --elf-debug <file>\n"
	"      Dump debug information related with the analysis of ELF files. Every time\n"
	"      an executable file is open (CPU program of GPU kernel binary), detailed\n"
	"      information about its symbols, sections, strings, etc. is dumped here.\n"
	"\n"
	"  --max-time <time>\n"
	"      Maximum simulation time in seconds. The simulator will stop once this time\n"
	"      is exceeded. A value of 0 (default) means no time limit.\n"
	"\n"
	"  --trace <file>.gz\n"
	"      Generate a trace file with debug information on the configuration of the\n"
	"      modeled CPUs, GPUs, and memory system, as well as their dynamic simulation.\n"
	"      The trace is a compressed plain-text file in format. The user should watch\n"
	"      the size of the generated trace as simulation runs, since the trace file\n"
	"      can quickly become extremely large.\n"
	"\n"
	"  --visual <file>.gz\n"
	"      Run the Multi2Sim Visualization Tool. This option consumes a file\n"
	"      generated with the '--trace' option in a previous simulation. This option\n"
	"      is only available on systems with support for GTK 3.0 or higher.\n"
	"\n"
	"\n"
	"================================================================================\n"
	"x86 CPU Options\n"
	"================================================================================\n"
	"\n"
	"  --x86-config <file>\n"
	"      Configuration file for the x86 CPU timing model, including parameters\n"
	"      describing stage bandwidth, structures size, and other parameters of\n"
	"      processor cores and threads. Type 'm2s --x86-help' for details on the file\n"
	"      format.\n"
	"\n"
	"  --x86-debug-call <file>\n"
	"      Dump debug information about function calls and returns. The control flow\n"
	"      of an x86 program can be observed leveraging ELF symbols present in the\n"
	"      program binary.\n"
	"\n"
	"  --x86-debug-loader <file>\n"
	"      Dump debug information extending the analysis of the ELF program binary.\n"
	"      This information shows which ELF sections and symbols are loaded to the\n"
	"      initial program memory image.\n"
	"\n"
	"  --x86-debug-glut <file>\n"
	"      Debug information for GLUT runtime calls performed by an OpenGL program\n"
	"      based on the GLUT library.\n"
	"\n"
	"  --x86-debug-clrt <file>\n"
	"      Debug information for the newer implementation of the OpenCL runtime\n"
	"      library (not available yet).\n"
	"\n"
	"  --x86-debug-opengl <file>\n"
	"      Debug information for OpenGL runtime calls.\n"
	"\n"
	"  --x86-debug-isa\n"
	"      Debug information for dynamic execution of x86 instructions. Updates on\n"
	"      the processor state can be analyzed using this information.\n"
	"\n"
	"  --x86-debug-syscall\n"
	"      Debug information for system calls performed by an x86 program, including\n"
	"      system call code, arguments, and return value.\n"
	"\n"
	"  --x86-disasm <file>\n"
	"      Disassemble the x86 ELF file provided in <file>, using the internal x86\n"
	"      disassembler. This option is incompatible with any other option.\n"
	"\n"
	"  --x86-help\n"
	"      Display a help message describing the format of the x86 CPU context\n"
	"      configuration file.\n"
	"\n"
	"  --x86-last-inst <bytes>\n"
	"      Stop simulation when the specified instruction is fetched. Can be used to\n"
	"      trigger a checkpoint with option '--x86-save-checkpoint'. The instruction\n"
	"      must be given as a sequence of hexadecimal digits, including trailing\n"
	"      zeros if needed.\n"
	"\n"
	"  --x86-load-checkpoint <file>\n"
	"      Load a checkpoint of the x86 architectural state, created in a previous\n"
	"      execution of the simulator with option '--x86-save-checkpoint'.\n"
	"\n"
	"  --x86-max-cycles <cycles>\n"
	"      Maximum number of cycles for x86 timing simulation. Use 0 (default) for no\n"
	"      limit. This option is only valid for detailed x86 simulation (option\n"
	"      '--x86-sim detailed').\n"
	"  \n"
	"  --x86-max-inst <inst>\n"
	"      Maximum number of CPU x86 instructions executed. For functional x86\n"
	"      simulation, this is the number of instructions emulated. For detailed x86\n"
	"      simulation, it refers to the number of committed instructions,\n"
	"      disregarding speculative instructions. Use 0 (default) for unlimited.\n"
	"  \n"
	"  --x86-report <file>\n"
	"      File to dump a report of the x86 CPU pipeline, including statistics such\n"
	"      as the number of instructions handled in every pipeline stage, read/write\n"
	"      accesses performed on pipeline queues, etc. This option is only valid for\n"
	"      detailed x86 simulation (option '--x86-sim detailed').\n"
	"\n"
	"  --x86-save-checkpoint <file>\n"
	"      Save a checkpoint of x86 architectural state at the end of simulation.\n"
	"      Useful options to use together with this are '--x86-max-inst' and\n"
	"      '--x86-last-inst' to force the simulation to stop and create a checkpoint.\n"
	"\n"
	"  --x86-sim {functional|detailed}\n"
	"      Choose a functional simulation (emulation) of an x86 program, versus\n"
	"      a detailed (architectural) simulation. Simulation is functional by default.\n"
	"\n"
	"\n"
	"================================================================================\n"
	"AMD Evergreen GPU Options\n"
	"================================================================================\n"
	"\n"
	"  --evg-calc <prefix>\n"
	"      If this option is set, a kernel execution will cause three GPU occupancy\n"
	"      plots to be dumped in files '<prefix>.<ndrange_id>.<plot>.eps', where\n"
	"      <ndrange_id> is the identifier of the current ND-Range, and <plot> is\n"
	"      {work_items|registers|local_mem}. This options requires 'gnuplot' to be\n"
	"      installed in the system.\n"
	"\n"
	"  --evg-config <file>\n"
	"      Configuration file for the Evergreen GPU timing model, including\n"
	"      parameters such as number of compute units, stream cores, or wavefront\n"
	"      size. Type 'm2s --evg-help' for details on the file format.\n"
	"\n"
	"  --evg-debug-isa <file>\n"
	"      Dump debug information on the Evergreen ISA instructions emulated, and\n"
	"      their updates in the architectural GPU state.\n"
	"\n"
	"  --evg-debug-opencl <file>\n"
	"      Dump debug information on OpenCL system calls performed by the x86 host\n"
	"      program. The information includes OpenCL call code, arguments, and return\n"
	"      values.\n"
	"\n"
	"  --evg-disasm <file>\n"
	"      Disassemble OpenCL kernel binary provided in <file>. This option must be\n"
	"      used with no other options.\n"
	"\n"
	"  --evg-disasm-opengl <file> <index>\n"
	"      Disassemble OpenGL shader binary provided in <file>. The shader identifier\n"
	"      is specified in <index>. This option must be used with no other options.\n"
	"\n"
	"  --evg-help\n"
	"      Display a help message describing the format of the Evergreen GPU\n"
	"      configuration file, passed with option '--evg-config <file>'.\n"
	"\n"
	"  --evg-kernel-binary <file>\n"
	"      Specify OpenCL kernel binary to be loaded when the OpenCL host program\n"
	"      performs a call to 'clCreateProgramWithSource'. Since on-line compilation\n"
	"      of OpenCL kernels is not supported, this is a possible way to load them.\n"
	"\n"
	"  --evg-max-cycles <cycles>\n"
	"      Maximum number of Evergreen GPU cycles for detailed simulation. Use 0\n"
	"      (default) for no limit.\n"
	"\n"
	"  --evg-max-inst <inst>\n"
	"      Maximum number of Evergreen ISA instructions. An instruction executed in\n"
	"      common for a whole wavefront counts as 1 toward this limit. Use 0\n"
	"      (default) for no limit.\n"
	"\n"
	"  --evg-max-kernels <kernels>\n"
	"      Maximum number of Evergreen GPU kernels (0 for no maximum). After the last\n"
	"      kernel finishes execution, the simulator will stop.\n"
	"\n"
	"  --evg-report-kernel <file>\n"
	"      File to dump report of a GPU device kernel emulation. The report includes\n"
	"      statistics about type of instructions, VLIW packing, thread divergence, etc.\n"
	"\n"
	"  --evg-report <file>\n"
	"      File to dump a report of the GPU pipeline, such as active execution engines,\n"
	"      compute units occupancy, stream cores utilization, etc. Use together with a\n"
	"      detailed GPU simulation (option '--evg-sim detailed').\n"
	"\n"
	"  --evg-sim {functional|detailed}\n"
	"      Functional simulation (emulation) of the AMD Evergreen GPU kernel, versus\n"
	"      detailed (architectural) simulation. Functional simulation is default.\n"
	"\n"
	"\n"
	"================================================================================\n"
	"AMD Southern Islands GPU Options\n"
	"================================================================================\n"
	"\n"
	"  --si-debug-isa <file>\n"
	"      Debug information on the emulation of Southern Islands ISA instructions,\n"
	"      including architectural state updates on registers and memory locations.\n"
	"\n"
	"  --si-disasm <file>\n"
	"      Disassemble a Southern Islands kernel binary. This option is incompatible\n"
	"      with othe command-line options.\n"
	"\n"
	"  --si-kernel-binary <file>\n"
	"      Use <file> as the returned kernel binary upon an OpenCL call to\n"
	"      'clLoadProgramWithSource'.\n"
	"\n"
	"  --si-sim {functional|detailed}\n"
	"      Functional (default) or detailed simulation for the AMD Southern Islands\n"
	"      GPU model.\n"
	"\n"
	"\n"
	"================================================================================\n"
	"ARM CPU Options\n"
	"================================================================================\n"
	"\n"
	"  --arm-disasm <file>\n"
	"      Disassemble an ARM binary using Multi2Sim's internal disassembler. This\n"
	"      option is incompatible with any other command-line option.\n"
	"\n"
	"  --arm-debug-loader <file>\n"
	"      Dump debug information extending the analysis of the ELF program binary.\n"
	"      This information shows which ELF sections and symbols are loaded to the\n"
	"      initial program memory image.\n"
	"\n"
	"  --arm-debug-isa <file>\n"
	"      Debug information for dynamic execution of Arm instructions. Updates on\n"
	"      the processor state can be analyzed using this information.\n"
	"\n"
	"\n"
	"================================================================================\n"
	"NVIDIA Fermi GPU Options\n"
	"================================================================================\n"
	"\n"
	"  --frm-debug-cuda <file>\n"
	"      Debug information on the emulation of Fermi CUDA driver APIs.\n"
	"\n"
	"  --frm-disasm <file>\n"
	"      Disassemble a Fermi kernel binary (cubin format). This option is\n"
	"      incompatible with any other command-line option.\n"
	"\n"
	"  --frm-sim {functional|detailed}\n"
	"      Functional (default) or detailed simulation for the NVIDIA Fermi\n"
	"      GPU model.\n"
	"\n"
	"\n"
	"================================================================================\n"
	"Memory System Options\n"
	"================================================================================\n"
	"\n"
	"  --mem-config <file>\n"
	"      Configuration file for memory hierarchy. Run 'm2s --mem-help' for a\n"
	"      description of the file format.\n"
	"\n"
	"  --mem-debug <file>\n"
	"      Dump debug information about memory accesses, cache memories, main memory,\n"
	"      and directories.\n"
	"\n"
	"  --mem-help\n"
	"      Print help message describing the format of the memory configuration file,\n"
	"      passed to the simulator with option '--mem-config <file>'.\n"
	"\n"
	"  --mem-report\n"
	"      File for a report on the memory hierarchy, including cache hits, misses,\n"
	"      evictions, etc. This option must be used together with detailed simulation\n"
	"      of any CPU/GPU architecture.\n"
	"\n"
	"  --mem-report-access\n"
	"      File to dump a report of memory access. The file contains a list of\n"
	"      allocated memory pages, ordered as per number of accesses. It lists read,\n"
	"      write, and execution accesses to each physical memory page.\n"
	"\n"
	"\n"
	"================================================================================\n"
	"Network Options\n"
	"================================================================================\n"
	"\n"
	"  --net-config <file>\n"
	"      Network configuration file. Networks in the memory hierarchy can be\n"
	"      defined here and referenced in other configuration files. For a\n"
	"      description of the format, use option '--net-help'.\n"
	"\n"
	"  --net-debug\n"
	"      Debug information related with interconnection networks, including packet\n"
	"      transfers, link usage, etc.\n"
	"\n"
	"  --net-help\n"
	"      Print help message describing the network configuration file, passed to\n"
	"      the simulator with option '--net-config <file>'.\n"
	"\n"
	"  --net-injection-rate <rate>\n"
	"      For network simulation, packet injection rate for nodes (e.g. 0.01 means\n"
	"      one packet every 100 cycles on average. Nodes will inject packets into\n"
	"      the network using random delays with exponential distribution with lambda\n"
	"      = <rate>. This option must be used together with '--net-sim'.\n"
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
	"  --net-report <file>\n"
	"      File to dump detailed statistics for each network defined in the network\n"
	"      configuration file (option '--net-config'). The report includes statistics\n"
	"      on bandwidth utilization, network traffic, etc.\n"
	"\n"
	"  --net-sim <network>\n"
	"      Runs a network simulation using synthetic traffic, where <network> is the\n"
	"      name of a network specified in the network configuration file (option\n"
	"      '--net-config').\n"
	"\n";


static char *m2s_err_note =
	"Please type 'm2s --help' for a list of valid Multi2Sim command-line options.\n";


static void m2s_need_argument(int argc, char **argv, int argi)
{
	if (argi == argc - 1)
		fatal("option '%s' required one argument.\n%s",
			argv[argi], m2s_err_note);
}


static void m2s_read_command_line(int *argc_ptr, char **argv)
{
	int argc = *argc_ptr;
	int argi;
	int arg_discard = 0;

	char *net_sim_last_option = NULL;

	for (argi = 1; argi < argc; argi++)
	{
		/*
		 * General Options
		 */

		/* Context configuration file */
		if (!strcmp(argv[argi], "--ctx-config"))
		{
			m2s_need_argument(argc, argv, argi);
			ctx_config_file_name = argv[++argi];
			continue;
		}

		/* Help for context configuration file format */
		if (!strcmp(argv[argi], "--ctx-config-help"))
		{
			fprintf(stderr, "%s", x86_loader_help);
			continue;
		}

		/* Context debug file */
		if (!strcmp(argv[argi], "--ctx-debug"))
		{
			m2s_need_argument(argc, argv, argi);
			ctx_debug_file_name = argv[++argi];
			continue;
		}

		/* ELF debug file */
		if (!strcmp(argv[argi], "--elf-debug"))
		{
			m2s_need_argument(argc, argv, argi);
			elf_debug_file_name = argv[++argi];
			continue;
		}

		/* Show help */
		if (!strcmp(argv[argi], "--help"))
		{
			fprintf(stderr, "%s", m2s_help);
			continue;
		}

		/* Simulation time limit */
		if (!strcmp(argv[argi], "--max-time"))
		{
			m2s_need_argument(argc, argv, argi);
			m2s_max_time = atoll(argv[++argi]);
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


		/*
		 * x86 CPU Options
		 */

		/* CPU configuration file */
		if (!strcmp(argv[argi], "--x86-config"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_config_file_name = argv[++argi];
			continue;
		}

		/* Function calls debug file */
		if (!strcmp(argv[argi], "--x86-debug-call"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_call_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenCL runtime debug file */
		if (!strcmp(argv[argi], "--debug-x86-clrt"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_clrt_debug_file_name = argv[++argi];
			continue;
		}

		/* GLUT debug file */
		if (!strcmp(argv[argi], "--x86-debug-glut"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_glut_debug_file_name = argv[++argi];
			continue;
		}

		/* ISA debug file */
		if (!strcmp(argv[argi], "--x86-debug-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_isa_debug_file_name = argv[++argi];
			continue;
		}

		/* Loader debug file */
		if (!strcmp(argv[argi], "--x86-debug-loader"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_loader_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenGL debug file */
		if (!strcmp(argv[argi], "--x86-debug-opengl"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_opengl_debug_file_name = argv[++argi];
			continue;
		}

		/* System call debug file */
		if (!strcmp(argv[argi], "--x86-debug-syscall"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_sys_debug_file_name = argv[++argi];
			continue;
		}

		/* x86 disassembler */
		if (!strcmp(argv[argi], "--x86-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_disasm_file_name = argv[++argi];
			continue;
		}

		/* Help for x86 CPU configuration file */
		if (!strcmp(argv[argi], "--x86-help"))
		{
			fprintf(stderr, "%s", x86_config_help);
			continue;
		}

		/* Last x86 instruction */
		if (!strcmp(argv[argi], "--x86-last-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_last_inst_bytes = malloc(20);
			if (!x86_emu_last_inst_bytes)
				fatal("%s: out of memory", __FUNCTION__);
			hex_str_to_byte_array(x86_emu_last_inst_bytes, argv[++argi], 20);
			continue;
		}

		/* CPU load checkpoint file name */
		if (!strcmp(argv[argi], "--x86-load-checkpoint"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_load_checkpoint_file_name = argv[++argi];
			continue;
		}

		/* Maximum number of cycles */
		if (!strcmp(argv[argi], "--x86-max-cycles"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_max_cycles = atoll(argv[++argi]);
			continue;
		}

		/* Maximum number of instructions */
		if (!strcmp(argv[argi], "--x86-max-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_emu_max_inst = atoll(argv[++argi]);
			continue;
		}

		/* File name to save checkpoint */
		if (!strcmp(argv[argi], "--x86-save-checkpoint"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_save_checkpoint_file_name = argv[++argi];
			continue;
		}

		/* x86 CPU pipeline report */
		if (!strcmp(argv[argi], "--x86-report"))
		{
			m2s_need_argument(argc, argv, argi);
			x86_cpu_report_file_name = argv[++argi];
			continue;
		}

		/* x86 simulation accuracy */
		if (!strcmp(argv[argi], "--x86-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				x86_emu_kind = x86_emu_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				x86_emu_kind = x86_emu_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], m2s_err_note);
			continue;
		}


		/*
		 * Evergreen GPU Options
		 */

		/* Evergreen GPU occupancy calculation plots */
		if (!strcmp(argv[argi], "--evg-calc"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_gpu_calc_file_name = argv[++argi];
			continue;
		}

		/* Evergreen GPU configuration file */
		if (!strcmp(argv[argi], "--evg-config"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_gpu_config_file_name = argv[++argi];
			continue;
		}

		/* Evergreen ISA debug file */
		if (!strcmp(argv[argi], "--evg-debug-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_isa_debug_file_name = argv[++argi];
			continue;
		}

		/* GPU-REL: debug file for stack pushes/pops */
		if (!strcmp(argv[argi], "--evg-debug-stack"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_stack_debug_file_name = argv[++argi];
			continue;
		}

		/* GPU-REL: debug file for faults */
		if (!strcmp(argv[argi], "--evg-debug-faults"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_faults_debug_file_name = argv[++argi];
			continue;
		}

		/* OpenCL debug file */
		if (!strcmp(argv[argi], "--evg-debug-opencl"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_opencl_debug_file_name = argv[++argi];
			continue;
		}

		/* Evergreen disassembler */
		if (!strcmp(argv[argi], "--evg-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_disasm_file_name = argv[++argi];
			continue;
		}

		/* Evergreen OpenGL shader binary disassembler */
		if (!strcmp(argv[argi], "--evg-disasm-opengl"))
		{
			if (argc != 4)
				fatal("option '%s' requires two argument.\n%s",
					argv[argi], m2s_err_note);
			evg_opengl_disasm_file_name = argv[++argi];
			evg_opengl_disasm_shader_index = atoi(argv[++argi]);
			continue;
		}

		/* GPU-REL: file to introduce faults  */
		if (!strcmp(argv[argi], "--evg-faults"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_faults_file_name = argv[++argi];
			continue;
		}

		/* Help for GPU configuration file */
		if (!strcmp(argv[argi], "--evg-help"))
		{
			fprintf(stderr, "%s", evg_gpu_config_help);
			continue;
		}

		/* Evergreen OpenCL kernel binary */
		if (!strcmp(argv[argi], "--evg-kernel-binary"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_opencl_binary_name = argv[++argi];
			continue;
		}

		/* Maximum number of cycles */
		if (!strcmp(argv[argi], "--evg-max-cycles"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_max_cycles = atoll(argv[++argi]);
			continue;
		}

		/* Maximum number of GPU instructions */
		if (!strcmp(argv[argi], "--evg-max-inst"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_max_inst = atoll(argv[++argi]);
			continue;
		}

		/* Maximum number of GPU kernels */
		if (!strcmp(argv[argi], "--evg-max-kernels"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_max_kernels = atoi(argv[++argi]);
			continue;
		}

		/* Evergreen GPU timing report */
		if (!strcmp(argv[argi], "--evg-report"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_gpu_report_file_name = argv[++argi];
			continue;
		}

		/* Evergreen GPU emulation report */
		if (!strcmp(argv[argi], "--evg-report-kernel"))
		{
			m2s_need_argument(argc, argv, argi);
			evg_emu_report_file_name = argv[++argi];
			continue;
		}

		/* Evergreen simulation accuracy */
		if (!strcmp(argv[argi], "--evg-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				evg_emu_kind = evg_emu_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				evg_emu_kind = evg_emu_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], m2s_err_note);
			continue;
		}


		/*
		 * Southern Islands GPU Options
		 */

		/* Southern Islands ISA debug file */
		if (!strcmp(argv[argi], "--si-debug-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			si_isa_debug_file_name = argv[++argi];
			continue;
		}

		/* Southern Islands GPU configuration file */
		if (!strcmp(argv[argi], "--si-config"))
		{
			m2s_need_argument(argc, argv, argi);
			si_gpu_config_file_name = argv[++argi];
			continue;
		}


		/* Souther Islands disassembler */
		if (!strcmp(argv[argi], "--si-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			si_disasm_file_name = argv[++argi];
			continue;
		}

		/* Southern Islands OpenCL binary */
		if (!strcmp(argv[argi], "--si-kernel-binary"))
		{
			m2s_need_argument(argc, argv, argi);
			si_emu_opencl_binary_name = argv[++argi];
			continue;
		}

		/* Southern Islands simulation accuracy */
		if (!strcmp(argv[argi], "--si-sim"))
		{
			si_emulator = 1;
			m2s_need_argument(argc, argv, argi);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				si_emu_kind = si_emu_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				si_emu_kind = si_emu_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], m2s_err_note);
			continue;
		}


		/*
		 * Fermi GPU Options
		 */

		/* Fermi CUDA debug file */
		if (!strcmp(argv[argi], "--frm-debug-cuda"))
		{
			m2s_need_argument(argc, argv, argi);
			frm_cuda_debug_file_name = argv[++argi];
			continue;
		}

		/* Fermi disassembler */
		if (!strcmp(argv[argi], "--frm-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			frm_disasm_file_name = argv[++argi];
			continue;
		}

		/* Fermi simulation accuracy */
		if (!strcmp(argv[argi], "--frm-sim"))
		{
			frm_emulator = 1;
			m2s_need_argument(argc, argv, argi);
			argi++;
			if (!strcasecmp(argv[argi], "functional"))
				frm_emu_kind = frm_emu_kind_functional;
			else if (!strcasecmp(argv[argi], "detailed"))
				frm_emu_kind = frm_emu_kind_detailed;
			else
				fatal("option '%s': invalid argument ('%s').\n%s",
					argv[argi - 1], argv[argi], m2s_err_note);
			continue;
		}


		/*
		 * ARM CPU Options
		 */

		/* Arm disassembler */
		if (!strcmp(argv[argi], "--arm-disasm"))
		{
			m2s_need_argument(argc, argv, argi);
			arm_disasm_file_name = argv[++argi];
			continue;
		}

		/* Arm loader debug file */
		if (!strcmp(argv[argi], "--arm-debug-loader"))
		{
			m2s_need_argument(argc, argv, argi);
			arm_loader_debug_file_name = argv[++argi];
			continue;
		}

		/* Arm isa debug file */
		if (!strcmp(argv[argi], "--arm-debug-isa"))
		{
			m2s_need_argument(argc, argv, argi);
			arm_isa_debug_file_name = argv[++argi];
			continue;
		}

		/* System call debug file */
		if (!strcmp(argv[argi], "--arm-debug-syscall"))
		{
			m2s_need_argument(argc, argv, argi);
			arm_sys_debug_file_name = argv[++argi];
			continue;
		}

		/* Function calls debug file */
		if (!strcmp(argv[argi], "--arm-debug-call"))
		{
			m2s_need_argument(argc, argv, argi);
			arm_call_debug_file_name = argv[++argi];
			continue;
		}

		/*
		 * Memory System Options
		 */

		/* Memory hierarchy configuration file */
		if (!strcmp(argv[argi], "--mem-config"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_config_file_name = argv[++argi];
			continue;
		}

		/* Memory hierarchy debug file */
		if (!strcmp(argv[argi], "--mem-debug"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_debug_file_name = argv[++argi];
			continue;
		}

		/* Help for memory hierarchy configuration file */
		if (!strcmp(argv[argi], "--mem-help"))
		{
			fprintf(stderr, "%s", mem_config_help);
			continue;
		}

		/* Memory hierarchy report */
		if (!strcmp(argv[argi], "--mem-report"))
		{
			m2s_need_argument(argc, argv, argi);
			mem_report_file_name = argv[++argi];
			continue;
		}

		/* Memory accesses report */
		if (!strcmp(argv[argi], "--mem-report-access"))
		{
			m2s_need_argument(argc, argv, argi);
			mmu_report_file_name = argv[++argi];
			continue;
		}


		/*
		 * Network Options
		 */

		/* Interconnect debug file */
		if (!strcmp(argv[argi], "--net-debug"))
		{
			m2s_need_argument(argc, argv, argi);
			net_debug_file_name = argv[++argi];
			continue;
		}

		/* Network configuration file */
		if (!strcmp(argv[argi], "--net-config"))
		{
			m2s_need_argument(argc, argv, argi);
			net_config_file_name = argv[++argi];
			continue;
		}

		/* Help for network configuration file */
		if (!strcmp(argv[argi], "--net-help"))
		{
			fprintf(stderr, "%s", net_config_help);
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

		/* Network report file */
		if (!strcmp(argv[argi], "--net-report"))
		{
			m2s_need_argument(argc, argv, argi);
			net_report_file_name = argv[++argi];
			continue;
		}

		/* Network simulation */
		if (!strcmp(argv[argi], "--net-sim"))
		{
			m2s_need_argument(argc, argv, argi);
			net_sim_network_name = argv[++argi];
			continue;
		}


		/*
		 * Rest
		 */

		/* Invalid option */
		if (argv[argi][0] == '-')
		{
			fatal("'%s' is not a valid command-line option.\n%s",
				argv[argi], m2s_err_note);
		}

		/* End of options */
		break;
	}

	/* Options only allowed for x86 detailed simulation */
	if (x86_emu_kind == x86_emu_kind_functional)
	{
		char *msg = "option '%s' not valid for functional x86 simulation.\n"
			"\tPlease use option '--x86-sim detailed' as well.\n";

		if (*x86_config_file_name)
			fatal(msg, "--x86-config");
		if (x86_emu_max_cycles)
			fatal(msg, "--x86-max-cycles");
		if (*x86_cpu_report_file_name)
			fatal(msg, "--x86-report");
	}

	/* Options that only make sense for GPU detailed simulation */
	if (evg_emu_kind == evg_emu_kind_functional)
	{
		char *msg = "option '%s' not valid for functional GPU simulation.\n"
			"\tPlease use option '--evg-sim detailed' as well.\n";

		if (*evg_gpu_config_file_name)
			fatal(msg, "--evg-config");
		if (*evg_faults_debug_file_name)  /* GPU-REL */
			fatal(msg, "--evg-debug-faults");
		if (*evg_stack_debug_file_name)
			fatal(msg, "--evg-debug-stack");
		if (evg_emu_max_cycles)
			fatal(msg, "--evg-max-cycles");
		if (*evg_gpu_report_file_name)
			fatal(msg, "--evg-report");
	}

	/* Options that only make sense for GPU detailed simulation */
	if (si_emu_kind == si_emu_kind_functional)
	{
		char *msg = "option '%s' not valid for functional GPU simulation.\n"
			"\tPlease use option '--si-sim detailed' as well.\n";

		if (*si_gpu_config_file_name)
			fatal(msg, "--si-config");
		if (si_emu_max_cycles)
			fatal(msg, "--si-max-cycles");
		if (*si_gpu_report_file_name)
			fatal(msg, "--si-report");
	}

	/* Options that only make sense when there is at least one architecture
	 * using detailed simulation. */
	if (evg_emu_kind == evg_emu_kind_functional
		&& x86_emu_kind == x86_emu_kind_functional
		&& si_emu_kind == si_emu_kind_functional
		&& arm_emu_kind == arm_emu_kind_functional
		&& frm_emu_kind == frm_emu_kind_functional)
	{
		char *msg = "option '%s' needs architectural simulation.\n"
			"\tPlease use option '--<arch>-sim detailed' as well, where <arch> is any\n"
			"\tof the architectures supported in Multi2Sim.\n";

		if (*mmu_report_file_name)
			fatal(msg, "--mem-report");
		if (*mem_config_file_name)
			fatal(msg, "--mem-config");
		if (*mem_report_file_name)
			fatal(msg, "--report-mem");
	}

	/* Other checks */
	if (*evg_disasm_file_name && argc > 3)
		fatal("option '--evg-disasm' is incompatible with any other options.");
	if (*si_disasm_file_name && argc > 3)
		fatal("option '--si-disasm' is incompatible with any other options.");
	if (*evg_opengl_disasm_file_name && argc != 4)
		fatal("option '--evg-disasm-opengl' is incompatible with any other options.");	
	if (*frm_disasm_file_name && argc > 3)
		fatal("option '--frm-disasm' is incompatible with any other options.");
	if (*x86_disasm_file_name && argc > 3)
		fatal("option '--x86-disasm' is incompatible with other options.");
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


void m2s_load_programs(int argc, char **argv)
{
	struct config_t *config;

	char section[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];
	char *exe_file_name;
	char *cwd_path;

	Elf32_Ehdr ehdr;

	int id;

	/* Load guest program specified in the command line */
	if (argc > 1)
	{
		/* Load program depending on architecture */
		elf_file_read_header(argv[1], &ehdr);
		switch (ehdr.e_machine)
		{
		case EM_386:
			x86_loader_load_from_command_line(argc - 1, argv + 1);
			break;

		case EM_ARM:
			arm_ctx_load_from_command_line(argc - 1, argv + 1);
			break;

		default:
			fatal("%s: unsupported ELF architecture", argv[1]);
		}
	}

	/* Continue processing the context configuration file, if specified. */
	if (!*ctx_config_file_name)
		return;

	/* Open file */
	config = config_create(ctx_config_file_name);
	if (!config_load(config))
		fatal("%s: cannot open context configuration file",
			ctx_config_file_name);

	/* Iterate through consecutive contexts */
	for (id = 0; ; id++)
	{
		/* Read section */
		snprintf(section, sizeof section, "Context %d", id);
		if (!config_section_exists(config, section))
			break;

		/* Read executable full path */
		exe_file_name = config_read_string(config, section, "Exe", "");
		cwd_path = config_read_string(config, section, "Cwd", "");
		file_full_path(exe_file_name, cwd_path, exe_full_path, sizeof exe_full_path);

		/* Load context depending on architecture */
		elf_file_read_header(exe_full_path, &ehdr);
		switch (ehdr.e_machine)
		{
		case EM_386:
			x86_loader_load_from_ctx_config(config, section);
			break;

		case EM_ARM:
			arm_ctx_load_from_ctx_config(config, section);
			break;

		default:
			fatal("%s: unsupported ELF architecture", argv[1]);
		}
	}

	/* Close file */
	config_check(config);
	config_free(config);

}


void m2s_dump_summary(FILE *f)
{
	double time_in_sec;

	/* No summary dumped if no simulation was run */
	if (m2s_loop_iter < 2)
		return;

	/* Header */
	fprintf(f, "\n");
	fprintf(f, ";\n");
	fprintf(f, "; Simulation Statistics Summary\n");
	fprintf(f, ";\n");
	fprintf(f, "\n");

	/* Calculate statistics */
	time_in_sec = (double) esim_real_time() / 1.0e6;

	/* General statistics */
	fprintf(f, "[ General ]\n");
	fprintf(f, "Time = %.2f\n", time_in_sec);
	fprintf(f, "SimEnd = %s\n", map_value(&esim_finish_map, esim_finish));

	/* General detailed simulation statistics */
	if (esim_cycle > 1)
	{
		fprintf(f, "Cycles = %lld\n", esim_cycle);
	}

	/* End */
	fprintf(f, "\n");

	/* x86 */
	x86_emu_dump_summary(f);

	/* Arm */
	arm_emu_dump_summary(f);

	/* Evergreen */
	evg_emu_dump_summary(f);

	/* Southern Islands */
	si_emu_dump_summary(f);
}


/* Signal handler while functional simulation loop is running */
void m2s_signal_handler(int signum)
{
	switch (signum)
	{

	case SIGINT:

		/* Second time signal was received, abort. */
		if (esim_finish)
			abort();

		/* Try to normally finish simulation */
		esim_finish = esim_finish_signal;
		fprintf(stderr, "SIGINT received\n");
		break;

	default:

		fprintf(stderr, "Signal %d received\n", signum);
		exit(1);
	}
}


void m2s_loop(void)
{
	int timing_running;
	int emu_running;

	while (!esim_finish)
	{
		/* Assume initially that no architecture is doing any useful emulation
		 * or timing simulation. */
		emu_running = 0;
		timing_running = 0;

		/* x86 CPU simulation */
		if (x86_emu_kind == x86_emu_kind_functional)
			emu_running |= x86_emu_run();
		else
			timing_running |= x86_cpu_run();

		/* Evergreen GPU simulation */
		if (evg_emu_kind == evg_emu_kind_functional)
			emu_running |= evg_emu_run();
		else
			timing_running |= evg_gpu_run();

		/* Southern Islands GPU simulation */
		if (si_emu_kind == si_emu_kind_functional)
			emu_running |= si_emu_run();
		else
			timing_running |= si_gpu_run();

		/* arm CPU simulation */
		if (arm_emu_kind == arm_emu_kind_functional)
			emu_running |= arm_emu_run();
		else
			timing_running |= arm_cpu_run();


		/* Event-driven simulation. Only process events and advance to next global
		 * simulation cycle if any architecture performed a useful timing simulation. */
		if (timing_running)
			esim_process_events();

		/* If neither functional or timing simulation was performed for any architecture,
		 * it means that all guest contexts finished execution - simulation can end. */
		if (!emu_running && !timing_running)
			esim_finish = esim_finish_ctx;

		/* Count loop iterations, and check for limit in simulation time only every
		 * 128k iterations. This avoids a constant overhead of system calls. */
		m2s_loop_iter++;
		if (m2s_max_time && !(m2s_loop_iter & ((1 << 17) - 1))
			&& esim_real_time() > m2s_max_time * 1000000)
			esim_finish = esim_finish_max_time;
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
	if (*x86_disasm_file_name)
		x86_disasm_file(x86_disasm_file_name);

	/* Evergreen disassembler tool */
	if (*evg_disasm_file_name)
		evg_emu_disasm(evg_disasm_file_name);

	/* SI disassembler tool */
	if (*si_disasm_file_name)
		si_disasm(si_disasm_file_name);

	/* OpenGL disassembler tool */
	if (*evg_opengl_disasm_file_name)
		evg_emu_opengl_disasm(evg_opengl_disasm_file_name, evg_opengl_disasm_shader_index);

	/* Fermi disassembler tool */
	if (*frm_disasm_file_name)
		frm_emu_disasm(frm_disasm_file_name);

	/* Arm disassembler tool */
	if (*arm_disasm_file_name)
		arm_emu_disasm(arm_disasm_file_name);

	/* Memory hierarchy visualization tool */
	if (*visual_file_name)
		visual_run(visual_file_name);

	/* Network simulation tool */
	if (*net_sim_network_name)
		net_sim(net_debug_file_name);

	/* Debug */
	debug_init();
	x86_isa_inst_debug_category = debug_new_category(x86_isa_debug_file_name);
	x86_isa_call_debug_category = debug_new_category(x86_call_debug_file_name);
	elf_debug_category = debug_new_category(elf_debug_file_name);
	net_debug_category = debug_new_category(net_debug_file_name);
	x86_loader_debug_category = debug_new_category(x86_loader_debug_file_name);
	x86_sys_debug_category = debug_new_category(x86_sys_debug_file_name);
	x86_ctx_debug_category = debug_new_category(ctx_debug_file_name);
	mem_debug_category = debug_new_category(mem_debug_file_name);
	evg_opencl_debug_category = debug_new_category(evg_opencl_debug_file_name);
	evg_isa_debug_category = debug_new_category(evg_isa_debug_file_name);
	evg_stack_debug_category = debug_new_category(evg_stack_debug_file_name);  /* GPU-REL */
	evg_faults_debug_category = debug_new_category(evg_faults_debug_file_name);  /* GPU-REL */
	si_opencl_debug_category = debug_new_category(evg_opencl_debug_file_name);
	si_isa_debug_category = debug_new_category(si_isa_debug_file_name);
	frm_cuda_debug_category = debug_new_category(frm_cuda_debug_file_name);
	x86_glut_debug_category = debug_new_category(x86_glut_debug_file_name);
	x86_clrt_debug_category = debug_new_category(x86_clrt_debug_file_name);
	x86_opengl_debug_category = debug_new_category(x86_opengl_debug_file_name);
	arm_loader_debug_category = debug_new_category(arm_loader_debug_file_name);
	arm_isa_inst_debug_category = debug_new_category(arm_isa_debug_file_name);
	arm_sys_debug_category = debug_new_category(arm_sys_debug_file_name);
	arm_isa_call_debug_category = debug_new_category(arm_call_debug_file_name);
	/* Trace */
	trace_init(trace_file_name);
	mem_trace_category = trace_new_category();
	x86_trace_category = trace_new_category();

	/* Initialization for functional simulation */
	esim_init();
	x86_emu_init();
	arm_emu_init();
	net_init();

	/* Select the GPU emulator */
	if (si_emulator)
		x86_emu->gpu_emulator = gpu_emulator_si;

	/* Initialization for detailed simulation */
	if (x86_emu_kind == x86_emu_kind_detailed)
		x86_cpu_init();
	if (evg_emu_kind == evg_emu_kind_detailed)
	{
		evg_trace_category = trace_new_category();
		evg_gpu_init();
	}
	if (si_emu_kind == si_emu_kind_detailed)
	{
		si_trace_category = trace_new_category();
		si_gpu_init();
	}

	/* Memory hierarchy initialization, done after we initialized CPU cores
	 * and GPU compute units. */
	mem_system_init();

	/* Load architectural state checkpoint */
	if (x86_load_checkpoint_file_name[0])
		x86_checkpoint_load(x86_load_checkpoint_file_name);

	/* Load programs */
	m2s_load_programs(argc, argv);

	/* Install signal handlers */
	signal(SIGINT, &m2s_signal_handler);
	signal(SIGABRT, &m2s_signal_handler);

	/* Multi2Sim Central Simulation Loop */
	m2s_loop();

	/* Save architectural state checkpoint */
	if (x86_save_checkpoint_file_name[0])
		x86_checkpoint_save(x86_save_checkpoint_file_name);

	free(x86_emu_last_inst_bytes);
	free(arm_emu_last_inst_bytes);

	/* Restore default signal handlers */
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);

	/* Flush event-driven simulation, only if the reason for simulation
	 * completion was not a simulation stall. If it was, draining the
	 * event-driven simulation could cause another stall! */
	if (esim_finish != esim_finish_stall)
		esim_process_all_events();

	/* Dump statistics summary */
	m2s_dump_summary(stderr);

	/* Finalization of memory system */
	mem_system_done();

	/* Finalization of detailed CPU simulation */
	if (x86_emu_kind == x86_emu_kind_detailed)
		x86_cpu_done();

	/* Finalization of detailed GPU simulation */
	if (evg_emu_kind == evg_emu_kind_detailed)
		evg_gpu_done();

	/* Finalization of detailed GPU simulation */
	if (si_emu_kind == si_emu_kind_detailed)
		si_gpu_done();

	/* Finalization */
	net_done();
	esim_done();
	trace_done();
	x86_emu_done();
	arm_emu_done();
	debug_done();
	mhandle_done();

	/* End */
	return 0;
}
