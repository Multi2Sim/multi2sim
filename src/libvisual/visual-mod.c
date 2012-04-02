/*
 *  Multi2Sim Tools
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

#include <visual-private.h>


static struct string_map_t visual_mod_block_state_map =
{
	6, {
		{ "I", 0 },
		{ "M", 1 },
		{ "O", 2 },
		{ "E", 3 },
		{ "S", 4 },
		{ "N", 5 }
	}
};


/*
 * Private Functions
 */


static struct visual_mod_dir_entry_t *visual_mod_get_dir_entry(struct visual_mod_t *mod,
	int set, int way, int sub_block)
{
	struct visual_mod_block_t *block;
	struct visual_mod_dir_entry_t *dir_entry;

	assert(IN_RANGE(set, 0, mod->num_sets - 1));
	assert(IN_RANGE(way, 0, mod->assoc - 1));
	assert(IN_RANGE(sub_block, 0, mod->num_sub_blocks - 1));

	block = &mod->blocks[set * mod->assoc + way];
	dir_entry = (struct visual_mod_dir_entry_t *) (((void *) block->dir_entries)
		+ VISUAL_MOD_DIR_ENTRY_SIZE(mod) * sub_block);

	return dir_entry;
}




/*
 * Public Functions
 */


struct visual_mod_t *visual_mod_create(struct trace_line_t *trace_line)
{
	struct visual_mod_t *mod;

	char *high_net_name;
	char *low_net_name;
	char *name;

	int set;
	int way;

	/* Allocate */
	mod = calloc(1, sizeof(struct visual_mod_t));
	if (!mod)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	name = trace_line_get_symbol_value(trace_line, "name");
	mod->name = strdup(name);
	if (!mod->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Get module parameters */
	mod->num_sets = trace_line_get_symbol_value_int(trace_line, "num_sets");
	mod->assoc = trace_line_get_symbol_value_int(trace_line, "assoc");
	mod->block_size = trace_line_get_symbol_value_int(trace_line, "block_size");
	mod->sub_block_size = trace_line_get_symbol_value_int(trace_line, "sub_block_size");
	mod->num_sub_blocks = mod->block_size / mod->sub_block_size;
	mod->num_sharers = trace_line_get_symbol_value_int(trace_line, "num_sharers");
	mod->level = trace_line_get_symbol_value_int(trace_line, "level");

	/* High network */
	high_net_name = trace_line_get_symbol_value(trace_line, "high_net");
	mod->high_net_node_index = trace_line_get_symbol_value_int(trace_line, "high_net_node");
	mod->high_net = hash_table_get(visual_mem_system->net_table, high_net_name);

	/* Low network */
	low_net_name = trace_line_get_symbol_value(trace_line, "low_net");
	mod->low_net_node_index = trace_line_get_symbol_value_int(trace_line, "low_net_node");
	mod->low_net = hash_table_get(visual_mem_system->net_table, low_net_name);

	/* Attach module to networks */
	if (mod->high_net)
		visual_net_attach_mod(mod->high_net, mod, mod->high_net_node_index);
	if (mod->low_net)
		visual_net_attach_mod(mod->low_net, mod, mod->low_net_node_index);

	/* Blocks */
	mod->blocks = calloc(mod->num_sets * mod->assoc, sizeof(struct visual_mod_block_t));
	for (set = 0; set < mod->num_sets; set++)
	{
		for (way = 0; way < mod->assoc; way++)
		{
			struct visual_mod_block_t *block;

			int sub_block;

			block = &mod->blocks[set * mod->assoc + way];
			block->mod = mod;
			block->set = set;
			block->way = way;
			block->access_list = linked_list_create();
			block->dir_entries = calloc(mod->num_sub_blocks, VISUAL_MOD_DIR_ENTRY_SIZE(mod));

			for (sub_block = 0; sub_block < mod->num_sub_blocks; sub_block++)
			{
				struct visual_mod_dir_entry_t *dir_entry;

				dir_entry = visual_mod_get_dir_entry(mod, set, way, sub_block);
				dir_entry->owner = -1;
			}
		}
	}

	/* Return */
	return mod;
}


void visual_mod_free(struct visual_mod_t *mod)
{
	struct visual_mod_block_t *block;
	struct linked_list_t *access_list;

	int i;

	/* Free blocks */
	for (i = 0; i < mod->num_sets * mod->assoc; i++)
	{
		/* Get block */
		block = &mod->blocks[i];
		access_list = block->access_list;

		/* Free accesses */
		while (linked_list_count(access_list))
		{
			linked_list_head(access_list);
			visual_mod_access_free(linked_list_get(access_list));
			linked_list_remove(access_list);
		}
		linked_list_free(access_list);

		/* Directory entries */
		free(mod->blocks[i].dir_entries);
	}
	free(mod->blocks);

	/* Free module */
	free(mod->name);
	free(mod);
}


void visual_mod_set_block(struct visual_mod_t *mod, int set, int way,
	unsigned int tag, char *state)
{
	struct visual_mod_block_t *block;

	if (!IN_RANGE(set, 0, mod->num_sets - 1))
		fatal("%s: invalid set", __FUNCTION__);
	if (!IN_RANGE(way, 0, mod->assoc - 1))
		fatal("%s: invalid way", __FUNCTION__);

	block = &mod->blocks[set * mod->assoc + way];
	block->tag = tag;
	block->state = map_string(&visual_mod_block_state_map, state);
}
