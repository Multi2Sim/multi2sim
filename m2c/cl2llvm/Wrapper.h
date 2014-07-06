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

#ifndef M2C_CL2LLVM_WRAPPER_H
#define M2C_CL2LLVM_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif


struct Cl2llvmContext;

struct Cl2llvmContext *Cl2llvmContextCreate();
void Cl2llvmContextDestroy(struct Cl2llvmContext *context);

void Cl2llvmContextParse(struct Cl2llvmContext *context,
		const char *in,
		const char *out,
		int opt_level);

#ifdef __cplusplus
}   // extern "C"
#endif


#endif

