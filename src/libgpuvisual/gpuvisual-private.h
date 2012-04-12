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


#ifndef GPUVISUAL_PRIVATE_H_
#define GPUVISUAL_PRIVATE_H_

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <list.h>
#include <linked-list.h>
#include <gpuvisual.h>
#include <debug.h>
#include <misc.h>
#include <mhandle.h>



/*
 * Global variables
 */

extern char img_close_path[MAX_STRING_SIZE];
extern char img_close_sel_path[MAX_STRING_SIZE];

extern GtkWidget *main_window;



/*
 * Forward declarations
 */

struct vgpu_t;




/*
 * List Layout
 */

struct list_layout_item_t
{
	struct list_layout_t *list_layout;  /* List layout where it belongs */
	GtkWidget *event_box;  /* Event box containing label */
	GtkWidget *label;  /* Label with item text */
	void *item;
};

struct list_layout_t
{
	char title[MAX_STRING_SIZE];

	GtkWidget *parent_window;
	GtkWidget *layout;

	GtkWidget *popup_window;
	GtkWidget *img_close;

	struct list_t *item_list;
	void (*item_get_name)(void *item, char *buf, int size);
	void (*item_info_popup)(void *item);
	int text_size;

	/* List of 'list_layout_item_t' */
	struct list_t *list_layout_item_list;
	struct list_t *list_layout_popup_item_list;
};

struct list_layout_t *list_layout_new(GtkWidget *parent_window,
	char *title,
	struct list_t *list,
	int text_size,
	void (*item_get_name)(void *item, char *buf, int size),
	void (*item_info_popup)(void *item));
void list_layout_refresh(struct list_layout_t *list_layout);
struct list_layout_item_t *list_layout_item_create(GtkWidget *event_box, GtkWidget *label,
	void *item, struct list_layout_t *list_layout);
void list_layout_item_list_clear(struct list_t *list_layout_item_list);

gboolean list_layout_label_enter_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data);
gboolean list_layout_label_leave_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data);




/*
 * vgpu
 */

struct vgpu_state_checkpoint_t
{
	int cycle;
	long trace_file_pos;
	long state_file_pos;
};

struct vgpu_t
{
	/* Trace file */
	char trace_file_name[MAX_STRING_SIZE];
	char state_file_name[MAX_STRING_SIZE];
	FILE *trace_file;
	FILE *state_file;
	int trace_line_number;

	/* Associated graphic controls */
	GtkWidget *widget;  /* Main widget */
	GtkWidget *layout;  /* Layout containing sub-widgets */
	int layout_width;
	int layout_height;

	/* Properties */
	int num_compute_units;
	int num_work_groups;
	int num_mapped_work_groups;
	int max_cycles;

	/* State */
	int cycle;
	struct list_t *state_checkpoint_list;  /* Elements of type 'struct vgpu_state_info_t */
	struct list_t *work_group_list;
	struct list_t *pending_work_group_list;
	struct list_t *finished_work_group_list;
	struct list_t *compute_unit_list;

	/* Status bar text */
	char *status_text;
	int status_text_size;  /* Size allocated for status text */
	GtkWidget *status_label;

	/* Kernel source */
	struct list_t *kernel_source_strings;
};

extern char vgpu_trace_err[MAX_STRING_SIZE];

struct vgpu_t *vgpu_create(char *trace_file_name);
void vgpu_free(struct vgpu_t *vgpu);

void vgpu_status_write(struct vgpu_t *vgpu, char *fmt, ...);
void vgpu_status_clear(struct vgpu_t *vgpu);

void vgpu_store_state(struct vgpu_t *vgpu);
void vgpu_load_state(struct vgpu_t *vgpu);

int vgpu_trace_parse(struct vgpu_t *vgpu);
int vgpu_trace_cycle(struct vgpu_t *vgpu, int cycle);

GtkWidget *vgpu_widget_new(struct vgpu_t *vgpu);
void vgpu_widget_refresh(struct vgpu_t *vgpu);




/*
 * Work-Group
 */

struct vgpu_work_group_t
{
	int id;

	int work_item_id_first;
	int work_item_id_last;
	int work_item_count;

	int wavefront_id_first;
	int wavefront_id_last;
	int wavefront_count;
};

struct vgpu_work_group_t *vgpu_work_group_create(int id);
void vgpu_work_group_free(struct vgpu_work_group_t *work_group);

void work_group_get_name(void *work_group, char *buf, int size);
void work_group_info_popup(void *work_group);




/*
 * Uop
 */

enum vgpu_engine_t
{
	VGPU_ENGINE_CF,
	VGPU_ENGINE_ALU,
	VGPU_ENGINE_TEX,
	VGPU_ENGINE_COUNT
};

enum vgpu_stage_t
{
	VGPU_STAGE_FETCH,
	VGPU_STAGE_DECODE,
	VGPU_STAGE_READ,
	VGPU_STAGE_EXECUTE,
	VGPU_STAGE_WRITE,
	VGPU_STAGE_COMPLETE,
	VGPU_STAGE_COUNT
};

struct vgpu_uop_t
{
	int id;

	char name[MAX_STRING_SIZE];
	char vliw_slot[5][20];
	char vliw_slot_args[5][40];
	int compute_unit_id;
	int work_group_id;
	int wavefront_id;
	enum vgpu_engine_t engine;
	enum vgpu_stage_t stage;
	int stage_cycle;  /* Cycle when the last stage transition occurred */
	int finished;
};

extern double vgpu_stage_color[VGPU_ENGINE_COUNT][VGPU_STAGE_COUNT][3];

struct vgpu_uop_t *vgpu_uop_create(int id);
void vgpu_uop_free(struct vgpu_uop_t *uop);
struct vgpu_uop_t *vgpu_uop_dup(struct vgpu_uop_t *uop);

