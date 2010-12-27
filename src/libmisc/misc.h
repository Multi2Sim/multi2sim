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

#ifndef MISC_H
#define MISC_H

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


/* max string size */
#define MAX_STRING_SIZE			200

/* min max bool */
#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
#define BOOL(X) ((X) ? 't' : 'f')
#define IN_RANGE(X, X1, X2) ((X)>=(X1)&&(X)<=(X2))

/* round */
#define ROUND_UP(N,ALIGN)	(((N) + ((ALIGN)-1)) & ~((ALIGN)-1))
#define ROUND_DOWN(N,ALIGN)	((N) & ~((ALIGN)-1))

/* alignment */
#define DWORD_ALIGN(N) ROUND_DOWN((N),8)
#define WORD_ALIGN(N) ROUND_DOWN((N), 4)
#define HALF_ALIGN(N) ROUND_DOWN((N), 2)

/* modulo */
#define MOD(X, Y)		(((X) + (Y)) % (Y))

/* endian control */
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


/* sign extend */
#define SEXT32(X, B)		(((uint32_t)(X))&(1U<<(B-1))?((uint32_t)(X))|~((1U<<B)-1):(X))
#define SEXT64(X, B)		(((uint64_t)(X))&(1ULL<<(B-1))?((uint64_t)(X))|~((1ULL<<B)-1):(X))

/* extract bits from HI to LO from X */
#define BITS32(X, HI, LO)	((((uint32_t)(X))>>(LO))&((1U<<((HI)-(LO)+1))-1))
#define BITS64(X, HI, LO)	((((uint64_t)(X))>>(LO))&((1ULL<<((HI)-(LO)+1ULL))-1ULL))

/* bits */
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




/* Bit map functions */

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




/* String Maps */
struct string_map_t {
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

/* strings */
void strccpy(char *dest, char *src, int size);
void strccat(char *dest, char *src);
void strdump(char *dest, char *src, int size);

/* File management */
FILE *open_read(char *fname);
FILE *open_write(char *fname);
int can_open_read(char *fname);
int can_open_write(char *fname);
int read_line(FILE *f, char *line, int size);
void close_file(FILE *f);
FILE *create_temp_file(char *ret_path, int ret_path_size);

/* other */
void dump_bin(int x, int digits, FILE *f);
void dump_ptr(void *ptr, int size, FILE *stream);
int log_base2(int x);

/* Buffers */
int write_buffer(char *file_name, void *buf, int size);
void *read_buffer(char *file_name, int *psize);
void free_buffer(void *buf);

#endif

