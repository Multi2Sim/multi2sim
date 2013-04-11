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

#ifndef ARCH_FERMI_SPATIAL_REPORT_H
#define ARCH_FERMI_SPATIAL_REPORT_H


/*
 * Public variable
 */
extern int frm_spatial_report_active  ;
struct frm_sm_t;


void frm_report_new_inst(struct frm_sm_t *sm);

void frm_alu_report_new_inst(struct frm_sm_t *sm);

void frm_report_mapped_thread_block(struct frm_sm_t *sm);


/* Used in vector unit to keep track of num of mem accesses in flight */
void frm_report_global_mem_inflight( struct frm_sm_t *sm, int long long pending_accesses);

void frm_report_global_mem_finish( struct frm_sm_t *sm, int long long completed_accesses);

struct config_t;

void frm_spatial_report_config_read(struct config_t *config);

void frm_sm_interval_update(struct frm_sm_t *sm);

void frm_sm_spatial_report_done();


#endif
