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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "misc.h"


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
 * File management
 */


/* Open file, choosing from "stdout", "stderr" or <name> */
FILE *open_read(char *fname)
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


int can_open_read(char *fname)
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


FILE *open_write(char *fname)
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


int can_open_write(char *fname)
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
int read_line(FILE *f, char *line, int size)
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


void close_file(FILE *f)
{
	if (f && f != stdout && f != stderr)
		fclose(f);
}


FILE *create_temp_file(char *ret_path, int ret_path_size)
{
	char path[MAX_STRING_SIZE];
	FILE *f;
	int fd;

	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1 || (f = fdopen(fd, "w+")) == NULL)
		return NULL;
	if (ret_path)
		strncpy(ret_path, path, ret_path_size);
	return f;
}




/*
 * String maps
 */


int map_string(struct string_map_t *map, char *string)
{
	int i;
	for (i = 0; i < map->count; i++)
		if (!strcmp(string, map->map[i].string))
			return map->map[i].value;
	return 0;
}


/* Map string ignoring case */
int map_string_case(struct string_map_t *map, char *s)
{
	int i;
	for (i = 0; i < map->count; i++)
		if (!strcasecmp(s, map->map[i].string))
			return map->map[i].value;
	return 0;
}


static char *unknown = "<unknown>";
char *map_value(struct string_map_t *map, int value)
{
	int i;
	for (i = 0; i < map->count; i++)
		if (map->map[i].value == value)
			return map->map[i].string;
	return unknown;
}


void map_value_string(struct string_map_t *map, int value, char *buf, int size)
{
	int i;
	for (i = 0; i < map->count; i++) {
		if (map->map[i].value == value) {
			snprintf(buf, size, "%s", map->map[i].string);
			return;
		}
	}
	snprintf(buf, size, "%d", value);
}


void map_flags(struct string_map_t *map, int flags, char *out, int size)
{
	int i;
	char *comma = "", temp[size];

	strccpy(out, "{", size);
	for (i = 0; i < 32; i++) {
		if (flags & (1U << i)) {
			strccat(out, comma);
			map_value_string(map, 1U << i, temp, size);
			strccat(out, temp);
			comma = "|";
		}
	}
	strccat(out, "}");
}




/*
 * String functions
 */

static int memoryleft;

void strccat(char *dest, char *src)
{
	int destlen = strlen(dest);
	int srclen = strlen(src);
	if (memoryleft <= 1)
		return;
	srclen = MIN(srclen, memoryleft - 1);
	memcpy(dest + destlen, src, srclen);
	dest[destlen + srclen] = 0;
	memoryleft -= srclen;
}


void strccpy(char *dest, char *src, int size)
{
	int srclen = strlen(src);
	memoryleft = size;
	if (memoryleft <= 1)
		return;
	srclen = MIN(srclen, memoryleft - 1);
	memcpy(dest, src, srclen);
	dest[srclen] = 0;
	memoryleft -= srclen;
}


void strdump(char *dest, char *src, int size)
{
	int i;
	for (i = 0; i < size - 1 && *src; i++) {
		*dest = *src > 31 ? *src : '.';
		src++, dest++;
	}
	*dest = 0;
}


void str_single_spaces(char *dest, char *src, int size)
{
	int spc = 0;

	/* Remove initial spaces */
	while (*src == ' ' || *src == '\n')
		src++;
	
	/* Remove duplicated and final spaces */
	while (*src) {
		if (*src != ' ' && *src != '\n') {
			if (spc && size) {
				*dest++ = ' ';
				size--;
			}
			if (size) {
				*dest++ = *src;
				size--;
			}
		}

		/* Next character */
		spc = *src == ' ' || *src == '\n';
		src++;
	}

	/* Null-terminate */
	if (size)
		*dest = '\0';
}

/* Return 1 if 'suffix' is a suffix of 'str' */
int str_suffix(char *str, char *suffix)
{
	int str_len;
	int suffix_len;

	str_len = strlen(str);
	suffix_len = strlen(suffix);
	if (str_len < suffix_len)
		return 0;
	if (strcmp(str + str_len - suffix_len, suffix))
		return 0;
	return 1;
}


/* Return 1 if 'prefix' is a prefix of 'str' */
int str_prefix(char *str, char *prefix)
{
	int str_len;
	int prefix_len;

	str_len = strlen(str);
	prefix_len = strlen(prefix);
	if (str_len < prefix_len)
		return 0;
	if (!strncmp(str, prefix, prefix_len))
		return 1;
	return 0;
}


/* Copy a substring of 'src' into 'dest' */
void str_substr(char *dest, int dest_size, char *src, int src_pos, int src_count)
{
	int src_len;
	int count;

	src_len = strlen(src);
	count = src_count;
	if (count > dest_size - 1)
		count = dest_size - 1;
	if (count > src_len - src_pos)
		count = src_len - src_pos;
	memcpy(dest, src + src_pos, count);
	dest[count] = '\0';
}


