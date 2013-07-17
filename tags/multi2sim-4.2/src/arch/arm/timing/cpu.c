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

#include <lib/util/misc.h>

#include "cpu.h"


/*
 * Class 'ARMCpu'
 */

CLASS_IMPLEMENTATION(ARMCpu);

void ARMCpuCreate(ARMCpu *self)
{
	/* Parent */
	TimingCreate(asTiming(self));

	/* Virtual functions */
	asObject(self)->Dump = ARMCpuDump;
	asTiming(self)->DumpSummary = ARMCpuDumpSummary;
	asTiming(self)->Run = ARMCpuRun;
}


void ARMCpuDestroy(ARMCpu *self)
{
}


void ARMCpuDump(Object *self, FILE *f)
{
}


void ARMCpuDumpSummary(Timing *self, FILE *f)
{
	/* Call parent */
	TimingDumpSummary(asTiming(self), f);
}


int ARMCpuRun(Timing *self)
{
	return FALSE;
}




/*
 * Non-Class Public Stuff
 */

ARMCpu *arm_cpu;


void arm_cpu_read_config(void)
{
}


void arm_cpu_init(void)
{
	/* Classes */
	CLASS_REGISTER(ARMCpu);

	/* Create CPU */
	arm_cpu = new(ARMCpu);
}


void arm_cpu_done(void)
{
	/* Free CPU */
	delete(arm_cpu);
}
