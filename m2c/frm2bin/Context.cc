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

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Token.h"


using namespace Misc;

namespace frm2bin
{


/*
 * Class 'InstInfo'
 */

InstInfo::InstInfo(Fermi::InstInfo *info)
{
	/* Initialize */
	this->info = info;
	next = nullptr;
	name = info->name;

	/* Create list of tokens from format string */
	StringTokenize(info->fmt_str, str_tokens, ", ");
	assert(str_tokens.size());
	name = str_tokens[0];
	for (unsigned i = 1; i < str_tokens.size(); i++)
	{
		/* Get token type */
		bool error;
		TokenType type = (TokenType) StringMapStringCase(TokenTypeMap,
				str_tokens[i], error);
		if (error)
			panic("%s: invalid token string: %s",
					__FUNCTION__, str_tokens[i].c_str());

		/* Add token */
		tokens.emplace_back(new Token(type));
	}
}



/*
 * Class 'Context'
 */


/* Global context */
Context context;


Context::Context()
{
	/* Initialize hash table and list with instruction information. */
	for (int i = 0; i < Fermi::InstOpcodeCount; i++)
	{
		/* Instruction info from disassembler */
		Fermi::InstInfo *inst_info = as.GetInstInfo((Fermi::InstOpcode)i);
		if (!inst_info->name || !inst_info->fmt_str)
			continue;

		/* Create info and add to array */
		InstInfo *info = new InstInfo(inst_info);
		inst_info_array[i].reset(info);

		/* Insert instruction info structure into hash table. There could
		 * be already an instruction encoding with the same name but a
		 * different encoding. They all form a linked list. */
		InstInfo *prev_info = getInstInfo((const std::string)(inst_info->name));
		if (prev_info)
		{
			/* Non vop3 instructions are added first into list.
			 * Add vop3 version to end of list */
			info->InsertAfter(prev_info);

			/* Non vop3 instructions are added first but vop3 version
			 * is added to the front of list */
			//info->next = prev_info;
			//hash_table_set(si2bin_inst_info_table, info->name, info);
		}
		else
		{
			inst_info_table[info->getName()] = info;
		}
	}
}


