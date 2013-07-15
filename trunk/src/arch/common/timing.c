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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/timer.h>

#include "arch.h"
#include "emu.h"
#include "timing.h"



/*
 * Class 'Timing'
 */


CLASS_IMPLEMENTATION(Timing);


void TimingCreate(Timing *self)
{
	/* Virtual functions */
	asObject(self)->Dump = TimingDump;
	self->Run = TimingRun;
	self->MemConfigDefault = TimingMemConfigDefault;
	self->MemConfigCheck = TimingMemConfigCheck;
	self->MemConfigParseEntry = TimingMemConfigParseEntry;
}


void TimingDestroy(Timing *self)
{
}


void TimingDump(Object *self, FILE *f)
{
}


void TimingDumpSummary(Timing *self, FILE *f)
{
	double time_in_sec;
	double cycles_per_sec;
	double cycle_time;  /* In nanoseconds */

	struct arch_t *arch;

	Emu *emu;
	Timing *timing;

	/* Obtain objects */
	timing = asTiming(self);
	arch = timing->arch;
	assert(arch);
	emu = arch->emu;
	assert(emu);

	/* Calculate statistics */
	time_in_sec = (double) m2s_timer_get_value(emu->timer) / 1.0e6;
	cycles_per_sec = time_in_sec > 0.0 ? (double) timing->cycle / time_in_sec : 0.0;
	cycle_time = (double) esim_domain_cycle_time(timing->frequency_domain) / 1000.0;

	/* Print */
	fprintf(f, "SimTime = %.2f [ns]\n", timing->cycle * cycle_time);
	fprintf(f, "Frequency = %d [MHz]\n", timing->frequency);
	fprintf(f, "Cycles = %lld\n", timing->cycle);
	fprintf(f, "CyclesPerSecond = %.0f\n", cycles_per_sec);
}


int TimingRun(Timing *self)
{
	panic("%s: abstract function not overridden",
			__FUNCTION__);
	return 0;
}


void TimingMemConfigDefault(Timing *self, struct config_t *config)
{
	panic("%s: abstract function not overridden",
			__FUNCTION__);
}


void TimingMemConfigCheck(Timing *self, struct config_t *config)
{
	panic("%s: abstract function not overridden",
			__FUNCTION__);
}


void TimingMemConfigParseEntry(Timing *self, struct config_t *config, char *section)
{
	panic("%s: abstract function not overridden",
			__FUNCTION__);
}
