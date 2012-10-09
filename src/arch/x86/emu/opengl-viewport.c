/*
 *  Multi2Sim
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

#include "opengl-viewport.h"

 
struct x86_opengl_viewport_attributes_t *x86_opengl_viewport_create(void)
{
	int width;
	int height;
	
	/* Variables */
	struct x86_opengl_viewport_attributes_t *vpt;

	/* Allocate */
	vpt = calloc(1, sizeof(struct x86_opengl_viewport_attributes_t));
	if(!vpt)
		fatal("%s: out of memory", __FUNCTION__);


	/* Initialize */
	width = 0;  // FIXME
	height = 0;  // FIXME

	vpt->x = 0;
	vpt->y = 0;
	vpt->width = width;
	vpt->height = height;

	/* Return */	
	return vpt;
}

void x86_opengl_viewport_free(struct x86_opengl_viewport_attributes_t *vpt)
{
	free(vpt);
}
