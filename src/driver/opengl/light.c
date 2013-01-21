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
#include <math.h>

#include "light.h"
#include "material.h"
#include "opengl.h"
#include "vector.h"
#include "vertex.h"


struct opengl_light_t *opengl_light_create()
{
	struct opengl_light_t *lgt;

	lgt = xcalloc(1, sizeof(struct opengl_light_t));

	/* Initialize */
	lgt->Ambient[R_COMP] = 0.0f;
	lgt->Ambient[G_COMP] = 0.0f;
	lgt->Ambient[B_COMP] = 0.0f;
	lgt->Ambient[A_COMP] = 1.0f;

	lgt->EyePosition[X_COMP] = 0.0f;
	lgt->EyePosition[Y_COMP] = 0.0f;
	lgt->EyePosition[Z_COMP] = 1.0f;
	lgt->EyePosition[W_COMP] = 0.0f;

	lgt->SpotDirection[X_COMP] = 0.0f;
	lgt->SpotDirection[Y_COMP] = 0.0f;
	lgt->SpotDirection[Z_COMP] = -1.0f;
	lgt->SpotDirection[W_COMP] = 0.0f;

	lgt->SpotExponent = 0;
	lgt->SpotCutoff = 180;
	lgt->ConstantAttenuation = 1.0f;
	lgt->LinearAttenuation = 0.0f;
	lgt->QuadraticAttenuation = 0.0f;

	return lgt;

}

void opengl_light_free(struct opengl_light_t *lgt)
{
	free(lgt);
}

struct opengl_light_model_t *opengl_light_model_create()
{
	struct opengl_light_model_t *lgt_mdl;

	/* Allocate */
	lgt_mdl = xcalloc(1, sizeof(struct opengl_light_model_t));

	/* Initialize */
	lgt_mdl->Ambient[R_COMP] = 0.2f;
	lgt_mdl->Ambient[G_COMP] = 0.2f;
	lgt_mdl->Ambient[B_COMP] = 0.2f;
	lgt_mdl->Ambient[A_COMP] = 1.0f;
	lgt_mdl->LocalViewer = GL_FALSE;
	lgt_mdl->TwoSide = GL_FALSE;

	/* Return */	
	return lgt_mdl;
}

void opengl_light_model_free(struct opengl_light_model_t *lgt_mdl)
{
	free(lgt_mdl);
}

struct opengl_light_attrib_t *opengl_light_attrib_create()
{
	int i;
	struct opengl_light_attrib_t *lgt_attrb;

	/* Allocate */
	lgt_attrb = xcalloc(1, sizeof(struct opengl_light_attrib_t));

	/* Initialize */
	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		lgt_attrb->Light[i] = opengl_light_create();		
	}
	
	/* Special setup for Light 0 */
	lgt_attrb->Light[0]->Diffuse[0] = 1.0f;
	lgt_attrb->Light[0]->Diffuse[1] = 1.0f;
	lgt_attrb->Light[0]->Diffuse[2] = 1.0f;
	lgt_attrb->Light[0]->Diffuse[3] = 1.0f;

	lgt_attrb->Light[0]->Specular[0]  = 1.0f;
	lgt_attrb->Light[0]->Specular[1]  = 1.0f;
	lgt_attrb->Light[0]->Specular[2]  = 1.0f;
	lgt_attrb->Light[0]->Specular[3]  = 1.0f;

	lgt_attrb->Model = opengl_light_model_create();
	lgt_attrb->Material = opengl_material_create();
	lgt_attrb->ShadeModel = GL_SMOOTH;

	/* Return */	
	return lgt_attrb;
}

void opengl_light_attrib_free(struct opengl_light_attrib_t *lgt_attrb)
{
	int i;

	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		opengl_light_free(lgt_attrb->Light[i]);		
	}
	opengl_light_model_free(lgt_attrb->Model);
	opengl_material_free(lgt_attrb->Material);

	free(lgt_attrb);
}

static GLfloat opengl_light_attenuation(GLfloat k0, GLfloat k1, GLfloat k2, const GLfloat *v, const GLfloat *ppli)
{
	GLfloat distance;
	GLfloat denominator;

	if (fabs(ppli[4] - 0.0f) < 0.0000001)
	{
		distance = sqrt((ppli[0] - v[0]) * (ppli[0] - v[0]) + (ppli[1] - v[1]) * (ppli[1] - v[1]) + (ppli[2] - v[2]) * (ppli[2] - v[2]));
		opengl_debug("\t\t\tDistance = %f\n", distance);
		denominator = k0 + k1 * distance + k2 * distance * distance;
		opengl_debug("\t\t\tDenominator = %f\n", denominator);
		if (distance != 0.0)
			return 1.0f / denominator;
		else
		{
			opengl_debug("\t\tWarning: Distance = 0!\n");
			return 0.0f;
		}
	}
	else
		return 1.0f;
}

