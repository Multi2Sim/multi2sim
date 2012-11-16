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

#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "opengl-light.h"


struct x86_opengl_light_t *x86_opengl_light_create()
{
	struct x86_opengl_light_t *lgt;

	/* Initialize */
	lgt = xcalloc(1, sizeof(struct x86_opengl_light_t));

	/* Return */
	return lgt;
}

void x86_opengl_light_free(struct x86_opengl_light_t *lgt)
{
	free(lgt);
}

struct x86_opengl_light_model_t *x86_opengl_light_model_create()
{
	struct x86_opengl_light_model_t *lgt_mdl;

	/* Initialize */
	lgt_mdl = xcalloc(1, sizeof(struct x86_opengl_light_model_t));

	/* Return */	
	return lgt_mdl;
}

void x86_opengl_light_model_free(struct x86_opengl_light_model_t *lgt_mdl)
{
	free(lgt_mdl);
}

struct x86_opengl_light_attrib_t *x86_opengl_light_attrib_create()
{
	int i;
	struct x86_opengl_light_attrib_t *lgt_attrb;

	/* Initialize */
	lgt_attrb = xcalloc(1, sizeof(struct x86_opengl_light_attrib_t));
	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		lgt_attrb->Light[i] = x86_opengl_light_create();		
	}

	lgt_attrb->Model = x86_opengl_light_model_create();

	lgt_attrb->ShadeModel = GL_SMOOTH;

	/* Return */	
	return lgt_attrb;
}

void x86_opengl_light_attrib_free(struct x86_opengl_light_attrib_t *lgt_attrb)
{
	int i;
	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		x86_opengl_light_free(lgt_attrb->Light[i]);		
	}

	x86_opengl_light_model_free(lgt_attrb->Model);

	free(lgt_attrb);

}
