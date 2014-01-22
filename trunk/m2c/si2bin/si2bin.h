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

#ifndef M2C_SI2BIN_SI2BIN_H
#define M2C_SI2BIN_SI2BIN_H

#include <lib/class/class.h>


/* Forward declaration */
struct list_t;

extern char *si2bin_machine_name;

/*
 * Class 'Si2bin'
 */

CLASS_BEGIN(Si2bin, Object)

CLASS_END(Si2bin)

void Si2binCreate(Si2bin *self);
void Si2binDestroy(Si2bin *self);

void Si2binCompile(Si2bin *self,
		struct list_t *source_file_list,
		struct list_t *bin_file_list);

#endif

