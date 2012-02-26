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


#include <mem-system.h>


/*
 * Memory Module
 */

struct mod_t *mod_create(char *name, enum mod_kind_t kind,
	int bank_count, int read_port_count, int write_port_count,
	int block_size, int latency)
{
	struct mod_t *mod;

	/* Allocate */
	mod = calloc(1, sizeof(struct mod_t));
	if (!mod)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	mod->name = strdup(name);
	if (!mod->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	mod->kind = kind;
	mod->bank_count = bank_count;
	mod->read_port_count = read_port_count;
	mod->write_port_count = write_port_count;
	mod->banks = calloc(1, mod->bank_count * SIZEOF_MOD_BANK(mod));
	mod->latency = latency;

	/* Lists */
	mod->low_mod_list = linked_list_create();
	mod->high_mod_list = linked_list_create();

	/* Block size */
	mod->block_size = block_size;
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	mod->log_block_size = log_base2(block_size);

	return mod;
}


void mod_free(struct mod_t *mod)
{
	linked_list_free(mod->low_mod_list);
	linked_list_free(mod->high_mod_list);
	if (mod->cache)
		cache_free(mod->cache);
	free(mod->banks);
	free(mod->name);
	free(mod);
}


void mod_dump(struct mod_t *mod, FILE *f)
{
	struct mod_bank_t *bank;
	struct mod_port_t *port;
	struct mod_stack_t *stack;
	int i, j;

	/* Read ports */
	fprintf(f, "module '%s'\n", mod->name);
	for (i = 0; i < mod->bank_count; i++) {
		fprintf(f, "  bank %d:\n", i);
		bank = MOD_BANK_INDEX(mod, i);
		for (j = 0; j < mod->read_port_count; j++) {
			port = MOD_READ_PORT_INDEX(mod, bank, j);
			fprintf(f, "  read port %d: ", j);

			/* Waiting list */
			fprintf(f, "waiting={");
			for (stack = port->waiting_list_head; stack; stack = stack->waiting_next)
				fprintf(f, " %lld", (long long) stack->id);
			fprintf(f, " }\n");
		}
	}
}


/* Access a mod.
 * Argument 'access' defines whether it is a read (1) or a write (2).
 * Variable 'witness', if specified, will be increased when the access completes. */
void mod_access(struct mod_t *mod, int access, uint32_t addr, uint32_t size, int *witness_ptr)
{
	struct mod_stack_t *stack;
	int event;

	mod_stack_id++;
	stack = mod_stack_create(mod_stack_id,
		mod, addr, ESIM_EV_NONE, NULL);
	stack->witness_ptr = witness_ptr;
	assert(access == 1 || access == 2);
	event = access == 1 ? EV_GPU_MEM_READ : EV_GPU_MEM_WRITE;
	esim_schedule_event(event, stack, 0);
}


/* Return the low module serving a given address. */
struct mod_t *mod_get_low_mod(struct mod_t *mod, uint32_t addr)
{
	/* FIXME - for now, just return first module in list */
	/* Check that there is at least one lower-level module */
	if (mod->kind != mod_kind_cache)
		panic("%s: invalid module kind", __FUNCTION__);
	if (!mod->low_mod_list->count)
		fatal("%s: no low memory module", mod->name);

	/* Return first element */
	linked_list_head(mod->low_mod_list);
	return linked_list_get(mod->low_mod_list);
}




