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

#include "mem-system.h"
#include "mod.h"
#include "mod-access.h"
#include "mod-widget.h"
#include "panel.h"



/*
 * Module detailed view
 */

struct vi_mod_window_t
{
	GtkWidget *widget;

	/* Module that window shows */
	struct vi_mod_t *mod;

	/* Toggle button that causes activation of window */
	GtkWidget *parent_toggle_button;

	/* Module widget */
	struct vi_mod_widget_t *mod_widget;
};


static struct vi_mod_window_t *vi_mod_window_create(struct vi_mod_t *mod, GtkWidget *parent_toggle_button);
static void vi_mod_window_free(struct vi_mod_window_t *mod_window);


static void vi_mod_window_destroy(GtkWidget *widget, struct vi_mod_window_t *mod_window)
{
	vi_mod_window_free(mod_window);
}


static gboolean vi_mod_window_delete(GtkWidget *widget, GdkEvent *event, struct vi_mod_window_t *mod_window)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mod_window->parent_toggle_button), FALSE);
	return TRUE;
}


static struct vi_mod_window_t *vi_mod_window_create(struct vi_mod_t *mod, GtkWidget *parent_toggle_button)
{
	struct vi_mod_window_t *mod_window;

	char str[MAX_STRING_SIZE];

	/* Initialize */
	mod_window = xcalloc(1, sizeof(struct vi_mod_window_t));
	mod_window->mod = mod;
	mod_window->parent_toggle_button = parent_toggle_button;

	/* Main window */
	snprintf(str, sizeof str, "Module %s", mod->name);
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), str);

	/* Module widget */
	struct vi_mod_widget_t *mod_widget;
	mod_widget = vi_mod_widget_create(mod->name);
	gtk_container_add(GTK_CONTAINER(window), vi_mod_widget_get_widget(mod_widget));
	mod_window->mod_widget = mod_widget;

	/* Associate widget */
	mod_window->widget = window;
	gtk_widget_show_all(mod_window->widget);
	g_signal_connect(G_OBJECT(mod_window->widget), "destroy", G_CALLBACK(vi_mod_window_destroy), mod_window);
	g_signal_connect(G_OBJECT(mod_window->widget), "delete_event", G_CALLBACK(vi_mod_window_delete), mod_window);

	/* Return */
	return mod_window;
}


static void vi_mod_window_free(struct vi_mod_window_t *mod_window)
{
	free(mod_window);
}


static void vi_mod_window_refresh(struct vi_mod_window_t *mod_window)
{
	vi_mod_widget_refresh(mod_window->mod_widget);
}


static GtkWidget *vi_mod_window_get_widget(struct vi_mod_window_t *mod_window)
{
	return mod_window->widget;
}




/*
 * Memory Module Board
 */

#define VI_MOD_BOARD_PADDING		10
#define VI_MOD_BOARD_WIDTH		100
#define VI_MOD_BOARD_HEIGHT		100


struct vi_mod_board_t
{
	/* Main widget */
	GtkWidget *widget;

	/* Pop-up window showing detail */
	struct vi_mod_window_t *mod_window;

	/* Toggle button to activate pop-up window */
	GtkWidget *toggle_button;

	struct vi_mod_t *mod;
	struct vi_list_t *access_list;
	struct vi_led_t *led;
};


static struct vi_mod_board_t *vi_mod_board_create(struct vi_mod_t *mod);
static void vi_mod_board_free(struct vi_mod_board_t *board);

static void vi_mod_board_refresh(struct vi_mod_board_t *board);


static void vi_mod_board_destroy(GtkWidget *widget, struct vi_mod_board_t *board)
{
	vi_mod_board_free(board);
}


static gboolean vi_mod_board_toggle_button_toggled(GtkWidget *widget, struct vi_mod_board_t *board)
{
	struct vi_mod_window_t *mod_window;

	int active;

	/* Get button state */
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(board->toggle_button));

	/* Show */
	if (active && !board->mod_window)
	{
		mod_window = vi_mod_window_create(board->mod, board->toggle_button);
		board->mod_window = mod_window;
	}

	/* Hide */
	if (!active && board->mod_window)
	{
		gtk_widget_destroy(vi_mod_window_get_widget(board->mod_window));
		board->mod_window = NULL;
	}

	return FALSE;
}


static struct vi_mod_board_t *vi_mod_board_create(struct vi_mod_t *mod)
{
	struct vi_mod_board_t *board;

	char str[MAX_STRING_SIZE];

	/* Initialize */
	board = xcalloc(1, sizeof(struct vi_mod_board_t));
	board->mod = mod;

	/* Frame */
	GtkWidget *frame = gtk_frame_new(NULL);
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), frame);
	gtk_widget_set_size_request(frame, VI_MOD_BOARD_WIDTH, VI_MOD_BOARD_HEIGHT);

	/* Vertical box */
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* Name */
	snprintf(str, sizeof str, "<b>%s</b>", mod->name);
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

	/* Toggle button */
	GtkWidget *toggle_button = gtk_toggle_button_new_with_label("Detail");
	gtk_box_pack_start(GTK_BOX(hbox), toggle_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(toggle_button), "toggled",
		G_CALLBACK(vi_mod_board_toggle_button_toggled), board);
	board->toggle_button = toggle_button;

	/* Access list */
	struct vi_list_t *access_list = vi_list_create("Access list", 10, 10, (vi_list_get_elem_name_func_t)
		vi_mod_access_get_name_short, (vi_list_get_elem_desc_func_t) vi_mod_access_get_desc);
	gtk_box_pack_start(GTK_BOX(vbox), vi_list_get_widget(access_list), TRUE, TRUE, 0);
	board->access_list = access_list;

	/* Main widget */
	board->widget = event_box;
	g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_mod_board_destroy), board);

	/* Return */
	return board;
}