void str_printf(char **pbuf, int *psize, char *fmt, ...)
{
	va_list va;
	int len;

	if (*psize <= 1)
		return;
	va_start(va, fmt);
	len = vsnprintf(*pbuf, *psize, fmt, va);
	if (len >= *psize)
		len = *psize - 1;
	*psize -= len;
	*pbuf += len;
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

	buf = malloc(alloc_size);
	if (!buf)
		return NULL;
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
 * Bit Map Functions
 */

struct bit_map_t
{
	unsigned int size;
	unsigned int word_count;  /* Size in words */
	unsigned int data[0];
};


struct bit_map_t *bit_map_create(unsigned int size)
{
	struct bit_map_t *map;
	unsigned int word_count;

	word_count = (size + 31) / 32;
	map = calloc(1, sizeof(struct bit_map_t) + word_count * 4);
	if (!map)
		return NULL;
	map->size = size;
	map->word_count = word_count;
	return map;
}


void bit_map_free(struct bit_map_t *bit_map)
{
	free(bit_map);
}


/* Set 'size' bits to 'value' starting at position 'where'. Argument 'size' cannot
 * exceed 32 bits. */
void bit_map_set(struct bit_map_t *bit_map, unsigned int where, unsigned int size,
	unsigned int value)
{
	unsigned int where_word1, where_offset1, size_align1;
	unsigned int where_word2, size_align2;
	unsigned int *pword1, word1_mask, word1_value;
	unsigned int *pword2, word2_mask, word2_value;

	if (size > 32)
		abort();
	
	/* Bits to get from first word */
	where_word1 = where / 32;
	where_offset1 = where % 32;
	size_align1 = MIN(size, 32 - where_offset1);
	if (where_word1 >= bit_map->word_count)
		return;
	pword1 = &bit_map->data[where_word1];

	/* Special case: 32-bit aligned word */
	if (size == 32 && !where_offset1) {
		*pword1 = value;
		return;
	}

	/* Get shift and mask */
	word1_mask = ((1 << size_align1) - 1) << where_offset1;
	word1_value = (value << where_offset1) & word1_mask;
	*pword1 &= ~word1_mask;
	*pword1 |= word1_value;

	/* If the whole size was covered in the first chunk, exit */
	if (size_align1 == size)
		return;

	/* Bits to get from second word */
	where_word2 = where_word1 + 1;
	size_align2 = size - size_align1;
	if (where_word2 >= bit_map->word_count)
		return;
	pword2 = &bit_map->data[where_word2];

	/* Add to result */
	word2_mask = (1 << size_align2) - 1;
	word2_value = (value >> size_align1) & word2_mask;
	*pword2 &= ~word2_mask;
	*pword2 |= word2_value;
}


unsigned int bit_map_get(struct bit_map_t *bit_map, unsigned int where, unsigned int size)
{
	unsigned int where_word1, where_offset1, size_align1;
	unsigned int size_align2;
	unsigned int word1, word1_mask;
	unsigned int word2_mask;
	unsigned int result;

	if (size > 32)
		abort();
	
	/* Bits to get from first word */
	where_word1 = where / 32;
	where_offset1 = where % 32;
	size_align1 = MIN(size, 32 - where_offset1);
	word1 = where_word1 < bit_map->word_count ? bit_map->data[where_word1] : 0;

	/* Special case: 32-bit aligned word */
	if (size == 32 && !where_offset1)
		return word1;

	/* Get shift and mask */
	word1_mask = ((1 << size_align1) - 1) << where_offset1;
	result = (word1 & word1_mask) >> where_offset1;

	/* If the whole size was covered in the first chunk, exit */
	if (size_align1 == size)
		return result;

	/* Bits to get from second word */
	size_align2 = size - size_align1;

	/* Add to result */
	word2_mask = (1 << size_align2) - 1;
	result |= (word1 & word2_mask) << size_align1;
	return result;
}


int bit_map_count_ones(struct bit_map_t *bit_map, unsigned int where, unsigned int size)
{
	unsigned int i;
	int count = 0;

	for (i = 0; i < size; i++)
		count += bit_map_get(bit_map, where + i, 1);
	return count;
}


void bit_map_copy(struct bit_map_t *dst, unsigned int dst_where,
	struct bit_map_t *src, unsigned int src_where, unsigned int size)
{
	unsigned int current_size;
	unsigned int word;

	while (size) {
		current_size = MIN(size, 32);
		word = bit_map_get(src, src_where, current_size);
		bit_map_set(dst, dst_where, current_size, word);
		size -= current_size;
		src_where += current_size;
		dst_where += current_size;
	}
}


void bit_map_dump(struct bit_map_t *bit_map, unsigned int where, unsigned int size, FILE *f)
{
	unsigned int i;
	for (i = 0; i < size; i++)
		fprintf(f, "%d", bit_map_get(bit_map, where + i, 1));
}




/*
 * Other
 */


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
	if (!digits) {
		fprintf(f, "0");
		return;
	}
	digits = MAX(MIN(digits, 32), 1);
	for (i = 0; i < digits; i++)
		s[i] = x & (1 << (digits - i - 1)) ? '1' : '0';
	s[digits] = 0;
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
	{
		fprintf(stderr, "%s: error when calling 'readlink'", __FUNCTION__);
		exit(1);
	}
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
	fprintf(stderr, "distribution file '%s' not found.\n"
		"\tOne or more distribution files cannot be located by Multi2Sim.\n"
		"\tPlease report this bug to 'webmaster@multi2sim.org', and it will\n"
		"\tbe resolved as quickly as possible.\n"
		"\tThe current value of PACKAGE_DATA_DIR is '%s'.\n",
		file_name, PACKAGE_DATA_DIR);
	exit(1);
}
