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

#ifndef MISC_H
#define MISC_H

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


/* Maximum string size */
#define MAX_LONG_STRING_SIZE 2000
#define MAX_STRING_SIZE  200
#define MAX_PATH_SIZE  200

/* Min, Max */
#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif
#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

/* Min, Max, Book, Range */
#define BOOL(X) ((X) ? 't' : 'f')
#define IN_RANGE(X, X1, X2) ((X)>=(X1)&&(X)<=(X2))

/* Round */
#define ROUND_UP(N,ALIGN)	(((N) + ((ALIGN)-1)) & ~((ALIGN)-1))
#define ROUND_DOWN(N,ALIGN)	((N) & ~((ALIGN)-1))

/* Alignment */
#define DWORD_ALIGN(N) ROUND_DOWN((N),8)
#define WORD_ALIGN(N) ROUND_DOWN((N), 4)
#define HALF_ALIGN(N) ROUND_DOWN((N), 2)

/* Modulo handling negative numbers */
#define MOD(X, Y)		(((X) + (Y)) % (Y))

/* Endian control */
#define SWAPH(X)	(((((half)(X)) & 0xff) << 8) | \
			((((half)(X)) & 0xff00) >> 8))
#define SWAPW(X)	((((uint32_t)(X)) << 24) |			\
			((((uint32_t)(X)) << 8)  & 0x00ff0000) |		\
			((((uint32_t)(X)) >> 8)  & 0x0000ff00) |		\
			((((uint32_t)(X)) >> 24) & 0x000000ff))
#define SWAPDW(X)	((((uint64_t)(X)) << 56) |				\
			((((uint64_t)(X)) << 40) & 0x00ff000000000000ULL) |	\
			((((uint64_t)(X)) << 24) & 0x0000ff0000000000ULL) |	\
			((((uint64_t)(X)) << 8)  & 0x000000ff00000000ULL) |	\
			((((uint64_t)(X)) >> 8)  & 0x00000000ff000000ULL) |	\
			((((uint64_t)(X)) >> 24) & 0x0000000000ff0000ULL) |	\
			((((uint64_t)(X)) >> 40) & 0x000000000000ff00ULL) |	\
			((((uint64_t)(X)) >> 56) & 0x00000000000000ffULL))


/* Sign extension */
#define SEXT32(X, B)		(((uint32_t)(X))&(1U<<(B-1))?((uint32_t)(X))|~((1U<<B)-1):(X))
#define SEXT64(X, B)		(((uint64_t)(X))&(1ULL<<(B-1))?((uint64_t)(X))|~((1ULL<<B)-1):(X))

/* Extract bits from HI to LO from X */
#define BITS32(X, HI, LO)	((((uint32_t)(X))>>(LO))&((1U<<((HI)-(LO)+1))-1))
#define BITS64(X, HI, LO)	((((uint64_t)(X))>>(LO))&((1ULL<<((HI)-(LO)+1ULL))-1ULL))

/* Bit-handling macros */
#define GETBIT32(X, B)		((uint32_t)(X)&(1U<<(B)))
#define GETBIT64(X, B)		((uint64_t)(X)&(1ULL<<(B)))
#define SETBIT32(X, B)		((uint32_t)(X)|(1U<<(B)))
#define SETBIT64(X, B)		((uint64_t)(X)|(1ULL<<(B)))
#define CLEARBIT32(X, B)	((uint32_t)(X)&(~(1U<<(B))))
#define CLEARBIT64(X, B)	((uint64_t)(X)&(~(1ULL<<(B))))
#define SETBITVALUE32(X, B, V)	((V) ? SETBIT32((X),(B)) : CLEARBIT32((X),(B)))
#define SETBITVALUE64(X, B, V)	((V) ? SETBIT64((X),(B)) : CLEARBIT64((X),(B)))


/* Bitmaps manipulation */
#define BITMAP_TYPE(NAME, SIZE) \
	unsigned char NAME[((SIZE) + 7) >> 3]
