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

#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>

#include "gpu.h"

#include "cycle-interval-report.h"
#include "compute-unit.h"

int evg_spatial_report_active = 0 ;

static int spatial_profiling_interval = 10000;
static char *evg_spatial_report_section_name = "SpatialReport";
static FILE * spatial_report_file;
static char * spatial_report_filename = "report-cu-spatial";


void evg_spatial_report_config_read(struct config_t *config)
{
	char *section;

	/* Nothing if section or config is not present */

	section = evg_spatial_report_section_name;
	if (!config_section_exists(config, section))
	{
		/*no spatial profiling */
		return;
	}

	/* Spatial reports are active */
	evg_spatial_report_active = 1;

	/* Interval */
	config_var_enforce(config, section, "Interval");
	spatial_profiling_interval = config_read_int(config, section,
		"Interval", spatial_profiling_interval);

	spatial_report_file = file_open_for_write(spatial_report_filename);
	if (!spatial_report_file)
		fatal("%s: could not open spatial report file", spatial_report_filename);

	//FILE *f = spatial_report_file ;
	/*fprintf(f,"CU,CFInst,MemAcc,TEXInstn,ALUInstn,Cycles\n");*/

}


void evg_cf_spatial_report_init()
{

}

void evg_cu_spatial_report_done()
{
	fclose(spatial_report_file);
	spatial_report_file = NULL;

}

void evg_cu_spatial_report_dump(struct evg_compute_unit_t *compute_unit)
{
	FILE *f = spatial_report_file;

	fprintf(f,"CU,%d,CFInst,%lld,MemAcc,%lld,TEXInstn,%lld,ALUInstn,%lld,Cycles,%lld \n",
			compute_unit->id,
			compute_unit->cf_engine.interval_inst_count,
			compute_unit->inflight_mem_accesses,
			compute_unit->tex_engine.interval_inst_count,
			compute_unit->alu_engine.interval_inst_count,
			asTiming(evg_gpu)->cycle);

}

/* Called in fetch stage cf engine */
void evg_cf_report_new_inst(struct evg_compute_unit_t *compute_unit)
{
	compute_unit->cf_engine.interval_inst_count++;
}

void evg_tex_report_new_inst(struct evg_compute_unit_t *compute_unit)
{
	compute_unit->tex_engine.interval_inst_count++;

}

void evg_alu_report_new_inst(struct evg_compute_unit_t *compute_unit)
{
	compute_unit->alu_engine.interval_inst_count++;
}


void evg_tex_report_global_mem_inflight( struct evg_compute_unit_t *compute_unit, int long long pending_accesses)
{
	/* Read stage adds a negative number for accesses added */
	/* Write stage adds a positive number for accesses finished */
	compute_unit->inflight_mem_accesses += pending_accesses;

}

void evg_tex_report_global_mem_finish( struct evg_compute_unit_t *compute_unit, int long long completed_accesses)
{
	/* Read stage adds a negative number for accesses added */
	/* Write stage adds a positive number for accesses finished */
	compute_unit->inflight_mem_accesses = compute_unit->inflight_mem_accesses - completed_accesses;

}


void evg_cu_interval_update(struct evg_compute_unit_t *compute_unit)
{
	/* If interval - reset the counters in all the engines */
	compute_unit->interval_cycle ++;
	if (!(asTiming(evg_gpu)->cycle % spatial_profiling_interval))
	{
		evg_cu_spatial_report_dump(compute_unit);

		compute_unit->cf_engine.interval_inst_count = 0;
		compute_unit->alu_engine.interval_inst_count = 0;
		compute_unit->tex_engine.interval_inst_count = 0;
		/* This counter is not reset since memory accesses could still be in flight in the hierarchy*/
		/* compute_unit->inflight_mem_accesses = 0; */
		compute_unit->interval_cycle = 0;

	}

}

