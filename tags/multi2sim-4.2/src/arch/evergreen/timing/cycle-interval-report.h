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

#ifndef ARCH_EVERGREEN_SPATIAL_REPORT_H
#define ARCH_EVERGREEN_SPATIAL_REPORT_H

/*
 * Public variable
 */
extern int evg_spatial_report_active  ;

/* Called in fetch stage cf engine */
void evg_cf_report_new_inst(struct evg_compute_unit_t *compute_unit);

/* Called in fetch stage tex engine */
void evg_tex_report_new_inst(struct evg_compute_unit_t *compute_unit);

/* Called in fetch stage alu engine */
void evg_alu_report_new_inst(struct evg_compute_unit_t *compute_unit);

void evg_tex_report_global_mem_inflight( struct evg_compute_unit_t *compute_unit, int long long pending_accesses);

void evg_tex_report_global_mem_finish( struct evg_compute_unit_t *compute_unit, int long long completed_accesses);

void evg_cu_interval_update(struct evg_compute_unit_t *compute_unit);

struct config_t;
void evg_spatial_report_config_read(struct config_t *config);

void evg_cu_spatial_report_done();


#endif
