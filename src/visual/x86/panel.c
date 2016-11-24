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

#include "context.h"
#include "core.h"
#include "cpu.h"
#include "panel.h"
#include "time-dia.h"


/*
 * Timing Diagram Window
 */

#define VI_X86_TIME_DIA_WINDOW_SIZE_X	600
#define VI_X86_TIME_DIA_WINDOW_SIZE_Y	250

struct vi_x86_time_dia_window_t
{
	GtkWidget *widget;

	/* Core that window shows */
	struct vi_x86_core_t *core;

	/* Toggle button that causes activation of window */
	GtkWidget *parent_toggle_button;

	/* Timing diagram */
	struct vi_x86_time_dia_t *time_dia;
};


static struct vi_x86_time_dia_window_t *vi_x86_time_dia_window_create(struct vi_x86_core_t *core,
	GtkWidget *parent_toggle_button);
static void vi_x86_time_dia_window_free(struct vi_x86_time_dia_window_t *time_dia_window);


static void vi_x86_time_dia_window_destroy(GtkWidget *widget,
	struct vi_x86_time_dia_window_t *time_dia_window)
{
	vi_x86_time_dia_window_free(time_dia_window);
}


static gboolean vi_x86_time_dia_window_delete(GtkWidget *widget,
	GdkEvent *event, struct vi_x86_time_dia_window_t *time_dia_window)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(time_dia_window->parent_toggle_button), FALSE);
	return TRUE;
}


static struct vi_x86_time_dia_window_t *vi_x86_time_dia_window_create(struct vi_x86_core_t *core,
	GtkWidget *parent_toggle_button)
{
	struct vi_x86_time_dia_window_t *time_dia_window;

	/* Initialize */
	time_dia_window = xcalloc(1, sizeof(struct vi_x86_time_dia_window_t));
	time_dia_window->core = core;
	time_dia_window->parent_toggle_button = parent_toggle_button;

	/* Main window */
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), core->name);
	gtk_window_set_default_size(GTK_WINDOW(window), VI_X86_TIME_DIA_WINDOW_SIZE_X,
		VI_X86_TIME_DIA_WINDOW_SIZE_Y);

	/* Time diagram */
	struct vi_x86_time_dia_t *time_dia;
	time_dia = vi_x86_time_dia_create(core);
	gtk_container_add(GTK_CONTAINER(window), vi_x86_time_dia_get_widget(time_dia));
	time_dia_window->time_dia = time_dia;

	/* Associate widget */
	time_dia_window->widget = window;
	gtk_widget_show_all(time_dia_window->widget);
	g_signal_connect(G_OBJECT(time_dia_window->widget), "destroy",
		G_CALLBACK(vi_x86_time_dia_window_destroy), time_dia_window);
	g_signal_connect(G_OBJECT(time_dia_window->widget), "delete_event",
		G_CALLBACK(vi_x86_time_dia_window_delete), time_dia_window);

	/* Return */
	return time_dia_window;
}


static void vi_x86_time_dia_window_free(struct vi_x86_time_dia_window_t *time_dia_window)
{
	free(time_dia_window);
}


static void vi_x86_time_dia_window_go_to_cycle(struct vi_x86_time_dia_window_t *time_dia_window,
	long long cycle)
{
	vi_x86_time_dia_go_to_cycle(time_dia_window->time_dia, cycle);
}


static void vi_x86_time_dia_window_refresh(struct vi_x86_time_dia_window_t *time_dia_window)
{
	vi_x86_time_dia_refresh(time_dia_window->time_dia);
}


static GtkWidget *vi_x86_time_dia_window_get_widget(struct vi_x86_time_dia_window_t *time_dia_window)
{
	return time_dia_window->widget;
}




/*
 * Core Board
 */

#define VI_X86_CORE_BOARD_PADDING	10
#define VI_X86_CORE_BOARD_WIDTH		100
#define VI_X86_CORE_BOARD_HEIGHT	100


struct vi_x86_core_board_t
{
	/* Main widget */
	GtkWidget *widget;

	GtkWidget *time_dia_toggle_button;

	struct vi_x86_time_dia_window_t *time_dia_window;

	struct vi_x86_core_t *core;
	struct vi_led_t *led;
	struct vi_list_t *context_list;
};


/* Forward declarations */

static struct vi_x86_core_board_t *vi_x86_core_board_create(struct vi_x86_core_t *core);
static void vi_x86_core_board_free(struct vi_x86_core_board_t *board);

static void vi_x86_core_board_refresh(struct vi_x86_core_board_t *board);


/* Functions */

static void vi_x86_core_board_destroy(GtkWidget *widget, struct vi_x86_core_board_t *board)
{
	vi_x86_core_board_free(board);
}


static gboolean vi_x86_core_board_time_dia_toggled(GtkWidget *widget, struct vi_x86_core_board_t *board)
{
	struct vi_x86_time_dia_window_t *time_dia_window;

	int active;

	/* Get button state */
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(board->time_dia_toggle_button));

	/* Show */
	if (active && !board->time_dia_window)
	{
		time_dia_window = vi_x86_time_dia_window_create(board->core,
			board->time_dia_toggle_button);
		board->time_dia_window = time_dia_window;
	}

	/* Hide */
	if (!active && board->time_dia_window)
	{
		gtk_widget_destroy(vi_x86_time_dia_window_get_widget(board->time_dia_window));
		board->time_dia_window = NULL;
	}
	return FALSE;
}


static struct vi_x86_core_board_t *vi_x86_core_board_create(struct vi_x86_core_t *core)
{
	struct vi_x86_core_board_t *board;

	char str[MAX_STRING_SIZE];

