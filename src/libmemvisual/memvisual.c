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



struct vmem_t
{
	GtkWidget *window;

	struct vmod_panel_t *vmod_panel;
	struct vcache_t *vcache;
};


static void vmem_destroy_event(GtkWidget *widget, struct vmem_t *vmem)
{
	vmem_free(vmem);
	gtk_main_quit();
}


struct vmem_t *vmem_create(void)
{
	struct vmem_t *vmem;

	/* Create */
	vmem = calloc(1, sizeof(struct vmem_t));
	if (!vmem)
		fatal("%s: out of memory", __FUNCTION__);

	/* State file */
	state_file_new_category(visual_state_file, "Memory hierarchy",
		vmem_write_checkpoint, vmem_read_checkpoint, vmem);
	state_file_new_command(visual_state_file, "mem.ttag", vmem_process_trace_line, vmem);
	state_file_new_command(visual_state_file, "mem.blk", vmem_process_trace_line, vmem);

	/* Main window */
	vmem->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(vmem->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(vmem->window), "Multi2Sim Memory Hierarchy Visualization Tool");
	gtk_container_set_border_width(GTK_CONTAINER(vmem->window), 0);
	g_signal_connect(G_OBJECT(vmem->window), "destroy", G_CALLBACK(vmem_destroy_event), vmem);

	/* Panel */
	vmem->vmod_panel = vmod_panel_create();
	gtk_container_add(GTK_CONTAINER(vmem->window), vmem->vmod_panel->widget);

	/* Show window */
	gtk_widget_show_all(vmem->window);

	/* Return */
	return vmem;
}


void vmem_free(struct vmem_t *vmem)
{
	vmod_panel_free(vmem->vmod_panel);
	free(vmem);
}


void vmem_read_checkpoint(void *user_data, FILE *f)
{
}


void vmem_write_checkpoint(void *user_data, FILE *f)
{
}


void vmem_process_trace_line(void *user_data, struct trace_line_t *trace_line)
{
	struct vmem_t *vmem = user_data;

	char *command;

	command = trace_line_get_command(trace_line);
	if (!strcmp(command, "mem.ttag"))
	{
	}
	else if (!strcmp(command, "mem.blk"))
	{
		struct vmod_t *vmod;

		char *vmod_name;
		char *state;

		int set;
		int way;

		unsigned int tag;

		/* Get fields */
		vmod_name = trace_line_get_symbol_value(trace_line, "cache");
		set = trace_line_get_symbol_value_int(trace_line, "set");
		way = trace_line_get_symbol_value_int(trace_line, "way");
		tag = trace_line_get_symbol_value_hex(trace_line, "tag");
		state = trace_line_get_symbol_value(trace_line, "state");

		/* Get module */
		vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
		if (!vmod)
			fatal("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

		/* Set block */
		vcache_set_block(vmod->vcache, set, way, tag, state);
	}
	else
		fatal("%s: unknown command '%s'", __FUNCTION__, command);
}




/*
 * Main Program
 */

struct state_file_t *visual_state_file;

void vmem_run(char *file_name)
{
	char *m2s_images_path = "images";

	/* Initialization */
	m2s_dist_file("close.png", m2s_images_path, m2s_images_path,
		vlist_image_close_path, sizeof vlist_image_close_path);
	m2s_dist_file("close-sel.png", m2s_images_path, m2s_images_path,
		vlist_image_close_sel_path, sizeof vlist_image_close_sel_path);

	/* State file */
	visual_state_file = state_file_create(file_name);

	/* Initialize GTK */
	gtk_init(NULL, NULL);

	/* Create main window */
	vmem_create();

	/* Parse trace file and create checkpoints */
	state_file_create_checkpoints(visual_state_file);

	/* Run GTK */
	gtk_main();

	/* Free main window */
	state_file_free(visual_state_file);
}