static GLfloat opengl_direction_mul(GLfloat *d1, GLfloat *d2)
{
	GLfloat direction = DOT3(d1, d2);
	return( MAX2(direction, 0.0f));
}

static GLfloat opengl_light_spot(GLfloat *ppli, GLfloat *v, GLfloat *sdli, GLfloat srli, GLfloat crli)
{
	GLfloat spot_direction;
	GLfloat unit_ppliv[4];
	GLfloat unit_sdli[4];
	GLfloat cos_crli;
	GLfloat spot_factor;

	/* Get the unit vectors */
	opengl_vector_unit(unit_ppliv, ppli, v);
	COPY_3V(unit_sdli, sdli);
	NORMALIZE_3FV(unit_sdli);

	/* Calculate the direction */
	spot_direction = opengl_direction_mul(unit_ppliv, unit_sdli);
	cos_crli = cos(crli);

	if (fabs(crli - 180.0f) < 0.00000001f)
		return 1.0f;
	else
	{
		if (spot_direction >= cos_crli)
		{
			spot_factor = pow(spot_direction, srli);
			return spot_factor;
		}
		else
		{
			return 0.0f;
		}		
	}
}

static GLint opengl_light_fi(GLfloat *nrml, GLfloat *vct, GLfloat *ppli)
{
	GLfloat result;
	GLfloat unit_v_to_ppli[4];

	opengl_vector_unit(unit_v_to_ppli, vct, ppli);
	result = DOT3(nrml, unit_v_to_ppli);
	
	if (MAX2(result, 0.0f) != 0)
		return 1;
	else
		return 0;
}

static void opengl_light_hi(GLfloat *rslt, GLfloat *vct, GLfloat *ppli, GLfloat *pe, GLboolean vbs)
{
	GLfloat unit_v_to_ppli[4];
	GLfloat unit_v_to_pe[4];
	GLfloat unit[4] = { 0.0f, 0.0f, 1.0f, 0.0f};
	GLfloat result[4];

	opengl_vector_unit(unit_v_to_ppli, vct, ppli);
	opengl_vector_unit(unit_v_to_pe, vct, pe);

	if (vbs == GL_TRUE)
		ADD_4V(result, unit_v_to_ppli, unit_v_to_pe);
	else
		ADD_4V(result, unit_v_to_ppli, unit);

	COPY_4V(rslt, result);
}

