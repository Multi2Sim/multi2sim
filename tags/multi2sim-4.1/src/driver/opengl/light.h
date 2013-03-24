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

#ifndef DRIVER_OPENGL_LIGHT_H
#define DRIVER_OPENGL_LIGHT_H


#include <GL/gl.h>

#define MAX_LIGHTS			8

#define LIGHT_SPOT			0x1
#define LIGHT_LOCAL_VIEWER	0x2
#define LIGHT_POSITIONAL		0x4
#define LIGHT_NEED_VERTICES	(LIGHT_POSITIONAL|LIGHT_LOCAL_VIEWER)

struct opengl_material_t;
struct opengl_vertex_t;

struct opengl_light_t
{
	// struct opengl_light_t *next;	/**< double linked list with sentinel */
	// struct opengl_light_t *prev;

	GLfloat Ambient[4];		/**< ambient color */
	GLfloat Diffuse[4];		/**< diffuse color */
	GLfloat Specular[4];		/**< specular color */
	GLfloat EyePosition[4];		/**< position in eye coordinates */
	GLfloat SpotDirection[4];	/**< spotlight direction in eye coordinates */
	GLfloat SpotExponent;
	GLfloat SpotCutoff;		/**< in degrees */
	GLfloat _CosCutoff;		/**< = MAX(0, cos(SpotCutoff)) */
	GLfloat ConstantAttenuation;
	GLfloat LinearAttenuation;
	GLfloat QuadraticAttenuation;
	GLboolean Enabled;		/**< On/off flag */

	/** 
	* \name Derived fields
	*/
	/*@{*/
	GLbitfield _Flags;		/**< Mask of LIGHT_x bits defined above */

	GLfloat _Position[4];		/**< position in eye/obj coordinates */
	GLfloat _VP_inf_norm[3];	/**< Norm direction to infinite light */
	GLfloat _h_inf_norm[3];	/**< Norm( _VP_inf_norm + <0,0,1> ) */
	GLfloat _NormSpotDirection[4]; /**< normalized spotlight direction */
	GLfloat _VP_inf_spot_attenuation;

	GLfloat _MatAmbient[2][3];	/**< material ambient * light ambient */
	GLfloat _MatDiffuse[2][3];	/**< material diffuse * light diffuse */
	GLfloat _MatSpecular[2][3];	/**< material spec * light specular */
	
};



/**
 * Light model state.
 */
struct opengl_light_model_t
{
	GLfloat Ambient[4];		/**< ambient color */
	GLboolean LocalViewer;	/**< Local (or infinite) view point? */
	GLboolean TwoSide;		/**< Two (or one) sided lighting? */
	GLenum ColorControl;		/**< either GL_SINGLE_COLOR
				 *    or GL_SEPARATE_SPECULAR_COLOR */
};

struct opengl_light_attrib_t
{
	struct opengl_light_t *Light[MAX_LIGHTS];	/**< Array of light sources */
	struct opengl_light_model_t *Model;		/**< Lighting model */

	/**
	* Front and back material values.
	* Note: must call FLUSH_VERTICES() before using.
	*/
	struct opengl_material_t *Material;

	GLboolean Enabled;			/**< Lighting enabled flag */
	GLenum ShadeModel;			/**< GL_FLAT or GL_SMOOTH */
	GLenum ProvokingVertex;              /**< GL_EXT_provoking_vertex */
	GLenum ColorMaterialFace;		/**< GL_FRONT, BACK or FRONT_AND_BACK */
	GLenum ColorMaterialMode;		/**< GL_AMBIENT, GL_DIFFUSE, etc */
	GLbitfield _ColorMaterialBitmask;	/**< bitmask formed from Face and Mode */
	GLboolean ColorMaterialEnabled;
	GLenum ClampVertexColor;             /**< GL_TRUE, GL_FALSE, GL_FIXED_ONLY */
	GLboolean _ClampVertexColor;

	// struct opengl_light_t EnabledList;         /**< List sentinel */

	/** 
	* Derived state for optimizations: 
	*/
	/*@{*/
	GLboolean _NeedEyeCoords;		
	GLboolean _NeedVertices;		/**< Use fast shader? */
	GLfloat _BaseColor[2][3];
	/*@}*/
};

struct opengl_light_t *opengl_light_create();
void opengl_light_free(struct opengl_light_t *lgt);

struct opengl_light_model_t *opengl_light_model_create();
void opengl_light_model_free(struct opengl_light_model_t *lgt_mdl);

struct opengl_light_attrib_t *opengl_light_attrib_create();
void opengl_light_attrib_free(struct opengl_light_attrib_t *lgh_attrb);

void opengl_light_apply_all(struct opengl_vertex_t *vtx, struct opengl_light_attrib_t *lgt_attrb);

#endif
