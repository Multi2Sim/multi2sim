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


#include <lib/mhandle/mhandle.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <visual/common/cycle-bar.h>
#include <visual/common/led.h>
#include <visual/common/list.h>
#include <visual/common/state.h>

#include "compute-unit.h"
#include "gpu.h"
#include "panel.h"
#include "time-dia.h"
#include "work-group.h"



/*
 * Timing Diagram Window
 */

#define VI_SI_TIME_DIA_WINDOW_SIZE_X	600
#define VI_SI_TIME_DIA_WINDOW_SIZE_Y	250

struct vi_si_time_dia_window_t
{
	GtkWidget *widget;

	/* Compute unit that window shows */
	struct vi_si_compute_unit_t *compute_unit;

	/* Toggle button that causes activation of window */
	GtkWidget *parent_toggle_button;

	/* Timing diagram */
	struct vi_si_time_dia_t *time_dia;
};


static struct vi_si_time_dia_window_t *vi_si_time_dia_window_create(
	struct vi_si_compute_unit_t *compute_unit,
	GtkWidget *parent_toggle_button);
static void vi_si_time_dia_window_free(struct vi_si_time_dia_window_t *time_dia_window);


static void vi_si_time_dia_window_destroy(GtkWidget *widget,
	struct vi_si_time_dia_window_t *time_dia_window)
{
	vi_si_time_dia_window_free(time_dia_window);
}


static gboolean vi_si_time_dia_window_delete(GtkWidget *widget,
	GdkEvent *event, struct vi_si_time_dia_window_t *time_dia_window)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(time_dia_window->parent_toggle_button), FALSE);
	return TRUE;
}


static struct vi_si_time_dia_window_t *vi_si_time_dia_window_create(
	struct vi_si_compute_unit_t *compute_unit, GtkWidget *parent_toggle_button)
{
	struct vi_si_time_dia_window_t *time_dia_window;

	char str[MAX_STRING_SIZE];

	/* Initialize */
	time_dia_window = xcalloc(1, sizeof(struct vi_si_time_dia_window_t));
	time_dia_window->compute_unit = compute_unit;
	time_dia_window->parent_toggle_button = parent_toggle_button;

	/* Main window */
	snprintf(str, sizeof str, "Compute Unit %s", compute_unit->name);
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), str);
	gtk_window_set_default_size(GTK_WINDOW(window), VI_SI_TIME_DIA_WINDOW_SIZE_X,
		VI_SI_TIME_DIA_WINDOW_SIZE_Y);

	/* Time diagram */
	struct vi_si_time_dia_t *time_dia;
	time_dia = vi_si_time_dia_create(compute_unit);
	gtk_container_add(GTK_CONTAINER(window), vi_si_time_dia_get_widget(time_dia));
	time_dia_window->time_dia = time_dia;

	/* Associate widget */
	time_dia_window->widget = window;
	gtk_widget_show_all(time_dia_window->widget);
	g_signal_connect(G_OBJECT(time_dia_window->widget), "destroy",
		G_CALLBACK(vi_si_time_dia_window_destroy), time_dia_window);
	g_signal_connect(G_OBJECT(time_dia_window->widget), "delete_event",
		G_CALLBACK(vi_si_time_dia_window_delete), time_dia_window);

	/* Return */
	return time_dia_window;
}


static void vi_si_time_dia_window_free(struct vi_si_time_dia_window_t *time_dia_window)
{
	free(time_dia_window);
}


static void vi_si_time_dia_window_go_to_cycle(struct vi_si_time_dia_window_t *time_dia_window,
	long long cycle)
{
	vi_si_time_dia_go_to_cycle(time_dia_window->time_dia, cycle);
}


static void vi_si_time_dia_window_refresh(struct vi_si_time_dia_window_t *time_dia_window)
{
	vi_si_time_dia_refresh(time_dia_window->time_dia);
}


static GtkWidget *vi_si_time_dia_window_get_widget(struct vi_si_time_dia_window_t *time_dia_window)
{
	return time_dia_window->widget;
}




/*
 * Compute Unit Board
 */

#define VI_CU_BOARD_PADDING		10
#define VI_CU_BOARD_WIDTH		100
#define VI_CU_BOARD_HEIGHT		100


