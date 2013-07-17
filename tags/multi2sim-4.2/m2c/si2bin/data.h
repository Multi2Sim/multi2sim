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

#ifndef M2C_SI2BIN_DATA_H
#define M2C_SI2BIN_DATA_H

enum si2bin_data_type_t
{
	si2bin_data_invalid = 0,

	si2bin_data_int,
	si2bin_data_short,
	si2bin_data_float,
	si2bin_data_word,
	si2bin_data_half,
	si2bin_data_byte
};

struct si2bin_data_t
{
	enum si2bin_data_type_t data_type;
	
	union
	{
		int int_value;
		short short_value;
		float float_value;
		unsigned int word_value;
		unsigned short half_value;
		unsigned char byte_value;
	};

};
		
struct si2bin_data_t *si2bin_data_create(void);
void si2bin_data_free(struct si2bin_data_t *data);

#endif
