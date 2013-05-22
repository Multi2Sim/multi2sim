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

#include <clcc/amd/amd.h>
#include <clcc/cl2llvm/cl2llvm.h>
#include <clcc/llvm2si/llvm2si.h>
#include <clcc/si2bin/si2bin.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "define.h"



/*
 * Global Variables
 */

/* Output file name passed with option '-o' */
char clcc_out_file_name[MAX_STRING_SIZE];

int clcc_amd_run;  /* Run AMD native compiler */
int clcc_llvm2si_run;  /* Run LLVM-to-SI back-end */
int clcc_si2bin_run;  /* Run Southern Islands assembler */


/* File names computed from source files */
struct list_t *clcc_source_file_list;  /* Source file names */
struct list_t *clcc_llvm_file_list;  /* LLVM files, extension, '.llvm' */
struct list_t *clcc_asm_file_list;  /* Assembly files, extension '.s' */
struct list_t *clcc_bin_file_list;  /* Binary files, extension '.bin' */

/* List of macros passed with '-D' options in the command line. */
struct list_t *clcc_define_list;  /* Elements of type 'struct clcc_define_t *' */





static char *syntax =
	"\n"
	"Syntax:\n"
	"\n"
	"\tm2s-clcc [<options>] <sources>\n"
	"\n"
	"Options:\n"
	"\n"
	"--amd\n"
	"\tUse AMD's OpenCL driver installed on the machine to compile the\n"
	"\tsources. This tool will act as a command-line wrapper of the native\n"
	"\tAMD compiler.\n"
	"\n"
	"--amd-list, -l\n"
	"\tPrint a list of available devices for the native AMD driver. This\n"
	"\toption should be used together with option '--amd'.\n"
	"\n"
	"--amd-device <device1>[,<device2>...], -d <device1>[,<device2>...]\n"
	"\tSelect a list of target devices for native AMD compilation. There\n"
	"\tshould be no spaces between device names/identifiers when separated\n"
	"\tby commas. When more than one device is selected, all binaries are\n"
	"\tpacked into one single Multi2Sim-specific ELF binary format.\n"
	"\tThis option must be combined with option '--amd-.\n"
	"\n"
	"--amd-dump-all, -a\n"
	"\tDump all intermediate file is generated during compilation. This\n"
	"\toption must be used together with '--amd'.\n"
	"\n"
	"--define <symbol>=<value>, -D <symbol>=<value>\n"
	"\tAdd a definition for additional symbols, equivalent to #define\n"
	"\tcompiler directives. This argument can be used multiple times.\n"
	"\n"
	"--help, -h\n"
	"\tShow help message with command-line options.\n"
	"\n"
	"--llvm2si\n"
	"\tInterpret sources as LLVM binaries and generate Southern Islands\n"
	"\tassembly output in a '.s' file.\n"
	"\n"
	"-o <file>\n"
	"\tOutput kernel binary. If no output file is specified, each kernel\n"
	"\tsource is compiled into a kernel binary with the same name but\n"
	"\tusing the '.bin' extension.\n"
	"\n"
	"--si-asm\n"
	"\tTreat the input files as source files containing Southern Islands\n"
	"\tassembly code. Run the Southern Islands assembler and generate a\n"
	"\tkernel binary.\n"
	"\n";



