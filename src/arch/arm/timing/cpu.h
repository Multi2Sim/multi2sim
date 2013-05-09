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

#ifndef ARCH_ARM_TIMING_CPU_H
#define ARCH_ARM_TIMING_CPU_H

#include <stdio.h>


/*
 * Public Functions
 */

void arm_cpu_read_config(void);

void arm_cpu_init(void);
void arm_cpu_done(void);

int arm_cpu_run(void);

void arm_cpu_dump(FILE *f);
void arm_cpu_dump_summary(FILE *f);

#endif
