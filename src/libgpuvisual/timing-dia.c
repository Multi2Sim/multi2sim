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


#include <gpuvisual-private.h>


static cairo_font_extents_t timing_dia_font_extents;
int timing_dia_col_width;
int timing_dia_row_height;


/* Set 'width' and 'height' (either can be NULL), and set values for other constants */
static void timing_dia_get_size(struct vgpu_compute_unit_t *compute_unit, int *width, int *height)
{
	GdkWindow *window;
	cairo_t *cr;

	int widget_width;
	int widget_height;

	widget_width = gtk_widget_get_allocated_width(compute_unit->timing_dia_area);
	widget_height = gtk_widget_get_allocated_height(compute_unit->timing_dia_area);

	window = gtk_widget_get_window(compute_unit->timing_dia_area);
	cr = gdk_cairo_create(window);
	cairo_font_extents(cr, &timing_dia_font_extents);
	timing_dia_col_width = timing_dia_font_extents.max_x_advance * 4;
	timing_dia_row_height = timing_dia_font_extents.ascent + timing_dia_font_extents.descent;

	if (height)
		*height = widget_height / timing_dia_row_height + 2;
	if (width)
		*width = widget_width / timing_dia_col_width + 2;

	cairo_destroy(cr);
}


static char *vgpu_stage_name[VGPU_STAGE_COUNT] = { "FE", "DEC", "RD", "EX", "WR", "CO" };

double vgpu_stage_color[VGPU_ENGINE_COUNT][VGPU_STAGE_COUNT][3] = {
	{
		{ 0.6, 1, 0.6 },  /* FE */
		{ 0.4, 1, 0.4 },  /* DEC */
		{ 0, 0, 0 },
		{ 0.2, 1, 0.2 },  /* EX */
		{ 0, 0, 0 },
		{ 0, 1, 0 }  /* CO */
	},
	{
		{ 1, 0.8, 0.8 },  /* FE */
		{ 1, 0.6, 0.6 },  /* DEC */
		{ 1, 0.4, 0.4 },  /* RD */
		{ 1, 0.2, 0.2 },  /* EX */
		{ 1, 0, 0 },  /* WR */
		{ 0, 0, 0 }
	},
	{
		{ 0.9, 0.9, 1 },  /* FE */
		{ 0.8, 0.8, 1 },  /* DEC */
		{ 0.7, 0.7, 1 },  /* RD */
		{ 0, 0, 0 },
		{ 0.6, 0.6, 1 },  /* WR */
		{ 0, 0, 0 }
	}
};


static void timing_dia_refresh(struct vgpu_compute_unit_t *compute_unit)
{
	struct vgpu_t *gpu = compute_unit->gpu;
	int vgpu_cycle;
	int i, j;
	int err;

	/* Free timing diagram */
	if (compute_unit->timing_dia)
		free(compute_unit->timing_dia);

	/* Free uops in instruction list */
	vgpu_uop_list_clear(compute_unit->timing_inst_uops);

	/* Get size of diagram */
	timing_dia_get_size(compute_unit, &compute_unit->timing_dia_width, &compute_unit->timing_dia_height);
	compute_unit->timing_dia_cycle_first = compute_unit->timing_dia_hscrollbar_value / timing_dia_col_width;
	compute_unit->timing_dia_uop_first = compute_unit->timing_dia_vscrollbar_value / timing_dia_row_height;

	/* Create timing diagram */
	vgpu_cycle = gpu->cycle;
	compute_unit->timing_dia = calloc(compute_unit->timing_dia_width * compute_unit->timing_dia_height, sizeof(struct timing_dia_entry_t));
	for (i = 0; i < compute_unit->timing_dia_width; i++)
	{
		struct vgpu_uop_t *uop_head;

		/* Go to cycle */
		err = vgpu_trace_cycle(gpu, compute_unit->timing_dia_cycle_first + i);
		if (err)
			break;

		/* Get uop at the head of the list */
		uop_head = list_get(compute_unit->uop_list, 0);
		if (!uop_head)
			continue;

		/* Fill cycle column */
		for (j = 0; j < compute_unit->timing_dia_height; j++)
		{
			struct vgpu_uop_t *uop, *dup_uop;
			struct timing_dia_entry_t *timing_dia_entry;

			/* Get associated uop and diagram entries */
			uop = list_get(compute_unit->uop_list, j - uop_head->id + compute_unit->timing_dia_uop_first);
			if (!uop)
				continue;
			assert(uop->engine >= 0 && uop->engine < VGPU_ENGINE_COUNT);
			assert(uop->stage >= 0 && uop->stage < VGPU_STAGE_COUNT);

			/* If uop is not present in instruction list, make a copy */
			dup_uop = list_get(compute_unit->timing_inst_uops, j);
			if (!dup_uop) {
				dup_uop = vgpu_uop_dup(uop);
				while (list_count(compute_unit->timing_inst_uops) <= j)
					list_add(compute_unit->timing_inst_uops, NULL);
				list_set(compute_unit->timing_inst_uops, j, dup_uop);
			}

			/* Timing diagram cell */
			if (uop->finished && gpu->cycle > uop->stage_cycle)
				continue;
			timing_dia_entry = &compute_unit->timing_dia[i * compute_unit->timing_dia_height + j];
			snprintf(timing_dia_entry->text, sizeof(timing_dia_entry->text), "%s", vgpu_stage_name[uop->stage]);
			timing_dia_entry->fill = TRUE;
			timing_dia_entry->fill_r = vgpu_stage_color[uop->engine][uop->stage][0];
			timing_dia_entry->fill_g = vgpu_stage_color[uop->engine][uop->stage][1];
			timing_dia_entry->fill_b = vgpu_stage_color[uop->engine][uop->stage][2];
		}
	}

	/* Restore original GPU cycle */
	vgpu_trace_cycle(gpu, vgpu_cycle);
}


