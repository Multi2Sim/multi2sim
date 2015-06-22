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

#include <cctype>

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Arch.h"
#include "Disassembler.h"


namespace comm
{

Disassembler::Disassembler(const std::string &name) :
		name(name)
{
	// Register disassembler in architecture pool
	ArchPool *arch_pool = ArchPool::getInstance();
	arch_pool->RegisterDisassembler(name, this);
}


bool Disassembler::isToken(const std::string &fmt, const std::string &token,
		int &length)
{
	// Token is not prefix
	length = 0;
	if (!misc::StringPrefix(fmt, token))
		return false;
	
	// Token is not end of word
	if (fmt.size() > token.size() && isalnum(fmt[token.size()]))
		return false;

	// Token found
	length = token.size();
	return true;
}


}  // namespace comm

