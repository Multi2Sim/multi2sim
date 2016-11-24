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

#ifndef LIB_UTIL_STRING_H
#define LIB_UTIL_STRING_H

#include <stdio.h>


/*
 * String Maps
 */

struct str_map_t
{
	int count;
	struct {
		char *string;
		int value;
	} map[];
};

int str_map_string(struct str_map_t *map, char *s);
int str_map_string_err(struct str_map_t *map, char *s, int *err_ptr);
int str_map_string_err_msg(struct str_map_t *map, char *s, char *err_msg);

int str_map_string_case(struct str_map_t *map, char *s);
int str_map_string_case_err(struct str_map_t *map, char *s, int *err_ptr);
int str_map_string_case_err_msg(struct str_map_t *map, char *s, char *err_msg);

char *str_map_value(struct str_map_t *map, int value);
void str_map_value_buf(struct str_map_t *map, int value, char *buf, int size);

void str_map_flags(struct str_map_t *map, int flags, char *out, int length);




/*
 * Token list
 */

struct list_t *str_token_list_create(char *str, char *delim);
void str_token_list_free(struct list_t *token_list);

char *str_token_list_shift(struct list_t *token_list);
char *str_token_list_first(struct list_t *token_list);

/* Return the index of token 'token' inside of the token list. If the token is
 * not present, return -1. */
int str_token_list_find(struct list_t *token_list, char *token);

/* Same as 'str_token_list_find', but perform case-insensitive comparisons to
 * search for the requested token. */
int str_token_list_find_case(struct list_t *token_list, char *token);

void str_token_list_dump(struct list_t *token_list, FILE *f);



/*
 * Strings
 */

/* Maximum string size */
#define MAX_LONG_STRING_SIZE (1024 * 8)
#define MAX_STRING_SIZE  200
#define MAX_PATH_SIZE  400

void str_single_spaces(char *dest, int size, char *src);

int str_suffix(char *str, char *suffix);
int str_prefix(char *str, char *prefix);

void str_substr(char *dest, int dest_size, char *src, int src_pos, int src_count);

void str_token(char *dest, int dest_size, char *src, int index, char *delim);

/* Remove spaces from the beginning and end of string 'src'. The resulting string
 * is placed in buffer 'dest' with a maximum capacity of 'size' bytes. The resulting
 * string is always NULL-terminated. */
void str_trim(char *dest, int size, char *src);

/* Return a string corresponding to the error code passed in 'err'. The value in
 * 'err' is the error code returned by 'str_xxx' functions. */
char *str_error(int err);

/* Convert a string 'str' into an integer, accepting the following modifiers.
 * If conversion fails due to wrong formatting of 'str', an error code is returned
 * in argument 'err' (can be NULL).
 *
 * String prefixes:
 *   Prefix '0x' - Use base 16 for conversion.
 *   Prefix '0' - Use base 8 for conversion.
 * String suffixes:
 *   Suffix 'k' - Multiply by 1024.
 *   Suffix 'K' - Multiply by 1000.
 *   Suffix 'm' - Multiply by 1024*1024.
 *   Suffix 'M' - Multiply by 1000*1000.
 *   Suffix 'g' - Multiply by 1024*1024*1024.
 *   Suffix 'G' - Multiply by 1000*1000*1000.
 */
int str_to_int(char *str, int *err);
long long str_to_llint(char *str, int *err);

/* Convert an integer value into a sequence of alphanumeric characters. Each
 * position of the destination string can encode a value between 0 and 61,
 * where:
 *  0-9    =>   '0'...'9'
 *  10-35  =>   'a'...'z'
 *  36-61  =>   'A'...'Z'
 * The destination string is stored in 'str', a buffer with a maximum capacity
 * of 'size' bytes. If 'size' is greater than 0, 'str' will be null-terminated.
 */
void str_int_to_alnum(char *str, int size, unsigned int value);
unsigned int str_alnum_to_int(char *str);

/* Dump formatted string into a buffer with a specific size. Its size is then
 * decreased, and the buffer is advanced to the end of the dumped string.
 * This function is useful for being used in other functions that dump
 * several strings into a buffer, with the header
 *   obj_dump(struct obj_t *obj, char *buf, int size); */
void str_printf(char **pbuf, int *psize, char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

void str_read_from_file(FILE *f, char *buf, int buf_size);
void str_write_to_file(FILE *f, char *buf);

/* Dynamic creation and destruction of strings.
 * Function 'str_set' returns a copy of 'new_str', and frees the old string in
 * argument 'old_str' if it is not NULL.
 * Funciton 'str_free' frees the string in 'str' if it is not NULL. */
char *str_set(char *old_str, char *new_str);
char *str_free(char *str);


#endif

