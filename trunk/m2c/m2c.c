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

#include <assert.h>
#include <getopt.h>

#include <m2c/amd/amd.h>
#include <m2c/gl/gl.h>
#include <m2c/cl2llvm/cl2llvm.h>
#include <m2c/common/basic-block.h>
#include <m2c/common/ctree.h>
#include <m2c/common/node.h>
#include <m2c/frm2bin/frm2bin.h>
#include <m2c/llvm2si/llvm2si.h>
#include <m2c/si2bin/si2bin.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/list.h>
#include <lib/util/string.h>



/*
 * Global Variables
 */

/* Output file name passed with option '-o' */
char m2c_out_file_name[MAX_STRING_SIZE];

/* OpenGL compiler options */
int m2c_gl_run;  /* Run OpenGL native compiler */

int m2c_amd_run;  /* Run AMD native compiler */
int m2c_preprocess_run;  /* Run stand-alone preprocessor */
int m2c_cl2llvm_run;  /* Run OpenCL-to-LLVM stand-alone front-end */
int m2c_frm2bin_run;  /* Run Fermi stand-alone assembler */
int m2c_llvm2si_run;  /* Run LLVM-to-SI stand-alone back-end */
int m2c_si2bin_run;  /* Run Southern Islands stand-alone assembler */
int m2c_opt_level = 2;  /* Optimization level */

/* File names computed from source files */
struct list_t *m2c_source_file_list;  /* Source file names */
struct list_t *m2c_clp_file_list;  /* Preprocessed source list, extension '.clp' */
struct list_t *m2c_llvm_file_list;  /* LLVM files, extension, '.llvm' */
struct list_t *m2c_asm_file_list;  /* Assembly files, extension '.s' */
struct list_t *m2c_bin_file_list;  /* Binary files, extension '.bin' */

/* List of macros passed with '-D' options in the command line. */
struct list_t *m2c_define_list;  /* Elements of type 'char *' */





static char *syntax =
	"\n"
	"Syntax:\n"
	"\n"
	"\tm2c [<options>] <sources>\n"
	"\n"
	"Options:\n"
	"\n"
	"--amd\n"
	"\tUse AMD's OpenCL driver installed on the machine to compile the\n"
	"\tsources. This tool will act as a command-line wrapper of the native\n"
	"\tAMD compiler. If the '-O<opt>' option is used, it will be passed\n"
	"\tdirectly to the native compiler.\n"
	"\n"
	"--amd-list\n"
	"\tPrint a list of available devices for the native AMD driver. This\n"
	"\toption should be used together with option '--amd'.\n"
	"\n"
	"--amd-device <device1>[,<device2>...]\n"
	"\tSelect a list of target devices for native AMD compilation. There\n"
	"\tshould be no spaces between device names/identifiers when separated\n"
	"\tby commas. When more than one device is selected, all binaries are\n"
	"\tpacked into one single Multi2Sim-specific ELF binary format.\n"
	"\tThis option must be combined with option '--amd-.\n"
	"\n"
	"--amd-dump-all\n"
	"\tDump all intermediate files is generated during compilation. This\n"
	"\toption must be used together with '--amd'.\n"
	"\n"
	"--cl2llvm\n"
	"\tRun stand-alone OpenCL C to LLVM front-end, consuming OpenCL C\n"
	"\tsource files and generating LLVM outputs with the '.llvm'\n"
	"\tfile extension.\n"
	"\n"
	"--define <symbol>=<value>, -D<symbol>=<value>\n"
	"\tAdd a definition for additional symbols, equivalent to #define\n"
	"\tcompiler directives. This argument can be used multiple times.\n"
	"\n"
	"--frm-asm, --frm2bin\n"
	"\tTreat the input files as source files containing Fermi assembly\n"
	"\tcode. Run the Fermi assembler and generate a CUDA kernel binary.\n"
	"\n"
	"--gl\n"
	"\tUse the OpenGL driver present on the machine to compile OpenGL\n"
	"\tshaders. When this option is used, the list of sources files\n"
	"\tshould be composed of exactly two file names: the first for\n"
	"\tthe vertex shader and the second for the fragment shader.\n"
	"\n"
	"--gl-control <file>\n"
	"\tSpecify a source file containing an OpenGL control shader. This\n"
	"\toption must be used together with '--gl'.\n"
	"\n"
	"--gl-dump-all\n"
	"\tDump all intermediate files generated during the compilation of\n"
	"\tand OpenGL shader. This option must be used with '--gl'.\n"
	"\n"
	"--gl-eval <file>\n"
	"\tSpecify a source file containing an OpenGL evaluation shader.\n"
	"\tThis option must be used together with '--gl'.\n"
	"\n"
	"--gl-geo <file>\n"
	"\tSpecify a source file containing an OpenGL geometry shader.\n"
	"\tThis option must be used together with '--gl'.\n"
	"\n"
	"--help, -h\n"
	"\tShow help message with command-line options.\n"
	"\n"
	"--llvm2si\n"
	"\tInterpret sources as LLVM binaries and generate Southern Islands\n"
	"\tassembly output in a '.s' file.\n"
	"\n"
	"--llvm2si-config <file>\n"
	"\tConfiguration file for Southern Islands back-end.\n"
	"\n"
	"--llvm2si-debug <file>\n"
	"\tDebug information for the Southern Islands back-end.\n"
	"\n"
	"-m <arch>\n"
	"\tTarget architecture for the generated binary. Each assembler has a\n"
	"\tdifferent set of possible values (* means default):\n\n"
	"\tSouthern Islands: capeverde, pitcairn, tahiti*\n"
	"\n"
	"-o <file>\n"
	"\tOutput kernel binary. If no output file is specified, each kernel\n"
	"\tsource is compiled into a kernel binary with the same name but\n"
	"\tusing the '.bin' extension. When used, the option affects the\n"
	"\tfirst file in the list of sources.\n"
	"\n"
	"-O <level> (default = -O2)\n"
	"\tOptimization level.\n"
	"\n"
	"--preprocess, -E\n"
	"\tRun the stand-alone C preprocessor. This command is equivalent to\n"
	"\tan external call to command 'cpp', replacing compiler directives\n"
	"\tand macros.\n"
	"\n"
	"--si-asm, --si2bin\n"
	"\tTreat the input files as source files containing Southern Islands\n"
	"\tassembly code. Run the Southern Islands assembler and generate a\n"
	"\tkernel binary.\n"
	"\n";



