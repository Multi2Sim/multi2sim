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

#include <memvisual.h>
#include <stdlib.h>
#include <list.h>
#include <misc.h>



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

char *trace_line_get_command(struct trace_line_t *line);
char *trace_line_get_symbol_value(struct trace_line_t *line, char *symbol_name);




/*
 * Info Pop-up
 */

struct info_popup_t
{
	GtkWidget *window;
};

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

	/* List of low and high modules */
	struct list_t *low_vmod_list;
	struct list_t *high_vmod_list;

	/* Visual list of accesses */
	struct vlist_t *access_list;
};


struct vmod_t *vmod_create(char *name, int level);
void vmod_free(struct vmod_t *vmod);

gboolean vmod_draw_event(GtkWidget *widget, GdkEventConfigure *event, struct vmod_t *vmod);




/*
 * Panel with memory modules
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


/* Panel representing memory hierarchy */
struct vmod_panel_t
{
	/* Widget representing the panel */
	GtkWidget *widget;

	/* List of modules (vmod_t) */
	struct list_t *vmod_list;

	/* List of memory levels (vmod_level_t) */
	struct list_t *vmod_level_list;

	/* Maximum number of modules in a level */
	int max_modules_in_level;

};


struct vmod_panel_t *vmod_panel_create(void);
void vmod_panel_free(struct vmod_panel_t *panel);

void vmod_panel_refresh(struct vmod_panel_t *panel);

gboolean vmod_panel_draw_event(GtkWidget *widget, GdkEventConfigure *event,
	struct vmod_panel_t *panel);




/*
 * Visual Cache
 */

struct vcache_dir_entry_t
{
	int owner;

	/* Bit map of sharers (last field in variable-size structure) */
	unsigned char sharers[0];
};

struct vcache_block_t
{
	unsigned int tag;
	int state;

	struct vcache_dir_entry_t *dir_entries;
};

struct vcache_t
{
	/* GTK */
	GtkWidget *widget;
	GtkWidget *layout;

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


#endif