struct vi_cu_board_t
{
	/* Main widget */
	GtkWidget *widget;

	GtkWidget *time_dia_toggle_button;

	struct vi_si_time_dia_window_t *time_dia_window;

	struct vi_si_compute_unit_t *compute_unit;
	struct vi_led_t *led;
	struct vi_list_t *work_group_list;
};


/* Forward declarations */

static struct vi_cu_board_t *vi_cu_board_create(struct vi_si_compute_unit_t *compute_unit);
static void vi_cu_board_free(struct vi_cu_board_t *board);

static void vi_cu_board_refresh(struct vi_cu_board_t *board);


/* Functions */

static void vi_cu_board_destroy(GtkWidget *widget, struct vi_cu_board_t *board)
{
	vi_cu_board_free(board);
}


static gboolean vi_cu_board_time_dia_toggled(GtkWidget *widget, struct vi_cu_board_t *board)
{
	struct vi_si_time_dia_window_t *time_dia_window;

	int active;

	/* Get button state */
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(board->time_dia_toggle_button));

	/* Show */
	if (active && !board->time_dia_window)
	{
		time_dia_window = vi_si_time_dia_window_create(board->compute_unit,
			board->time_dia_toggle_button);
		board->time_dia_window = time_dia_window;
	}

	/* Hide */
	if (!active && board->time_dia_window)
	{
		gtk_widget_destroy(vi_si_time_dia_window_get_widget(board->time_dia_window));
		board->time_dia_window = NULL;
	}

	return FALSE;
}


static struct vi_cu_board_t *vi_cu_board_create(struct vi_si_compute_unit_t *compute_unit)
{
	struct vi_cu_board_t *board;

	char str[MAX_STRING_SIZE];

	/* Initialize */
	board = xcalloc(1, sizeof(struct vi_cu_board_t));
	board->compute_unit = compute_unit;

	/* Frame */
	GtkWidget *frame = gtk_frame_new(NULL);
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), frame);
	gtk_widget_set_size_request(frame, VI_CU_BOARD_WIDTH, VI_CU_BOARD_HEIGHT);

	/* Vertical box */
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* Name */
	snprintf(str, sizeof str, "<b>%s</b>", compute_unit->name);
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), str);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);

	/* Horizontal box */
	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	/* LED */
	struct vi_led_t *led = vi_led_create(13);
	gtk_box_pack_start(GTK_BOX(hbox), vi_led_get_widget(led), FALSE, TRUE, 0);
	board->led = led;

	/* Time Diagram Toggle button */
	GtkWidget *time_dia_toggle_button = gtk_toggle_button_new_with_label("Detail");
	gtk_box_pack_start(GTK_BOX(hbox), time_dia_toggle_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(time_dia_toggle_button), "toggled",
		G_CALLBACK(vi_cu_board_time_dia_toggled), board);
	board->time_dia_toggle_button = time_dia_toggle_button;

	/* Work-group list */
	struct vi_list_t *work_group_list = vi_list_create("Work-group list", 10, 10,
		(vi_list_get_elem_name_func_t) vi_si_work_group_get_name_short,
		(vi_list_get_elem_desc_func_t) vi_si_work_group_get_desc);
	gtk_box_pack_start(GTK_BOX(vbox), vi_list_get_widget(work_group_list), TRUE, TRUE, 0);
	board->work_group_list = work_group_list;

	/* Main widget */
	board->widget = event_box;
	g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_cu_board_destroy), board);

	/* Return */
	return board;
}


static void vi_cu_board_free(struct vi_cu_board_t *board)
{
	/* Free work-group list */
	while (vi_list_count(board->work_group_list))
		free(vi_list_remove_at(board->work_group_list, 0));
	vi_list_free(board->work_group_list);

	/* Destroy time diagram */
	if (board->time_dia_window)
		gtk_widget_destroy(vi_si_time_dia_window_get_widget(board->time_dia_window));

	/* Free */
	free(board);
}


static GtkWidget *vi_cu_board_get_widget(struct vi_cu_board_t *board)
{
	return board->widget;
}