static void m2c_process_option(const char *option, char *optarg)
{
	/*
	 * Native AMD related options
	 */
	
	if (!strcmp(option, "amd"))
	{
		m2c_amd_run = 1;
		return;
	}

	if (!strcmp(option, "amd-dump-all"))
	{
		amd_dump_all = 1;
		return;
	}

	if (!strcmp(option, "amd-device"))
	{
		amd_device_name = optarg;
		return;
	}

	if (!strcmp(option, "amd-list"))
	{
		amd_list_devices = 1;
		return;
	}

	if (!strcmp(option, "define") || !strcmp(option, "D"))
	{
		list_add(m2c_define_list, xstrdup(optarg));
		return;
	}

	if (!strcmp(option, "cl2llvm"))
	{
		m2c_cl2llvm_run = 1;
		return;
	}

	if (!strcmp(option, "ctree-config"))
	{
		ctree_config_file_name = optarg;
		return;
	}

	if (!strcmp(option, "ctree-debug"))
	{
		ctree_debug_file_name = optarg;
		return;
	}

	if (!strcmp(option, "frm-asm") || !strcmp(option, "frm2bin"))
	{
		m2c_frm2bin_run = 1;
		return;
	}

	if (!strcmp(option, "gl"))
	{
		m2c_gl_run = 1;
		return;
	}

	if (!strcmp(option, "gl-control"))
	{
		gl_control_shader = optarg;
		return;
	}

	if (!strcmp(option, "gl-dump-all"))
	{
		gl_dump_all = 1;
		return;
	}

	if (!strcmp(option, "gl-eval"))
	{
		gl_eval_shader = optarg;
		return;
	}

	if (!strcmp(option, "gl-geo"))
	{
		gl_geo_shader = optarg;
		return;
	}
	
	if (!strcmp(option, "help") || !strcmp(option, "h"))
	{
		printf("%s", syntax);
		exit(0);
	}

	if (!strcmp(option, "llvm2si"))
	{
		m2c_llvm2si_run = 1;
		return;
	}

	if (!strcmp(option, "m"))
	{
		si2bin_machine_name = optarg;
		return;
	}

	if (!strcmp(option, "o"))
	{
		snprintf(m2c_out_file_name, sizeof m2c_out_file_name,
				"%s", optarg);
		return;
	}

	if (!strcmp(option, "O"))
	{
		int err;
		m2c_opt_level = str_to_int(optarg, &err);
		amd_opt_level = m2c_opt_level;
		if (err)
			fatal("%s: %s", optarg, str_error(err));
		if (!IN_RANGE(m2c_opt_level, 0, 2))
			fatal("%s: invalid value", optarg);
		return;
	}

	if (!strcmp(option, "preprocess") || !strcmp(option, "E"))
	{
		m2c_preprocess_run = 1;
		return;
	}

	if (!strcmp(option, "si-asm") || !strcmp(option, "si2bin"))
	{
		m2c_si2bin_run = 1;
		return;
	}


	/* Option not found. Error message has been shown already by the call
	 * to 'getopts'. */
	exit(1);
}


