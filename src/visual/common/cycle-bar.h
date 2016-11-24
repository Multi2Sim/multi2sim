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

#ifndef VISUAL_COMMON_CYCLE_BAR_H
#define VISUAL_COMMON_CYCLE_BAR_H

#include <gtk/gtk.h>


typedef void (*vi_cycle_bar_refresh_func_t)(void *user_data, long long cycle);

void vi_cycle_bar_init(vi_cycle_bar_refresh_func_t refresh_func, void *user_data);
void vi_cycle_bar_done(void);

void vi_cycle_bar_set_refresh_func(vi_cycle_bar_refresh_func_t refresh_func, void *user_data);

GtkWidget *vi_cycle_bar_get_widget(void);
long long vi_cycle_bar_get_cycle(void);


#endif

