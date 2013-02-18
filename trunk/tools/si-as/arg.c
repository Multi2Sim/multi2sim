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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>

#include "arg.h"


struct si_arg_t *si_arg_create(void)
{
	struct si_arg_t *arg;
	
	/* Allocate */
	arg = xcalloc(1, sizeof(struct si_arg_t));
	
	/* Return */
	return arg;
	
}


void si_arg_free(struct si_arg_t *inst_arg)
{
	free(inst_arg);
}


void si_arg_dump(struct si_arg_t *inst_arg, FILE *f)
{
	switch (inst_arg->type)
	{
	
	case si_arg_invalid:
		fprintf(f, "\t\tinvalid\n");
		break;
		
	case si_arg_scalar_register:
		fprintf(f, "\t\tscalar register\n");
		fprintf(f, "\t\ts%d\n", inst_arg->value.scalar_register.id);
		break;
		
	case si_arg_vector_register:
		fprintf(f, "\t\tvector register\n");
		fprintf(f, "\t\tv%d\n", inst_arg->value.vector_register.id);
		break;
		
	case si_arg_register_range:
		fprintf(f, "\t\tregister range\n");
		fprintf(f, "\t\ts[%d:%d]\n", inst_arg->value.register_range.id_low,
			inst_arg->value.register_range.id_high);
		break;
			
	case si_arg_literal:
		fprintf(f, "\t\tliteral constant\n");
		fprintf(f, "\t\t0x%x (%d)\n", inst_arg->value.literal.val,
			inst_arg->value.literal.val);
		break;
		
	case si_arg_waitcnt:
	{
		fprintf(f, "\t\twaitcnt\n");
		fprintf(f, "\t\tvmcnt: %d\n", inst_arg->value.wait_cnt.vmcnt_value);
		fprintf(f, "\t\texpcnt: %d\n", inst_arg->value.wait_cnt.expcnt_value);
		fprintf(f, "\t\tlgkmcnt: %d\n", inst_arg->value.wait_cnt.lgkmcnt_value);			
		break;
	}
	case si_arg_special_register:
		fprintf(f, "\t\tspecial register\n");
		if (inst_arg->value.special_register.type == si_arg_special_register_vcc)
			fprintf(f, "\t\tvcc\n");
		else if (inst_arg->value.special_register.type == si_arg_special_register_scc)
			fprintf(f, "\t\tscc\n");
		break;
	
	case si_arg_mtype_register:
		fprintf(f, "\t\tm-type register\n");
		fprintf(f, "\t\tm%d\n", inst_arg->value.mtype_register.id);
		break;
	
	case si_arg_format:
		fprintf(f, "\t\tformat\n");
		fprintf(f, "\t\toffen: %c\n", inst_arg->value.format.offen ? 't' : 'f');
		fprintf(f, "\t\tdata format: %s\n", inst_arg->value.format.data_format);
		fprintf(f, "\t\tnum format: %s\n", inst_arg->value.format.num_format);
		fprintf(f, "\t\toffset: %d\n", inst_arg->value.format.offset);
		break;

	case si_arg_label:
		fprintf(f, "\tlabel\n");
		break;
		
	default:
		panic("%s: invalid argument type", __FUNCTION__);
		break;
	}
}




/*
 * Object 'si_formal_arg_t'
 */

struct str_map_t si_formal_arg_token_map =
{
	si_formal_arg_token_count,
	{
		{ "<invalid>", si_formal_arg_token_invalid },

		{ "\%64_sdst", si_formal_arg_token_64_sdst },
		{ "\%64_ssrc0", si_formal_arg_token_64_ssrc0 },
		{ "\%64_ssrc1", si_formal_arg_token_64_ssrc1 },
		{ "\%64_src0", si_formal_arg_token_64_src0 },
		{ "\%64_src1", si_formal_arg_token_64_src1 },
		{ "\%64_src2", si_formal_arg_token_64_src2 },
		{ "\%64_svdst", si_formal_arg_token_64_svdst },
		{ "\%64_vdst", si_formal_arg_token_64_vdst },
		{ "\%label", si_formal_arg_token_label },
		{ "\%offset", si_formal_arg_token_offset },
		{ "\%sdst", si_formal_arg_token_sdst },
		{ "\%series_sbase", si_formal_arg_token_series_sbase },
		{ "\%series_sdst", si_formal_arg_token_series_sdst },
		{ "\%simm16", si_formal_arg_token_simm16 },
		{ "\%smrd_sdst", si_formal_arg_token_smrd_sdst },
		{ "\%src0", si_formal_arg_token_src0 },
		{ "\%src1", si_formal_arg_token_src1 },
		{ "\%src2", si_formal_arg_token_src2 },
		{ "\%ssrc0", si_formal_arg_token_ssrc0 },
		{ "\%ssrc1", si_formal_arg_token_ssrc1 },
		{ "\%vdst", si_formal_arg_token_vdst },
		{ "\%wait_cnt", si_formal_arg_token_wait_cnt }
	}
};


struct si_formal_arg_t *si_formal_arg_create(int token)
{
	struct si_formal_arg_t *arg;

	arg = xcalloc(1, sizeof(struct si_formal_arg_t));
	arg->token = token;
	return arg;
}


void si_formal_arg_free(struct si_formal_arg_t *arg)
{
	free(arg);
}

