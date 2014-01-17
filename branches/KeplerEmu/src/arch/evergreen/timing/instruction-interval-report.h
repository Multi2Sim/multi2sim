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

#ifndef ARCH_EVERGREEN_PERIODIC_REPORT_H
#define ARCH_EVERGREEN_PERIODIC_REPORT_H

extern int evg_periodic_report_active;
extern char *evg_periodic_report_file_name;

/* Forward declarations */
struct config_t;
struct evg_uop_t;


void evg_periodic_report_config_read(struct config_t *config);

void evg_periodic_report_wavefront_init(EvgGpu *gpu, EvgWavefront *wavefront);
void evg_periodic_report_wavefront_done(EvgGpu *gpu, EvgWavefront *wavefront);

void evg_periodic_report_new_inst(EvgGpu *gpu, struct evg_uop_t *uop);

#endif