static void vi_cu_board_refresh(struct vi_cu_board_t *board)
{
	struct hash_table_t *work_group_table;
	struct vi_si_work_group_t *work_group;

	char *work_group_name;

	long long cycle;

	/* Go to cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Empty work-group list */
	while (vi_list_count(board->work_group_list))
		free(vi_list_remove_at(board->work_group_list, 0));

	/* Refresh work-group list */
	work_group_table = board->compute_unit->work_group_table;
	HASH_TABLE_FOR_EACH(work_group_table, work_group_name, work_group)
	{
		/* Add to list */
		work_group_name = xstrdup(work_group_name);
		vi_list_add(board->work_group_list, work_group_name);
	}
	vi_list_refresh(board->work_group_list);

	/* LED */
	GdkColor color;
	if (vi_list_count(board->work_group_list))
		gdk_color_parse("red", &color);
	else
		gdk_color_parse("green", &color);
	vi_led_set_color(board->led, &color);

	/* Refresh time diagram */
	if (board->time_dia_window)
	{
		vi_si_time_dia_window_go_to_cycle(board->time_dia_window, cycle);
		vi_si_time_dia_window_refresh(board->time_dia_window);
	}
}




/*
 * GPU Panel
 */

#define VI_SI_PANEL_PADDING	5

#define VI_SI_PANEL_WIDTH	100
#define VI_SI_PANEL_HEIGHT	100


struct vi_si_panel_t
{
	GtkWidget *widget;

	struct list_t *board_list;
};


static void vi_si_panel_destroy(GtkWidget *widget,
	struct vi_si_panel_t *panel)
{
	list_free(panel->board_list);
	vi_si_panel_free(panel);
}


struct vi_si_panel_t *vi_si_panel_create(void)
{
	struct vi_si_compute_unit_t *compute_unit;
	struct vi_si_panel_t *panel;

	struct vi_cu_board_t *board;

	int layout_width;
	int layout_height;

	int i;

	/* Initialize */
	panel = xcalloc(1, sizeof(struct vi_si_panel_t));
	panel->board_list = list_create();

	/* Frame */
	GtkWidget *frame;
	frame = gtk_frame_new("Southern Islands GPU");

	/* Layout */
	GtkWidget *layout;
	GdkColor color;
	layout = gtk_layout_new(NULL, NULL);
	gdk_color_parse("white", &color);
	gtk_widget_modify_bg(layout, GTK_STATE_NORMAL, &color);

	/* Scrolled window */
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), layout);
	gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

	/* Boards */
	layout_width = VI_CU_BOARD_PADDING;
	layout_height = VI_CU_BOARD_HEIGHT + 2 * VI_CU_BOARD_PADDING;
	LIST_FOR_EACH(vi_si_gpu->compute_unit_list, i)
	{
		/* Create board and insert */
		compute_unit = list_get(vi_si_gpu->compute_unit_list, i);
		board = vi_cu_board_create(compute_unit);
		list_add(panel->board_list, board);
		gtk_layout_put(GTK_LAYOUT(layout), vi_cu_board_get_widget(board),
			layout_width, VI_CU_BOARD_PADDING);

		/* Accumulate width */
		layout_width += VI_CU_BOARD_WIDTH + VI_CU_BOARD_PADDING;
	}

	/* Sizes of scrolled window */
	gtk_widget_set_size_request(layout, layout_width, layout_height);
	gtk_widget_set_size_request(scrolled_window, VI_CU_BOARD_WIDTH * 3 / 2, layout_height);

	/* Widget */
	panel->widget = frame;
	g_signal_connect(G_OBJECT(panel->widget), "destroy",
		G_CALLBACK(vi_si_panel_destroy), panel);

	/* Return */
	return panel;
}


void vi_si_panel_free(struct vi_si_panel_t *panel)
{
	free(panel);
}


void vi_si_panel_refresh(struct vi_si_panel_t *panel)
{
	struct vi_cu_board_t *board;

	int board_id;

	/* Refresh all module boards */
	LIST_FOR_EACH(panel->board_list, board_id)
	{
		board = list_get(panel->board_list, board_id);
		vi_cu_board_refresh(board);
	}
}


GtkWidget *vi_si_panel_get_widget(struct vi_si_panel_t *panel)
{
	return panel->widget;
}