#define BITMAP_INIT(NAME, SIZE) { int _i; \
	for (_i = 0; _i < (((SIZE) + 7) >> 3); _i++) \
	NAME[_i] = 0; }
#define BITMAP_SET(NAME, BIT) \
	(NAME[(BIT) >> 3] |= 1 << ((BIT) & 7))
#define BITMAP_CLEAR(NAME, BIT) \
	(NAME[(BIT) >> 3] &= ~(1 << ((BIT) & 7)))
#define BITMAP_SET_VAL(NAME, BIT, VAL) \
	((VAL) ? BITMAP_SET((NAME), (BIT)) : BITMAP_CLEAR((NAME), (BIT)))
#define BITMAP_GET(NAME, BIT) \
	((NAME[(BIT) >> 3] & (1 << ((BIT) & 7))) > 0)
#define BITMAP_SET_RANGE(NAME, LO, HI) { int _i; \
	for (_i = (LO); _i <= (HI); _i++) \
	BITMAP_SET((NAME), _i); }
#define BITMAP_CLEAR_RANGE(NAME, LO, HI) { int _i; \
	for (_i = (LO); _i <= (HI); _i++) \
	BITMAP_CLEAR((NAME), _i); }


/* Double linked list handling macros.
 * NAME is the name of a list.
 * CONT is a pointer to a structure containing the following fields:
 *   struct * CONT->NAME_list_head
 *   struct * CONT->NAME_list_tail
 *   int CONT->NAME_list_count
 *   int CONT->NAME_list_max
 * ELEM is an element of the list containing the following fields:
 *   struct * ELEM->NAME_list_next
 *   struct * ELEM->NAME_list_prev
 */

