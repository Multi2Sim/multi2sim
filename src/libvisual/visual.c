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



struct vmem_t
{
	GtkWidget *window;

	struct cycle_bar_t *cycle_bar;
	struct vmod_panel_t *vmod_panel;
};


/* Trace command 'set_transient_tag' */
static void vmem_process_trace_line_set_transient_tag(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
}


/* Trace command 'set_block' */
static void vmem_process_trace_line_set_block(struct vmem_t *vmem, struct trace_line_t *trace_line)
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
		panic("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

	/* Set block */
	vcache_set_block(vmod->vcache, set, way, tag, state);
}


/* Trace command 'set_sharer' */
static void vmem_process_trace_line_set_sharer(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	char *vmod_name;

	struct vmod_t *vmod;

	int x;
	int y;
	int z;
	int sharer;

	/* Get module */
	vmod_name = trace_line_get_symbol_value(trace_line, "dir");
	vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
	if (!vmod)
		panic("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");
	sharer = trace_line_get_symbol_value_int(trace_line, "sharer");

	/* Set sharer */
	vcache_dir_entry_set_sharer(vmod->vcache, x, y, z, sharer);
}


/* Trace command 'clear_sharer' */
static void vmem_process_trace_line_clear_sharer(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	char *vmod_name;

	struct vmod_t *vmod;

	int x;
	int y;
	int z;
	int sharer;

	/* Get module */
	vmod_name = trace_line_get_symbol_value(trace_line, "dir");
	vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
	if (!vmod)
		panic("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");
	sharer = trace_line_get_symbol_value_int(trace_line, "sharer");

	/* Set sharer */
	vcache_dir_entry_clear_sharer(vmod->vcache, x, y, z, sharer);
}


/* Trace command 'clear_all_sharers' */
static void vmem_process_trace_line_clear_all_sharers(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	char *vmod_name;

	struct vmod_t *vmod;

	int x;
	int y;
	int z;

	/* Get module */
	vmod_name = trace_line_get_symbol_value(trace_line, "dir");
	vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
	if (!vmod)
		panic("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");

	/* Set sharer */
	vcache_dir_entry_clear_all_sharers(vmod->vcache, x, y, z);
}