/* Return true if the timing diagram needs to be refreshed due to a change in its size
 * or its scrolling properties. */
static int timing_dia_needs_refresh(struct vgpu_compute_unit_t *compute_unit)
{
	int timing_dia_height, timing_dia_width;
	int timing_dia_cycle_first, timing_dia_uop_first;

	/* Check if the size of the diagram has changed */
	timing_dia_get_size(compute_unit, &timing_dia_width, &timing_dia_height);
	if (timing_dia_height != compute_unit->timing_dia_height || timing_dia_width != compute_unit->timing_dia_width)
		return TRUE;

	/* Check if scrolling changed */
	timing_dia_cycle_first = compute_unit->timing_dia_hscrollbar_value / timing_dia_col_width;
	timing_dia_uop_first = compute_unit->timing_dia_vscrollbar_value / timing_dia_row_height;
	if (compute_unit->timing_dia_cycle_first != timing_dia_cycle_first
		|| compute_unit->timing_dia_uop_first != timing_dia_uop_first)
		return TRUE;

	/* No refresh needed */
	return FALSE;
}


static gboolean timing_inst_title_area_draw_event(GtkWidget *widget, cairo_t *cr, struct vgpu_compute_unit_t *compute_unit)
{
	GtkWidget *timing_inst_title_area = compute_unit->timing_inst_title_area;
	gtk_widget_set_size_request(timing_inst_title_area, -1, timing_dia_row_height);
	return FALSE;
}


static gboolean timing_inst_layout_clicked_event(GtkWidget *widget, GdkEventButton *event, struct vgpu_uop_t *uop)
{
	vgpu_uop_info_popup(uop);
	return FALSE;
}


static void gtk_container_destroy_children(GtkContainer *container)
{
	GList *children;
	for (children = gtk_container_get_children(container); children; children = children->next)
		gtk_widget_destroy(GTK_WIDGET(children->data));
}