static void m2c_read_command_line(int argc, char **argv)
{
	int option_index = 0;
	int opt;
	char option[10];

	static struct option long_options[] =
	{
		{ "amd", no_argument, 0, 0 },
		{ "amd-device", required_argument, 0, 0 },
		{ "amd-dump-all", no_argument, 0, 0 },
		{ "amd-list", no_argument, 0, 0 },
		{ "cl2llvm", no_argument, 0, 0 },
		{ "ctree-config", required_argument, 0, 0 },
		{ "ctree-debug", required_argument, 0, 0 },
		{ "frm-asm", no_argument, 0, 0 },
		{ "frm2bin", no_argument, 0, 0 },
		{ "gl", no_argument, 0, 0 },
		{ "gl-control", required_argument, 0, 0 },
		{ "gl-dump-all", no_argument, 0, 0 },
		{ "gl-eval", required_argument, 0, 0 },
		{ "gl-geo", required_argument, 0, 0 },
		{ "define", required_argument, 0, 'D' },
		{ "help", no_argument, 0, 'h' },
		{ "llvm2si", no_argument, 0, 0 },
		{ "preprocess", no_argument, 0, 'E' },
		{ "si-asm", no_argument, 0, 0 },
		{ "si2bin", no_argument, 0, 0 },
		{ 0, 0, 0, 0 }
	};
	
	/* No arguments given */
	if (argc == 1)
	{
		printf("\n");
		printf("Multi2Sim " VERSION " OpenCL C Compiler\n");
		printf("Please run 'm2c --help' for a list of command-line options\n");
		printf("\n");
		exit(0);
	}
	
	/* Process options */
	while ((opt = getopt_long(argc, argv, "ho:D:EO:m:", long_options,
			&option_index)) != -1)
	{
		if (opt)
		{
			option[0] = opt;
			option[1] = '\0';
			m2c_process_option(option, optarg);
		}
		else
		{
			m2c_process_option(long_options[option_index].name,
				optarg);
		}
	}

	/* The rest are source files */
	while (optind < argc)
		list_add(m2c_source_file_list, xstrdup(argv[optind++]));
}


/* If a file was specified with option '-o', replace the first file name in the
 * list with the output file. */
static void m2c_replace_out_file_name(struct list_t *file_list)
{
	char *file_name;

	/* Nothing to do if output file name is not given */
	if (!m2c_out_file_name[0])
		return;

	/* Free old string */
	assert(list_count(file_list));
	file_name = list_get(file_list, 0);
	free(file_name);

	/* Set new name */
	file_name = xstrdup(m2c_out_file_name);
	list_set(file_list, 0, file_name);
}


