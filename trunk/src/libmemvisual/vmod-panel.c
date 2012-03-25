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

#include <memvisual-private.h>


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

			/* Create module and add to list */
			vmod = vmod_create(name, num_sets, assoc, block_size, sub_block_size,
					num_sharers, level);
			hash_table_insert(panel->vmod_table, name, vmod);
		}
	}
}


static void vmod_panel_populate(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	struct vmod_level_t *level;

	int num_levels = 0;
	int i;

	int level_id;
	int vmod_id;

	char *vmod_name;

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

	/* Insert levels */
	LIST_FOR_EACH(panel->vmod_level_list, level_id)
	{
		level = list_get(panel->vmod_level_list, level_id);

		/* Horizontal box for a new level */
		GtkWidget *hbox = gtk_hbox_new(0, VMOD_PADDING);
		gtk_container_add(GTK_CONTAINER(panel->widget), hbox);

		/* Modules */
		LIST_FOR_EACH(level->vmod_list, vmod_id)
		{
			vmod = list_get(level->vmod_list, vmod_id);
			gtk_container_add(GTK_CONTAINER(hbox), vmod->widget);

			if (vmod_id < level->vmod_list->count - 1)
				gtk_container_add(GTK_CONTAINER(hbox), gtk_vseparator_new());
		}

		/* Horizontal bar */
		if (level_id < panel->vmod_level_list->count - 1)
			gtk_container_add(GTK_CONTAINER(panel->widget), gtk_hseparator_new());
	}
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
	panel->vmod_level_list = list_create();

	/* Create widget */
	panel->widget = gtk_vbox_new(0, 0);

	/* Read and add components to the panel */
	vmod_panel_read_config(panel);
	vmod_panel_populate(panel);

	/* Return */
	return panel;
}


void vmod_panel_free(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	char *vmod_name;
	int i;

	/* Free modules */
	HASH_TABLE_FOR_EACH(panel->vmod_table, vmod_name, vmod)
		vmod_free(vmod);
	hash_table_free(panel->vmod_table);

	/* Free levels */
	LIST_FOR_EACH(panel->vmod_level_list, i)
		vmod_level_free(list_get(panel->vmod_level_list, i));
	list_free(panel->vmod_level_list);

	/* Free panel */
	free(panel);
}