static void timing_inst_layout_refresh(struct vgpu_compute_unit_t *compute_unit)
{
	struct vgpu_uop_t *uop;
	int x, y, top_y;
	int i;

	/* Check if diagram needs to get recalculated */
	if (timing_dia_needs_refresh(compute_unit))
		timing_dia_refresh(compute_unit);

	/* Remove all child widgets in 'timing_inst_layout' */
	gtk_container_destroy_children(GTK_CONTAINER(compute_unit->timing_inst_layout));

	/* White background */
	GdkColor color;
	gdk_color_parse("white", &color);

	/* Attributes for labels */
	PangoAttrList *attrs;
	PangoAttribute *size_attr;
	attrs = pango_attr_list_new();
	size_attr = pango_attr_size_new_absolute(13 << 10);
	pango_attr_list_insert(attrs, size_attr);

	/* Instruction list */
	top_y = -((int) compute_unit->timing_dia_vscrollbar_value % timing_dia_row_height);
	y = top_y;
	for (i = 0; i < compute_unit->timing_dia_height; i++)
	{
		char text[MAX_STRING_SIZE];

		GtkWidget *label;
		GtkWidget *event_box;
		GtkRequisition req;

		uop = list_get(compute_unit->timing_inst_uops, i);
		if (uop)
		{
			/* Create label + event box for 'I-x' */
			x = 0;
			snprintf(text, sizeof(text), "I-%d", uop->id);
			label = gtk_label_new(text);
			event_box = gtk_event_box_new();
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(compute_unit->timing_inst_layout), event_box, x, y);
			gtk_widget_set_size_request(event_box, 40, timing_dia_row_height);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);
			g_signal_connect(G_OBJECT(event_box), "enter-notify-event", G_CALLBACK(list_layout_label_enter_notify_event), NULL);
			g_signal_connect(G_OBJECT(event_box), "leave-notify-event", G_CALLBACK(list_layout_label_leave_notify_event), NULL);
			g_signal_connect(G_OBJECT(event_box), "button-press-event", G_CALLBACK(timing_inst_layout_clicked_event), uop);

			/* Create label + event box for instruction name */
			x = 42;
			label = gtk_label_new(NULL);
			event_box = gtk_event_box_new();
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
			vgpu_uop_get_markup(uop, text, sizeof(text));
			gtk_label_set_markup(GTK_LABEL(label), text);
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(compute_unit->timing_inst_layout), event_box, x, y);
			gtk_widget_size_request(label, &req);
			gtk_widget_set_size_request(event_box, req.width, timing_dia_row_height);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);
			gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

		}
		y += timing_dia_row_height;
	}
	gtk_widget_show_all(compute_unit->timing_inst_layout);
}


static gboolean timing_dia_title_area_draw_event(GtkWidget *widget, cairo_t *cr, struct vgpu_compute_unit_t *compute_unit)
{
	GtkWidget *timing_dia_title_area = compute_unit->timing_dia_title_area;
	GdkWindow *window;

	int i;
	int widget_width;
	int widget_height;

	/* Check if diagram needs to get recalculated */
	if (timing_dia_needs_refresh(compute_unit))
		timing_dia_refresh(compute_unit);
	
	/* Get widget size */
	widget_width = gtk_widget_get_allocated_width(timing_dia_title_area);
	widget_height = gtk_widget_get_allocated_height(timing_dia_title_area);

	/* Set the proper height for title */
	gtk_widget_set_size_request(timing_dia_title_area, -1, timing_dia_row_height);
	window = gtk_widget_get_window(timing_dia_title_area);
	cr = gdk_cairo_create(window);

	/* White background */
	cairo_rectangle(cr, 0, 0, widget_width, widget_height);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);

	/* Draw */
	cairo_set_line_width(cr, 1);
	for (i = 0; i < compute_unit->timing_dia_width; i++) {
		int x1, x2, y1, y2;
		char text[MAX_STRING_SIZE];
		cairo_text_extents_t text_extents;

		x1 = i * timing_dia_col_width - (int) compute_unit->timing_dia_hscrollbar_value % timing_dia_col_width;
		x2 = x1 + timing_dia_col_width;
		y1 = 0;
		y2 = timing_dia_row_height;

		cairo_set_source_rgb(cr, .8, .8, .8);
		cairo_rectangle(cr, x1, y1, timing_dia_col_width, timing_dia_row_height);
		cairo_stroke(cr);

		cairo_save(cr);
		cairo_rectangle(cr, x1 + 1, y1 + 1, timing_dia_col_width - 2, timing_dia_row_height - 2);
		cairo_clip(cr);

		/* Show text */
		snprintf(text, sizeof(text), "%d", compute_unit->timing_dia_cycle_first + i);
		cairo_text_extents(cr, text, &text_extents);
		cairo_move_to(cr, (x1 + x2) / 2 - text_extents.x_advance / 2, y2 - timing_dia_font_extents.descent);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_show_text(cr, text);
		void timing_widget_refresh(struct vgpu_compute_unit_t *compute_unit);
		cairo_restore(cr);
	}

	/* Destroy */
	cairo_destroy(cr);
	return FALSE;
}


