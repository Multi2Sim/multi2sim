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

#include "material.h"
#include "opengl.h"


struct opengl_material_t *opengl_material_create()
{
	struct opengl_material_t *mtrl;

	/* Allocate */
	mtrl = xcalloc(1, sizeof(struct opengl_material_t));

	/* Initialize */
	/* The initial ambient reflectance is (0.2, 0.2, 0.2, 1.0) */
	mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][0] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][1] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][2] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][3] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_AMBIENT][0] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_BACK_AMBIENT][1] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_BACK_AMBIENT][2] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_BACK_AMBIENT][3] = 1.0f;

	/* The initial ambient reflectance is (0.2, 0.2, 0.2, 1.0) */
	mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][0] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][1] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][2] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][3] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_DIFFUSE][0] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_BACK_DIFFUSE][1] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_BACK_DIFFUSE][2] = 0.2f;
	mtrl->Attrib[MAT_ATTRIB_BACK_DIFFUSE][3] = 1.0f;

	/* The initial specular reflectance is (0, 0, 0, 1) */
	mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][1] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][2] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][3] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SPECULAR][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SPECULAR][1] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SPECULAR][2] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SPECULAR][3] = 1.0f;

	/* The initial emission intensity is (0, 0, 0, 1) */	
	mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][1] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][2] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][3] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_EMISSION][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_EMISSION][1] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_EMISSION][2] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_EMISSION][3] = 1.0f;

	/* The initial specular exponent is 0 */
	mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS][1] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS][2] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS][3] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SHININESS][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SHININESS][1] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SHININESS][2] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_SHININESS][3] = 0.0f;

	/* The initial index */
	mtrl->Attrib[MAT_ATTRIB_FRONT_INDEXES][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_INDEXES][1] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_INDEXES][2] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_FRONT_INDEXES][3] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_INDEXES][0] = 0.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_INDEXES][1] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_INDEXES][2] = 1.0f;
	mtrl->Attrib[MAT_ATTRIB_BACK_INDEXES][3] = 0.0f;

	/* Return */	
	return mtrl;
}

void opengl_material_free(struct opengl_material_t *mtrl)
{
	free(mtrl);
}

int opengl_material_setup(struct opengl_material_t * mtrl, GLenum face, GLenum pname, const GLfloat *params)
{
	opengl_debug("\t\tSetting Material...\n");

	if(face == GL_FRONT_AND_BACK || face == GL_FRONT || face == GL_BACK)
	{
		switch(pname)
		{
			case GL_AMBIENT:
			{
				if(face == GL_FRONT || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT], params, 4 * sizeof(GLfloat));
				if(face == GL_BACK || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_AMBIENT], params, 4 * sizeof(GLfloat));
				opengl_debug("\t\tMaterial ambient = [%f, %f, %f, %f]\n",
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][0], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][1], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][2], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][3] );
				break;
			}
			case GL_DIFFUSE:
			{
				if(face == GL_FRONT || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE], params, 4 * sizeof(GLfloat));
				if(face == GL_BACK || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_DIFFUSE], params, 4 * sizeof(GLfloat));
				opengl_debug("\t\tMaterial diffuse = [%f, %f, %f, %f]\n",
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][0], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][1], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][2], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][3] );			
				break;
			}
			case GL_SPECULAR:
			{
				if(face == GL_FRONT || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR], params, 4 * sizeof(GLfloat));
				if(face == GL_BACK || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_SPECULAR], params, 4 * sizeof(GLfloat));
				opengl_debug("\t\tMaterial specular = [%f, %f, %f, %f]\n",
								mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][0], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][1], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][2], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR][3] );
				break;
			}
			case GL_EMISSION:
			{
				if(face == GL_FRONT || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION], params, 4 * sizeof(GLfloat));
				if(face == GL_BACK || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_EMISSION], params, 4 * sizeof(GLfloat));
				opengl_debug("\t\tMaterial emission = [%f, %f, %f, %f]\n",
								mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][0], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][1], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][2], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION][3] );
				break;
			}
			case GL_SHININESS:
			{
				if(face == GL_FRONT || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS], params, 1 * sizeof(GLfloat));
				if(face == GL_BACK || face == GL_FRONT_AND_BACK)
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_SHININESS], params, 1 * sizeof(GLfloat));
				break;
			}
			case GL_AMBIENT_AND_DIFFUSE:
			{
				if(face == GL_FRONT || face == GL_FRONT_AND_BACK)
				{
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT], params, 4 * sizeof(GLfloat));
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE], params, 4 * sizeof(GLfloat));	
					opengl_debug("\t\tMaterial ambient = [%f, %f, %f, %f]\n",
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][0], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][1], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][2], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT][3] );
					opengl_debug("\t\tMaterial diffuse = [%f, %f, %f, %f]\n",
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][0], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][1], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][2], 
								mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE][3] );
				}
				if(face == GL_BACK || face == GL_FRONT_AND_BACK)
				{
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_DIFFUSE], params, 4 * sizeof(GLfloat));
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_AMBIENT], params, 4 * sizeof(GLfloat));
				}
				break;		
			}
			case GL_COLOR_INDEX:
			{
				if(face == GL_FRONT || face == GL_FRONT_AND_BACK)
				{
					memcpy(mtrl->Attrib[MAT_ATTRIB_FRONT_INDEXES], params, 3 * sizeof(GLfloat));
				}
				if(face == GL_BACK || face == GL_FRONT_AND_BACK)
				{
					memcpy(mtrl->Attrib[MAT_ATTRIB_BACK_INDEXES], params, 3 * sizeof(GLfloat));
				}
				break;					
			}
			default:
				break;
		}

		if(mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS][0] > 128 || mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS][0] < 0)
			return GL_INVALID_VALUE;
		else 
			return 0;		
	}
	else
		return GL_INVALID_VALUE;

}
