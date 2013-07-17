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

#ifndef M2C_SI2BIN_METADATA_H
#define M2C_SI2BIN_METADATA_H

/* Forward Declarations */
struct list_t;
struct si_arg_t;


/*
 * Metadata
 */
	
	
struct si2bin_metadata_t
{

	struct list_t *arg_list; /* Elements of type si_arg_t */
	
	int uniqueid;
	int uavprivate;
	int hwregion;
	int hwlocal;

};

struct si2bin_metadata_t *si2bin_metadata_create(void);
void si2bin_metadata_free(struct si2bin_metadata_t *metadata);

void si2bin_metadata_add_arg(struct si2bin_metadata_t *metadata, struct si_arg_t *arg);

#endif

