/*
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

#ifndef CACHESYSTEM_H
#define CACHESYSTEM_H

#include <mhandle.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <network.h>
#include <misc.h>
#include <esim.h>

#include <config.h>
#include <mem-system.h>
#include <debug.h>
#include <linked-list.h>




/*
 * Global variables
 */


extern char *cache_system_config_help;




/*
 * Coherent Cache
 */

struct ccache_access_t
{
	enum mod_access_kind_t access_kind;  /* Read or write */
	uint32_t address;  /* Block address */
	uint64_t id;  /* Access identifier */
	struct linked_list_t *eventq;  /* Event queue to modify when access finishes */
	void *eventq_item;  /* Item to enqueue when finished */
	struct ccache_access_t *next;  /* Alias (same address/access, but different eventq_item */
};

struct mod_t *__mod_create(char *name, enum mod_kind_t kind);
void __mod_free(struct mod_t *mod);

int __mod_find_block(struct mod_t *mod, uint32_t addr,
	uint32_t *pset, uint32_t *pway, uint32_t *ptag, int *pstatus);
void __mod_dump(struct mod_t *mod, FILE *f);
struct mod_t *__mod_get_low_mod(struct mod_t *mod);




/*
 * Cache System
 */

extern char *cache_system_config_file_name;
extern char *cache_system_report_file_name;

extern int cache_system_iperfect;
extern int cache_system_dperfect;

extern int cache_min_block_size;
extern int cache_max_block_size;

enum cache_kind_t
{
	cache_kind_inst,
	cache_kind_data
};

struct cache_system_stack_t
{
	int core;
	int thread;

	enum cache_kind_t cache_kind;
	enum mod_access_kind_t access_kind;

	uint32_t addr;

	struct linked_list_t *eventq;
	void *eventq_item;

	int retevent;
	void *retstack;
};

struct cache_system_stack_t *cache_system_stack_create(int core, int thread, uint32_t addr,
	int retevent, void *retstack);
void cache_system_stack_return(struct cache_system_stack_t *stack);
void cache_system_handler(int event, void *data);

extern int EV_CACHE_SYSTEM_ACCESS;
extern int EV_CACHE_SYSTEM_ACCESS_CACHE;
extern int EV_CACHE_SYSTEM_ACCESS_FINISH;

void cache_system_init(int def_cores, int def_threads);
void cache_system_done(void);

void cache_system_print_stats(FILE *f);
void cache_system_dump(FILE *f);

/* Return block size of the first cache when accessing the cache system
 * by a given core-thread and cache kind. */
int cache_system_block_size(int core, int thread,
	enum cache_kind_t cache_kind);

/* Return true if cache system can be accesses. */
int cache_system_can_access(int core, int thread, enum cache_kind_t cache_kind,
	enum mod_access_kind_t access_kind, uint32_t addr);

/* Return true if the access to address addr or with identifier 'access'
 * has completed. Parameter cache_kind must be dl1 or il1. */
int cache_system_pending_address(int core, int thread,
	enum cache_kind_t cache_kind, uint32_t addr);
int cache_system_pending_access(int core, int thread,
	enum cache_kind_t cache_kind, uint64_t access);

/* Functions to access cache system */
uint64_t cache_system_read(int core, int thread, enum cache_kind_t cache_kind,
	uint32_t addr, struct linked_list_t *eventq, void *item);
uint64_t cache_system_write(int core, int thread, enum cache_kind_t cache_kind,
	uint32_t addr, struct linked_list_t *eventq, void *eventq_item);


#endif

