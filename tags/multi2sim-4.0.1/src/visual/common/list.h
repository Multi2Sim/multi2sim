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


#ifndef VISUAL_COMMON_LIST_H
#define VISUAL_COMMON_LIST_H

#include <gtk/gtk.h>

#include <lib/util/string.h>


extern char vi_list_image_close_path[MAX_PATH_SIZE];
extern char vi_list_image_close_sel_path[MAX_PATH_SIZE];

typedef void (*vi_list_get_elem_name_func_t)(void *elem, char *buf, int size);
typedef void (*vi_list_get_elem_desc_func_t)(void *elem, char *buf, int size);

#define VI_LIST_FOR_EACH(list, iter) \
	for ((iter) = 0; (iter) < vi_list_count((list)); (iter)++)

struct vi_list_t;

struct vi_list_t *vi_list_create(char *title, int width, int height,
	vi_list_get_elem_name_func_t get_elem_name,
	vi_list_get_elem_name_func_t get_elem_desc);
void vi_list_free(struct vi_list_t *list);

int vi_list_count(struct vi_list_t *list);
void vi_list_add(struct vi_list_t *list, void *elem);
void *vi_list_get(struct vi_list_t *list, int index);
void *vi_list_remove_at(struct vi_list_t *list, int index);

void vi_list_refresh(struct vi_list_t *list);

GtkWidget *vi_list_get_widget(struct vi_list_t *list);


#endif

