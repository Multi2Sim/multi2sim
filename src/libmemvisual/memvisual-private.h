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


#ifndef MEMVISUAL_PRIVATE_H
#define MEMVISUAL_PRIVATE_H

#include <assert.h>
#include <math.h>
#include <gtk/gtk.h>

#include <hash-table.h>
#include <memvisual.h>
#include <list.h>
#include <misc.h>
#include <stdlib.h>



/*
 * Trace File
 */

struct trace_file_t;

struct trace_file_t *trace_file_create(char *file_name);
void trace_file_free(struct trace_file_t *file);


struct trace_line_t;

struct trace_line_t *trace_line_create_from_file(FILE *f);
struct trace_line_t *trace_line_create_from_trace_file(struct trace_file_t *f);
void trace_line_free(struct trace_line_t *line);

void trace_line_dump(struct trace_line_t *line, FILE *f);
void trace_line_dump_plain_text(struct trace_line_t *line, FILE *f);

long int trace_line_get_offset(struct trace_line_t *line);

char *trace_line_get_command(struct trace_line_t *line);
char *trace_line_get_symbol_value(struct trace_line_t *line, char *symbol_name);
int trace_line_get_symbol_value_int(struct trace_line_t *line, char *symbol_name);
long long trace_line_get_symbol_value_long_long(struct trace_line_t *line, char *symbol_name);
unsigned int trace_line_get_symbol_value_hex(struct trace_line_t *line, char *symbol_name);




/*
 * State File
 */

struct state_file_t;

typedef void (*state_file_write_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*state_file_read_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*state_file_process_trace_line_func_t)(void *user_data, struct trace_line_t *trace_line);
typedef void (*state_file_refresh_func_t)(void *user_data);

extern struct state_file_t *visual_state_file;

struct state_file_t *state_file_create(char *trace_file_name);
void state_file_free(struct state_file_t *file);

long long state_file_get_num_cycles(struct state_file_t *file);

void state_file_create_checkpoints(struct state_file_t *file);

void state_file_new_category(struct state_file_t *file, char *name,
	state_file_read_checkpoint_func_t read_checkpoint_func,
	state_file_write_checkpoint_func_t write_checkpoint_func,
	state_file_refresh_func_t refresh_func,
	void *user_data);
void state_file_new_command(struct state_file_t *file, char *command_name,
	state_file_process_trace_line_func_t process_trace_line_func,
	void *user_data);

struct trace_line_t *state_file_header_first(struct state_file_t *file);
struct trace_line_t *state_file_header_next(struct state_file_t *file);

void state_file_refresh(struct state_file_t *file);
void state_file_go_to_cycle(struct state_file_t *file, long long cycle);



/*
 * Info Pop-up
 */

struct info_popup_t *info_popup_create(char *text);
void info_popup_free(struct info_popup_t *popup);

void info_popup_show(char *text);



/*
 * Visual List
 */


extern char vlist_image_close_path[MAX_PATH_SIZE];
extern char vlist_image_close_sel_path[MAX_PATH_SIZE];


struct vlist_item_t
{
	/* Associated GTK widgets */
	GtkWidget *event_box;
	GtkWidget *label;

	/* Visual list where it belongs */
	struct vlist_t *vlist;

	/* Associated data element from 'vlist->elem_list' */
	void *elem;
};


struct vlist_popup_t
{
	/* GTK widgets */
	GtkWidget *window;
	GtkWidget *image_close;

	/* List of 'vlist_item_t' elements */
	struct list_t *item_list;

	/* Visual list that triggered the pop-up */
	struct vlist_t *vlist;
};

struct vlist_popup_t *vlist_popup_create(struct vlist_t *vlist);
void vlist_popup_free(struct vlist_popup_t *popup);

void vlist_popup_show(struct vlist_t *vlist);


struct vlist_t
{
	/* Widget showing list */
	GtkWidget *widget;

	/* Dimensions after last refresh */
	int width;
	int height;

	/* List of elements in the list. These elements can have any external type.
	 * They are controlled with 'vlist_add', 'vlist_remove', etc. macros. */
	struct list_t *elem_list;

	/* List of elements 'vlist_item_t' currently displayed in the list. This
	 * list will synchronize with 'elem_list' upon a call to 'vlist_refresh'. */
	struct list_t *item_list;

	/* Call-back functions to get element names and descriptions */
	void (*get_elem_name)(void *elem, char *buf, int size);
	void (*get_elem_desc)(void *elem, char *buf, int size);

	/* Properties */
	char *title;
	int text_size;
};


struct vlist_t *vlist_create(char *title, int width, int height,
	void (*get_elem_name)(void *elem, char *buf, int size),
	void (*get_elem_desc)(void *elem, char *buf, int size));
void vlist_free(struct vlist_t *vlist);

#define vlist_count(vlist) list_count((vlist)->elem_list)
#define vlist_add(vlist, elem) list_add((vlist)->elem_list, (elem))
#define vlist_get(vlist, index) list_get((vlist)->elem_list, (index))
#define vlist_set(vlist, index, elem) list_get((vlist)->elem_list, (index), (elem))
#define vlist_insert(vlist, index, elem) list_insert((vlist)->elem_list, (index), (elem))
#define vlist_remove_at(vlist, index) list_remove_at((vlist)->elem_list, (index))

