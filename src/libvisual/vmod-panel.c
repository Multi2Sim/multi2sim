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


/*
 * Module Level
 */

#define VMOD_PADDING  5

#define VMOD_DEFAULT_WIDTH  100
#define VMOD_DEFAULT_HEIGHT 100


/* One level of the memory hierarchy */
struct vmod_level_t
{
	/* List of modules in this level */
	struct list_t *vmod_list;
};


static struct vmod_level_t *vmod_level_create(void)
{
	struct vmod_level_t *level;

	/* Create */
	level = calloc(1, sizeof(struct vmod_level_t));
	if (!level)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	level->vmod_list = list_create();

	/* Return */
	return level;
}


static void vmod_level_free(struct vmod_level_t *level)
{
	list_free(level->vmod_list);
	free(level);
}




/*
 * Visual Module Panel
 */

static void vmod_panel_read_config(struct vmod_panel_t *panel)
{
	struct trace_line_t *trace_line;

	for (trace_line = state_file_header_first(visual_state_file);
		trace_line;
		trace_line = state_file_header_next(visual_state_file))
	{
		char *command;

		/* End of header */
		command = trace_line_get_command(trace_line);
		if (!strcmp(command, "c"))
		{
			trace_line_free(trace_line);
			break;
		}
		else if (!strcmp(command, "mem.new_mod"))
		{
			struct vmod_t *vmod;

			struct vnet_t *high_vnet;
			struct vnet_t *low_vnet;

			int high_vnet_node_index;
			int low_vnet_node_index;

			char *high_vnet_name;
			char *low_vnet_name;
			char *name;

			int num_sets;
			int assoc;
			int block_size;
			int sub_block_size;
			int num_sharers;
			int level;

			/* Get module parameters */
			name = trace_line_get_symbol_value(trace_line, "name");
			num_sets = trace_line_get_symbol_value_int(trace_line, "num_sets");
			assoc = trace_line_get_symbol_value_int(trace_line, "assoc");
			block_size = trace_line_get_symbol_value_int(trace_line, "block_size");
			sub_block_size = trace_line_get_symbol_value_int(trace_line, "sub_block_size");
			num_sharers = trace_line_get_symbol_value_int(trace_line, "num_sharers");
			level = trace_line_get_symbol_value_int(trace_line, "level");

			/* High network */
			high_vnet_name = trace_line_get_symbol_value(trace_line, "high_net");
			high_vnet_node_index = trace_line_get_symbol_value_int(trace_line, "high_net_node");
			high_vnet = hash_table_get(panel->vnet_table, high_vnet_name);

			/* Low network */
			low_vnet_name = trace_line_get_symbol_value(trace_line, "low_net");
			low_vnet_node_index = trace_line_get_symbol_value_int(trace_line, "low_net_node");
			low_vnet = hash_table_get(panel->vnet_table, low_vnet_name);

			/* Create module and add to list */
			vmod = vmod_create(panel, name, num_sets, assoc, block_size, sub_block_size,
				num_sharers, level, high_vnet, high_vnet_node_index,
				low_vnet, low_vnet_node_index);
			hash_table_insert(panel->vmod_table, name, vmod);

			/* Attach modules to networks */
			if (high_vnet)
				vnet_attach_vmod(high_vnet, vmod, high_vnet_node_index);
			if (low_vnet)
				vnet_attach_vmod(low_vnet, vmod, low_vnet_node_index);
		}
		else if (!strcmp(command, "mem.new_net"))
		{
			struct vnet_t *vnet;

			char *name;

			int num_nodes;

			/* Get network parameters */
			name = trace_line_get_symbol_value(trace_line, "name");
			num_nodes = trace_line_get_symbol_value_int(trace_line, "num_nodes");

			/* Create network and add to list */
			vnet = vnet_create(name, num_nodes);
			hash_table_insert(panel->vnet_table, name, vnet);
		}
	}
}


