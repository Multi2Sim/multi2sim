#ifndef MISC_H
#define MISC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>



#define MAX_STRING_SIZE 200

#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
#define IN_RANGE(X, X1, X2) ((X)>=(X1)&&(X)<=(X2))

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


void fatal(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void dump_buf(char **pbuf, int *psize, char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

struct string_map_t {
	int count;
	struct {
		char *string;
		int value;
	} map[];
};

void strccat(char *dest, char *src);
void strccpy(char *dest, char *src, int size);

char *map_value(struct string_map_t *map, int value);
void map_value_string(struct string_map_t *map, int value, char *buf, int size);
void map_flags(struct string_map_t *map, int flags, char *out, int size);

int write_buffer(char *file_name, void *buf, size_t size);
void *read_buffer(char *file_name, size_t *psize);

#endif
