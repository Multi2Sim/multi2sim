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
#include <clcc/si2bin/si2bin.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>



/*
 * Global Variables
 */

char clcc_out_file_name[MAX_STRING_SIZE];
struct list_t *clcc_source_file_list;  /* Elements of type 'char *' */
struct list_t *clcc_llvm_file_list;  /* Elements of type 'char *' */
struct list_t *clcc_bin_file_list;  /* Elements of type 'char *' */





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
	"--amd-device <device1>[,<device2>...]\n"
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
	"--help, -h\n"
	"\tShow help message with command-line options.\n"
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
		amd_native = 1;
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

	if (!strcmp(option, "o"))
	{
		snprintf(clcc_out_file_name, sizeof clcc_out_file_name,
				"%s", optarg);
		return;
	}

	if (!strcmp(option, "si-asm"))
	{
		si2bin_assemble = 1;
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
		{ "si-asm", no_argument, 0, 0 },
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
	while ((opt = getopt_long(argc, argv, "ad:hlo:", long_options,
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
		list_add(clcc_source_file_list, argv[optind++]);
}


void clcc_init(void)
{
	/* List of source files */
	clcc_source_file_list = list_create();
	clcc_llvm_file_list = list_create();
	clcc_bin_file_list = list_create();

	/* Initialize compiler modules */
	cl2llvm_init();
	si2bin_init();
}


void clcc_done(void)
{
	/* Free list of source files */
	list_free(clcc_source_file_list);
	list_free(clcc_llvm_file_list);
	list_free(clcc_bin_file_list);

	/* Finalize compiler modules */
	cl2llvm_done();
	si2bin_done();
}


int main(int argc, char **argv)
{
	/* Initialize */
	clcc_init();

	/* Read command line */
	clcc_read_command_line(argc, argv);

	/* List AMD devices */
	if (amd_list_devices)
	{
		amd_dump_device_list(stdout);
		goto out;
	}

	/* Native AMD compilation */
	if (amd_native)
	{
		amd_compile(clcc_source_file_list, clcc_out_file_name);
		goto out;
	}

	/* Southern Islands assembler */
	if (si2bin_assemble)
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