/* Trace command 'set_owner' */
static void vmem_process_trace_line_set_owner(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	char *vmod_name;

	struct vmod_t *vmod;

	int x;
	int y;
	int z;
	int owner;

	/* Get module */
	vmod_name = trace_line_get_symbol_value(trace_line, "dir");
	vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
	if (!vmod)
		panic("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

	/* Directory entry and sharer */
	x = trace_line_get_symbol_value_int(trace_line, "x");
	y = trace_line_get_symbol_value_int(trace_line, "y");
	z = trace_line_get_symbol_value_int(trace_line, "z");
	owner = trace_line_get_symbol_value_int(trace_line, "owner");

	/* Set sharer */
	vcache_dir_entry_set_owner(vmod->vcache, x, y, z, owner);
}


/* Trace command 'new_access' */
static void vmem_process_trace_line_new_access(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	char *name;
	char *state;

	struct vmod_panel_t *panel = vmem->vmod_panel;
	struct vlist_t *access_list = panel->vmod_access_list;
	struct vmod_access_t *access;

	/* Read fields */
	name = trace_line_get_symbol_value(trace_line, "name");
	state = trace_line_get_symbol_value(trace_line, "state");

	/* Create new access */
	access = vmod_access_create(name);
	vmod_access_set_state(access, state);

	/* Add access to list */
	vlist_add(access_list, access);
}


/* Trace command 'end_access' */
static void vmem_process_trace_line_end_access(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	struct vmod_panel_t *panel = vmem->vmod_panel;
	struct vmod_access_t *access;

	char *name;

	/* Read fields */
	name = trace_line_get_symbol_value(trace_line, "name");

	/* Find access */
	access = vmod_panel_remove_access(panel, name);
	if (!access)
		panic("%s: access not found", __FUNCTION__);

	/* Free access */
	vmod_access_free(access);
}


/* Trace command 'new_access_mod' */
static void vmem_process_trace_line_new_access_mod(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	struct vmod_access_t *access;
	struct vmod_t *vmod;

	char *vmod_name;
	char *access_name;

	/* Read fields */
	vmod_name = trace_line_get_symbol_value(trace_line, "mod");
	access_name = trace_line_get_symbol_value(trace_line, "access");

	/* Module */
	vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
	if (!vmod)
		panic("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

	/* Create new access */
	access = vmod_access_create(access_name);

	/* Add access to list */
	vlist_add(vmod->vmod_access_list, access);
}


/* Trace command 'end_access_mod' */
static void vmem_process_trace_line_end_access_mod(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	struct vmod_access_t *access;
	struct vmod_t *vmod;

	char *vmod_name;
	char *access_name;

	/* Read fields */
	vmod_name = trace_line_get_symbol_value(trace_line, "mod");
	access_name = trace_line_get_symbol_value(trace_line, "access");

	/* Module */
	vmod = hash_table_get(vmem->vmod_panel->vmod_table, vmod_name);
	if (!vmod)
		panic("%s: invalid module name '%s'", __FUNCTION__, vmod_name);

	/* Remove access */
	access = vmod_remove_access(vmod, access_name);
	if (!access)
		panic("%s: access not found", __FUNCTION__);

	/* Free access */
	vmod_access_free(access);
}


/* Trace command 'new_access_block' */
static void vmem_process_trace_line_new_access_block(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
}


/* Trace command 'end_access_block' */
static void vmem_process_trace_line_end_access_block(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
}


/* Trace command 'access' */
static void vmem_process_trace_line_access(struct vmem_t *vmem, struct trace_line_t *trace_line)
{
	struct vmod_panel_t *panel = vmem->vmod_panel;
	struct vmod_access_t *access;

	char *name;
	char *state;

	/* Read fields */
	name = trace_line_get_symbol_value(trace_line, "name");
	state = trace_line_get_symbol_value(trace_line, "state");

	/* Find access */
	access = vmod_panel_find_access(panel, name);
	if (!access)
		panic("%s: access not found", __FUNCTION__);

	/* Update access */
	vmod_access_set_state(access, state);
}


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
	state_file_new_command(visual_state_file, "mem.set_transient_tag",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_set_transient_tag, vmem);
	state_file_new_command(visual_state_file, "mem.set_block",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_set_block, vmem);
	state_file_new_command(visual_state_file, "mem.set_sharer",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_set_sharer, vmem);
	state_file_new_command(visual_state_file, "mem.clear_sharer",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_clear_sharer, vmem);
	state_file_new_command(visual_state_file, "mem.clear_all_sharers",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_clear_all_sharers, vmem);
	state_file_new_command(visual_state_file, "mem.set_owner",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_set_owner, vmem);
	state_file_new_command(visual_state_file, "mem.new_access",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_new_access, vmem);
	state_file_new_command(visual_state_file, "mem.end_access",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_end_access, vmem);
	state_file_new_command(visual_state_file, "mem.new_access_mod",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_new_access_mod, vmem);
	state_file_new_command(visual_state_file, "mem.end_access_mod",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_end_access_mod, vmem);
	state_file_new_command(visual_state_file, "mem.new_access_block",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_new_access_block, vmem);
	state_file_new_command(visual_state_file, "mem.end_access_block",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_end_access_block, vmem);
	state_file_new_command(visual_state_file, "mem.access",
		(state_file_process_trace_line_func_t) vmem_process_trace_line_access, vmem);

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
	vmod_panel_read_checkpoint(vmem->vmod_panel, f);
}


void vmem_write_checkpoint(struct vmem_t *vmem, FILE *f)
{
	vmod_panel_write_checkpoint(vmem->vmod_panel, f);
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