void opengl_light_apply_all(struct opengl_vertex_t *vtx, struct opengl_light_attrib_t *lgt_attrb)
{
	int i;
 	struct opengl_light_t *lgt;
 	struct opengl_material_t *mtrl;
 	struct opengl_light_model_t *mdl;

 	GLfloat color_final[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	GLfloat color_const[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	GLfloat color_accum[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	GLfloat *acm;
	GLfloat *dcm;
	GLfloat *scm;
	GLfloat *ecm;
	GLfloat srm;
	GLfloat *acs;
	GLfloat atti;
	GLfloat spoti;
	GLfloat atti_mul_spoti;
	GLfloat *acli;
	GLfloat acm_mul_acli[4];
	GLfloat *dcli;
	GLfloat dcm_mul_dcli[4];
	GLfloat unit_v_to_ppli[4];
	GLfloat *normal;
	GLfloat dcm_mul_dcli_factor;
	GLfloat fi;
	GLfloat hi[4];
	GLfloat pe[4] = { 0.0f, 0.0f, 0.0f, 1.0f};
	GLboolean vbs;
	GLfloat direction;
	GLfloat scm_mul_scli[4];
	GLfloat scm_mul_scli_factor;
	GLfloat *scli;

	/* Apply lights */
	/* Constant part comes from material itself */
	/* Get Materal and Model information */
	mtrl = lgt_attrb->Material;
	mdl = lgt_attrb->Model;
	ecm = mtrl->Attrib[MAT_ATTRIB_FRONT_EMISSION];
	acm = mtrl->Attrib[MAT_ATTRIB_FRONT_AMBIENT];
	dcm = mtrl->Attrib[MAT_ATTRIB_FRONT_DIFFUSE];
	scm = mtrl->Attrib[MAT_ATTRIB_FRONT_SPECULAR];
	srm = mtrl->Attrib[MAT_ATTRIB_FRONT_SHININESS][0];
	acs = mdl->Ambient;
	normal = vtx->normal;

	/* color_const = ecm + acm * acs */
	ACC_SCALE_4V(color_const, acm, acs);
	ACC_4V(color_const, ecm);

	opengl_debug("\t\tMaterial Emission = [");
	for (i = 0; i < 4; ++i)
		opengl_debug("%f ", ecm[i]);
	opengl_debug("]\n");

	opengl_debug("\t\tMaterial Ambient = [");
	for (i = 0; i < 4; ++i)
		opengl_debug("%f ", acm[i]);
	opengl_debug("]\n");

	opengl_debug("\t\tModel Ambient of scene = [");
	for (i = 0; i < 4; ++i)
		opengl_debug("%f ", acs[i]);
	opengl_debug("]\n");

	opengl_debug("\t\tColor const = [");
	for (i = 0; i < 4; ++i)
		opengl_debug("%f ", color_const[i]);
	opengl_debug("]\n");

	/* Accumulation part comes from 8 lights */
	for (i = 0; i < MAX_LIGHTS; ++i)
	{
		lgt = lgt_attrb->Light[i];
		vbs = lgt_attrb->Model->LocalViewer;
		GLfloat intermidiate_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		if (lgt->Enabled)
		{
			/* Calculate the 1st part in lighting equation */
			opengl_debug("\t\tLight position= [");
			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", lgt->EyePosition[i]);
			opengl_debug("]\n");

			atti = opengl_light_attenuation(lgt->ConstantAttenuation, lgt->LinearAttenuation, lgt->QuadraticAttenuation, vtx->pos, lgt->EyePosition);
			opengl_debug("\t\tAtti = %f\n", atti);
			spoti = opengl_light_spot(lgt->EyePosition, vtx->pos, lgt->SpotDirection, lgt->SpotExponent, lgt->SpotCutoff);
			opengl_debug("\t\tSpoti = %f\n", spoti);
			atti_mul_spoti = atti * spoti;
			opengl_debug("\t\tAtti * Spoti = %f\n", atti_mul_spoti);

			/* Calculate the 2nd part in lighting equation */
			acli = lgt->Ambient;
			SCALE_4V(acm_mul_acli, acm, acli);
			COPY_4V(intermidiate_color, acm_mul_acli);
			opengl_debug("\t\tAcm * Acli = [");
			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", acm_mul_acli[i]);
			opengl_debug("]\n");

			/* Calculate the 3rd part in lighting equation */
			dcli = lgt->Diffuse;
			opengl_debug("\t\tDcli = [");
			SCALE_4V(dcm_mul_dcli, dcm, dcli);

			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", dcli[i]);
			opengl_debug("]\n");
			opengl_debug("\t\tDcm = [");
			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", dcm[i]);
			opengl_debug("]\n");				
			opengl_debug("\t\tDcm * Dcli = [");
			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", dcm_mul_dcli[i]);
			opengl_debug("]\n");

			opengl_vector_unit(unit_v_to_ppli, vtx->pos, lgt->EyePosition);
			opengl_debug("\t\tV -> Ppli = [");
			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", unit_v_to_ppli[i]);
			opengl_debug("]\n");

			dcm_mul_dcli_factor = DOT4(normal, unit_v_to_ppli);
			opengl_debug("\t\tNormal = [");
			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", normal[i]);
			opengl_debug("]\n");

			opengl_debug("\t\tFactor = %f\n" , dcm_mul_dcli_factor);

			SELF_SCALE_SCALAR_4V(dcm_mul_dcli, dcm_mul_dcli_factor);
			ACC_4V(intermidiate_color, dcm_mul_dcli);

			/* Calculate the 4th part in lighting equation */
			scli = lgt->Specular;
			SCALE_4V(scm_mul_scli, scm, scli);
			opengl_debug("\t\tScm * Scli = [");
			for (i = 0; i < 4; ++i)
				opengl_debug("%f ", dcm_mul_dcli[i]);
			opengl_debug("]\n");			
			fi = opengl_light_fi(normal, vtx->pos, lgt->EyePosition);
			opengl_light_hi(hi, vtx->pos, lgt->EyePosition, pe, vbs);
			direction = opengl_direction_mul(vtx->normal, hi);
			scm_mul_scli_factor = pow(direction, srm);
			scm_mul_scli_factor *= fi;
			opengl_debug("\t\tScm * Scli factor = %f\n", scm_mul_scli_factor);
			SELF_SCALE_SCALAR_4V(scm_mul_scli, scm_mul_scli_factor);

			ACC_4V(intermidiate_color, scm_mul_scli);
			SELF_SCALE_SCALAR_4V(intermidiate_color, atti_mul_spoti);
		}
		ACC_4V(color_accum, intermidiate_color);
	}

	ADD_4V(color_final, color_const, color_accum);

	opengl_debug("\t\tFinal Vertex Color = [");
	for (int i = 0; i < 4; ++i)
		opengl_debug("%f ", color_final[i]);
	opengl_debug("]\n");

	for (i = 0; i < 4; ++i)
	{
		vtx->color[i] = 255*color_final[i];
	}
}


