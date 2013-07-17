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

#ifndef ARCH_COMMON_TIMING_H
#define ARCH_COMMON_TIMING_H

#include <lib/util/class.h>

/* Forward declarations */
struct config_t;


/*
 * Class 'Timing'
 */

CLASS_BEGIN(Timing, Object)

	/* Architecture that this timing simulator belongs to */
	struct arch_t *arch;
	
	/* Current cycle */
	long long cycle;

	/* Frequency domain, as returned by 'esim_new_domain()'.
	 * This variable is initialized by the child class. */
	int frequency;
	int frequency_domain;



	/*** Virtual functions ***/

	/* Print statistics summary */
	void (*DumpSummary)(Timing *self, FILE *f);

	/* Virtual abstract function to run one step of the timing simulation
	 * loop. The function returns TRUE if any valid simulation was
	 * performed by the architecture. */
	int (*Run)(Timing *self);

	/* Function related with the creation of default memory hierarchies and
	 * processing of memory configuration files. These are all abstract
	 * functions that must be overridden by children. */
	void (*MemConfigDefault)(Timing *self, struct config_t *config);
	void (*MemConfigCheck)(Timing *self, struct config_t *config);
	void (*MemConfigParseEntry)(Timing *self, struct config_t *config,
			char *section);

CLASS_END(Timing)


void TimingCreate(Timing *self);
void TimingDestroy(Timing *self);

void TimingDump(Object *self, FILE *f);
void TimingDumpSummary(Timing *self, FILE *f);

int TimingRun(Timing *self);

void TimingMemConfigDefault(Timing *self, struct config_t *config);
void TimingMemConfigCheck(Timing *self, struct config_t *config);
void TimingMemConfigParseEntry(Timing *self, struct config_t *config, char *section);


#endif
