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

#include <visual-memory.h>





struct vi_led_t
{
	GtkWidget *widget;
};


static void vi_led_destroy(GtkWidget *widget, struct vi_led_t *led)
{
	vi_led_free(led);
}


static gboolean vi_led_draw(GtkWidget *widget, GdkEventConfigure *event, struct vi_led_t *led)
{
	GdkWindow *window;
	cairo_t *cr;

	int width;
	int height;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	window = gtk_widget_get_window(widget);
	cr = gdk_cairo_create(window);

	/* Color */
	if (1)
		cairo_set_source_rgb(cr, 1, 0, 0);
	else
		cairo_set_source_rgb(cr, 0, 1, 0);

	/* Circle */
	cairo_set_line_width(cr, 1);
	cairo_arc(cr, width / 2, height / 2.0, MIN(width, height) / 3.0, 0., 2 * M_PI);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, 0, 0, 0);

	/* Finish */
	cairo_stroke(cr);
	cairo_destroy(cr);
	return FALSE;
}


struct vi_led_t *vi_led_create(int radius)
{
	struct vi_led_t *led;

	/* Allocate */
	led = calloc(1, sizeof(struct vi_led_t));
	if (!led)
		fatal("%s: out of memory", __FUNCTION__);

	/* Drawing box */
	GtkWidget *drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, radius * 2, radius * 2);
	g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(vi_led_draw), led);

	/* Main widget */
	led->widget = drawing_area;
	g_signal_connect(G_OBJECT(led->widget), "destroy", G_CALLBACK(vi_led_destroy), led);

	/* Return */
	return led;
}


void vi_led_free(struct vi_led_t *led)
{
	free(led);
}


GtkWidget *vi_led_get_widget(struct vi_led_t *led)
{
	return led->widget;
}















/*
 * Memory Module Board
 */

#define VI_MOD_BOARD_PADDING		10
#define VI_MOD_BOARD_WIDTH		100
#define VI_MOD_BOARD_HEIGHT		100


struct vi_mod_board_t
{
	GtkWidget *widget;

	struct vi_mod_t *mod;
	struct vi_list_t *access_list;
};


static struct vi_mod_board_t *vi_mod_board_create(struct vi_mod_t *mod);
static void vi_mod_board_free(struct vi_mod_board_t *board);

static void vi_mod_board_refresh(struct vi_mod_board_t *board);


static void vi_mod_board_destroy(GtkWidget *widget, struct vi_mod_board_t *board)
{
	vi_mod_board_free(board);
}


static struct vi_mod_board_t *vi_mod_board_create(struct vi_mod_t *mod)
{
	struct vi_mod_board_t *board;

	char str[MAX_STRING_SIZE];

	/* Allocate */
	board = calloc(1, sizeof(struct vi_mod_board_t));
	if (!board)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	board->mod = mod;

	/* Frame */
	GtkWidget *frame = gtk_frame_new(NULL);
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), frame);
	gtk_widget_set_size_request(frame, VI_MOD_BOARD_WIDTH, VI_MOD_BOARD_HEIGHT);

	/* Vertical box */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* Name */
	snprintf(str, sizeof str, "<b>%s</b>", mod->name);
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), str);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);

	/* Horizontal box */
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	/* LED */
	struct vi_led_t *led = vi_led_create(13);
	gtk_box_pack_start(GTK_BOX(hbox), vi_led_get_widget(led), FALSE, TRUE, 0);

	/* Toggle button */
	GtkWidget *toggle = gtk_toggle_button_new_with_label("Detail");
	gtk_box_pack_start(GTK_BOX(hbox), toggle, TRUE, TRUE, 0);

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
		/* Duplicate name */
		access_name = strdup(access_name);
		if (!access_name)
			fatal("%s: out of memory", __FUNCTION__);

		/* Add to list */
		vi_list_add(board->access_list, access_name);
	}
	vi_list_refresh(board->access_list);
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

	/* Allocate */
	panel = calloc(1, sizeof(struct vi_mem_panel_t));
	if (!panel)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	panel->board_list = list_create();

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
	gtk_widget_set_size_request(scrolled_window, VI_MOD_BOARD_WIDTH * 3 / 2, VI_MOD_BOARD_HEIGHT * 3 / 2);

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
	panel->widget = scrolled_window;
	g_signal_connect(G_OBJECT(panel->widget), "destroy",
		G_CALLBACK(vi_mem_panel_destroy), panel);