static void vmod_panel_create_widget(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	struct vmod_level_t *level;

	int num_levels = 0;
	int i;

	int level_id;
	int vmod_id;

	char *vmod_name;

	GtkWidget *vbox;

	/* Get number of levels */
	HASH_TABLE_FOR_EACH(panel->vmod_table, vmod_name, vmod)
	{
		if (vmod->level < 0)
			fatal("%s: invalid level", __FUNCTION__);
		num_levels = MAX(num_levels, vmod->level + 1);
	}

	/* Create levels */
	for (i = 0; i < num_levels; i++)
	{
		level = vmod_level_create();
		list_add(panel->vmod_level_list, level);
	}

	/* Add modules to levels */
	HASH_TABLE_FOR_EACH(panel->vmod_table, vmod_name, vmod)
	{
		level = list_get(panel->vmod_level_list, vmod->level);
		list_add(level->vmod_list, vmod);
	}

	/* Insert access list */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), vlist_get_widget(panel->vmod_access_list), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);

	/* Insert levels */
	LIST_FOR_EACH(panel->vmod_level_list, level_id)
	{
		level = list_get(panel->vmod_level_list, level_id);

		/* Empty level */
		if (!list_count(level->vmod_list))
			continue;

		/* Horizontal box for a new level */
		GtkWidget *hbox = gtk_hbox_new(0, VMOD_PADDING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

		/* Modules */
		LIST_FOR_EACH(level->vmod_list, vmod_id)
		{
			vmod = list_get(level->vmod_list, vmod_id);
			gtk_box_pack_start(GTK_BOX(hbox), vmod_get_widget(vmod), TRUE, TRUE, 0);

			if (vmod_id < level->vmod_list->count - 1)
				gtk_box_pack_start(GTK_BOX(hbox), gtk_vseparator_new(), FALSE, FALSE, 0);
		}

		/* Horizontal bar */
		if (level_id < panel->vmod_level_list->count - 1)
			gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
	}

	/* Assign panel widget */
	panel->widget = vbox;
}


struct vmod_panel_t *vmod_panel_create(void)
{
	struct vmod_panel_t *panel;

	/* Allocate */
	panel = calloc(1, sizeof(struct vmod_panel_t));
	if (!panel)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	panel->vmod_table = hash_table_create(0, FALSE);
	panel->vnet_table = hash_table_create(0, FALSE);
	panel->vmod_level_list = list_create();
	panel->vmod_access_list = vlist_create("List of accesses", 50, 50,
		(vlist_get_elem_name_func_t) vmod_access_get_name_str,
		(vlist_get_elem_desc_func_t) vmod_access_get_desc_str);

	/* Read and add components to the panel */
	vmod_panel_read_config(panel);
	vmod_panel_create_widget(panel);

	/* Return */
	return panel;
}


void vmod_panel_free(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	struct vnet_t *vnet;

	char *vmod_name;
	char *vnet_name;

	int i;

	/* Free modules */
	HASH_TABLE_FOR_EACH(panel->vmod_table, vmod_name, vmod)
		vmod_free(vmod);
	hash_table_free(panel->vmod_table);

	/* Free networks */
	HASH_TABLE_FOR_EACH(panel->vnet_table, vnet_name, vnet)
		vnet_free(vnet);
	hash_table_free(panel->vnet_table);

	/* Free levels */
	LIST_FOR_EACH(panel->vmod_level_list, i)
		vmod_level_free(list_get(panel->vmod_level_list, i));
	list_free(panel->vmod_level_list);

	/* Free accesses */
	VLIST_FOR_EACH(panel->vmod_access_list, i)
		vmod_access_free(vlist_get(panel->vmod_access_list, i));
	vlist_free(panel->vmod_access_list);

	/* Free panel */
	free(panel);
}


