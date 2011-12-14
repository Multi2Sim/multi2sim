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


/*
 * Variables
 */

/* Images */
char block_dia_cf_engine_image_path[MAX_STRING_SIZE];
char block_dia_alu_engine_image_path[MAX_STRING_SIZE];
char block_dia_tex_engine_image_path[MAX_STRING_SIZE];



/*
 * Private Functions
 */

static gboolean block_dia_window_delete_event(GtkWidget *widget, GdkEvent *event, struct vgpu_compute_unit_t *compute_unit)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(compute_unit->block_dia_toggle_button), FALSE);
	return TRUE;
}




/*
 * Public Functions
 */

void block_dia_window_refresh(struct vgpu_compute_unit_t *compute_unit)
{
	struct vgpu_t *gpu = compute_unit->gpu;
	struct list_t *uop_list;
	int i;

	/* Ignore if timing diagram is not being shown */
	if (!compute_unit->block_dia_active)
		return;

	/* Empty lists */
	list_clear(compute_unit->uop_list_cf_fetch);
	list_clear(compute_unit->uop_list_cf_decode);
	list_clear(compute_unit->uop_list_cf_execute);
	list_clear(compute_unit->uop_list_cf_complete);
	list_clear(compute_unit->uop_list_alu_fetch);
	list_clear(compute_unit->uop_list_alu_decode);
	list_clear(compute_unit->uop_list_alu_read);
	list_clear(compute_unit->uop_list_alu_execute);
	list_clear(compute_unit->uop_list_alu_write);
	list_clear(compute_unit->uop_list_tex_fetch);
	list_clear(compute_unit->uop_list_tex_decode);
	list_clear(compute_unit->uop_list_tex_read);
	list_clear(compute_unit->uop_list_tex_write);

	/* Fill lists */
	uop_list = compute_unit->uop_list;
	for (i = 0; i < list_count(uop_list); i++)
	{
		struct vgpu_uop_t *uop;
		uop = list_get(uop_list, i);
		if (uop->finished && uop->stage_cycle < gpu->cycle)
			continue;

		if (uop->engine == VGPU_ENGINE_CF) {
			if (uop->stage == VGPU_STAGE_FETCH)
				list_add(compute_unit->uop_list_cf_fetch, uop);
			else if (uop->stage == VGPU_STAGE_DECODE)
				list_add(compute_unit->uop_list_cf_decode, uop);
			else if (uop->stage == VGPU_STAGE_EXECUTE)
				list_add(compute_unit->uop_list_cf_execute, uop);
			else if (uop->stage == VGPU_STAGE_COMPLETE)
				list_add(compute_unit->uop_list_cf_complete, uop);
		} else if (uop->engine == VGPU_ENGINE_ALU) {
			if (uop->stage == VGPU_STAGE_FETCH)
				list_add(compute_unit->uop_list_alu_fetch, uop);
			else if (uop->stage == VGPU_STAGE_DECODE)
				list_add(compute_unit->uop_list_alu_decode, uop);
			else if (uop->stage == VGPU_STAGE_READ)
				list_add(compute_unit->uop_list_alu_read, uop);
			else if (uop->stage == VGPU_STAGE_EXECUTE)
				list_add(compute_unit->uop_list_alu_execute, uop);
			else if (uop->stage == VGPU_STAGE_WRITE)
				list_add(compute_unit->uop_list_alu_write, uop);
		} else if (uop->engine == VGPU_ENGINE_TEX) {
			if (uop->stage == VGPU_STAGE_FETCH)
				list_add(compute_unit->uop_list_tex_fetch, uop);
			else if (uop->stage == VGPU_STAGE_DECODE)
				list_add(compute_unit->uop_list_tex_decode, uop);
			else if (uop->stage == VGPU_STAGE_READ)
				list_add(compute_unit->uop_list_tex_read, uop);
			else if (uop->stage == VGPU_STAGE_WRITE)
				list_add(compute_unit->uop_list_tex_write, uop);
		}
	}

	/* Refresh list layouts */
	list_layout_refresh(compute_unit->list_layout_cf_fetch);
	list_layout_refresh(compute_unit->list_layout_cf_decode);
	list_layout_refresh(compute_unit->list_layout_cf_execute);
	list_layout_refresh(compute_unit->list_layout_cf_complete);
	list_layout_refresh(compute_unit->list_layout_alu_fetch);
	list_layout_refresh(compute_unit->list_layout_alu_decode);
	list_layout_refresh(compute_unit->list_layout_alu_read);
	list_layout_refresh(compute_unit->list_layout_alu_execute);
	list_layout_refresh(compute_unit->list_layout_alu_write);
	list_layout_refresh(compute_unit->list_layout_tex_fetch);
	list_layout_refresh(compute_unit->list_layout_tex_decode);
	list_layout_refresh(compute_unit->list_layout_tex_read);
	list_layout_refresh(compute_unit->list_layout_tex_write);
}


