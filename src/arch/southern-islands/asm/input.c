/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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
#include <runtime/include/GL/gl.h>

#include "input.h"

/*
 * Public Functions
 */

struct si_input_t *si_input_create()
{
	struct si_input_t *input;

	/* Allocate */
	input = xcalloc(1, sizeof(struct si_input_t));

	/* NOTE:  the data type input expected is configured by runtime call glVertexAttribPointer */

	/* Return */	
	return input;
}

void si_input_free(struct si_input_t *input)
{
	free(input);
}

void si_input_set_usage_index(struct si_input_t *input, unsigned int usage_index)
{
	if (input)
		input->usage_index = usage_index;
}

unsigned int si_input_get_type(unsigned int gl_data_type)
{
	switch(gl_data_type)
	{

	case GL_BYTE:
		return si_input_byte;
	case GL_UNSIGNED_BYTE:
		return si_input_ubyte;
	case GL_SHORT:
		return si_input_short;
	case GL_UNSIGNED_SHORT:
		return si_input_ushort;
	case GL_INT:
		return si_input_int;
	case GL_UNSIGNED_INT:
		return si_input_uint;
	case GL_HALF_FLOAT:
		return si_input_hfloat;
	case GL_FLOAT:
		return si_input_float;
	case GL_DOUBLE:
		return si_input_double;
	case GL_FIXED:
		return si_input_fixed;
	case GL_INT_2_10_10_10_REV:
		return si_input_int_2_10_10_10_rev;
	case GL_UNSIGNED_INT_2_10_10_10_REV:
		return si_input_uint_2_10_10_10_rev;
	default:
		fatal("Unrecognized GLenum type=%x",gl_data_type);
		break;
	}

	/* Return */
	return 0;
}

/* Infer inputs size from its data type */
int si_input_get_data_size(enum si_input_data_type_t data_type)
{
	switch (data_type)
	{
	case si_input_byte:
	case si_input_ubyte:

		return 1;

	case si_input_short:
	case si_input_ushort:

		return 2;

	case si_input_int:
	case si_input_uint:
	case si_input_hfloat:
	case si_input_float:
	case si_input_fixed:
	case si_input_int_2_10_10_10_rev:
	case si_input_uint_2_10_10_10_rev:

		return 4;

	case si_input_double:

		return 8;

	default:

		panic("%s: invalid data type (%d)",
				__FUNCTION__, data_type);
		return 0;
	}
}

