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


#ifndef MEMVISUAL_PRIVATE_H
#define MEMVISUAL_PRIVATE_H

#include <memvisual.h>
#include <stdlib.h>
#include <gtk/gtk.h>



/*
 * Visual Memory Module
 */

struct vmod_t
{
	GtkWidget *layout;
};


struct vmod_t *vmod_create(void);
void vmod_free(struct vmod_t *vmod);




/*
 * Panel with memory modules
 */

struct vmod_panel_t
{
	GtkWidget *layout;
};

struct vmod_panel_t *vmod_panel_create(void);
void vmod_panel_free(struct vmod_panel_t *panel);


#endif

