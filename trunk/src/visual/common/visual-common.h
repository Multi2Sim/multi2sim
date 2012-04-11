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


#ifndef VISUAL_COMMON_H
#define VISUAL_COMMON_H

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include <debug.h>
#include <hash-table.h>
#include <linked-list.h>
#include <list.h>
#include <misc.h>


/* Function to invoke visualization tool.
 * The name of the trace file is passed as the argument. */
void visual_run(char *file_name);

#ifdef HAVE_GTK

/* Add code for visualization tool only below.*/

#include <gtk/gtk.h>




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

/* Global state file */
extern struct state_file_t *visual_state_file;

typedef void (*state_file_write_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*state_file_read_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*state_file_process_trace_line_func_t)(void *user_data, struct trace_line_t *trace_line);
typedef void (*state_file_refresh_func_t)(void *user_data);

#define STATE_FILE_FOR_EACH_HEADER(state_file, trace_line) \
	for ((trace_line) = state_file_header_first((state_file)); \
	(trace_line); (trace_line) = state_file_header_next((state_file)))

struct state_file_t *state_file_create(char *trace_file_name);
void state_file_free(struct state_file_t *file);

long long state_file_get_num_cycles(struct state_file_t *file);
long long state_file_get_cycle(struct state_file_t *file);

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

struct trace_line_t *state_file_trace_line_first(struct state_file_t *file, long long cycle);
struct trace_line_t *state_file_trace_line_next(struct state_file_t *file);

void state_file_refresh(struct state_file_t *file);
void state_file_go_to_cycle(struct state_file_t *file, long long cycle);




/*
 * Visual List
 */


extern char visual_list_image_close_path[MAX_PATH_SIZE];
extern char visual_list_image_close_sel_path[MAX_PATH_SIZE];

typedef void (*visual_list_get_elem_name_func_t)(void *elem, char *buf, int size);
typedef void (*visual_list_get_elem_desc_func_t)(void *elem, char *buf, int size);

#define VISUAL_LIST_FOR_EACH(list, iter) \
	for ((iter) = 0; (iter) < visual_list_count((list)); (iter)++)

struct visual_list_t;

struct visual_list_t *visual_list_create(char *title, int width, int height,
	visual_list_get_elem_name_func_t get_elem_name,
	visual_list_get_elem_name_func_t get_elem_desc);
void visual_list_free(struct visual_list_t *list);

int visual_list_count(struct visual_list_t *list);
void visual_list_add(struct visual_list_t *list, void *elem);
void *visual_list_get(struct visual_list_t *list, int index);
void *visual_list_remove_at(struct visual_list_t *list, int index);

void visual_list_refresh(struct visual_list_t *list);

GtkWidget *visual_list_get_widget(struct visual_list_t *list);




/*
 * Info Pop-up
 */

struct info_popup_t *info_popup_create(char *text);
void info_popup_free(struct info_popup_t *popup);

void info_popup_show(char *text);




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

/* Global cycle bar */
extern struct cycle_bar_t *visual_cycle_bar;

typedef void (*cycle_bar_refresh_func_t)(void *user_data, long long cycle);

struct cycle_bar_t *cycle_bar_create(void);
void cycle_bar_free(struct cycle_bar_t *cycle_bar);

void cycle_bar_set_refresh_func(struct cycle_bar_t *cycle_bar,
	cycle_bar_refresh_func_t refresh_func, void *user_data);

GtkWidget *cycle_bar_get_widget(struct cycle_bar_t *cycle_bar);
long long cycle_bar_get_cycle(struct cycle_bar_t *cycle_bar);





#endif
#endif
