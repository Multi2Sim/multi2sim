/*
 *  Multi2Sim Tools
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/mhandle/mhandle.h>
#include <lib/misc/misc.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/matrix.h>


/* Function to invoke visualization tool.
 * The name of the trace file is passed as the argument. */
void visual_run(char *file_name);

#ifdef HAVE_GTK

/* Add code for visualization tool only below.*/

#include <gtk/gtk.h>




/*
 * Trace File
 */

struct vi_trace_t;

struct vi_trace_t *vi_trace_create(char *file_name);
void vi_trace_free(struct vi_trace_t *trace);


struct vi_trace_line_t;

struct vi_trace_line_t *vi_trace_line_create_from_file(FILE *f);
struct vi_trace_line_t *vi_trace_line_create_from_trace(struct vi_trace_t *trace);
void vi_trace_line_free(struct vi_trace_line_t *line);

void vi_trace_line_dump(struct vi_trace_line_t *line, FILE *f);
void vi_trace_line_dump_plain_text(struct vi_trace_line_t *line, FILE *f);

long int vi_trace_line_get_offset(struct vi_trace_line_t *line);

char *vi_trace_line_get_command(struct vi_trace_line_t *line);
char *vi_trace_line_get_symbol(struct vi_trace_line_t *line, char *symbol_name);
int vi_trace_line_get_symbol_int(struct vi_trace_line_t *line, char *symbol_name);
long long vi_trace_line_get_symbol_long_long(struct vi_trace_line_t *line, char *symbol_name);
unsigned int vi_trace_line_get_symbol_hex(struct vi_trace_line_t *line, char *symbol_name);




/*
 * State File
 */

typedef void (*vi_state_write_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*vi_state_read_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*vi_state_process_trace_line_func_t)(void *user_data, struct vi_trace_line_t *trace_line);

#define VI_STATE_FOR_EACH_HEADER(trace_line) \
	for ((trace_line) = vi_state_header_first(); \
	(trace_line); (trace_line) = vi_state_header_next())

void vi_state_init(char *trace_file_name);
void vi_state_done(void);

long long vi_state_get_num_cycles(void);
long long vi_state_get_current_cycle(void);

void vi_state_create_checkpoints(void);

void vi_state_new_category(char *name,
	vi_state_read_checkpoint_func_t read_checkpoint_func,
	vi_state_write_checkpoint_func_t write_checkpoint_func,
	void *user_data);
void vi_state_new_command(char *command_name,
	vi_state_process_trace_line_func_t process_trace_line_func,
	void *user_data);

struct vi_trace_line_t *vi_state_header_first(void);
struct vi_trace_line_t *vi_state_header_next(void);

struct vi_trace_line_t *vi_state_trace_line_first(long long cycle);
struct vi_trace_line_t *vi_state_trace_line_next(void);

void vi_state_go_to_cycle(long long cycle);




/*
 * Visual LED
 */

struct vi_led_t;

struct vi_led_t *vi_led_create(int radius);
void vi_led_free(struct vi_led_t *led);

void vi_led_set_color(struct vi_led_t *led, GdkColor *color);

GtkWidget *vi_led_get_widget(struct vi_led_t *led);




/*
 * Visual List
 */


extern char vi_list_image_close_path[MAX_PATH_SIZE];
extern char vi_list_image_close_sel_path[MAX_PATH_SIZE];

typedef void (*vi_list_get_elem_name_func_t)(void *elem, char *buf, int size);
typedef void (*vi_list_get_elem_desc_func_t)(void *elem, char *buf, int size);

#define VI_LIST_FOR_EACH(list, iter) \
	for ((iter) = 0; (iter) < vi_list_count((list)); (iter)++)

struct vi_list_t;

struct vi_list_t *vi_list_create(char *title, int width, int height,
	vi_list_get_elem_name_func_t get_elem_name,
	vi_list_get_elem_name_func_t get_elem_desc);
void vi_list_free(struct vi_list_t *list);

int vi_list_count(struct vi_list_t *list);
void vi_list_add(struct vi_list_t *list, void *elem);
void *vi_list_get(struct vi_list_t *list, int index);
void *vi_list_remove_at(struct vi_list_t *list, int index);

void vi_list_refresh(struct vi_list_t *list);

GtkWidget *vi_list_get_widget(struct vi_list_t *list);




/*
 * Pop-up Window with Text
 */

struct vi_popup_t;

struct vi_popup_t *vi_popup_create(char *text);
void vi_popup_free(struct vi_popup_t *popup);

void vi_popup_show(char *text);




/*
 * Cycle Bar
 */

typedef void (*vi_cycle_bar_refresh_func_t)(void *user_data, long long cycle);

void vi_cycle_bar_init(vi_cycle_bar_refresh_func_t refresh_func, void *user_data);
void vi_cycle_bar_done(void);

void vi_cycle_bar_set_refresh_func(vi_cycle_bar_refresh_func_t refresh_func, void *user_data);

GtkWidget *vi_cycle_bar_get_widget(void);
long long vi_cycle_bar_get_cycle(void);





#endif
#endif
