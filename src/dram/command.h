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

#ifndef DRAM_COMMAND_H
#define DRAM_COMMAND_H


/*
 * Local variable
 */

enum dram_command_type_t
{
	dram_command_invalid = 0,
	dram_command_refresh,
	dram_command_precharge,
	dram_command_activate,
	dram_command_read,
	dram_command_write,
	dram_command_nop
};


/*
 * Command
 */

struct dram_command_t
{
	long long id;
	long long cycle;
	enum dram_command_type_t type;
	struct dram_t *dram;
	struct bus_t *dram_bus;

	union
	{
		struct {
			int rank_id;
			int bank_id;
			int row_id;
		} refresh;

		struct {
			int rank_id;
			int bank_id;
		} precharge;

		struct {
			int rank_id;
			int bank_id;
			int row_id;
		} activate;

		struct {
			int rank_id;
			int bank_id;
			int column_id;
		} read;

		struct {
			int rank_id;
			int bank_id;
			int column_id;
		} write;
	} u;
};

struct dram_command_t *dram_command_create(void);
void dram_command_free(struct dram_command_t *dram_command);
void dram_command_dump(struct dram_command_t *dram_command, FILE *f);


#endif
