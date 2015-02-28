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

#include "Type.h"d#include "declarator-list.h"
#include "Declarator-list.h"

using namespace std;
using namespace cl2llvm;

void DeclaratorList::Merge(DeclaratorList& decl_list)
{
	if (decl_list.getTypeSpec() != NULL)
		this->type_spec = decl_list.getTypeSpec();
	if (decl_list.getAccessQual() != NULL)
		this->access_qual = decl_list.getAccessQual();
	if (decl_list.getKernelT() != NULL)
		this->kernel_t = decl_list.getKernelT();
	if (decl_list.getInlineT() != NULL)
		this->inline_t = decl_list.getInlineT();
	if (decl_list.getScSpec() != NULL)
		this->sc_spec = decl_list.getScSpec();	
	if (decl_list.getAddrQual() != 0)
		this->addr_qual = decl_list.getAddrQual();	
	if (decl_list.getTypeQual() != NULL)
		this->type_qual = decl_list.getTypeQual();
}
