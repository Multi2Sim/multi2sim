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

#include <assert.h>

#include <lib/class/list.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "inst-info.h"
#include "token.h"


/*
 * Class 'Si2binInstInfo'
 */

void Si2binInstInfoCreate(Si2binInstInfo *self, SIInstInfo *inst_info)
{
	Si2binToken *token;
	Si2binTokenType token_type;
	String *token_str;
	String *fmt_str;

	/* Initialize */
	self->inst_info = inst_info;

	/* Create list of tokens from format string */
	fmt_str = new(String, inst_info->fmt_str);
	self->str_token_list = StringTokenize(fmt_str, ", ");
	assert(self->str_token_list->count);
	delete(fmt_str);

	/* Create list of formal arguments */
	self->token_list = new(List);
	ListForEach(self->str_token_list, token_str, String)
	{
		/* Name */
		if (!self->str_token_list->current_index)
		{
			self->name = token_str;
			continue;
		}

		/* Add formal argument */
		int error = 0;
		token_type = str_map_string_case_err(&si2bin_token_map,
				token_str->text, &error);
		if (error)
			//panic("%s: invalid token string: %s", __FUNCTION__, token_str->text);
		token = new(Si2binToken, token_type);
		ListAdd(self->token_list, asObject(token));
	}
}


void Si2binInstInfoDestroy(Si2binInstInfo *self)
{
	ListDeleteObjects(self->str_token_list);
	ListDeleteObjects(self->token_list);
	delete(self->str_token_list);
	delete(self->token_list);
}
