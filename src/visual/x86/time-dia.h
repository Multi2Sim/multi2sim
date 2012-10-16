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

#ifndef VISUAL_X86_TIME_DIA_H
#define VISUAL_X86_TIME_DIA_H

#include <gtk/gtk.h>


struct vi_x86_time_dia_t;

struct vi_x86_core_t;
struct vi_x86_time_dia_t *vi_x86_time_dia_create(struct vi_x86_core_t *core);
void vi_x86_time_dia_free(struct vi_x86_time_dia_t *time_dia);

GtkWidget *vi_x86_time_dia_get_widget(struct vi_x86_time_dia_t *time_dia);

void vi_x86_time_dia_refresh(struct vi_x86_time_dia_t *time_dia);
void vi_x86_time_dia_go_to_cycle(struct vi_x86_time_dia_t *time_dia, long long cycle);


#endif