static gboolean timing_dia_area_draw_event(GtkWidget *widget, cairo_t *cr, struct vgpu_compute_unit_t *compute_unit)
{
	struct vgpu_t *gpu = compute_unit->gpu;
	GtkWidget *timing_dia_area = compute_unit->timing_dia_area;
	GdkWindow *window;

	int i, j, top_y;
	int widget_width;
	int widget_height;

	/* Check if diagram needs to get recalculated */
	if (timing_dia_needs_refresh(compute_unit))
		timing_dia_refresh(compute_unit);

	/* Get widget size */
	widget_width = gtk_widget_get_allocated_width(timing_dia_area);
	widget_height = gtk_widget_get_allocated_height(timing_dia_area);

	/* Instructions */
	window = gtk_widget_get_window(timing_dia_area);
	cr = gdk_cairo_create(window);

	/* White background */
	cairo_rectangle(cr, 0, 0, widget_width, widget_height);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);

	/* Draw */
	cairo_set_line_width(cr, 1);
	top_y = timing_dia_row_height - ((int) compute_unit->timing_dia_vscrollbar_value % timing_dia_row_height);
	for (i = 0; i < compute_unit->timing_dia_width; i++) {
		for (j = 0; j < compute_unit->timing_dia_height; j++)
		{
			struct timing_dia_entry_t *timing_dia_entry;
			int x1, x2, y1, y2;
			char text[MAX_STRING_SIZE];
			cairo_text_extents_t text_extents;

			timing_dia_entry = &compute_unit->timing_dia[i * compute_unit->timing_dia_height + j];
			x1 = i * timing_dia_col_width - (int) compute_unit->timing_dia_hscrollbar_value % timing_dia_col_width;
			x2 = x1 + timing_dia_col_width;
			y1 = j * timing_dia_row_height - timing_dia_row_height + top_y;
			y2 = y1 + timing_dia_row_height;

			cairo_set_source_rgb(cr, .8, .8, .8);
			cairo_rectangle(cr, x1, y1, timing_dia_col_width, timing_dia_row_height);
			cairo_stroke(cr);

			/* Clip region */
			cairo_save(cr);
			cairo_rectangle(cr, x1 + 1, y1 + 1, timing_dia_col_width - 2, timing_dia_row_height - 2);
			cairo_clip(cr);

			/* Fill */
			if (timing_dia_entry->fill) {
				cairo_set_source_rgb(cr, timing_dia_entry->fill_r, timing_dia_entry->fill_g,
					timing_dia_entry->fill_b);
				cairo_rectangle(cr, x1, y1, timing_dia_col_width, timing_dia_row_height);
				cairo_fill(cr);
			}

			/* Show text */
			snprintf(text, sizeof(text), "%s", timing_dia_entry->text);
			cairo_text_extents(cr, text, &text_extents);
			cairo_move_to(cr, (x1 + x2) / 2 - text_extents.x_advance / 2, y2 - timing_dia_font_extents.descent);
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_show_text(cr, text);

			cairo_restore(cr);
		}
	}

	/* Adjust increments for scroll bars */
	gtk_range_set_range(GTK_RANGE(compute_unit->timing_dia_hscrollbar), 0,
		(double) gpu->max_cycles * timing_dia_col_width);
	gtk_range_set_range(GTK_RANGE(compute_unit->timing_dia_vscrollbar), 0,
		(double) compute_unit->max_uops * timing_dia_row_height);
	compute_unit->timing_dia_hscrollbar_incr_step = (double) timing_dia_col_width * 0.4;
	compute_unit->timing_dia_hscrollbar_incr_page = (double) widget_width * 0.9;
	gtk_range_set_increments(GTK_RANGE(compute_unit->timing_dia_hscrollbar),
		compute_unit->timing_dia_hscrollbar_incr_step,
		compute_unit->timing_dia_hscrollbar_incr_page);
	compute_unit->timing_dia_vscrollbar_incr_step = 10.0;
	compute_unit->timing_dia_vscrollbar_incr_page = (double) widget_height * 0.9;
	gtk_range_set_increments(GTK_RANGE(compute_unit->timing_dia_vscrollbar),
		compute_unit->timing_dia_vscrollbar_incr_step,
		compute_unit->timing_dia_vscrollbar_incr_page);

	/* Destroy */
	cairo_destroy(cr);
	return FALSE;
}


static gboolean timing_dia_area_scroll_event(GtkWidget *widget, GdkEventScroll *event, struct vgpu_compute_unit_t *compute_unit)
{
	if (event->direction == GDK_SCROLL_UP) {
		compute_unit->timing_dia_vscrollbar_value = MAX(compute_unit->timing_dia_vscrollbar_value -
			compute_unit->timing_dia_vscrollbar_incr_step, 0);
	} else {
		compute_unit->timing_dia_vscrollbar_value = MIN(compute_unit->timing_dia_vscrollbar_value +
			compute_unit->timing_dia_vscrollbar_incr_step, (double) compute_unit->max_uops * timing_dia_row_height);
	}
	gtk_range_set_value(GTK_RANGE(compute_unit->timing_dia_vscrollbar),
		compute_unit->timing_dia_vscrollbar_value);
	timing_dia_window_refresh(compute_unit);
	return FALSE;
}