void vgpu_uop_get_markup(struct vgpu_uop_t *uop, char *buffer, int size);
void vgpu_uop_get_name(void *item, char *buf, int size);
void vgpu_uop_info_popup(void *item);

int vgpu_uop_list_add(struct list_t *list, struct vgpu_uop_t *uop);
struct vgpu_uop_t *vgpu_uop_list_get(struct list_t *list, int uop_id);
void vgpu_uop_list_dump(struct list_t *list, FILE *f);
void vgpu_uop_list_clear(struct list_t *list);
void vgpu_uop_list_free(struct list_t *list);





/*
 * Compute unit
 */

struct timing_dia_entry_t
{
	char text[10];
	int fill;  /* TRUE if cell should be filled */
	double fill_r, fill_g, fill_b;  /* Fill color */
};

struct vgpu_compute_unit_t
{
	struct vgpu_t *vgpu;

	int id;

	struct list_t *work_group_list;
	struct list_t *uop_list;

	/* Properties */
	int max_uops;  /* Total number of uops registered for this CU in the whole trace */
	int last_completed_uop_id;  /* ID of last completed uop */

	/* In vgpu widget */
	struct list_layout_t *work_group_list_layout;
	GtkWidget *status_widget;

	/* Timing diagram window */
	int timing_dia_active;  /* True if associated timing diagram window is showed */
	GtkWidget *timing_dia_toggle_button;  /* Toggle button activating window */
	GtkWidget *timing_window;  /* Window */
	GtkWidget *timing_inst_layout;  /* Drawing area for instructions */
	GtkWidget *timing_dia_area;  /* Drawing area for timing diagram */
	GtkWidget *timing_inst_title_area;  /* Title for instruction list */
	GtkWidget *timing_dia_title_area;  /* Cycles */
	GtkWidget *timing_dia_hscrollbar;
	GtkWidget *timing_dia_vscrollbar;
	int timing_dia_width;  /* Width of table last time it was drawn */
	int timing_dia_height;
	struct timing_dia_entry_t *timing_dia;  /* timing_dia_width * timing_dia_height elements */
	struct list_t *timing_inst_uops;  /* Column for instructions (list of 'timing_dia_height' elements of type 'vgpu_uop_t') */
	int timing_dia_uop_first;  /* Top uop id shown */
	int timing_dia_cycle_first;  /* Left cycle shown */
	int timing_dia_start_x;  /* Start drawing at this position */
	int timing_dia_start_y;
	double timing_dia_hscrollbar_value;
	double timing_dia_hscrollbar_incr_step;
	double timing_dia_hscrollbar_incr_page;
	double timing_dia_vscrollbar_value;
	double timing_dia_vscrollbar_incr_step;
	double timing_dia_vscrollbar_incr_page;

	/* Block diagram window */
	int block_dia_active;  /* True if associated bock diagram window is showed */
	GtkWidget *block_dia_window;  /* Window */
	GtkWidget *block_dia_toggle_button;  /* Toggle button activating window */

	/* Lists of uops in block diagram */
	struct list_layout_t *list_layout_cf_fetch;
	struct list_layout_t *list_layout_cf_decode;
	struct list_layout_t *list_layout_cf_execute;
	struct list_layout_t *list_layout_cf_complete;
	struct list_layout_t *list_layout_alu_fetch;
	struct list_layout_t *list_layout_alu_decode;
	struct list_layout_t *list_layout_alu_read;
	struct list_layout_t *list_layout_alu_execute;
	struct list_layout_t *list_layout_alu_write;
	struct list_layout_t *list_layout_tex_fetch;
	struct list_layout_t *list_layout_tex_decode;
	struct list_layout_t *list_layout_tex_read;
	struct list_layout_t *list_layout_tex_write;
	struct list_t *uop_list_cf_fetch;
	struct list_t *uop_list_cf_decode;
	struct list_t *uop_list_cf_execute;
	struct list_t *uop_list_cf_complete;
	struct list_t *uop_list_alu_fetch;
	struct list_t *uop_list_alu_decode;
	struct list_t *uop_list_alu_read;
	struct list_t *uop_list_alu_execute;
	struct list_t *uop_list_alu_write;
	struct list_t *uop_list_tex_fetch;
	struct list_t *uop_list_tex_decode;
	struct list_t *uop_list_tex_read;
	struct list_t *uop_list_tex_write;
};

struct vgpu_compute_unit_t *vgpu_compute_unit_create(struct vgpu_t *vgpu, int id);
void vgpu_compute_unit_free(struct vgpu_compute_unit_t *compute_unit);




/*
 * Timing diagram window
 */

void timing_dia_window_show(struct vgpu_compute_unit_t *compute_unit);
void timing_dia_window_hide(struct vgpu_compute_unit_t *compute_unit);

void timing_dia_window_refresh(struct vgpu_compute_unit_t *compute_unit);
void timing_dia_window_goto(struct vgpu_compute_unit_t *compute_unit, int cycle);




/*
 * Block diagram window
 */

extern char block_dia_cf_engine_image_path[MAX_STRING_SIZE];
extern char block_dia_alu_engine_image_path[MAX_STRING_SIZE];
extern char block_dia_tex_engine_image_path[MAX_STRING_SIZE];

void block_dia_window_show(struct vgpu_compute_unit_t *compute_unit);
void block_dia_window_hide(struct vgpu_compute_unit_t *compute_unit);

void block_dia_window_refresh(struct vgpu_compute_unit_t *compute_unit);




/*
 * Info Pop-up
 */

void vi_popup_show(char *text);


#endif /* GPUVISUAL_PRIVATE_H_ */
