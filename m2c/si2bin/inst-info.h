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

#ifndef M2C_SI2BIN_INST_INFO_H
#define M2C_SI2BIN_INST_INFO_H

#include <stdio.h>

#include <arch/southern-islands/asm/Wrapper.h>


/*
 * Class 'Si2binInstInfo'
 */

CLASS_BEGIN(Si2binInstInfo, Object)

	/* There can be multiple instruction encodings for the same instruction
	 * name. This points to the next one in the list. */
	Si2binInstInfo *next;

	/* Associated info structure in disassembler */
	SIInstInfo *inst_info;

	/* List of tokens in format string */
	List *str_token_list;  /* Element of type string */
	List *token_list;  /* Elements of type 'Si2binToken' */
	String *name;  /* Token 0 of this list */

CLASS_END(Si2binInstInfo)


void Si2binInstInfoCreate(Si2binInstInfo *self, SIInstInfo *inst_info);
void Si2binInfoInfoDestroy(Si2binInstInfo *self);


#endif

