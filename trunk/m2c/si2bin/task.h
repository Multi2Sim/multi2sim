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

#ifndef M2C_SI2BIN_TASK_H_OLD
#define M2C_SI2BIN_TASK_H_OLD

#include <lib/class/class.h>


/*
 * Class 'Si2binTask'
 */

CLASS_BEGIN(Si2binTask, Object)

	int offset;
	Si2binSymbol *symbol;

CLASS_END(Si2binTask)


void Si2binTaskCreate(Si2binTask *self, int offset, Si2binSymbol *symbol);
void Si2binTaskDestroy(Si2binTask *self);

void Si2binTaskDump(Si2binTask *self, FILE *f);
void Si2binTaskProcess(Si2binTask *self);


#endif
