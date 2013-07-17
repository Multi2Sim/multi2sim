/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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
#include <lib/util/string.h>

#include "gpu.h"
#include "sm.h"
#include "cycle-interval-report.h"


int frm_spatial_report_active = 0 ;

static int spatial_profiling_interval = 10000;
static char *frm_spatial_report_section_name = "FRMSpatialReport";
static FILE *spatial_report_file;
static char *spatial_report_filename = "report-sm-spatial";


void frm_spatial_report_config_read(struct config_t *config)
{
	char *section;
	char *file_name;

	/*Nothing if section or config is not present */
	section = frm_spatial_report_section_name;
	if (!config_section_exists(config, section))
	{
		/*no spatial profiling */
		return;
	}

	/* Spatial reports are active */
	frm_spatial_report_active = 1;

	/* Interval */
	config_var_enforce(config, section, "Interval");
	spatial_profiling_interval = config_read_int(config, section,
		"Interval", spatial_profiling_interval);

	/* File name */
	config_var_enforce(config, section, "File");
	file_name = config_read_string(config, section, "File", NULL);
	if (!file_name || !*file_name)
		fatal("%s: %s: invalid or missing value for 'File'",
			frm_spatial_report_section_name, section);
	spatial_report_filename = str_set(NULL, file_name);

	spatial_report_file = file_open_for_write(spatial_report_filename);
	if (!spatial_report_file)
		fatal("%s: could not open spatial report file",
				spatial_report_filename);

}

void frm_sm_spatial_report_init()
{

}

void frm_sm_spatial_report_done()
{

	fclose(spatial_report_file);
	spatial_report_file = NULL;
	str_free(spatial_report_filename);
}

void frm_sm_spatial_report_dump(struct frm_sm_t *sm)
{
	FILE *f = spatial_report_file;

	fprintf(f,"CU,%d,MemAcc,%lld,MappedWGs,%lld,Cycles,%lld\n",
			sm->id,
			sm->vector_mem_unit.inflight_mem_accesses,
			sm->interval_mapped_thread_blocks,
			asTiming(frm_gpu)->cycle);

}


void frm_report_global_mem_inflight( struct frm_sm_t *sm, int long long pending_accesses)
{
	/* Read stage adds a negative number for accesses added
	 * Write stage adds a positive number for accesses finished
	 */
	sm->vector_mem_unit.inflight_mem_accesses += pending_accesses;

}

void frm_report_global_mem_finish( struct frm_sm_t *sm, int long long completed_accesses)
{
	/* Read stage adds a negative number for accesses added */
	/* Write stage adds a positive number for accesses finished */
	sm->vector_mem_unit.inflight_mem_accesses -= completed_accesses;

}

void frm_report_mapped_thread_block(struct frm_sm_t *sm)
{
	/*TODO Add calculation here to change this to wavefront pool entries used */
	sm->interval_mapped_thread_blocks++;
}

void frm_sm_interval_update(struct frm_sm_t *sm)
{
	/* If interval - reset the counters in all the engines */
	sm->interval_cycle ++;
	if (!(asTiming(frm_gpu)->cycle % spatial_profiling_interval))
	{
		frm_sm_spatial_report_dump(sm);

		/*
		 * This counter is not reset since memory accesses could still
		 * be in flight in the hierarchy
		 * sm->inflight_mem_accesses = 0;
		 */
		sm->interval_cycle = 0;
		sm->interval_mapped_thread_blocks = 0;
	}
}