static void m2c_read_source_files(void)
{
	char *file_name_ptr;
	char file_name[MAX_STRING_SIZE];
	char file_name_prefix[MAX_STRING_SIZE];
	int index;

	/* Nothing to do for no sources */
	if (!m2c_source_file_list->count)
		return;

	/* Create file names */
	LIST_FOR_EACH(m2c_source_file_list, index)
	{
		char *dot_str;
		char *slash_str;

		/* Get file name */
		file_name_ptr = list_get(m2c_source_file_list, index);

		/* Get position of last '.' after last '/' */
		dot_str = rindex(file_name_ptr, '.');
		slash_str = rindex(file_name_ptr, '/');
		if (!dot_str || slash_str > dot_str)
			dot_str = file_name_ptr + strlen(file_name_ptr);

		/* Get prefix */
		str_substr(file_name_prefix, sizeof file_name_prefix,
				file_name_ptr, 0, dot_str - file_name_ptr);

		/* Pre-processed source with '.clp' extension */
		snprintf(file_name, sizeof file_name, "%s.clp", file_name_prefix);
		list_add(m2c_clp_file_list, xstrdup(file_name));

		/* LLVM binary with '.llvm' extension */
		snprintf(file_name, sizeof file_name, "%s.llvm", file_name_prefix);
		list_add(m2c_llvm_file_list, xstrdup(file_name));

		/* Assembly code with '.s' extension */
		snprintf(file_name, sizeof file_name, "%s.s", file_name_prefix);
		list_add(m2c_asm_file_list, xstrdup(file_name));

		/* Final binary with '.bin' extension */
		snprintf(file_name, sizeof file_name, "%s.bin", file_name_prefix);
		list_add(m2c_bin_file_list, xstrdup(file_name));
	}
}


static void m2c_preprocess(struct list_t *source_file_list,
		struct list_t *clp_file_list)
{
	char cmd[MAX_LONG_STRING_SIZE];
	char *cmd_ptr;
	int cmd_size;

	char *source_file;
	char *clp_file;
	char *define;

	int index;
	int ret;
	int j;

	LIST_FOR_EACH(source_file_list, index)
	{
		/* Get files */
		source_file = list_get(source_file_list, index);
		clp_file = list_get(clp_file_list, index);
		assert(source_file);
		assert(clp_file);

		/* Initialize command */
		cmd[0] = '\0';
		cmd_ptr = cmd;
		cmd_size = sizeof cmd;
		str_printf(&cmd_ptr, &cmd_size, "cpp %s -o %s", source_file, clp_file);

		/* Add '-D' flags */
		LIST_FOR_EACH(m2c_define_list, j)
		{
			define = list_get(m2c_define_list, j);
			str_printf(&cmd_ptr, &cmd_size, " -D%s", define);
		}

		/* Check command exceeding size */
		if (!cmd_size)
			fatal("%s: 'cpp' command exceeds buffer size",
					__FUNCTION__);

		/* Run command */
		ret = system(cmd);
		if (ret == -1)
			fatal("%s: cannot run preprocessor, command 'cpp' not found",
					__FUNCTION__);

		/* Any other error by 'cpp' */
		if (ret)
			exit(ret);
	}
}


/* Initialization before reading command line */
void m2c_pre_init(void)
{
	m2c_define_list = list_create();
	m2c_source_file_list = list_create();
}


void m2c_init(void)
{
	/* Classes */
	CLASS_REGISTER(Node);
	CLASS_REGISTER(LeafNode);
	CLASS_REGISTER(AbstractNode);
	CLASS_REGISTER(BasicBlock);
	CLASS_REGISTER(CTree);

	/* Libraries */
	debug_init();

	/* List of source files */
	m2c_clp_file_list = list_create();
	m2c_llvm_file_list = list_create();
	m2c_asm_file_list = list_create();
	m2c_bin_file_list = list_create();

	/* Initialize compiler modules */
	cl2llvm_init();
	llvm2si_init();
	si2bin_init();
	frm2bin_init();
	ctree_init();
}


