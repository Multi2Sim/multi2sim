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

#include <arch/southern-islands/asm/Wrapper.h>
#include <lib/class/elf-writer.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "outer-bin.h"
#include "PVars.h"
#include "si2bin.h"
#include "symbol.h"
#include "task.h"


/*
 * Task Object
 */

void Si2binTaskCreate(Si2binTask *self, int offset, Si2binSymbol *symbol)
{
	/* Initialize */
	self->offset = offset;
	self->symbol = symbol;
}


void Si2binTaskDestroy(Si2binTask *self)
{
}


void Si2binTaskDump(Si2binTask *self, FILE *f)
{
	fprintf(f, "offset=%d, symbol={", self->offset);
	Si2binSymbolDump(self->symbol, f);
	fprintf(f, "}");
}


void Si2binTaskProcess(Si2binTask *self)
{
	Si2binSymbol *label;
	SIInstBytes *inst;


	/* Check whether symbol is resolved */
	label = self->symbol;
	if (!label->defined)
		si2bin_yyerror_fmt("undefined label: %s", label->name->text);

	/* Resolve label */
	assert(IN_RANGE(self->offset, 0, si2bin_entry->text_section_buffer->offset - 4));
	inst = si2bin_entry->text_section_buffer->ptr + self->offset;
	inst->sopp.simm16 = (label->value - self->offset) / 4 - 1;
}
