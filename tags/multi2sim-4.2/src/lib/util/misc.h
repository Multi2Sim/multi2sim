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

#ifndef LIB_UTIL_MISC_H
#define LIB_UTIL_MISC_H

#include <stdio.h>

/* Boolean constants */
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

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
#define SWAPW(X)	((((unsigned int)(X)) << 24) |			\
			((((unsigned int)(X)) << 8)  & 0x00ff0000) |		\
			((((unsigned int)(X)) >> 8)  & 0x0000ff00) |		\
			((((unsigned int)(X)) >> 24) & 0x000000ff))
#define SWAPDW(X)	((((unsigned long long)(X)) << 56) |				\
			((((unsigned long long)(X)) << 40) & 0x00ff000000000000ULL) |	\
			((((unsigned long long)(X)) << 24) & 0x0000ff0000000000ULL) |	\
			((((unsigned long long)(X)) << 8)  & 0x000000ff00000000ULL) |	\
			((((unsigned long long)(X)) >> 8)  & 0x00000000ff000000ULL) |	\
			((((unsigned long long)(X)) >> 24) & 0x0000000000ff0000ULL) |	\
			((((unsigned long long)(X)) >> 40) & 0x000000000000ff00ULL) |	\
			((((unsigned long long)(X)) >> 56) & 0x00000000000000ffULL))


/* Sign extension */
#define SEXT32(X, B)		(((unsigned int) (X)) & (1U << ((B) - 1)) ? \
					((unsigned int) (X)) | \
					~((1U << (B)) - 1) : \
					((unsigned int) (X)) & ((1U << (B)) - 1))
#define SEXT64(X, B)		(((unsigned long long) (X)) & (1ULL << ((B) - 1)) ? \
					((unsigned long long) (X)) | \
					~((1ULL << (B)) - 1) : \
					((unsigned long long) (X)) & ((1ULL << (B)) - 1))

/* Extract bits from HI to LO from X */
#define BITS16(X, HI, LO)	((((unsigned short)(X))>>(LO))&((1U<<((HI)-(LO)+1))-1))
#define BITS32(X, HI, LO)	((((unsigned int)(X))>>(LO))&((1U<<((HI)-(LO)+1))-1))
#define BITS64(X, HI, LO)	((((unsigned long long)(X))>>(LO))&((1ULL<<((HI)-(LO)+1ULL))-1ULL))

/* Bit-handling macros */
#define GETBIT32(X, B)		((unsigned int)(X)&(1U<<(B)))
#define GETBIT64(X, B)		((unsigned long long)(X)&(1ULL<<(B)))
#define SETBIT32(X, B)		((unsigned int)(X)|(1U<<(B)))
#define SETBIT64(X, B)		((unsigned long long)(X)|(1ULL<<(B)))
#define CLEARBIT32(X, B)	((unsigned int)(X)&(~(1U<<(B))))
#define CLEARBIT64(X, B)	((unsigned long long)(X)&(~(1ULL<<(B))))
#define SETBITVALUE32(X, B, V)	((V) ? SETBIT32((X),(B)) : CLEARBIT32((X),(B)))
#define SETBITVALUE64(X, B, V)	((V) ? SETBIT64((X),(B)) : CLEARBIT64((X),(B)))

#define CLEAR_BITS_32(X, HI, LO) \
	((unsigned int) (X) & (((1ull << (LO)) - 1) \
	| ~((1ull << ((HI) + 1)) - 1)))

#define TRUNCATE_BITS_32(X, NUM) \
	((unsigned int) (X) & ((1ull << (NUM)) - 1))

#define SET_BITS_32(X, HI, LO, V) \
	(CLEAR_BITS_32((X), (HI), (LO)) | \
	(TRUNCATE_BITS_32((V), (HI) - (LO) + 1) << (LO)))

#define CLEAR_BITS_64(X, HI, LO) \
	((unsigned long long) (X) & (((1ull << (LO)) - 1) \
	| ~((1ull << ((HI) + 1)) - 1)))

#define TRUNCATE_BITS_64(X, NUM) \
	((unsigned long long) (X) & ((1ull << (NUM)) - 1))

#define SET_BITS_64(X, HI, LO, V) \
	(CLEAR_BITS_64((X), (HI), (LO)) | \
	(TRUNCATE_BITS_64((V), (HI) - (LO) + 1) << (LO)))


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


/* Macros to count the number of arguments passed to a macro using the '...'
 * notation in its prototype. Using PP_NARG(__VA_ARGS__) will return a number
 * between 1 and 64 containing the number of elements in __VA_ARGS__. */
#define PP_NARG(...) \
	PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
	PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
	_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
	_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
	_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
	_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
	_61,_62,_63,  N, ...) N
#define PP_RSEQ_N() \
	63,62,61,60,                   \
	59,58,57,56,55,54,53,52,51,50, \
	49,48,47,46,45,44,43,42,41,40, \
	39,38,37,36,35,34,33,32,31,30, \
	29,28,27,26,25,24,23,22,21,20, \
	19,18,17,16,15,14,13,12,11,10, \
	9, 8, 7, 6, 5, 4, 3, 2, 1, 0


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
 * Buffers
 */

int write_buffer(char *file_name, void *buf, int size);
void *read_buffer(char *file_name, int *psize);
void free_buffer(void *buf);



/*
 * Multi2Sim functions
 */

#define M2S_HOST_GUEST_MATCH(HOST_EXPR, GUEST_EXPR) \
	if ((HOST_EXPR) != (GUEST_EXPR)) \
		m2s_host_guest_match_error(#HOST_EXPR, (HOST_EXPR), (GUEST_EXPR));

void m2s_host_guest_match_error(char *expr, int host_value, int guest_value);

void m2s_dist_file(char *file_name, char *dist_path, char *non_dist_path,
	char *buffer, int size);



/*
 * Other
 */

int hex_str_to_byte_array(char *dest, char *str, int num);
void dump_bin(int x, int digits, FILE *f);
void dump_ptr(void *ptr, int size, FILE *stream);
int log_base2(int x);


#endif