#define DOUBLE_LINKED_LIST_INSERT_HEAD(CONT, NAME, ELEM) { \
	assert(!(ELEM)->NAME##_list_next && !(ELEM)->NAME##_list_prev); \
	(ELEM)->NAME##_list_next = (CONT)->NAME##_list_head; \
	if ((ELEM)->NAME##_list_next) \
		(ELEM)->NAME##_list_next->NAME##_list_prev = (ELEM); \
	(CONT)->NAME##_list_head = (ELEM); \
	if (!(CONT)->NAME##_list_tail) \
		(CONT)->NAME##_list_tail = (ELEM); \
	(CONT)->NAME##_list_count++; \
	(CONT)->NAME##_list_max = MAX((CONT)->NAME##_list_max, (CONT)->NAME##_list_count); \
}

#define DOUBLE_LINKED_LIST_INSERT_TAIL(CONT, NAME, ELEM) { \
	assert(!(ELEM)->NAME##_list_next && !ELEM->NAME##_list_prev); \
	(ELEM)->NAME##_list_prev = (CONT)->NAME##_list_tail; \
	if ((ELEM)->NAME##_list_prev) \
		(ELEM)->NAME##_list_prev->NAME##_list_next = (ELEM); \
	(CONT)->NAME##_list_tail = (ELEM); \
	if (!(CONT)->NAME##_list_head) \
		(CONT)->NAME##_list_head = (ELEM); \
	(CONT)->NAME##_list_count++; \
	(CONT)->NAME##_list_max = MAX((CONT)->NAME##_list_max, (CONT)->NAME##_list_count); \
}

#define DOUBLE_LINKED_LIST_MEMBER(CONT, NAME, ELEM) \
	((CONT)->NAME##_list_head == (ELEM) || (ELEM)->NAME##_list_prev || (ELEM)->NAME##_list_next)

#define DOUBLE_LINKED_LIST_REMOVE(CONT, NAME, ELEM) { \
	assert((CONT)->NAME##_list_count > 0); \
	assert(DOUBLE_LINKED_LIST_MEMBER(CONT, NAME, ELEM)); \
	if ((ELEM) == (CONT)->NAME##_list_head) \
		(CONT)->NAME##_list_head = (CONT)->NAME##_list_head->NAME##_list_next; \
	if ((ELEM) == (CONT)->NAME##_list_tail) \
		(CONT)->NAME##_list_tail = (CONT)->NAME##_list_tail->NAME##_list_prev; \
	if ((ELEM)->NAME##_list_prev) \
		(ELEM)->NAME##_list_prev->NAME##_list_next = (ELEM)->NAME##_list_next; \
	if ((ELEM)->NAME##_list_next) \
		(ELEM)->NAME##_list_next->NAME##_list_prev = (ELEM)->NAME##_list_prev; \
	(ELEM)->NAME##_list_prev = (ELEM)->NAME##_list_next = NULL; \
	(CONT)->NAME##_list_count--; \
}

#define DOUBLE_LINKED_LIST_FOR_EACH(CONT, NAME, ELEM) \
	for ((ELEM) = (CONT)->NAME##_list_head; (ELEM); (ELEM) = (ELEM)->NAME##_list_next)


/* Safe pointer assignment for values returned in arguments passed by reference */
#define PTR_ASSIGN(PTR, VALUE) if (PTR) *(PTR) = (VALUE)




/*
 * Bitmap functions
 */

struct bit_map_t;

struct bit_map_t *bit_map_create(unsigned int size);
void bit_map_free(struct bit_map_t *bit_map);

void bit_map_set(struct bit_map_t *bit_map, unsigned int where, unsigned int size,
	unsigned int value);
unsigned int bit_map_get(struct bit_map_t *bit_map, unsigned int where, unsigned int size);
int bit_map_count_ones(struct bit_map_t *bit_map, unsigned int where, unsigned int size);
void bit_map_copy(struct bit_map_t *dst, unsigned int dst_where,
	struct bit_map_t *src, unsigned int src_where, unsigned int size);
void bit_map_dump(struct bit_map_t *bit_map, unsigned int where, unsigned int size, FILE *f);




/*
 * String Maps
 */

struct string_map_t
{
	int count;
	struct {
		char *string;
		int value;
	} map[];
};

int map_string(struct string_map_t *map, char *string);
int map_string_case(struct string_map_t *map, char *s);
char *map_value(struct string_map_t *map, int value);
void map_value_string(struct string_map_t *map, int value, char *out, int length);
void map_flags(struct string_map_t *map, int flags, char *out, int length);




/*
 * Strings
 */

void strccpy(char *dest, char *src, int size);
void strccat(char *dest, char *src);
void strdump(char *dest, char *src, int size);

void str_single_spaces(char *dest, char *src, int size);

int str_suffix(char *str, char *suffix);
int str_prefix(char *str, char *prefix);

void str_substr(char *dest, int dest_size, char *src, int src_pos, int src_count);

void str_token(char *dest, int dest_size, char *src, int index, char *delim);

/* Convert a string into an integer. The string can represent a number in
 * decimal or hexadecimal format, prepended with '0x'. */
int str_to_int(char *str);

/* Dump formatted string into a buffer with a specific size. Its size is then
 * decreased, and the buffer is advanced to the end of the dumped string.
 * This function is useful for being used in other functions that dump
 * several strings into a buffer, with the header
 *   obj_dump(struct obj_t *obj, char *buf, int size); */
void str_printf(char **pbuf, int *psize, char *fmt, ...) __attribute__ ((format (printf, 3, 4)));




/*
 * File management
 */


FILE *open_read(char *fname);
FILE *open_write(char *fname);
void close_file(FILE *f);

int can_open_read(char *fname);
int can_open_write(char *fname);

int read_line(FILE *f, char *line, int size);
FILE *create_temp_file(char *ret_path, int ret_path_size);




/*
 * Buffers
 */

int write_buffer(char *file_name, void *buf, int size);
void *read_buffer(char *file_name, int *psize);
void free_buffer(void *buf);




/*
 * Other
 */

void dump_bin(int x, int digits, FILE *f);
void dump_ptr(void *ptr, int size, FILE *stream);
int log_base2(int x);
void search_dist_file(char *file_name, char *dist_path, char *non_dist_path,
	char *buffer, int size);


#endif