/* Find an access in the panel access list */
struct vmod_access_t *vmod_panel_find_access(struct vmod_panel_t *panel, char *name)
{
	struct vlist_t *access_list = panel->vmod_access_list;
	struct vmod_access_t *access;

	int index;

	/* Find access */
	VLIST_FOR_EACH(access_list, index)
	{
		access = vlist_get(access_list, index);
		if (!strcmp(name, vmod_access_get_name(access)))
			return access;
	}

	/* Access not found */
	return NULL;
}


struct vmod_access_t *vmod_panel_remove_access(struct vmod_panel_t *panel, char *name)
{
	struct vlist_t *access_list = panel->vmod_access_list;
	struct vmod_access_t *access;

	int index;

	/* Find access */
	VLIST_FOR_EACH(access_list, index)
	{
		access = vlist_get(access_list, index);
		if (!strcmp(name, vmod_access_get_name(access)))
			break;
	}

	/* Access not found */
	if (index == vlist_count(access_list))
		return NULL;

	/* Remove access */
	vlist_remove_at(access_list, index);
	return access;
}


GtkWidget *vmod_panel_get_widget(struct vmod_panel_t *panel)
{
	return panel->widget;
}


struct vnet_t *vmod_panel_get_vnet(struct vmod_panel_t *panel, char *name)
{
	return hash_table_get(panel->vnet_table, name);
}


struct vmod_t *vmod_panel_get_vmod(struct vmod_panel_t *panel, char *name)
{
	return hash_table_get(panel->vmod_table, name);
}


void vmod_panel_read_checkpoint(struct vmod_panel_t *panel, FILE *f)
{
	char vmod_name[MAX_STRING_SIZE];

	struct vmod_t *vmod;
	struct vmod_access_t *access;

	int num_accesses;
	int count;
	int i;

	/* Empty access list */
	while (vlist_count(panel->vmod_access_list))
	{
		access = vlist_remove_at(panel->vmod_access_list, 0);
		vmod_access_free(access);
	}

	/* Read number of accesses */
	count = fread(&num_accesses, 1, 4, f);
	if (count != 4)
		fatal("%s: error reading from checkpoint", __FUNCTION__);

	/* Read accesses */
	for (i = 0; i < num_accesses; i++)
	{
		access = vmod_access_create(NULL);
		vmod_access_read_checkpoint(access, f);
		vlist_add(panel->vmod_access_list, access);
	}

	/* Read caches */
	for (i = 0; i < hash_table_count(panel->vmod_table); i++)
	{
		/* Get module */
		str_read_from_file(f, vmod_name, sizeof vmod_name);
		vmod = hash_table_get(panel->vmod_table, vmod_name);
		if (!vmod)
			panic("%s: invalid module name", __FUNCTION__);

		/* Read cache checkpoint */
		vcache_read_checkpoint(vmod->vcache, f);
	}
}


void vmod_panel_write_checkpoint(struct vmod_panel_t *panel, FILE *f)
{
	struct vmod_t *vmod;
	struct vmod_access_t *access;

	char *vmod_name;

	int num_accesses;
	int count;
	int i;

	/* Dump access list */
	num_accesses = vlist_count(panel->vmod_access_list);
	count = fwrite(&num_accesses, 1, 4, f);
	VLIST_FOR_EACH(panel->vmod_access_list, i)
	{
		access = vlist_get(panel->vmod_access_list, i);
		vmod_access_write_checkpoint(access, f);
	}
	if (count != 4)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

	/* Dump caches */
	HASH_TABLE_FOR_EACH(panel->vmod_table, vmod_name, vmod)
	{
		str_write_to_file(f, vmod->name);
		vmod_write_checkpoint(vmod, f);
	}
}


void vmod_panel_refresh(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;

	char *vmod_name;

	vlist_refresh(panel->vmod_access_list);
	HASH_TABLE_FOR_EACH(panel->vmod_table, vmod_name, vmod)
		vmod_refresh(vmod);
}
