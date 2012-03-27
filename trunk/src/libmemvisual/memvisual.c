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

	struct cycle_bar_t *cycle_bar;
	struct vmod_panel_t *vmod_panel;
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
		(state_file_read_checkpoint_func_t) vmem_read_checkpoint,
		(state_file_write_checkpoint_func_t) vmem_write_checkpoint,
		(state_file_refresh_func_t) vmem_refresh, vmem);
	state_file_new_command(visual_state_file, "mem.ttag",
		(state_file_process_trace_line_func_t) vmem_process_trace_line, vmem);
	state_file_new_command(visual_state_file, "mem.blk",
		(state_file_process_trace_line_func_t) vmem_process_trace_line, vmem);

	/* Main window */
	vmem->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(vmem->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(vmem->window), "Multi2Sim Memory Hierarchy Visualization Tool");
	gtk_container_set_border_width(GTK_CONTAINER(vmem->window), 0);
	g_signal_connect(G_OBJECT(vmem->window), "destroy", G_CALLBACK(vmem_destroy_event), vmem);

	/* Cycle bar */
	struct cycle_bar_t *cycle_bar;
	cycle_bar = cycle_bar_create();
	vmem->cycle_bar = cycle_bar;

	/* Panel */
	struct vmod_panel_t *vmod_panel;
	vmod_panel = vmod_panel_create();
	vmem->vmod_panel = vmod_panel;

	/* Vertical box */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), cycle_bar_get_widget(cycle_bar), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), vmod_panel_get_widget(vmod_panel), TRUE, TRUE, 0);

	/* Show window */
	gtk_container_add(GTK_CONTAINER(vmem->window), vbox);
	gtk_widget_show_all(vmem->window);

	/* Return */
	return vmem;
}


void vmem_free(struct vmem_t *vmem)
{
	vmod_panel_free(vmem->vmod_panel);
	free(vmem);
}


void vmem_read_checkpoint(struct vmem_t *vmem, FILE *f)
{
	char vmod_name[MAX_STRING_SIZE];

	struct vmod_t *vmod;

	int i;

	/* Read caches */
	for (i = 0; i < hash_table_count(vmem->vmod_panel->vmod_table); i++)
	{
		/* Get module */
		str_read_from_file(f, vmod_name, sizeof vmod_name);
		vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
		if (!vmod)
			panic("%s: invalid module name", __FUNCTION__);

		/* Read cache checkpoint */
		vcache_read_checkpoint(vmod->vcache, f);
	}
}


void vmem_write_checkpoint(struct vmem_t *vmem, FILE *f)
{
	struct vmod_t *vmod;

	char *vmod_name;

	/* Dump caches */
	HASH_TABLE_FOR_EACH(vmem->vmod_panel->vmod_table, vmod_name, vmod)
	{
		str_write_to_file(f, vmod->name);
		vmod_write_checkpoint(vmod, f);
	}
}


void vmem_process_trace_line(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
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


void vmem_refresh(struct vmem_t *vmem)
{
	long long cycle;

	cycle = cycle_bar_get_cycle(vmem->cycle_bar);
	state_file_go_to_cycle(visual_state_file, cycle);
	vmod_panel_refresh(vmem->vmod_panel);
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
	m2s_dist_file("back-single.png", m2s_images_path, m2s_images_path,
		cycle_bar_back_single_path, sizeof cycle_bar_back_single_path);
	m2s_dist_file("back-double.png", m2s_images_path, m2s_images_path,
		cycle_bar_back_double_path, sizeof cycle_bar_back_double_path);
	m2s_dist_file("back-triple.png", m2s_images_path, m2s_images_path,
		cycle_bar_back_triple_path, sizeof cycle_bar_back_triple_path);
	m2s_dist_file("forward-single.png", m2s_images_path, m2s_images_path,
		cycle_bar_forward_single_path, sizeof cycle_bar_forward_single_path);
	m2s_dist_file("forward-double.png", m2s_images_path, m2s_images_path,
		cycle_bar_forward_double_path, sizeof cycle_bar_forward_double_path);
	m2s_dist_file("forward-triple.png", m2s_images_path, m2s_images_path,
		cycle_bar_forward_triple_path, sizeof cycle_bar_forward_triple_path);
	m2s_dist_file("go.png", m2s_images_path, m2s_images_path,
		cycle_bar_go_path, sizeof cycle_bar_go_path);

	/* State file */
	visual_state_file = state_file_create(file_name);

	/* Initialize GTK */
	gtk_init(NULL, NULL);

	/* Create main window */
	vmem_create();

	/* Parse trace file and create checkpoints */
	state_file_create_checkpoints(visual_state_file);
	state_file_refresh(visual_state_file);

	/* Run GTK */
	gtk_main();

	/* Free main window */
	state_file_free(visual_state_file);
}

