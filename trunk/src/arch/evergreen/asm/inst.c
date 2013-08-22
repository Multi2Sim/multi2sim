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

#include "inst.h"


/*
 * Class 'EvgInst'
 */

struct str_map_t evg_inst_alu_map = {
	5, {
		{ "x", EvgInstAluX },
		{ "y", EvgInstAluY },
		{ "z", EvgInstAluZ },
		{ "w", EvgInstAluW },
		{ "t", EvgInstAluTrans }
	}
};


struct str_map_t evg_inst_alu_pv_map = {
	5, {
		{ "PV.x", EvgInstAluX },
		{ "PV.y", EvgInstAluY },
		{ "PV.z", EvgInstAluZ },
		{ "PV.w", EvgInstAluW },
		{ "PS", EvgInstAluTrans }
	}
};


void EvgInstCreate(EvgInst *self, EvgAsm *as)
{
	/* Initialize */
	self->as = as;
}


void EvgInstDestroy(EvgInst *self)
{
}