static void clcc_process_option(const char *option, char *optarg)
{
	/*
	 * Native AMD related options
	 */
	
	if (!strcmp(option, "amd"))
	{
		clcc_amd_run = 1;
		return;
	}

	if (!strcmp(option, "amd-dump-all") || !strcmp(option, "a"))
	{
		amd_dump_all = 1;
		return;
	}

	if (!strcmp(option, "amd-device") || !strcmp(option, "d"))
	{
		amd_device_name = optarg;
		return;
	}

	if (!strcmp(option, "define") || !strcmp(option, "D"))
	{
		struct list_t *token_list;
		struct clcc_define_t *define;

		/* Create list of tokens */
		token_list = str_token_list_create(optarg, "=");
		if (token_list->count != 2)
			fatal("wrong format for --define/-D option, "
				"should be <sym>=<val> (%s)", optarg);
	
		/* Create define element and add it to the list */
		define = clcc_define_create(list_get(token_list, 0),
				list_get(token_list, 1));
		list_add(clcc_define_list, define);

		/* Free token list */
		str_token_list_free(token_list);
		return;
	}

	if (!strcmp(option, "amd-list") || !strcmp(option, "l"))
	{
		amd_list_devices = 1;
		return;
	}
	
	if (!strcmp(option, "help") || !strcmp(option, "h"))
	{
		printf("%s", syntax);
		exit(0);
	}

	if (!strcmp(option, "llvm2si"))
	{
		clcc_llvm2si_run = 1;
		return;
	}

	if (!strcmp(option, "o"))
	{
		snprintf(clcc_out_file_name, sizeof clcc_out_file_name,
				"%s", optarg);
		return;
	}

	if (!strcmp(option, "si-asm"))
	{
		clcc_si2bin_run = 1;
		return;
	}


	/* Option not found */
	exit(1);
}


static void clcc_read_command_line(int argc, char **argv)
{
	int option_index = 0;
	int opt;
	char option[10];

	static struct option long_options[] =
	{
		{ "amd", no_argument, 0, 0 },
		{ "amd-device", required_argument, 0, 'd' },
		{ "amd-dump-all", no_argument, 0, 'a' },
		{ "amd-list", no_argument, 0, 'l' },
		{ "help", no_argument, 0, 'h' },
		{ "llvm2si", no_argument, 0, 0 },
		{ "si-asm", no_argument, 0, 0 },
		{ "define", required_argument, 0, 'D' },
		{ 0, 0, 0, 0 }
	};

	/* No arguments given */
	if (argc == 1)
	{
		printf("\n");
		printf("Multi2Sim " VERSION " OpenCL C Compiler\n");
		printf("Please run 'm2s-clcc --help' for a list of command-line options\n");
		printf("\n");
		exit(0);
	}
	
	/* Process options */
	while ((opt = getopt_long(argc, argv, "ad:hlo:D:", long_options,
			&option_index)) != -1)
	{
		if (opt)
		{
			option[0] = opt;
			option[1] = '\0';
			clcc_process_option(option, optarg);
		}
		else
		{
			clcc_process_option(long_options[option_index].name,
				optarg);
		}
	}

	/* The rest are source files */
	while (optind < argc)
		list_add(clcc_source_file_list, xstrdup(argv[optind++]));
}


void clcc_read_source_files(void)
{
	char *file_name_ptr;
	char file_name[MAX_STRING_SIZE];
	char file_name_prefix[MAX_STRING_SIZE];
	int index;

	/* Nothing to do for no sources */
	if (!clcc_source_file_list->count)
		return;

	/* Option '-o' no allowed when multiple source files are given. */
	if (clcc_source_file_list->count > 1 && clcc_out_file_name[0])
		fatal("option '-o' not allowed when multiple sources are given");

	/* Create file names */
	LIST_FOR_EACH(clcc_source_file_list, index)
	{
		char *dot_str;
		char *slash_str;

		/* Get file name */
		file_name_ptr = list_get(clcc_source_file_list, index);

		/* Get position of last '.' after last '/' */
		dot_str = rindex(file_name_ptr, '.');
		slash_str = rindex(file_name_ptr, '/');
		if (!dot_str || slash_str > dot_str)
			dot_str = file_name_ptr + strlen(file_name_ptr);

		/* Get prefix */
		str_substr(file_name_prefix, sizeof file_name_prefix,
				file_name_ptr, 0, dot_str - file_name_ptr);

		/* LLVM binary with '.llvm' extension */
		snprintf(file_name, sizeof file_name, "%s.llvm", file_name_prefix);
		list_add(clcc_llvm_file_list, xstrdup(file_name));

		/* Assembly code with '.s' extension */
		snprintf(file_name, sizeof file_name, "%s.s", file_name_prefix);
		list_add(clcc_asm_file_list, xstrdup(file_name));

		/* Final binary with '.bin' extension */
		snprintf(file_name, sizeof file_name, "%s.bin", file_name_prefix);
		list_add(clcc_bin_file_list, xstrdup(file_name));
	}

	/* Option '-o' given. Replace name of final binary. */
	if (clcc_out_file_name[0])
	{
		/* Free old string */
		assert(clcc_bin_file_list->count == 1);
		file_name_ptr = list_get(clcc_bin_file_list, 0);
		free(file_name_ptr);

		/* Set new string */
		file_name_ptr = xstrdup(clcc_out_file_name);
		list_set(clcc_bin_file_list, 0, file_name_ptr);
	}
}


