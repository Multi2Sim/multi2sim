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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "amd.h"



/*
 * Global Variables
 */

char *clcc_out_file_name = "";
struct list_t *clcc_source_file_list;  /* Elements of type 'char *' */





static char *syntax =
	"\n"
	"Syntax: m2s-clcc [<options>] <sources>\n"
	"\n"
	"\tOptions:\n"
	"\t-l            Print list of available devices\n"
	"\t-d <dev>      Select target device for compilation\n"
	"\t-o <file>     Specify output file (default is <kernel>.bin)\n"
	"\t-a            Dump intermediate files\n"
	"\t-e            ELF verbose\n"
	"\t-g            No optimizations (flags '-O0 -g' added)\n"
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

	if (!strcmp(option, "o"))
	{
		clcc_out_file_name = optarg;
		return;
	}



	/* Option not found */
	fprintf(stderr, "%s", syntax);
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
		{ 0, 0, 0, 0 }
	};
	
	/* Process options */
	while ((opt = getopt_long(argc, argv, "ad:lo:", long_options,
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
}


void clcc_done(void)
{
	/* Free list of source files */
	list_free(clcc_source_file_list);
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

	/* Non-native compilation not supported yet */
	if (list_count(clcc_source_file_list))
		fatal("only AMD native compilation supported (use --amd)");

	/* No input file given */
	fatal("no input file given");

out:
	/* Finish */
	clcc_done();
	mhandle_done();
	return 0;
}


#if 0
int main(int argc, char **argv)
{
	int opt;

	/* No arguments */
	if (argc == 1)
	{
		fprintf(stderr, syntax, argv[0]);
		return 1;
	}

	/* Process options */
	while ((opt = getopt(argc, argv, "ld:aego:")) != -1)
	{
		switch (opt)
		{

		case 'g':
			debug_info = 1;
			break;

		default:
			fprintf(stderr, syntax, argv[0]);
			return 1;
		}
	}

	/* The only remaining argument should be the kernel to compile */
	if (argc - optind > 1)
	{
		fprintf(stderr, syntax, argv[0]);
		return 1;
	}
	else if (argc - optind == 1)
		kernel_file_name = argv[optind];
	if (!kernel_file_name && !amd_list_devices)
		fatal("no kernel to compile");

	/* Platform */
	cl_int err;
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS)
		fatal("cannot get OpenCL platform");
	
	/* Get context */
	cl_context_properties cprops[5];
	cprops[0] = CL_CONTEXT_PLATFORM;
	cprops[1] = (cl_context_properties)platform;
	cprops[2] = CL_CONTEXT_OFFLINE_DEVICES_AMD;
	cprops[3] = (cl_context_properties) 1;
	cprops[4] = (cl_context_properties) NULL;
	context = clCreateContextFromType(cprops, CL_DEVICE_TYPE_ALL, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		fatal("cannot create context");
	
	/* Get device list from context */
	err = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(num_devices), &num_devices, NULL);
	if (err != CL_SUCCESS)
		fatal("cannot get number of devices");
	err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(devices), devices, NULL);
	if (err != CL_SUCCESS)
		fatal("cannot get list of devices");
	
	/* Get selected device */
	if (amd_device_name)
	{
		device_id = read_device(amd_device_name);
		device = devices[device_id];
	}
	
	/* List available devices */
	if (amd_list_devices)
		main_list_devices(stdout);
	
	/* Compile list of kernels */
	if (kernel_file_name)
		main_compile_kernel();
	
	/* End program */
	printf("\n");
	return 0;
}
#endif

