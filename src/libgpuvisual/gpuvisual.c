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

#include <debug.h>
#include <gpuvisual-private.h>
#include <gdk/gdkkeysyms.h>
#include <config.h>
#include <sys/stat.h>
#include <pwd.h>

static struct vgpu_t *vgpu;

GtkWidget *main_window;
GtkWidget *cycle_entry;
GtkWidget *cycle_scale;

struct list_layout_t *pending_work_group_list_layout;
struct list_layout_t *finished_work_group_list_layout;

static char m2s_icon_path[MAX_STRING_SIZE];

static char vgpu_config_path[MAX_STRING_SIZE];
static char vgpu_config_file_name[MAX_STRING_SIZE];
static struct config_t *vgpu_config_file;

static int vgpu_status_panel_width;
static int vgpu_status_panel_height;


static void vgpu_config_load(void)
{
        struct passwd *p;
        char *section;

	/* Get file name */
        p = getpwuid(getuid());
        if (!p)
        	fatal("%s: 'getpwnam' failed", __FUNCTION__);
        snprintf(vgpu_config_path, sizeof(vgpu_config_path), "%s/%s", p->pw_dir, ".multi2sim");
        snprintf(vgpu_config_file_name, sizeof(vgpu_config_file_name), "%s/%s", vgpu_config_path, "vgpu-visual.ini");
	mkdir(vgpu_config_path, 0770);

	/* Load configuration */
	vgpu_config_file = config_create(vgpu_config_file_name);
	if (!config_load(vgpu_config_file))
		return;

	/* Main window configuration */
	section = "MainWindow";
	config_read_int(vgpu_config_file, section, "StatusPanelWidth", vgpu_status_panel_width);
	config_read_int(vgpu_config_file, section, "StatusPanelHeight", vgpu_status_panel_height);

}


static void vgpu_config_store(void)
{
	config_save(vgpu_config_file);
	config_free(vgpu_config_file);
}


#if 0
static void show_kernel_source(struct vgpu_t *vgpu)
{
	char *line;
	int size;
	int i;
	char *buffer, *cursor;

	/* Get total size */
	size = 0;
	for (i = 0; i < list_count(vgpu->kernel_source_strings); i++)
	{
		line = list_get(vgpu->kernel_source_strings, i);
		size += strlen(line) + 1;
	}

	/* Allocate space */
	buffer = calloc(1, size);
	cursor = buffer;

	/* Copy lines */
	for (i = 0; i < list_count(vgpu->kernel_source_strings); i++)
	{
		line = list_get(vgpu->kernel_source_strings, i);
		strcpy(cursor, line);
		cursor += strlen(cursor);
	}

	/* Show popup */
	info_popup_show(buffer);
	free(buffer);
}
#endif


static void cycle_update(int cycle)
{
	struct vgpu_compute_unit_t *compute_unit;
	char text[100];
	int i, err;

	/* Adjust range */
	cycle = MAX(cycle, 0);
	cycle = MIN(cycle, vgpu->max_cycles);

	/* Write text in entry widget */
	snprintf(text, sizeof text, "%d", cycle);
	gtk_entry_set_text(GTK_ENTRY(cycle_entry), text);
	/* FIXME: select text in entry here */

	/* Set range widget position */
	gtk_range_set_value(GTK_RANGE(cycle_scale), cycle);

	/* Go to desired cycle */
	err = vgpu_trace_cycle(vgpu, cycle);
	if (err)
		g_print("trace file error: %s\n", vgpu_trace_err);

	/* Update status panel */
	gtk_label_set_markup(GTK_LABEL(vgpu->status_label), vgpu->status_text);

	/* Scroll timing diagrams to new cycle */
	for (i = 0; i < vgpu->num_compute_units; i++)
	{
		compute_unit = list_get(vgpu->compute_unit_list, i);
		timing_dia_window_goto(compute_unit, cycle);
	}

	/* Update vgpu status */
	vgpu_widget_refresh(vgpu);
	list_layout_refresh(pending_work_group_list_layout);
	list_layout_refresh(finished_work_group_list_layout);
}


