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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_MEM_CONFIG_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_MEM_CONFIG_H

/* Forward declarations */
struct config_t;


/*
 * Class 'SIGpu'
 * (Extra functions)
 */

void SIGpuMemConfigFused(Timing *self, struct config_t *config);
void SIGpuMemConfigDefault(Timing *self, struct config_t *config);
void SIGpuMemConfigParseEntry(Timing *self, struct config_t *config, char *section);
void SIGpuMemConfigCheck(Timing *self, struct config_t *config);

#endif
