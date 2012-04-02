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


struct visual_mod_t *visual_mod_create(struct trace_line_t *trace_line)
{
	struct visual_mod_t *mod;

	char *high_net_name;
	char *low_net_name;
	char *name;

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

	/* Return */
	return mod;
}


void visual_mod_free(struct visual_mod_t *mod)
{
	free(mod->name);
	free(mod);
}