void vlist_refresh(struct vlist_t *vlist);




/*
 * Access to a memory module
 */

struct vmod_access_t
{
	long long id;
};

struct vmod_access_t *vmod_access_create(long long id);
void vmod_access_free(struct vmod_access_t *access);

void vmod_access_get_name(void *access, char *buf, int size);
void vmod_access_get_desc(void *access, char *buf, int size);




/*
 * Visual Memory Module
 */

struct vmod_t
{
	char *name;
	int level;

	/* Widget representing module */
	GtkWidget *widget;

	/* Associated cache */
	struct vcache_t *vcache;

	/* Visual list of accesses */
	struct vlist_t *access_list;
};


struct vmod_t *vmod_create(char *name, int num_sets, int assoc, int block_size,
		int sub_block_size, int num_sharers, int level);
void vmod_free(struct vmod_t *vmod);

void vmod_read_checkpoint(struct vmod_t *vmod, FILE *f);
void vmod_write_checkpoint(struct vmod_t *vmod, FILE *f);

void vmod_refresh(struct vmod_t *vmod);

GtkWidget *vmod_get_widget(struct vmod_t *vmod);




/*
 * Panel with memory modules
 */

struct vmod_panel_t
{
	/* Widget representing the panel */
	GtkWidget *widget;

	/* Table of 'vmod_t' modules */
	struct hash_table_t *vmod_table;

	/* List of memory levels (vmod_level_t) */
	struct list_t *vmod_level_list;

	/* Maximum number of modules in a level */
	int max_modules_in_level;

};

struct vmod_panel_t *vmod_panel_create(void);
void vmod_panel_free(struct vmod_panel_t *panel);

GtkWidget *vmod_panel_get_widget(struct vmod_panel_t *panel);

void vmod_panel_refresh(struct vmod_panel_t *panel);




/*
 * Visual Cache
 */

struct vcache_dir_entry_t
{
	int owner;
	int num_sharers;

	/* Bit map of sharers (last field in variable-size structure) */
	unsigned char sharers[0];
};

struct vcache_block_t
{
	struct vcache_t *vcache;

	GdkColor sharers_label_color;
	GtkWidget *sharers_label;

	unsigned int tag;
	int state;

	struct vcache_dir_entry_t *dir_entries;
};

struct vcache_t
{
	/* GTK */
	GtkWidget *widget;
	GtkWidget *layout;
	GtkWidget *first_row_layout;
	GtkWidget *first_col_layout;

	GtkWidget *hscrollbar;
	GtkWidget *vscrollbar;

	/* Dimensions for last refresh */
	int width;
	int height;

	/* Displayed name */
	char *name;

	int num_sets;
	int assoc;
	int block_size;

	int num_sub_blocks;
	int sub_block_size;

	int num_sharers;

	struct vcache_block_t *blocks;
};

struct vcache_t *vcache_create(char *name, int num_sets, int assoc, int block_size,
	int sub_block_size, int num_sharers);
void vcache_free(struct vcache_t *vcache);

void vcache_set_block(struct vcache_t *vcache, int set, int way,
	unsigned int tag, char *state);

void vcache_dir_entry_set_sharer(struct vcache_t *vcache,
	int x, int y, int z, int sharer);
void vcache_dir_entry_clear_sharer(struct vcache_t *vcache,
	int x, int y, int z, int sharer);
void vcache_dir_entry_clear_all_sharers(struct vcache_t *vcache,
	int x, int y, int z);

void vcache_read_checkpoint(struct vcache_t *vcache, FILE *f);
void vcache_write_checkpoint(struct vcache_t *vcache, FILE *f);

void vcache_refresh(struct vcache_t *vcache);

GtkWidget *vcache_get_widget(struct vcache_t *vcache);




/*
 * Cycle Bar
 */

extern char cycle_bar_back_single_path[MAX_PATH_SIZE];
extern char cycle_bar_back_double_path[MAX_PATH_SIZE];
extern char cycle_bar_back_triple_path[MAX_PATH_SIZE];

extern char cycle_bar_forward_single_path[MAX_PATH_SIZE];
extern char cycle_bar_forward_double_path[MAX_PATH_SIZE];
extern char cycle_bar_forward_triple_path[MAX_PATH_SIZE];

extern char cycle_bar_go_path[MAX_PATH_SIZE];

struct cycle_bar_t;

struct cycle_bar_t *cycle_bar_create(void);
void cycle_bar_free(struct cycle_bar_t *cycle_bar);

GtkWidget *cycle_bar_get_widget(struct cycle_bar_t *cycle_bar);
long long cycle_bar_get_cycle(struct cycle_bar_t *cycle_bar);




/*
 * Main Window
 */

struct vmem_t *vmem_create(void);
void vmem_free(struct vmem_t *vmem);

void vmem_read_checkpoint(struct vmem_t *vmem, FILE *f);
void vmem_write_checkpoint(struct vmem_t *vmem, FILE *f);

void vmem_refresh(struct vmem_t *vmem);


#endif

