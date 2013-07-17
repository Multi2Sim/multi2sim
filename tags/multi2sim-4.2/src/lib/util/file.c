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
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "file.h"
#include "string.h"


/* Open file, choosing from "stdout", "stderr" or <name> */
FILE *file_open_for_read(char *fname)
{
	if (!fname[0])
		return NULL;
	if (!strcmp(fname, "stdout"))
		return stdout;
	else if (!strcmp(fname, "stderr"))
		return stderr;
	else
		return fopen(fname, "rt");
}


int file_can_open_for_read(char *fname)
{
	FILE *f;
	if (!fname[0])
		return 0;
	if (!strcmp(fname, "stdout") || !strcmp(fname, "stderr"))
		return 0;
	f = fopen(fname, "rt");
	if (!f)
		return 0;
	fclose(f);
	return 1;
}


FILE *file_open_for_write(char *fname)
{
	if (!fname[0])
		return NULL;
	if (!strcmp(fname, "stdout"))
		return stdout;
	else if (!strcmp(fname, "stderr"))
		return stderr;
	else
		return fopen(fname, "wt");
}


int file_can_open_for_write(char *fname)
{
	FILE *f;
	if (!fname[0])
		return 0;
	if (!strcmp(fname, "stdout") || !strcmp(fname, "stderr"))
		return 1;
	f = fopen(fname, "wt");
	if (!f)
		return 0;
	fclose(f);
	return 1;
}


/* Read a line from a text file, deleting final '\n';
 * if eof, return -1; else return length of string */
int file_read_line(FILE *f, char *line, int size)
{
	if (!f)
		return -1;
	line = fgets(line, size, f);
	if (!line || feof(f))
		return -1;
	while (strlen(line) && (line[strlen(line) - 1] == 13 ||
		line[strlen(line) - 1] == 10))
		line[strlen(line) - 1] = 0;
	return strlen(line);
}


void file_close(FILE *f)
{
	if (f && f != stdout && f != stderr)
		fclose(f);
}


FILE *file_create_temp(char *ret_path, int ret_path_size)
{
	char path[MAX_STRING_SIZE];
	FILE *f;
	int fd;

	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "w+")) == NULL)
	{
		fprintf(stderr, "%s: failed to create temporary file", __FUNCTION__);
		exit(1);
	}
	if (ret_path)
		snprintf(ret_path, ret_path_size, "%s", path);
	return f;
}


/* Return the absolute path for 'file_name'. If 'file_name' is an absolute path, the
 * same string is returned. If 'file_name' does not start with a slash '/', it is
 * considered relative to 'default_path'. If 'default_path' is NULL or an empty string,
 * 'default_path' defaults to the current working directory.
 * The absolute path is placed in 'full_path', a buffer with 'size' bytes allocated.
 * If the absolute path does not fit in 'full_path', the function fatals.
 */
void file_full_path(char *file_name, char *default_path, char *full_path, int size)
{
	char default_path_str[MAX_STRING_SIZE];

	/* Remove './' prefix from 'file_name' */
	while (file_name && !strncmp(file_name, "./", 2))
		file_name += 2;

	/* File name is NULL or empty */
	assert(full_path);
	if (!file_name || !*file_name)
	{
		snprintf(full_path, size, "%s", "");
		return;
	}

	/* File name is given as an absolute path */
	if (*file_name == '/')
	{
		if (size < strlen(file_name) + 1)
			fatal("%s: buffer too small", __FUNCTION__);
		snprintf(full_path, size, "%s", file_name);
		return;
	}

	/* Get default path as current directory, if not given */
	if (!default_path || !*default_path)
	{
		default_path = default_path_str;
		if (!getcwd(default_path, sizeof default_path_str))
			fatal("%s: buffer too small for 'getcwd'", __FUNCTION__);
	}

	/* Relative path */
	if (strlen(default_path) + strlen(file_name) + 2 > size)
		fatal("%s: buffer too small", __FUNCTION__);
	snprintf(full_path, size, "%s/%s", default_path, file_name);
}

