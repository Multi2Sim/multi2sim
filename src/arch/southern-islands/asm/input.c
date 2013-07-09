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

#include <unistd.h>
#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>

#include "input.h"

/*
 * Public Functions
 */

struct si_input_t *si_input_create(enum si_input_data_type_t type)
{
	struct si_input_t *input;

	/* Allocate */
	input = xcalloc(1, sizeof(struct si_input_t));

	/* Initialize */
	input->type = type;

	/* Return */	
	return input;
}
void si_input_free(struct si_input_t *input)
{
	free(input);
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

