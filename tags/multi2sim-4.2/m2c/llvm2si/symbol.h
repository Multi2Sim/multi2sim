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

#ifndef M2C_LLVM2SI_SYMBOL_H
#define M2C_LLVM2SI_SYMBOL_H

#include <stdio.h>

#include <lib/util/class.h>



/*
 * Class 'Llvm2siSymbol'
 */

extern struct str_map_t llvm2si_symbol_type_map;
enum llvm2si_symbol_type_t
{
	llvm2si_symbol_type_invalid = 0,
	llvm2si_symbol_vector_register,
	llvm2si_symbol_scalar_register
};


CLASS_BEGIN(Llvm2siSymbol, Object)

	char *name;
	enum llvm2si_symbol_type_t type;

	/* Register ID and number of registers */
	int reg;
	int count;

	/* Flag indicating whether the symbol contains a global memory
	 * address. */
	int address;

	/* If the symbol represents a global memory address (flag 'address'
	 * is set to 1, UAV identifier (0=uav10, 1=uav11, ...). */
	int uav_index;

CLASS_END(Llvm2siSymbol)


void Llvm2siSymbolCreateVReg(Llvm2siSymbol *self, char *name, int vreg);
void Llvm2siSymbolCreateSReg(Llvm2siSymbol *self, char *name, int sreg);

void Llvm2siSymbolCreateVRegSeries(Llvm2siSymbol *self, char *name,
		int vreg_lo, int vreg_hi);
void Llvm2siSymbolCreateSRegSeries(Llvm2siSymbol *self, char *name,
		int sreg_lo, int sreg_hi);

void Llvm2siSymbolDestroy(Llvm2siSymbol *self);

/* Virtual function from class 'Object' */
void Llvm2siSymbolDump(Object *self, FILE *f);

/* Set the symbol type to an address to global memory (flag 'address' is set to
 * 1) and associate it with a UAV as specified in 'uav_index' (0=uav10, 1=uav11,
 * ...). */
void Llvm2siSymbolSetUAVIndex(Llvm2siSymbol *symbol,
		int uav_index);


#endif

