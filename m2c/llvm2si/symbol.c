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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>

#include "symbol.h"


/*
 * Class 'Llvm2siSymbol'
 */

CLASS_IMPLEMENTATION(Llvm2siSymbol);


struct str_map_t llvm2si_symbol_type_map =
{
	3, {
		{ "vreg", llvm2si_symbol_vector_register },
		{ "sreg", llvm2si_symbol_scalar_register }
	}
};


static void Llvm2siSymbolCreate(Llvm2siSymbol *self, char *name,
		enum llvm2si_symbol_type_t type, int reg, int count)
{
	/* Check valid name */
	if (!name || !*name)
		fatal("%s: empty symbol name", __FUNCTION__);

	/* Initialize */
	self->name = str_set(self->name, name);
	self->type = type;
	self->reg = reg;
	self->count = count;
}


void Llvm2siSymbolCreateVReg(Llvm2siSymbol *self, char *name, int vreg)
{
	Llvm2siSymbolCreate(self, name, llvm2si_symbol_vector_register,
			vreg, 1);
}


void Llvm2siSymbolCreateSReg(Llvm2siSymbol *self, char *name, int sreg)
{
	Llvm2siSymbolCreate(self, name, llvm2si_symbol_scalar_register,
			sreg, 1);
}


void Llvm2siSymbolCreateVRegSeries(Llvm2siSymbol *self, char *name,
		int vreg_lo, int vreg_hi)
{
	assert(vreg_hi > vreg_lo);
	Llvm2siSymbolCreate(self, name, llvm2si_symbol_vector_register,
			vreg_lo, vreg_hi - vreg_lo + 1);
}


void Llvm2siSymbolCreateSRegSeries(Llvm2siSymbol *self, char *name,
		int sreg_lo, int sreg_hi)
{
	assert(sreg_hi > sreg_lo);
	Llvm2siSymbolCreate(self, name, llvm2si_symbol_scalar_register,
			sreg_lo, sreg_hi - sreg_lo + 1);
}


void Llvm2siSymbolDestroy(Llvm2siSymbol *self)
{
	self->name = str_free(self->name);
}


void Llvm2siSymbolDump(Object *self, FILE *f)
{
	Llvm2siSymbol *symbol;

	/* Name, type, register */
	symbol = asLlvm2siSymbol(self);
	fprintf(f, "name='%s', type=%s, reg=%d", symbol->name,
			str_map_value(&llvm2si_symbol_type_map, symbol->type),
			symbol->reg);

	/* UAV index */
	if (symbol->address)
		fprintf(f, ", uav%d", symbol->uav_index + 10);
	
	/* End */
	fprintf(f, "\n");
}


void Llvm2siSymbolSetUAVIndex(Llvm2siSymbol *self, int uav_index)
{
	self->address = 1;
	self->uav_index = uav_index;
}
