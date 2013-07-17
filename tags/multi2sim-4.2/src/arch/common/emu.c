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

#include <lib/mhandle/mhandle.h>
#include <lib/util/timer.h>

#include "emu.h"



/*
 * Class 'Emu'
 */


CLASS_IMPLEMENTATION(Emu);


void EmuCreate(Emu *self, char *name)
{
	/* Initialize */
	self->name = xstrdup(name);
	self->timer = m2s_timer_create(name);

	/* Virtual functions */
	asObject(self)->Dump = EmuDump;
}


void EmuDestroy(Emu *self)
{
	free(self->name);
	m2s_timer_free(self->timer);
}


void EmuDump(Object *self, FILE *f)
{
	/* Call parent */
	ObjectDump(asObject(self), f);
}


void EmuDumpSummary(Emu *self, FILE *f)
{
	double time_in_sec;
	double inst_per_sec;

	time_in_sec = (double) m2s_timer_get_value(self->timer) / 1.0e6;
	inst_per_sec = time_in_sec > 0.0 ? (double) self->instructions / time_in_sec : 0.0;

	fprintf(f, "[ %s ]\n", self->name);
	fprintf(f, "RealTime = %.2f [s]\n", time_in_sec);
	fprintf(f, "Instructions = %lld\n", self->instructions);
	fprintf(f, "InstructionsPerSecond = %.0f\n", inst_per_sec);

}