void m2c_done(void)
{
	int index;

	/* Free list of source files */
	LIST_FOR_EACH(m2c_source_file_list, index)
		free(list_get(m2c_source_file_list, index));
	list_free(m2c_source_file_list);

	/* Free list of pre-processed files */
	LIST_FOR_EACH(m2c_clp_file_list, index)
		free(list_get(m2c_clp_file_list, index));
	list_free(m2c_clp_file_list);

	/* Free list of LLVM object files */
	LIST_FOR_EACH(m2c_llvm_file_list, index)
		free(list_get(m2c_llvm_file_list, index));
	list_free(m2c_llvm_file_list);

	/* Free list of assembly files */
	LIST_FOR_EACH(m2c_asm_file_list, index)
		free(list_get(m2c_asm_file_list, index));
	list_free(m2c_asm_file_list);

	/* Free list of binary files */
	LIST_FOR_EACH(m2c_bin_file_list, index)
		free(list_get(m2c_bin_file_list, index));
	list_free(m2c_bin_file_list);

	/* Free list of '#define' directives */
	LIST_FOR_EACH(m2c_define_list, index)
		free(list_get(m2c_define_list, index));
	list_free(m2c_define_list);

	/* Finalize compiler modules */
	cl2llvm_done();
	llvm2si_done();
	si2bin_done();
	frm2bin_done();
	ctree_done();

	/* Libraries */
	debug_done();
}


int main(int argc, char **argv)
{
	/* Read command line */
	m2c_pre_init();
	m2c_read_command_line(argc, argv);

	/* Initialize */
	m2c_init();

	/* Process list of sources in 'm2c_source_file_list' and generate the
	 * rest of the file lists. */
	m2c_read_source_files();

	/* List AMD devices */
	if (amd_list_devices)
	{
		amd_dump_device_list(stdout);
		goto out;
	}

	/* Native AMD compilation */
	if (m2c_amd_run)
	{
		m2c_replace_out_file_name(m2c_bin_file_list);
		m2c_preprocess(m2c_source_file_list, m2c_clp_file_list);
		amd_compile(m2c_clp_file_list, m2c_bin_file_list);
		goto out;
	}

	/* OpenGL compilation */
	if (m2c_gl_run)
	{
		m2c_replace_out_file_name(m2c_bin_file_list);
		gl_compile(m2c_source_file_list, m2c_bin_file_list);
		goto out;
	}

	/* Stand-alone pre-processor */
	if (m2c_preprocess_run)
	{
		m2c_replace_out_file_name(m2c_clp_file_list);
		m2c_preprocess(m2c_source_file_list, m2c_clp_file_list);
		goto out;
	}

	/* OpenCL C to LLVM stand-alone front-end */
	if (m2c_cl2llvm_run)
	{
		m2c_replace_out_file_name(m2c_llvm_file_list);
		m2c_preprocess(m2c_source_file_list, m2c_clp_file_list);
		cl2llvm_compile(m2c_clp_file_list, m2c_llvm_file_list, m2c_opt_level);
		goto out;
	}

	/* LLVM to Southern Islands stand-alone back-end */
	if (m2c_llvm2si_run)
	{
		m2c_replace_out_file_name(m2c_asm_file_list);
		llvm2si_compile(m2c_source_file_list, m2c_asm_file_list);
		goto out;
	}

	/* Southern Islands assembler */
	if (m2c_si2bin_run)
	{
		m2c_replace_out_file_name(m2c_bin_file_list);
		si2bin_compile(m2c_source_file_list, m2c_bin_file_list);
		goto out;
	}

	/* Fermi assembler */
	if (m2c_frm2bin_run)
	{
		m2c_replace_out_file_name(m2c_bin_file_list);
		frm2bin_compile(m2c_source_file_list, m2c_bin_file_list);
		goto out;
	}

	/* Compilation steps */
	fprintf(stderr,
		"\n"
		"* WARNING: The version of Multi2C released together with Multi2Sim *\n"
		"* 4.2 is aimed to be a preliminary version of an open-source       *\n"
		"* OpenCL compiler generating compatible binaries for real GPUs.    *\n"
		"* Important features of OpenCL C are still missing or not fully    *\n"
		"* supported. To request support or provide contributions, please   *\n"
		"* email development@multi2sim.org.                                 *\n"
		"\n");
	m2c_replace_out_file_name(m2c_bin_file_list);
	m2c_preprocess(m2c_source_file_list, m2c_clp_file_list);
	cl2llvm_compile(m2c_clp_file_list, m2c_llvm_file_list, m2c_opt_level);
	llvm2si_compile(m2c_llvm_file_list, m2c_asm_file_list);
	si2bin_compile(m2c_asm_file_list, m2c_bin_file_list);

out:
	/* Finish */
	m2c_done();
	mhandle_done();
	return 0;
}