void clcc_init(void)
{
	/* List of source files */
	clcc_source_file_list = list_create();
	clcc_llvm_file_list = list_create();
	clcc_asm_file_list = list_create();
	clcc_bin_file_list = list_create();
	clcc_define_list = list_create();

	/* Initialize compiler modules */
	cl2llvm_init();
	llvm2si_init();
	si2bin_init();
}


void clcc_done(void)
{
	int index;

	/* Free list of source files */
	LIST_FOR_EACH(clcc_source_file_list, index)
		free(list_get(clcc_source_file_list, index));
	list_free(clcc_source_file_list);

	/* Free list of LLVM object files */
	LIST_FOR_EACH(clcc_llvm_file_list, index)
		free(list_get(clcc_llvm_file_list, index));
	list_free(clcc_llvm_file_list);

	/* Free list of assembly files */
	LIST_FOR_EACH(clcc_asm_file_list, index)
		free(list_get(clcc_asm_file_list, index));
	list_free(clcc_asm_file_list);

	/* Free list of binary files */
	LIST_FOR_EACH(clcc_bin_file_list, index)
		free(list_get(clcc_bin_file_list, index));
	list_free(clcc_bin_file_list);

	/* Free list of '#define' directives */
	LIST_FOR_EACH(clcc_define_list, index)
		clcc_define_free(list_get(clcc_define_list, index));
	list_free(clcc_define_list);

	/* Finalize compiler modules */
	cl2llvm_done();
	llvm2si_done();
	si2bin_done();
}


int main(int argc, char **argv)
{
	/* Initialize */
	clcc_init();

	/* Read command line */
	clcc_read_command_line(argc, argv);

	/* Process list of sources in 'clcc_source_file_list' and generate the
	 * rest of the file lists. */
	clcc_read_source_files();

	/* List AMD devices */
	if (amd_list_devices)
	{
		amd_dump_device_list(stdout);
		goto out;
	}

	/* Native AMD compilation */
	if (clcc_amd_run)
	{
		amd_compile(clcc_source_file_list, clcc_bin_file_list);
		goto out;
	}

	/* LLVM to Southern Islands back-end */
	if (clcc_llvm2si_run)
	{
		/* Replace output file if specified */
		if (clcc_out_file_name[1] && clcc_asm_file_list->count == 1)
		{
			free(list_get(clcc_asm_file_list, 0));
			list_set(clcc_asm_file_list, 0, xstrdup(clcc_out_file_name));
		}

		/* Run */
		llvm2si_compile(clcc_source_file_list, clcc_asm_file_list);
		goto out;
	}

	/* Southern Islands assembler */
	if (clcc_si2bin_run)
	{
		si2bin_compile(clcc_source_file_list, clcc_bin_file_list);
		goto out;
	}

	/* OpenCL-to-LLVM pass */
	cl2llvm_compile(clcc_source_file_list, clcc_llvm_file_list);

out:
	/* Finish */
	clcc_done();
	mhandle_done();
	return 0;
}
