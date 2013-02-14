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

#include <stdlib.h>

#include <lib/util/debug.h>

#include "inst-arg.h"


struct si_inst_arg_t *si_inst_arg_create()
{
	struct si_inst_arg_t *inst_arg;
	
	/* Allocate */
	inst_arg = calloc(1, sizeof(struct si_inst_arg_t));
	if (!inst_arg)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Return */
	return inst_arg;
	
}


void si_inst_arg_free(struct si_inst_arg_t *inst_arg)
{
	free(inst_arg);
}


void si_inst_arg_dump(struct si_inst_arg_t *inst_arg, FILE *f)
{
	switch (inst_arg->type)
	{
	
	case si_inst_arg_invalid:
		fprintf(f, "\tInvalid argument!\n");
		break;
		
	case si_inst_arg_scalar_register:
		fprintf(f, "\tType: Scalar Register\n");
		fprintf(f, "\tRegister ID: %d\n", inst_arg->value.scalar_register.id);
		break;
		
	case si_inst_arg_vector_register:
		fprintf(f, "\tType: Vector Register\n");
		fprintf(f, "\tRegister ID: %d\n", inst_arg->value.vector_register.id);
		break;
		
	case si_inst_arg_register_range:
		fprintf(f, "\tType: Register Range\n");
		fprintf(f, "\tLow: %d\n", inst_arg->value.register_range.id_low);
		fprintf(f, "\tHigh: %d\n", inst_arg->value.register_range.id_high);
		break;
			
	case si_inst_arg_literal:
		fprintf(f, "\tType: Literal\n");
		fprintf(f, "\tValue: %d\n", inst_arg->value.literal.val);
		break;
		
	case si_inst_arg_waitcnt:
	{
		fprintf(f, "\tType: Waitcnt\n");
		if(inst_arg->value.wait_cnt.vmcnt_active)
			fprintf(f, "\tvmcnt: %d\n", inst_arg->value.wait_cnt.vmcnt_value);
		if(inst_arg->value.wait_cnt.expcnt_active)
			fprintf(f, "\texpcnt: %d\n", inst_arg->value.wait_cnt.expcnt_value);
		if(inst_arg->value.wait_cnt.lgkmcnt_active)
			fprintf(f, "\tlgkmcnt: %d\n", inst_arg->value.wait_cnt.lgkmcnt_value);			
		break;
	}
	case si_inst_arg_special_register:
		fprintf(f, "\tType: Special Register\n");
		if (inst_arg->value.special_register.type == si_inst_arg_special_register_vcc)
			fprintf(f, "\tID: vcc\n");
		else if (inst_arg->value.special_register.type == si_inst_arg_special_register_scc)
			fprintf(f, "\tID: scc\n");
		break;
	
	case si_inst_arg_mtype_register:
		fprintf(f, "\tType: M-Type\n");
		fprintf(f, "\tID: %d\n", inst_arg->value.mtype_register.id);
		break;
	
	case si_inst_arg_format:
		fprintf(f, "\tType: Format\n");
		if (inst_arg->value.format.offen)
			fprintf(f, "\toffen: True\n");
		else
			fprintf(f, "\toffen: False\n");
		fprintf(f, "\tData Format: %s\n", inst_arg->value.format.data_format);
		fprintf(f, "\tNum Format: %s\n", inst_arg->value.format.num_format);
		fprintf(f, "\tOffset: %d\n", inst_arg->value.format.offset);
	case si_inst_arg_label:
		fprintf(f, "\tType: Label\n");
		break;
		
	default:
		fprintf(f, "Invalid arg type...\n");
		break;
	}
}

