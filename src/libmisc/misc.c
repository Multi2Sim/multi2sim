/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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




/* numeric functions */
int log_base2(int x) {
	int res = 0, value = x;
	if (!value)
		abort();
	while (!(value & 1)) {
		value >>= 1;
		res++;
	}
	if (value != 1)
		abort();
	return res;
}



/* open file, choosing from "stdout", "stderr" or <name> */
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


/* read a line from a text file, deleting final '\n';
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


/* dump memory contents, printing a dot for unprintable chars */
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


/* string mapping functions */
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



/* strings */
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
	unsigned int where_align1, where_word1, where_offset1, size_align1;
	unsigned int where_align2, where_word2, where_offset2, size_align2;
	unsigned int *pword1, word1_mask, word1_value;
	unsigned int *pword2, word2_mask, word2_value;

	if (size > 32)
		abort();
	
	/* Bits to get from first word */
	where_align1 = where;
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
	where_align2 = where_word2 * 32;
	where_offset2 = 0;
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
	unsigned int where_align1, where_word1, where_offset1, size_align1;
	unsigned int where_align2, where_word2, where_offset2, size_align2;
	unsigned int word1, word1_mask;
	unsigned int word2, word2_mask;
	unsigned int result;

	if (size > 32)
		abort();
	
	/* Bits to get from first word */
	where_align1 = where;
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
	where_word2 = where_word1 + 1;
	where_align2 = where_word2 * 32;
	where_offset2 = 0;
	size_align2 = size - size_align1;
	word2 = where_word2 < bit_map->word_count ? bit_map->data[where_word1] : 0;

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

