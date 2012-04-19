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

#include <visual-evergreen.h>



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

	GtkWidget *timing_toggle_button;
	GtkWidget *block_toggle_button;

	struct vi_evg_compute_unit_t *compute_unit;
	struct vi_led_t *led;
};


/* Forward declarations */

static struct vi_cu_board_t *vi_cu_board_create(struct vi_evg_compute_unit_t *compute_unit);
static void vi_cu_board_free(struct vi_cu_board_t *board);

static void vi_cu_board_refresh(struct vi_cu_board_t *board);


/* Functions */

static void vi_cu_board_destroy(GtkWidget *widget, struct vi_cu_board_t *board)
{
	vi_cu_board_free(board);
}


static struct vi_cu_board_t *vi_cu_board_create(struct vi_evg_compute_unit_t *compute_unit)
{
	struct vi_cu_board_t *board;

	char str[MAX_STRING_SIZE];

	/* Allocate */
	board = calloc(1, sizeof(struct vi_cu_board_t));
	if (!board)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	board->compute_unit = compute_unit;

	/* Frame */
	GtkWidget *frame = gtk_frame_new(NULL);
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), frame);
	gtk_widget_set_size_request(frame, VI_CU_BOARD_WIDTH, VI_CU_BOARD_HEIGHT);

	/* Vertical box */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* Name */
	snprintf(str, sizeof str, "<b>%s</b>", compute_unit->name);
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
	board->led = led;

	/* Timing Diagram Toggle button */
	GtkWidget *timing_toggle_button = gtk_toggle_button_new_with_label("T");
	gtk_box_pack_start(GTK_BOX(hbox), timing_toggle_button, TRUE, TRUE, 0);
	/*g_signal_connect(G_OBJECT(timing_toggle_button), "toggled",
		G_CALLBACK(vi_evg_board_timing_toggle_button_toggled), board);*/
	board->timing_toggle_button = timing_toggle_button;

	/* Block Diagram Toggle button */
	GtkWidget *block_toggle_button = gtk_toggle_button_new_with_label("B");
	gtk_box_pack_start(GTK_BOX(hbox), block_toggle_button, TRUE, TRUE, 0);
	/*g_signal_connect(G_OBJECT(block_toggle_button), "toggled",
		G_CALLBACK(vi_evg_board_block_toggle_button_toggled), board);*/
	board->block_toggle_button = block_toggle_button;

	/* Main widget */
	board->widget = event_box;
	g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_cu_board_destroy), board);

	/* Return */
	return board;
}


static void vi_cu_board_free(struct vi_cu_board_t *board)
{
	/* Free */
	free(board);
}


static GtkWidget *vi_cu_board_get_widget(struct vi_cu_board_t *board)
{
	return board->widget;
}


static void vi_cu_board_refresh(struct vi_cu_board_t *board)
{
}




/*
 * GPU Panel
 */

#define VI_EVG_PANEL_PADDING	5

#define VI_EVG_PANEL_WIDTH	100
#define VI_EVG_PANEL_HEIGHT	100


struct vi_evg_panel_t
{
	GtkWidget *widget;

	struct list_t *board_list;
};


static void vi_evg_panel_destroy(GtkWidget *widget,
	struct vi_evg_panel_t *panel)
{
	list_free(panel->board_list);
	vi_evg_panel_free(panel);
}


struct vi_evg_panel_t *vi_evg_panel_create(void)
{
	struct vi_evg_compute_unit_t *compute_unit;
	struct vi_evg_panel_t *panel;

	struct vi_cu_board_t *board;

	int layout_width;
	int layout_height;

	int i;

	/* Allocate */
	panel = calloc(1, sizeof(struct vi_evg_panel_t));
	if (!panel)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	panel->board_list = list_create();

	/* Frame */
	GtkWidget *frame;
	frame = gtk_frame_new("GPU");

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
	LIST_FOR_EACH(vi_evg_gpu->compute_unit_list, i)
	{
		/* Create board and insert */
		compute_unit = list_get(vi_evg_gpu->compute_unit_list, i);
		board = vi_cu_board_create(compute_unit);
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
		G_CALLBACK(vi_evg_panel_destroy), panel);

	/* Return */
	return panel;
}


void vi_evg_panel_free(struct vi_evg_panel_t *panel)
{
	free(panel);
}


void vi_evg_panel_refresh(struct vi_evg_panel_t *panel)
{
	struct vi_cu_board_t *board;

	long long cycle;

	int board_id;

	/* Go to cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Refresh all module boards */
	LIST_FOR_EACH(panel->board_list, board_id)
	{
		board = list_get(panel->board_list, board_id);
		vi_cu_board_refresh(board);
	}
}


GtkWidget *vi_evg_panel_get_widget(struct vi_evg_panel_t *panel)
{
	return panel->widget;
}