static gboolean timing_dia_scroll_event(GtkWidget *range, GtkScrollType scroll, gdouble value, struct vgpu_compute_unit_t *compute_unit)
{
	struct vgpu_t *gpu = compute_unit->gpu;

	if (range == compute_unit->timing_dia_hscrollbar) {
		value = MAX(value, 0);
		value = MIN(value, (double) gpu->max_cycles * timing_dia_col_width);
		compute_unit->timing_dia_hscrollbar_value = value;
	} else {
		value = MAX(value, 0);
		value = MIN(value, (double) compute_unit->max_uops * timing_dia_row_height);
		compute_unit->timing_dia_vscrollbar_value = value;
	}
	timing_dia_window_refresh(compute_unit);
	return FALSE;
}


static gboolean timing_window_delete_event(GtkWidget *widget, GdkEvent *event, struct vgpu_compute_unit_t *compute_unit)
{
	/* Toggle button. The evevents that this triggers will take car of
	 * destroying the window and freeing structures. */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(compute_unit->timing_dia_toggle_button), FALSE);

	/* Don't destroy the window yet */
	return TRUE;
}




/*
 * Public Functions
 */

void timing_dia_window_refresh(struct vgpu_compute_unit_t *compute_unit)
{
	/* Ignore if timing diagram is not being shown */
	if (!compute_unit->timing_dia_active)
		return;

	/* Call all expose events */
	timing_inst_title_area_draw_event(NULL, NULL, compute_unit);
	timing_inst_layout_refresh(compute_unit);
	timing_dia_title_area_draw_event(NULL, NULL, compute_unit);
	timing_dia_area_draw_event(NULL, NULL, compute_unit);
}


void timing_dia_window_goto(struct vgpu_compute_unit_t *compute_unit, int cycle)
{
	struct vgpu_t *gpu = compute_unit->gpu;
	double value;

	/* Ignore if timing diagram is not being shown */
	if (!compute_unit->timing_dia_active)
		return;

	/* Go to cycle horizontally */
	cycle = MIN(MAX(cycle, 0), gpu->max_cycles);
	value = (double) cycle * timing_dia_col_width;
	compute_unit->timing_dia_hscrollbar_value = value;
	gtk_range_set_value(GTK_RANGE(compute_unit->timing_dia_hscrollbar), value);

	/* Go to uop vertically */
	assert(compute_unit->last_completed_uop_id <= compute_unit->max_uops);
	value = (double) compute_unit->last_completed_uop_id * timing_dia_row_height;
	compute_unit->timing_dia_vscrollbar_value = value;
	gtk_range_set_value(GTK_RANGE(compute_unit->timing_dia_vscrollbar), value);

	/* Refresh window */
	timing_dia_window_refresh(compute_unit);
}


