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
#include <lib/util/string.h>

#include "gpu.h"
#include "cycle-interval-report.h"

#include "compute-unit.h"



/*
 * Public
 */

int si_spatial_report_active = 0 ;

static int spatial_profiling_interval = 10000;
static char *si_spatial_report_section_name = "SISpatialReport";
static FILE *spatial_report_file;
static char *spatial_report_filename = "report-cu-spatial";


void SISpatialReportReadConfig(struct config_t *config)
{
	char *section;
	char *file_name;

	/*Nothing if section or config is not present */
	section = si_spatial_report_section_name;
	if (!config_section_exists(config, section))
	{
		/*no spatial profiling */
		return;
	}

	/* Spatial reports are active */
	si_spatial_report_active = 1;

	/* Interval */
	config_var_enforce(config, section, "Interval");
	spatial_profiling_interval = config_read_int(config, section,
		"Interval", spatial_profiling_interval);

	/* File name */
	config_var_enforce(config, section, "File");
	file_name = config_read_string(config, section, "File", NULL);
	if (!file_name || !*file_name)
		fatal("%s: %s: invalid or missing value for 'File'",
			si_spatial_report_section_name, section);
	spatial_report_filename = str_set(NULL, file_name);

	spatial_report_file = file_open_for_write(spatial_report_filename);
	if (!spatial_report_file)
		fatal("%s: could not open spatial report file",
				spatial_report_filename);

}


void SISpatialReportInit(void)
{
}


void SISpatialReportDone(void)
{

	fclose(spatial_report_file);
	spatial_report_file = NULL;
	str_free(spatial_report_filename);
}




/*
 * Class 'SIComputeUnit'
 */

void SIComputeUnitReportDump(SIComputeUnit *self)
{
	SIGpu *gpu = self->gpu;
	FILE *f = spatial_report_file;

	fprintf(f,
		"CU,%d,MemAcc,%lld,MappedWGs,%lld,UnmappedWGs,%lld,"
		"ALUIssued,%lld,LDSIssued,%lld,Cycles,%lld\n",
		self->id,
		self->vector_mem_unit.inflight_mem_accesses,
		self->interval_mapped_work_groups,
		self->interval_unmapped_work_groups,
		self->interval_alu_issued,
		self->interval_lds_issued,
		asTiming(gpu)->cycle);
}


void SIComputeUnitReportNewLDSInst(SIComputeUnit *self)
{
	self->interval_lds_issued = self->interval_lds_issued + 1;
}


void SIComputeUnitReportNewALUInst(SIComputeUnit *self)
{
	self->interval_alu_issued ++ ;

}


void SIComputeUnitReportGlobalMemInFlight(SIComputeUnit *self,
		long long pending_accesses)
{
	/* Read stage adds a negative number for accesses added
	 * Write stage adds a positive number for accesses finished
	 */
	self->vector_mem_unit.inflight_mem_accesses += pending_accesses;


}


void SIComputeUnitReportGlobalMemFinish(SIComputeUnit *self,
		long long completed_accesses)
{
	/* Read stage adds a negative number for accesses added */
	/* Write stage adds a positive number for accesses finished */
	self->vector_mem_unit.inflight_mem_accesses -= completed_accesses;

}


void SIComputeUnitReportMapWorkGroup(SIComputeUnit *self)
{
	/*TODO Add calculation here to change this to wavefront pool entries used */
	self->interval_mapped_work_groups++;
}


void SIComputeUnitReportUnmapWorkGroup(SIComputeUnit *self)
{
	/*TODO Add calculation here to change this to wavefront pool entries used */
	self->interval_unmapped_work_groups++;
}


void SIComputeUnitReportUpdate(SIComputeUnit *self)
{
	SIGpu *gpu = self->gpu;

	/* If interval - reset the counters in all the engines */
	self->interval_cycle ++;

	if (!(asTiming(gpu)->cycle % spatial_profiling_interval))
	{
		SIComputeUnitReportDump(self);

		/*
		 * This counter is not reset since memory accesses could still
		 * be in flight in the hierarchy
		 * compute_unit->inflight_mem_accesses = 0;
		 */
		self->interval_cycle = 0;
		self->interval_mapped_work_groups = 0;
		self->interval_unmapped_work_groups = 0;
		self->interval_alu_issued = 0;
		self->interval_lds_issued = 0;
	}
}