static gboolean cycle_button_clicked_event(GtkWidget *button, int delta)
{
	const char *text;
	int cycle;

	/* Get new cycle */
	text = gtk_entry_get_text(GTK_ENTRY(cycle_entry));
	cycle = atoi(text) + delta;
	cycle_update(cycle);

	return FALSE;
}


static gboolean cycle_entry_key_press_event(GtkWidget *entry, GdkEventKey *event, struct vgpu_t *vgpu)
{
	const char *text;
	int cycle;

	if (event->type == GDK_KEY_PRESS && (event->keyval == 0xff8d || event->keyval == 0xff0d)) {
		text = gtk_entry_get_text(GTK_ENTRY(cycle_entry));
		cycle = atoi(text);
		cycle_update(cycle);
	}
	return FALSE;
}


static gboolean cycle_scale_change_value_event(GtkWidget *widget, GtkScrollType scroll, double value, struct vgpu_t *vgpu)
{
	/* Set new cycle */
	cycle_update(value);
	return FALSE;
}


static gboolean main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	/* Free vgpu */
	vgpu_free(vgpu);

	/* End program */
	gtk_main_quit();

	/* Destroy window */
	return TRUE;
}


static void main_window_show()
{
	/* Main window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(main_window), "Multi2Sim GPU Visualization Tool");
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
	g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(main_window_delete_event), G_OBJECT(main_window));

	/* Icon */
	GdkPixbuf *pixbuf;
	pixbuf = gdk_pixbuf_new_from_file(m2s_icon_path, NULL);
	if (pixbuf)
		gtk_window_set_icon(GTK_WINDOW(main_window), pixbuf);

	/* White */
	GdkColor color;
	gdk_color_parse("white", &color);

	/* Table */
	GtkWidget *table;
	table = gtk_table_new(6, 1, FALSE);

	/* Label for pending work-groups */
	GtkWidget *label1;
	GtkWidget *halign1;
	label1 = gtk_label_new("Pending Work-Groups");
	halign1 = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign1), label1);
	gtk_table_attach(GTK_TABLE(table), halign1, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	/* Pending work-groups */
	GtkWidget *pending_frame;
	pending_frame = gtk_frame_new(NULL);
	pending_work_group_list_layout = list_layout_new(main_window, "Work-group list",
		vgpu->pending_work_group_list, 12, work_group_get_name, work_group_info_popup);
	gtk_widget_set_size_request(pending_frame, 100, 50);
	gtk_container_add(GTK_CONTAINER(pending_frame), pending_work_group_list_layout->layout);
	gtk_widget_modify_bg(pending_work_group_list_layout->layout, GTK_STATE_NORMAL, &color);
	gtk_table_attach(GTK_TABLE(table), pending_frame, 0, 1, 1, 2, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
	gtk_table_set_row_spacing(GTK_TABLE(table), 1, 15);

	/* Label for finished work-groups */
	GtkWidget *label2;
	GtkWidget *halign2;
	label2 = gtk_label_new("Finished Work-Groups");
	halign2 = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign2), label2);
	gtk_table_attach(GTK_TABLE(table), halign2, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

	/* Finished work-groups */
	GtkWidget *finished_frame;
	finished_frame = gtk_frame_new(NULL);
	finished_work_group_list_layout = list_layout_new(main_window, "Work-group list",
		vgpu->finished_work_group_list, 12, work_group_get_name, work_group_info_popup);
	gtk_widget_set_size_request(finished_frame, 100, 50);
	gtk_container_add(GTK_CONTAINER(finished_frame), finished_work_group_list_layout->layout);
	gtk_widget_modify_bg(finished_work_group_list_layout->layout, GTK_STATE_NORMAL, &color);
	gtk_table_attach(GTK_TABLE(table), finished_frame, 0, 1, 3, 4, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);
	gtk_table_set_row_spacing(GTK_TABLE(table), 3, 15);

	/* Current cycle label */
	GtkWidget *label3;
	GtkWidget *halign3;
	label3 = gtk_label_new("Current Cycle");
	halign3 = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign3), label3);
	gtk_table_attach(GTK_TABLE(table), halign3, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);

	/* Current cycle frame */
	GtkWidget *cycle_frame;
	cycle_frame = gtk_frame_new(NULL);
	gtk_table_attach(GTK_TABLE(table), cycle_frame, 0, 1, 5, 6, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);

	/* Scale */
	cycle_scale = gtk_hscale_new_with_range(0, 100, 1);
	gtk_scale_set_draw_value(GTK_SCALE(cycle_scale), FALSE);
	gtk_range_set_range(GTK_RANGE(cycle_scale), 0, vgpu->max_cycles);
	g_signal_connect(G_OBJECT(cycle_scale), "change-value", G_CALLBACK(cycle_scale_change_value_event), vgpu);

	/* Under the scale */
	GtkWidget *button1;
	GtkWidget *button2;
	GtkWidget *button3;
	button1 = gtk_button_new_with_label("<<<");
	button2 = gtk_button_new_with_label("<<");
	button3 = gtk_button_new_with_label("<");
	GtkWidget *vsep1;
	vsep1 = gtk_vseparator_new();
	cycle_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(cycle_entry), "0");
	gtk_widget_set_size_request(cycle_entry, 70, -1);
	GtkWidget *button4;
	button4 = gtk_button_new_with_label("Go");
	GtkWidget *vsep2;
	vsep2 = gtk_vseparator_new();
	GtkWidget *button5;
	GtkWidget *button6;
	GtkWidget *button7;
	button5 = gtk_button_new_with_label(">");
	button6 = gtk_button_new_with_label(">>");
	button7 = gtk_button_new_with_label(">>>");
	GtkWidget *hbox1;
	hbox1 = gtk_hbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(hbox1), button1);
	gtk_container_add(GTK_CONTAINER(hbox1), button2);
	gtk_container_add(GTK_CONTAINER(hbox1), button3);
	gtk_container_add(GTK_CONTAINER(hbox1), vsep1);
	gtk_container_add(GTK_CONTAINER(hbox1), cycle_entry);
	gtk_container_add(GTK_CONTAINER(hbox1), button4);
	gtk_container_add(GTK_CONTAINER(hbox1), vsep2);
	gtk_container_add(GTK_CONTAINER(hbox1), button5);
	gtk_container_add(GTK_CONTAINER(hbox1), button6);
	gtk_container_add(GTK_CONTAINER(hbox1), button7);
	g_signal_connect(G_OBJECT(cycle_entry), "key-press-event", G_CALLBACK(cycle_entry_key_press_event), vgpu);
	g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(cycle_button_clicked_event), (gpointer) -100);
	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(cycle_button_clicked_event), (gpointer) -10);
	g_signal_connect(G_OBJECT(button3), "clicked", G_CALLBACK(cycle_button_clicked_event), (gpointer) -1);
	g_signal_connect(G_OBJECT(button4), "clicked", G_CALLBACK(cycle_button_clicked_event), (gpointer) 0);
	g_signal_connect(G_OBJECT(button5), "clicked", G_CALLBACK(cycle_button_clicked_event), (gpointer) 1);
	g_signal_connect(G_OBJECT(button6), "clicked", G_CALLBACK(cycle_button_clicked_event), (gpointer) 10);
	g_signal_connect(G_OBJECT(button7), "clicked", G_CALLBACK(cycle_button_clicked_event), (gpointer) 100);

	/* VBox for scale and hbox */
	GtkWidget *vbox3;
	vbox3 = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(vbox3), cycle_scale);
	gtk_container_add(GTK_CONTAINER(vbox3), hbox1);
	gtk_container_add(GTK_CONTAINER(cycle_frame), vbox3);

	/* vgpu label */
	GtkWidget *label4;
	GtkWidget *halign4;
	label4 = gtk_label_new("Compute Device");
	halign4 = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign4), label4);

	/* vgpu */
	GtkWidget *vgpu_frame;
	GtkWidget *vgpu_widget;
	vgpu_frame = gtk_frame_new(NULL);
	vgpu_widget = vgpu_widget_new(vgpu);
	gtk_container_add(GTK_CONTAINER(vgpu_frame), vgpu_widget);

	/* VBox containing halign4 and vgpu_frame */
	GtkWidget *vbox1;
	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), halign4, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), vgpu_frame, TRUE, TRUE, 0);

	/* Label for status panel */
	GtkWidget *label5;
	GtkWidget *halign5;
	label5 = gtk_label_new("Status");
	halign5 = gtk_alignment_new(0, 0, 0, 0);
	gtk_container_add(GTK_CONTAINER(halign5), label5);

	/* Status panel */
	GtkWidget *status_frame;
	GtkWidget *status_window;
	GtkWidget *status_label;
	status_window = gtk_scrolled_window_new(NULL, NULL);
	status_frame = gtk_frame_new(NULL);
	status_label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(status_label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(status_label), 0, 0);
	gtk_container_add(GTK_CONTAINER(status_frame), status_window);
	gtk_widget_set_size_request(status_frame, 400, 80);
	gtk_widget_modify_bg(status_window, GTK_STATE_NORMAL, &color);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(status_window), status_label);
	vgpu->status_label = status_label;

	/* Set label font attributes for status */
	PangoAttrList *attrs;
	attrs = pango_attr_list_new();
	PangoAttribute *size_attr = pango_attr_size_new_absolute(12 << 10);
	pango_attr_list_insert(attrs, size_attr);
	gtk_label_set_attributes(GTK_LABEL(status_label), attrs);

	/* VBox containing 'halign5' and 'status_frame' */
	GtkWidget *vbox2;
	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), halign5, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), status_frame, TRUE, TRUE, 0);

	/* Horizontal pane */
	GtkWidget *hpane;
	hpane = gtk_hpaned_new();
	gtk_paned_pack1(GTK_PANED(hpane), table, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(hpane), vbox1, TRUE, FALSE);

	/* 'vpane' containing 'hpane' and 'vbox2' */
	GtkWidget *vpane;
	vpane = gtk_vpaned_new();
	gtk_paned_pack1(GTK_PANED(vpane), hpane, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(vpane), vbox2, TRUE, TRUE);
	gtk_container_add(GTK_CONTAINER(main_window), vpane);

	/* Show window */
	gtk_widget_show_all(main_window);
	cycle_update(0);
}


void vgpu_run(char *file_name)
{
	char *img_folder = "images";

	/* Search for distribution files */
	m2s_dist_file("cf_engine.png", img_folder, img_folder, block_dia_cf_engine_image_path, MAX_STRING_SIZE);
	m2s_dist_file("alu_engine.png", img_folder, img_folder, block_dia_alu_engine_image_path, MAX_STRING_SIZE);
	m2s_dist_file("tex_engine.png", img_folder, img_folder, block_dia_tex_engine_image_path, MAX_STRING_SIZE);
	m2s_dist_file("m2s_icon.png", img_folder, img_folder, m2s_icon_path, MAX_STRING_SIZE);
	m2s_dist_file("close.png", img_folder, img_folder, img_close_path, sizeof img_close_path);
	m2s_dist_file("close-sel.png", img_folder, img_folder, img_close_sel_path, sizeof img_close_sel_path);

	/* Load configuration */
	vgpu_config_load();

	/* Create vgpu */
	vgpu = vgpu_create(file_name);
	if (!vgpu)
		fatal("Trace file error: %s\n", vgpu_trace_err);

	/* Initialize GTK */
	gtk_init(NULL, NULL);
	main_window_show();
	gtk_main();

	/* Save configuration */
	vgpu_config_store();
}

