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

#ifndef VISUAL_SOUTHERN_ISLANDS_PANEL_H
#define VISUAL_SOUTHERN_ISLANDS_PANEL_H

#include <gtk/gtk.h>


struct vi_si_panel_t;

struct vi_si_panel_t *vi_si_panel_create(void);
void vi_si_panel_free(struct vi_si_panel_t *panel);

void vi_si_panel_refresh(struct vi_si_panel_t *panel);

GtkWidget *vi_si_panel_get_widget(struct vi_si_panel_t *panel);


#endif