	/* Initialize */
	board = xcalloc(1, sizeof(struct vi_x86_core_board_t));
	board->core = core;

	/* Frame */
	GtkWidget *frame = gtk_frame_new(NULL);
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), frame);
	gtk_widget_set_size_request(frame, VI_X86_CORE_BOARD_WIDTH, VI_X86_CORE_BOARD_HEIGHT);

	/* Vertical box */
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* Name */
	snprintf(str, sizeof str, "<b>%s</b>", core->name);
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
		G_CALLBACK(vi_x86_core_board_time_dia_toggled), board);
	board->time_dia_toggle_button = time_dia_toggle_button;

	/* Context list */
	struct vi_list_t *context_list = vi_list_create("Context list", 10, 10,
		(vi_list_get_elem_name_func_t) vi_x86_context_get_name_short,
		(vi_list_get_elem_desc_func_t) vi_x86_context_get_desc);
	gtk_box_pack_start(GTK_BOX(vbox), vi_list_get_widget(context_list), TRUE, TRUE, 0);
	board->context_list = context_list;

	/* Main widget */
	board->widget = event_box;
	g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_x86_core_board_destroy), board);

	/* Return */
	return board;
}


static void vi_x86_core_board_free(struct vi_x86_core_board_t *board)
{
	/* Free context list */
	while (vi_list_count(board->context_list))
		free(vi_list_remove_at(board->context_list, 0));
	vi_list_free(board->context_list);

	/* Destroy time diagram */
	if (board->time_dia_window)
		gtk_widget_destroy(vi_x86_time_dia_window_get_widget(board->time_dia_window));

	/* Free */
	free(board);
}


static GtkWidget *vi_x86_core_board_get_widget(struct vi_x86_core_board_t *board)
{
	return board->widget;
}


static void vi_x86_core_board_refresh(struct vi_x86_core_board_t *board)
{
	struct hash_table_t *context_table;
	struct vi_x86_context_t *context;

	char *context_name;

	long long cycle;

	/* Go to cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Empty context list */
	while (vi_list_count(board->context_list))
		free(vi_list_remove_at(board->context_list, 0));

	/* Refresh context list */
	context_table = board->core->context_table;
	HASH_TABLE_FOR_EACH(context_table, context_name, context)
	{
		/* Add to list */
		context_name = xstrdup(context_name);
		vi_list_add(board->context_list, context_name);
	}
	vi_list_refresh(board->context_list);

	/* LED */
	GdkColor color;
	if (vi_list_count(board->context_list))
		gdk_color_parse("red", &color);
	else
		gdk_color_parse("green", &color);
	vi_led_set_color(board->led, &color);

	/* Refresh time diagram */
	if (board->time_dia_window)
	{
		vi_x86_time_dia_window_go_to_cycle(board->time_dia_window, cycle);
		vi_x86_time_dia_window_refresh(board->time_dia_window);
	}
}




/*
 * CPU Panel
 */

#define VI_X86_PANEL_PADDING	5

#define VI_X86_PANEL_WIDTH	100
#define VI_X86_PANEL_HEIGHT	100


struct vi_x86_panel_t
{
	GtkWidget *widget;

	struct list_t *board_list;
};


static void vi_x86_panel_destroy(GtkWidget *widget,
	struct vi_x86_panel_t *panel)
{
	list_free(panel->board_list);
	vi_x86_panel_free(panel);
}


struct vi_x86_panel_t *vi_x86_panel_create(void)
{
	struct vi_x86_core_t *core;
	struct vi_x86_panel_t *panel;

	struct vi_x86_core_board_t *board;

	int layout_width;
	int layout_height;

	int i;

	/* Initialize */
	panel = xcalloc(1, sizeof(struct vi_x86_panel_t));
	panel->board_list = list_create();

	/* Frame */
	GtkWidget *frame;
	frame = gtk_frame_new("x86 CPU");

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
	layout_width = VI_X86_CORE_BOARD_PADDING;
	layout_height = VI_X86_CORE_BOARD_HEIGHT + 2 * VI_X86_CORE_BOARD_PADDING;
	LIST_FOR_EACH(vi_x86_cpu->core_list, i)
	{
		/* Create board and insert */
		core = list_get(vi_x86_cpu->core_list, i);
		board = vi_x86_core_board_create(core);
		list_add(panel->board_list, board);
		gtk_layout_put(GTK_LAYOUT(layout), vi_x86_core_board_get_widget(board),
			layout_width, VI_X86_CORE_BOARD_PADDING);

		/* Accumulate width */
		layout_width += VI_X86_CORE_BOARD_WIDTH + VI_X86_CORE_BOARD_PADDING;
	}

	/* Sizes of scrolled window */
	gtk_widget_set_size_request(layout, layout_width, layout_height);
	gtk_widget_set_size_request(scrolled_window, VI_X86_CORE_BOARD_WIDTH * 3 / 2, layout_height);

	/* Widget */
	panel->widget = frame;
	g_signal_connect(G_OBJECT(panel->widget), "destroy",
		G_CALLBACK(vi_x86_panel_destroy), panel);

	/* Return */
	return panel;
}


void vi_x86_panel_free(struct vi_x86_panel_t *panel)
{
	free(panel);
}


void vi_x86_panel_refresh(struct vi_x86_panel_t *panel)
{
	struct vi_x86_core_board_t *board;

	int board_id;

	/* Refresh all module boards */
	LIST_FOR_EACH(panel->board_list, board_id)
	{
		board = list_get(panel->board_list, board_id);
		vi_x86_core_board_refresh(board);
	}
}


GtkWidget *vi_x86_panel_get_widget(struct vi_x86_panel_t *panel)
{
	return panel->widget;
}