void timing_dia_window_show(struct vgpu_compute_unit_t *compute_unit)
{
	struct vgpu_t *gpu = compute_unit->gpu;
	char text[MAX_STRING_SIZE];

	/* Activate */
	assert(!compute_unit->timing_dia_active);
	compute_unit->timing_dia_active = 1;

	/* Create lists */
	compute_unit->timing_inst_uops = list_create();

	/* Create window */
	GtkWidget *timing_window;
	timing_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(text, sizeof(text), "Compute Unit %d - Timing Diagram", compute_unit->id);
	gtk_window_set_title(GTK_WINDOW(timing_window), text);
	gtk_window_set_position(GTK_WINDOW(timing_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(timing_window), 700, 200);
	g_signal_connect(G_OBJECT(timing_window), "delete-event", G_CALLBACK(timing_window_delete_event), compute_unit);

	/* Instruction drawing area */
	GtkWidget *timing_inst_layout = gtk_layout_new(NULL, NULL);
	GtkWidget *timing_inst_title_area = gtk_drawing_area_new();
	GtkWidget *timing_inst_table = gtk_table_new(3, 1, FALSE);
	GtkWidget *timing_inst_frame = gtk_frame_new(NULL);
	gtk_widget_set_size_request(timing_inst_layout, 100, 200);
	gtk_table_attach(GTK_TABLE(timing_inst_table), timing_inst_title_area, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(timing_inst_table), gtk_hseparator_new(), 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(timing_inst_table), timing_inst_layout, 0, 1, 2, 3);
	gtk_container_add(GTK_CONTAINER(timing_inst_frame), timing_inst_table);
	g_signal_connect(G_OBJECT(timing_inst_title_area), "draw",
		G_CALLBACK(timing_inst_title_area_draw_event), compute_unit);

	/* White background for instruction layout */
	GdkColor color;
	gdk_color_parse("white", &color);
	gtk_widget_modify_bg(timing_inst_layout, GTK_STATE_NORMAL, &color);

	/* Diagram drawing area */
	GtkWidget *timing_dia_area = gtk_drawing_area_new();
	GtkWidget *timing_dia_title_area = gtk_drawing_area_new();
	GtkWidget *timing_dia_table = gtk_table_new(3, 1, FALSE);
	GtkWidget *timing_dia_frame = gtk_frame_new(NULL);
	gtk_table_attach(GTK_TABLE(timing_dia_table), timing_dia_title_area, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(timing_dia_table), gtk_hseparator_new(), 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(timing_dia_table), timing_dia_area, 0, 1, 2, 3);
	gtk_container_add(GTK_CONTAINER(timing_dia_frame), timing_dia_table);
	gtk_widget_set_size_request(timing_dia_area, 200, 200);
	gtk_widget_add_events(timing_dia_area, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(G_OBJECT(timing_dia_title_area), "draw",
		G_CALLBACK(timing_dia_title_area_draw_event), compute_unit);
	g_signal_connect(G_OBJECT(timing_dia_area), "draw",
		G_CALLBACK(timing_dia_area_draw_event), compute_unit);
	g_signal_connect(G_OBJECT(timing_dia_area), "scroll-event",
		G_CALLBACK(timing_dia_area_scroll_event), compute_unit);

	/* Horizontal pane */
	GtkWidget *hpane;
	hpane = gtk_hpaned_new();
	gtk_paned_pack1(GTK_PANED(hpane), timing_inst_frame, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(hpane), timing_dia_frame, TRUE, FALSE);

	/* Scrollbars */
	GtkWidget *hscrollbar = gtk_hscrollbar_new(NULL);
	GtkWidget *vscrollbar = gtk_vscrollbar_new(NULL);

	/* Table */
	GtkWidget *table;
	table = gtk_table_new(2, 2, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table), hpane, 0, 1, 0, 1);
	gtk_table_attach(GTK_TABLE(table), hscrollbar, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
	g_signal_connect(G_OBJECT(hscrollbar), "change-value", G_CALLBACK(timing_dia_scroll_event), compute_unit);
	g_signal_connect(G_OBJECT(vscrollbar), "change-value", G_CALLBACK(timing_dia_scroll_event), compute_unit);

	/* Frame */
	GtkWidget *frame;
	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(frame), table);
	gtk_container_add(GTK_CONTAINER(timing_window), frame);

	/* Save widgets and show */
	compute_unit->timing_inst_layout = timing_inst_layout;
	compute_unit->timing_dia_area = timing_dia_area;
	compute_unit->timing_inst_title_area = timing_inst_title_area;
	compute_unit->timing_dia_title_area = timing_dia_title_area;
	compute_unit->timing_dia_hscrollbar = hscrollbar;
	compute_unit->timing_dia_vscrollbar = vscrollbar;
	compute_unit->timing_window = timing_window;
	gtk_widget_show_all(timing_window);

	/* Go to current cycle */
	timing_dia_refresh(compute_unit);
	timing_dia_window_goto(compute_unit, gpu->cycle);
}


void timing_dia_window_hide(struct vgpu_compute_unit_t *compute_unit)
{
	assert(compute_unit->timing_inst_uops);
	assert(compute_unit->timing_dia_active);
	assert(compute_unit->timing_window);
	assert(compute_unit->timing_dia);

	/* Destroy window */
	gtk_widget_destroy(compute_unit->timing_window);

	/* Free structures */
	free(compute_unit->timing_dia);
	vgpu_uop_list_free(compute_unit->timing_inst_uops);
	compute_unit->timing_dia = NULL;
	compute_unit->timing_window = NULL;
	compute_unit->timing_inst_uops = NULL;
	compute_unit->timing_dia_active = 0;
}