#if 0
	struct vi_mod_t *mod;
	struct list_t *mod_list;

	int level_id;
	int mod_id;

	/* Initialize */
	panel->visual_mod_widget_list = list_create();

	/* Vertical box */
	GtkWidget *vbox;
	vbox = gtk_vbox_new(FALSE, 0);

	/* Access list */
	struct vi_list_t *access_list;
	access_list = vi_list_create("Access list", 200, 30,
		(vi_list_get_elem_name_func_t) vi_mod_access_get_name_long,
		(vi_list_get_elem_desc_func_t) vi_mod_access_get_desc);
	gtk_box_pack_start(GTK_BOX(vbox), vi_list_get_widget(access_list), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
	panel->access_list = access_list;

	/* Insert levels */
	LIST_FOR_EACH(vi_mem_system->mod_level_list, level_id)
	{
		mod_list = list_get(vi_mem_system->mod_level_list, level_id);

		/* Empty level */
		if (!list_count(mod_list))
			continue;

		/* Horizontal box for a new level */
		GtkWidget *hbox = gtk_hbox_new(0, VI_MEM_PANEL_PADDING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

		/* Modules */
		LIST_FOR_EACH(mod_list, mod_id)
		{
			/* Get module */
			mod = list_get(mod_list, mod_id);

			/* Create module widget */
			struct vi_mod_widget_t *visual_mod_widget;
			visual_mod_widget = vi_mod_widget_create(mod->name);
			list_add(panel->visual_mod_widget_list, visual_mod_widget);
			gtk_box_pack_start(GTK_BOX(hbox), vi_mod_widget_get_widget(visual_mod_widget),
				TRUE, TRUE, 0);

			/* Separator */
			if (mod_id < mod_list->count - 1)
				gtk_box_pack_start(GTK_BOX(hbox), gtk_vseparator_new(), FALSE, FALSE, 0);
		}

		/* Horizontal bar */
		if (level_id < vi_mem_system->mod_level_list->count - 1)
			gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
	}

	/* Assign panel widget */
	panel->widget = vbox;
	g_signal_connect(G_OBJECT(panel->widget), "destroy",
		G_CALLBACK(vi_mem_panel_destroy), panel);

#endif

	/* Return */
	return panel;
}


void vi_mem_panel_free(struct vi_mem_panel_t *panel)
{
#if 0
	/* Free access list */
	while (vi_list_count(panel->access_list))
		free(vi_list_remove_at(panel->access_list, 0));
	vi_list_free(panel->access_list);

	/* Free */
	list_free(panel->visual_mod_widget_list);
#endif
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

#if 0
	struct vi_mod_access_t *access;

	char *access_name;

	int i;

	long long cycle;

	/* Go to cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Empty access list */
	while (vi_list_count(panel->access_list))
		free(vi_list_remove_at(panel->access_list, 0));

	/* Refresh access list */
	HASH_TABLE_FOR_EACH(vi_mem_system->access_table, access_name, access)
	{
		/* Duplicate name */
		access_name = strdup(access_name);
		if (!access_name)
			fatal("%s: out of memory", __FUNCTION__);

		/* Add to list */
		vi_list_add(panel->access_list, access_name);
	}
	vi_list_refresh(panel->access_list);

	/* Module widgets */
	LIST_FOR_EACH(panel->visual_mod_widget_list, i)
		vi_mod_widget_refresh(list_get(panel->visual_mod_widget_list, i));
#endif
}


GtkWidget *vi_mem_panel_get_widget(struct vi_mem_panel_t *panel)
{
	return panel->widget;
}
