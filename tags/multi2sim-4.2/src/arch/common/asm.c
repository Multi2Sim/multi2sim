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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ctype.h>

#include <lib/mhandle/mhandle.h>

#include "asm.h"



/*
 * Class 'Asm'
 */


CLASS_IMPLEMENTATION(Asm);


void AsmCreate(Asm *self)
{
}


void AsmDestroy(Asm *self)
{
}




/*
 * Non-Class Functions
 */

int asm_is_token(char *fmt, char *token, int *length_ptr)
{
	int length;
	int is_token;

	assert(token);
	assert(fmt);

	/* Check for token */
	length = strlen(token);
	is_token = !strncmp(fmt, token, length) &&
		!isalnum(fmt[length]);

	/* Return its length if found */
	if (is_token && length_ptr)
		*length_ptr = length;

	/* Result */
	return is_token;
}

