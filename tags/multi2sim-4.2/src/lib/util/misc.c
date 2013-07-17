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

#include <unistd.h>
#include <sys/utsname.h>

#include <lib/mhandle/mhandle.h>

#include "debug.h"
#include "misc.h"
#include "string.h"


/*
 * Numeric functions
 */

int log_base2(int x)
{
	int res = 0, value = x;
	if (!value)
		abort();
	while (!(value & 1))
	{
		value >>= 1;
		res++;
	}
	if (value != 1)
		abort();
	return res;
}




/*
 * Buffers
 */

int write_buffer(char *file_name, void *buf, int size)
{
	FILE *f;
	if (!(f = fopen(file_name, "wb")))
		return 0;
	fwrite(buf, size, 1, f);
	fclose(f);
	return 1;
}


void *read_buffer(char *file_name, int *psize)
{
	FILE *f;
	void *buf;
	int size, alloc_size, read_size;

	f = fopen(file_name, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	alloc_size = size ? size : 1;
	fseek(f, 0, SEEK_SET);

	buf = xmalloc(alloc_size);
	read_size = fread(buf, 1, size, f);
	if (psize)
		*psize = read_size;
	return buf;
}


void free_buffer(void *buf)
{
	free(buf);
}



/*
 * Other
 */

/* Convert hex string to byte array of 'size' bytes.
 * Return the size of the byte array.
 */
int hex_str_to_byte_array(char *byte_array, char *str, int size)
{
	unsigned int byte;
	char *end = byte_array + size;
	int length = 0;

	while (str[0])
	{
		/* Checks */
		if (!str[1])
			fatal("%s: hex string ended mid-byte", __FUNCTION__);
		if (byte_array >= end)
			fatal("%s: hex string too long", __FUNCTION__);

		sscanf(str, "%2x", &byte);
		byte_array[0] = (unsigned char) byte;
		byte_array += 1;
		length++;
		str += 2;
	}

	return length;
}


/* Dump memory contents, printing a dot for unprintable chars */
void dump_ptr(void *ptr, int size, FILE *stream)
{
	int i, j, val;
	for (i = 0; i < size; i++, ptr++) {
		for (j = 0; j < 2; j++) {
			val = j ? *(unsigned char *) ptr & 0xf :
				*(unsigned char *) ptr >> 4;
			if (val < 10)
				fprintf(stream, "%d", val);
			else
				fprintf(stream, "%c", val - 10 + 'a');
		}
		fprintf(stream, " ");
	}
}


/* Dump binary value */
void dump_bin(int x, int digits, FILE *f)
{
	int i;
	char s[33];

	/* No digit */
	if (!digits)
		return;

	/* Create string */
	digits = MAX(MIN(digits, 32), 1);
	for (i = 0; i < digits; i++)
		s[i] = x & (1 << (digits - i - 1)) ? '1' : '0';
	s[digits] = 0;

	/* Print */
	fprintf(f, "%s", s);
}


/* Search for a file 'file_name' in two directories:
 *   -The distributed data path (relative to PACKAGE_DATA_DIR)
 *   -The non-distribution path (relative to top directory in build system)
 * The file absolute path is returned in 'buffer' with a maximum of 'size' bytes.
 * If the file is not found, the program is aborted.
 */

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR ""
#endif

void m2s_dist_file(char *file_name, char *dist_path, char *non_dist_path,
	char *buffer, int size)
{
	char dist_path_abs[MAX_STRING_SIZE];
	char non_dist_path_abs[MAX_STRING_SIZE];

	FILE *f;

	char exe_name[MAX_STRING_SIZE];
	int len, levels;

	/* Look for file in distribution directory */
	snprintf(dist_path_abs, MAX_STRING_SIZE, "%s/%s/%s",
		PACKAGE_DATA_DIR, dist_path, file_name);
	f = fopen(dist_path_abs, "r");
	if (f)
	{
		snprintf(buffer, size, "%s", dist_path_abs);
		fclose(f);
		return;
	}

	/* Look for file in non-distribution package.
	 * Assuming that the Multi2Sim executable is in '$(TOPDIR)/bin',
	 * distribution file should be in '$(TOPDIR)/', i.e., one level higher. */
	exe_name[0] = '\0';
	len = readlink("/proc/self/exe", exe_name, MAX_STRING_SIZE);
	if (len < 0 || len >= MAX_STRING_SIZE)
	{
		fprintf(stderr, "%s: error when calling 'readlink'", __FUNCTION__);
		exit(1);
	}
	exe_name[len] = '\0';

	/* Go up two levels */
	levels = 2;
	while (len && levels)
	{
		if (exe_name[len - 1] == '/')
			levels--;
		exe_name[--len] = '\0';
	}

	/* Construct path */
	snprintf(non_dist_path_abs, MAX_STRING_SIZE, "%s/%s/%s",
		exe_name, non_dist_path, file_name);
	f = fopen(non_dist_path_abs, "r");
	if (f)
	{
		snprintf(buffer, size, "%s", non_dist_path_abs);
		fclose(f);
		return;
	}

	/* File not found */
	fprintf(stderr, "distribution file '%s' not found.\n"
		"\tOne or more distribution files cannot be located by Multi2Sim.\n"
		"\tPlease report this bug to 'webmaster@multi2sim.org', and it will\n"
		"\tbe resolved as quickly as possible.\n"
		"\tThe current value of PACKAGE_DATA_DIR is '%s'.\n",
		file_name, PACKAGE_DATA_DIR);
	exit(1);
}


/* Check that host and guest expressions match */

static char *err_m2s_host_guest_match =
	"\tFor the emulation of machine instructions and system calls, Multi2Sim\n"
	"\tmakes some assumptions about common features of the modeled architecture\n"
	"\t(guest) and the machine Multi2Sim runs on (host). While most assumptions\n"
	"\tare not strictly necessary, they simplify the design of the simulator.\n"
	"\tPlease email 'development@multi2sim.org' to report this issue.\n";

void m2s_host_guest_match_error(char *expr, int host_value, int guest_value)
{
	struct utsname utsname;

	uname(&utsname);
	fprintf(stderr, "expression '%s' differs in host/guest architectures.\n"
		"\tHost architecture: %s %s %s %s\n"
		"\tValue in host: %d\n"
		"\tValue in guest: %d\n"
		"%s\n",
		expr, utsname.sysname, utsname.release, utsname.version, utsname.machine,
		host_value, guest_value, err_m2s_host_guest_match);
	exit(1);
}
