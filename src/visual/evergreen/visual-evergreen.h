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


#ifndef VISUAL_EVERGREEN_H
#define VISUAL_EVERGREEN_H


#include <visual-common.h>



/*
 * Panel
 */

struct vi_evg_panel_t;

struct vi_evg_panel_t *vi_evg_panel_create(void);
void vi_evg_panel_free(struct vi_evg_panel_t *panel);

void vi_evg_panel_refresh(struct vi_evg_panel_t *panel);

GtkWidget *vi_evg_panel_get_widget(struct vi_evg_panel_t *panel);




/*
 * Work-Group
 */

struct vi_evg_work_group_t
{
	int work_item_id_first;
	int work_item_count;

	int wavefront_id_first;
	int wavefront_count;
};

struct vi_evg_work_group_t *vi_evg_work_group_create(void);
void vi_evg_work_group_free(struct vi_evg_work_group_t *work_group);




/*
 * Compute Unit
 */

struct vi_evg_compute_unit_t
{
};

struct vi_evg_compute_unit_t *vi_evg_compute_unit_create(void);
void vi_evg_compute_unit_free(struct vi_evg_compute_unit_t *compute_unit);



/*
 * GPU
 */

struct vi_evg_gpu_t
{
	struct list_t *compute_unit_list;
	struct list_t *work_group_list;
};


void vi_evg_gpu_init(void);
void vi_evg_gpu_done(void);


#endif
