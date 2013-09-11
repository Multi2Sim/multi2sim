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

#ifndef M2C_SI2BIN_OUTER_BIN_H
#define M2C_SI2BIN_OUTER_BIN_H

#include "inner-bin.h"
#include "metadata.h"

/* Forward Declarations */
struct list_t;

enum si2bin_outer_bin_device_t
{
	si2bin_outer_bin_invalid = 0,

	si2bin_outer_bin_cape_verde,
	si2bin_outer_bin_pitcairn,
	si2bin_outer_bin_tahiti
};

/*
 * AMD External ELF Binary
 */

struct si2bin_outer_bin_t
{
	/* Device Type */
	enum si2bin_outer_bin_device_t device;

	/* List of si2bin_data_t elements to be added to global buffer */
	struct list_t *data_list;

	/* ELF file create internally.
	 * Private field. */
	ELFWriter *writer;

	/* List of internal ELFs. Each element is of type 'si2bin_inner_bin_t'.
	 * Private field. */
	struct list_t *inner_bin_list;

	/* Element of type si2bin_metadata_t */
	struct list_t *metadata_list;
};

struct si2bin_outer_bin_t *si2bin_outer_bin_create(void);
void si2bin_outer_bin_free(struct si2bin_outer_bin_t *outer_bin);
void si2bin_outer_bin_add_data(struct si2bin_outer_bin_t *outer_bin,
		Si2binData *data);
void si2bin_outer_bin_add(struct si2bin_outer_bin_t *outer_bin,
		struct si2bin_inner_bin_t *inner_bin, struct si2bin_metadata_t *metadata);
void si2bin_outer_bin_generate(struct si2bin_outer_bin_t *outer_bin,
		ELFWriterBuffer *buffer);

#endif