static void vi_mod_board_free(struct vi_mod_board_t *board)
{
	/* Destroy pop-up window */
	if (board->mod_window)
		gtk_widget_destroy(vi_mod_window_get_widget(board->mod_window));

	/* Free access list */
	while (vi_list_count(board->access_list))
		free(vi_list_remove_at(board->access_list, 0));
	vi_list_free(board->access_list);

	/* Free */
	free(board);
}


static void vi_mod_board_refresh(struct vi_mod_board_t *board)
{
	struct vi_mod_access_t *access;

	char *access_name;

	/* Empty access list */
	while (vi_list_count(board->access_list))
		free(vi_list_remove_at(board->access_list, 0));

	/* Refresh access list */
	HASH_TABLE_FOR_EACH(board->mod->access_table, access_name, access)
	{
		/* Add to list */
		access_name = xstrdup(access_name);
		vi_list_add(board->access_list, access_name);
	}
	vi_list_refresh(board->access_list);

	/* LED */
	GdkColor color;
	if (vi_list_count(board->access_list))
		gdk_color_parse("red", &color);
	else
		gdk_color_parse("green", &color);
	vi_led_set_color(board->led, &color);

	/* Refresh pop-up window */
	if (board->mod_window)
		vi_mod_window_refresh(board->mod_window);
}




/*
 * Memory Hierarchy Panel
 */

#define VI_MEM_PANEL_PADDING	5

#define VI_MEM_PANEL_WIDTH	100
#define VI_MEM_PANEL_HEIGHT	100


struct vi_mem_panel_t
{
	GtkWidget *widget;

	struct list_t *board_list;
};


static void vi_mem_panel_destroy(GtkWidget *widget,
	struct vi_mem_panel_t *panel)
{
	list_free(panel->board_list);
	vi_mem_panel_free(panel);
}


struct vi_mem_panel_t *vi_mem_panel_create(void)
{
	struct vi_mem_panel_t *panel;
	struct vi_mod_t *mod;

	struct list_t *mod_list;
	struct vi_mod_board_t *board;

	int level_id;
	int mod_id;

	int layout_width;
	int layout_height;

	/* Initialize */
	panel = xcalloc(1, sizeof(struct vi_mem_panel_t));
	panel->board_list = list_create();

	/* Layout */
	GtkWidget *layout;
	GdkColor color;
	layout = gtk_layout_new(NULL, NULL);
	gdk_color_parse("white", &color);
	gtk_widget_modify_bg(layout, GTK_STATE_NORMAL, &color);

	/* Frame */
	GtkWidget *frame;
	frame = gtk_frame_new("Memory Hierarchy");

	/* Scrolled window */
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), layout);
	gtk_widget_set_size_request(scrolled_window, VI_MOD_BOARD_WIDTH * 3 / 2, VI_MOD_BOARD_HEIGHT * 3 / 2);
	gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

	/* Insert module boards */
	layout_width = VI_MOD_BOARD_WIDTH;
	layout_height = VI_MOD_BOARD_HEIGHT;
	LIST_FOR_EACH(vi_mem_system->mod_level_list, level_id)
	{
		mod_list = list_get(vi_mem_system->mod_level_list, level_id);

		/* Modules */
		LIST_FOR_EACH(mod_list, mod_id)
		{
			int x;
			int y;

			/* Get module */
			mod = list_get(mod_list, mod_id);

			/* Get board position */
			x = VI_MOD_BOARD_PADDING + mod_id * (VI_MOD_BOARD_PADDING + VI_MOD_BOARD_WIDTH);
			y = VI_MOD_BOARD_PADDING + level_id * (VI_MOD_BOARD_PADDING + VI_MOD_BOARD_HEIGHT);

			/* Create board */
			board = vi_mod_board_create(mod);
			list_add(panel->board_list, board);
			gtk_layout_put(GTK_LAYOUT(layout), board->widget, x, y);

			/* Size of layout */
			layout_width = MAX(layout_width, x + VI_MOD_BOARD_WIDTH + VI_MOD_BOARD_PADDING);
			layout_height = MAX(layout_height, y + VI_MOD_BOARD_HEIGHT + VI_MOD_BOARD_PADDING);
		}
	}

	/* Set layout size */
	gtk_widget_set_size_request(layout, layout_width, layout_height);

	/* Assign panel widget */
	panel->widget = frame;
	g_signal_connect(G_OBJECT(panel->widget), "destroy",
		G_CALLBACK(vi_mem_panel_destroy), panel);

	/* Return */
	return panel;
}


void vi_mem_panel_free(struct vi_mem_panel_t *panel)
{
	free(panel);
}


void vi_mem_panel_refresh(struct vi_mem_panel_t *panel)
{
	struct vi_mod_board_t *board;

	long long cycle;

	int board_id;

	/* Go to cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Refresh all module boards */
	LIST_FOR_EACH(panel->board_list, board_id)
	{
		board = list_get(panel->board_list, board_id);
		vi_mod_board_refresh(board);
	}
}


GtkWidget *vi_mem_panel_get_widget(struct vi_mem_panel_t *panel)
{
	return panel->widget;
}
