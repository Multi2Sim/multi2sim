/*
 *  Multi2Sim Tools
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

#include <gpuvisual-private.h>



/*
 * Convenience functions
 */

/* Replace occurrences of spaces, '\t', and '\n' for one single space.
 * Returns the length of the resulting string. */
int str_single_space(char *dest, char *src, int size)
{
	int len = 0;
	int spc = 0;
	int first_char_found = 0;

	while (*src) {
		if (*src == ' ' || *src == '\n' || *src == '\t') {

			/* Record occurrence of space only if there was previous character already */
			if (first_char_found)
				spc = 1;

		} else {
			if (spc && len < size - 1) {
				len++;
				*dest++ = ' ';
			}
			if (len < size - 1) {
				len++;
				*dest++ = *src;
			}
			first_char_found = 1;
			spc = 0;
		}
		src++;
	}
	*dest++ = '\0';
	return len;
}


/* Search for a file 'file_name' in two directories:
 *   -The distributed data path (relative to PACKAGE_DATA_DIR)
 *   -The non-distribution path (relative to top directory in build system)
 * The file absolute path is returned in 'buffer' with a maximum of 'size' bytes.
 * If the file is not found, the program is aborted.
 */
void search_dist_file(char *file_name, char *dist_path, char *non_dist_path,
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
	if (f) {
		snprintf(buffer, size, "%s", dist_path_abs);
		fclose(f);
		return;
	}

	/* Look for file in non-distribution package.
	 * Assuming that 'm2s-debug-pipeline' runs in '$(TOPDIR)/src/',
	 * distribution file should be in '$(TOPDIR)/', i.e., one level higher. */
	exe_name[0] = '\0';
	len = readlink("/proc/self/exe", exe_name, MAX_STRING_SIZE);
	if (len < 0 || len >= MAX_STRING_SIZE)
		fatal("%s: error when calling 'readlink'", __FUNCTION__);
	exe_name[len] = '\0';

	levels = 2;
	while (len && levels) {
		if (exe_name[len - 1] == '/')
			levels--;
		exe_name[--len] = '\0';
	}

	snprintf(non_dist_path_abs, MAX_STRING_SIZE, "%s/%s/%s",
		exe_name, non_dist_path, file_name);
	f = fopen(non_dist_path_abs, "r");
	if (f) {
		snprintf(buffer, size, "%s", non_dist_path_abs);
		fclose(f);
		return;
	}

	/* File not found */
	fatal("distribution file '%s' not found.\n"
		"\tOne or more distribution files cannot be located by Multi2Sim.\n"
		"\tPlease report this bug to 'webmaster@multi2sim.org', and it will\n"
		"\tbe resolved as quickly as possible.\n"
		"\tThe current value of PACKAGE_DATA_DIR is '%s'.\n",
		file_name, PACKAGE_DATA_DIR);
}

