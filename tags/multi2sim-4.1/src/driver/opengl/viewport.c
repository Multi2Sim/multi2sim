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


#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "viewport.h"


struct opengl_viewport_attributes_t *opengl_viewport_create(void)
{
	int width;
	int height;
	
	/* Variables */
	struct opengl_viewport_attributes_t *vpt;

	/* Allocate */
	vpt = xcalloc(1, sizeof(struct opengl_viewport_attributes_t));
	if(!vpt)
		fatal("%s: out of memory", __FUNCTION__);


	/* Initialize */
	width = 0;  // FIXME
	height = 0;  // FIXME

	vpt->x = 0;
	vpt->y = 0;
	vpt->width = width;
	vpt->height = height;

	vpt->near = 0.0f;
	vpt->far = 1.0f;

	/* Return */	
	return vpt;
}

void opengl_viewport_free(struct opengl_viewport_attributes_t *vpt)
{
	free(vpt);
}
