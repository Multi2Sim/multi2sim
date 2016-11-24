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


#ifndef VISUAL_SOUTHERN_ISLANDS_WORK_GROUP_H
#define VISUAL_SOUTHERN_ISLANDS_WORK_GROUP_H


struct vi_si_work_group_t
{
	char *name;

	int id;

	int work_item_id_first;
	int work_item_count;

	int wavefront_id_first;
	int wavefront_count;
};

struct vi_si_work_group_t *vi_si_work_group_create(char *name, int id, int work_item_id_first,
	int work_item_count, int wavefront_id_first, int wavefront_count);
void vi_si_work_group_free(struct vi_si_work_group_t *work_group);

void vi_si_work_group_get_name_short(char *work_group_name, char *buf, int size);
void vi_si_work_group_get_desc(char *work_group_name, char *buf, int size);

void vi_si_work_group_read_checkpoint(struct vi_si_work_group_t *work_group, FILE *f);
void vi_si_work_group_write_checkpoint(struct vi_si_work_group_t *work_group, FILE *f);


#endif