#define BLOCK_DIA_LAYOUT(LAYOUT, STG, X, Y, W, H, VGPU_ENGINE, VGPU_STG) { \
	GdkColor color; \
	char str[MAX_STRING_SIZE]; \
	snprintf(str, sizeof(str), "#%02x%02x%02x", \
		(int)(vgpu_stage_color[VGPU_ENGINE][VGPU_STG][0] * 0xff), \
		(int)(vgpu_stage_color[VGPU_ENGINE][VGPU_STG][1] * 0xff), \
		(int)(vgpu_stage_color[VGPU_ENGINE][VGPU_STG][2] * 0xff)); \
	gdk_color_parse(str, &color); \
	frame = gtk_frame_new(NULL); \
	compute_unit->uop_list_##STG = list_create(); \
	compute_unit->list_layout_##STG = list_layout_new(compute_unit->uop_list_##STG, 10, vgpu_uop_get_name, vgpu_uop_info_popup); \
	gtk_container_add(GTK_CONTAINER(frame), compute_unit->list_layout_##STG->layout); \
	gtk_widget_modify_bg(compute_unit->list_layout_##STG->layout, GTK_STATE_NORMAL, &color); \
	gtk_widget_set_size_request(frame, (W), (H)); \
	gtk_layout_put(GTK_LAYOUT(LAYOUT), frame, (X), (Y)); \
}


void block_dia_window_show(struct vgpu_compute_unit_t *compute_unit)
{
	char text[MAX_STRING_SIZE];
	GtkRequisition req;
	GtkWidget *frame;

	/* Activate */
	assert(!compute_unit->block_dia_active);
	compute_unit->block_dia_active = 1;

	/* Create window */
	GtkWidget *block_dia_window;
	block_dia_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	snprintf(text, sizeof(text), "Compute Unit %d - Block Diagram", compute_unit->id);
	gtk_window_set_title(GTK_WINDOW(block_dia_window), text);
	gtk_window_set_resizable(GTK_WINDOW(block_dia_window), FALSE);
	gtk_window_set_position(GTK_WINDOW(block_dia_window), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT(block_dia_window), "delete-event", G_CALLBACK(block_dia_window_delete_event), compute_unit);
	compute_unit->block_dia_window = block_dia_window;

	/* CF Engine */
	GtkWidget *cf_layout;
	GtkWidget *cf_image;
	cf_layout = gtk_layout_new(NULL, NULL);
	cf_image = gtk_image_new_from_file(block_dia_cf_engine_image_path);
	gtk_widget_size_request(cf_image, &req);
	gtk_layout_put(GTK_LAYOUT(cf_layout), cf_image, 0, 0);
	gtk_widget_set_size_request(cf_layout, req.width, req.height);

	/* CF Engine stages */
	BLOCK_DIA_LAYOUT(cf_layout, cf_fetch, 170, 57, 40, 49, VGPU_ENGINE_CF, VGPU_STAGE_FETCH);
	BLOCK_DIA_LAYOUT(cf_layout, cf_decode, 303, 57, 40, 49, VGPU_ENGINE_CF, VGPU_STAGE_DECODE);
	BLOCK_DIA_LAYOUT(cf_layout, cf_execute, 435, 57, 40, 49, VGPU_ENGINE_CF, VGPU_STAGE_EXECUTE);
	BLOCK_DIA_LAYOUT(cf_layout, cf_complete, 511, 57, 40, 49, VGPU_ENGINE_CF, VGPU_STAGE_COMPLETE);

	/* ALU Engine */
	GtkWidget *alu_layout;
	GtkWidget *alu_image;
	alu_layout = gtk_layout_new(NULL, NULL);
	alu_image = gtk_image_new_from_file(block_dia_alu_engine_image_path);
	gtk_widget_size_request(alu_image, &req);
	gtk_layout_put(GTK_LAYOUT(alu_layout), alu_image, 0, 0);
	gtk_widget_set_size_request(alu_layout, req.width, req.height);

	/* ALU Engine stages */
	BLOCK_DIA_LAYOUT(alu_layout, alu_fetch, 95, 104, 40, 49, VGPU_ENGINE_ALU, VGPU_STAGE_FETCH);
	BLOCK_DIA_LAYOUT(alu_layout, alu_decode, 246, 104, 40, 49, VGPU_ENGINE_ALU, VGPU_STAGE_DECODE);
	BLOCK_DIA_LAYOUT(alu_layout, alu_read, 397, 104, 40, 49, VGPU_ENGINE_ALU, VGPU_STAGE_READ);
	BLOCK_DIA_LAYOUT(alu_layout, alu_execute, 473, 95, 68, 68, VGPU_ENGINE_ALU, VGPU_STAGE_EXECUTE);
	BLOCK_DIA_LAYOUT(alu_layout, alu_write, 577, 104, 40, 49, VGPU_ENGINE_ALU, VGPU_STAGE_WRITE);

	/* TEX Layout and image */
	GtkWidget *tex_layout;
	GtkWidget *tex_image;
	tex_layout = gtk_layout_new(NULL, NULL);
	tex_image = gtk_image_new_from_file(block_dia_tex_engine_image_path);
	gtk_widget_size_request(tex_image, &req);
	gtk_layout_put(GTK_LAYOUT(tex_layout), tex_image, 0, 0);
	gtk_widget_set_size_request(tex_layout, req.width, req.height);

	/* TEX Engine stages */
	BLOCK_DIA_LAYOUT(tex_layout, tex_fetch, 142, 94, 40, 49, VGPU_ENGINE_TEX, VGPU_STAGE_FETCH);
	BLOCK_DIA_LAYOUT(tex_layout, tex_decode, 293, 94, 40, 49, VGPU_ENGINE_TEX, VGPU_STAGE_DECODE);
	BLOCK_DIA_LAYOUT(tex_layout, tex_read, 435, 94, 40, 49, VGPU_ENGINE_TEX, VGPU_STAGE_READ);
	BLOCK_DIA_LAYOUT(tex_layout, tex_write, 511, 94, 40, 49, VGPU_ENGINE_TEX, VGPU_STAGE_WRITE);

	/* Hbox */
	GtkWidget *vbox;
	vbox = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(vbox), cf_layout);
	gtk_container_add(GTK_CONTAINER(vbox), alu_layout);
	gtk_container_add(GTK_CONTAINER(vbox), tex_layout);
	gtk_container_add(GTK_CONTAINER(block_dia_window), vbox);

	/* Show */
	gtk_widget_show_all(block_dia_window);
	block_dia_window_refresh(compute_unit);
}


void block_dia_window_hide(struct vgpu_compute_unit_t *compute_unit)
{
	assert(compute_unit->block_dia_active);
	assert(compute_unit->block_dia_window);
	gtk_widget_destroy(compute_unit->block_dia_window);
	compute_unit->block_dia_window = NULL;
	compute_unit->block_dia_active = 0;

	/* Free uop lists */
	list_free(compute_unit->uop_list_cf_fetch);
	list_free(compute_unit->uop_list_cf_decode);
	list_free(compute_unit->uop_list_cf_execute);
	list_free(compute_unit->uop_list_cf_complete);
	list_free(compute_unit->uop_list_alu_fetch);
	list_free(compute_unit->uop_list_alu_decode);
	list_free(compute_unit->uop_list_alu_read);
	list_free(compute_unit->uop_list_alu_execute);
	list_free(compute_unit->uop_list_alu_write);
	list_free(compute_unit->uop_list_tex_fetch);
	list_free(compute_unit->uop_list_tex_decode);
	list_free(compute_unit->uop_list_tex_read);
	list_free(compute_unit->uop_list_tex_write);
}
