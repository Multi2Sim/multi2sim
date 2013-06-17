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

/* NOTE: update 'llvm2si_symbol_type_map' if this enumeration is extended. */
enum llvm2si_symbol_type_t
{
	llvm2si_symbol_type_invalid = 0,
	llvm2si_symbol_vector_register,
	llvm2si_symbol_scalar_register
};

struct llvm2si_symbol_t
{
	char *name;
	enum llvm2si_symbol_type_t type;
	int reg;

	/* Flag indicating whether the symbol contains a global memory
	 * address. */
	int address;

	/* If the symbol represents a global memory address (flag 'address'
	 * is set to 1, UAV identifier (0=uav10, 1=uav11, ...). */
	int uav_index;
};

struct llvm2si_symbol_t *llvm2si_symbol_create(char *name,
		enum llvm2si_symbol_type_t type, int reg);
void llvm2si_symbol_free(struct llvm2si_symbol_t *symbol);
void llvm2si_symbol_dump(struct llvm2si_symbol_t *symbol, FILE *f);

/* Set the symbol type to an address to global memory (flag 'address' is set to
 * 1) and associate it with a UAV as specified in 'uav_index' (0=uav10, 1=uav11,
 * ...). */
void llvm2si_symbol_set_uav_index(struct llvm2si_symbol_t *symbol,
		int uav_index);


#endif

