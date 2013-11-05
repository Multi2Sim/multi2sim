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

#include <memory>
#include <vector>

#include <src/lib/cpp/ELFWriter.h>

#include "InnerBin.h"
#include "Metadata.h"
#include "Data.h"

namespace si2bin
{

enum OuterBinDevice
{
	OuterBinInvalid = 0,

	OuterBinCapeVerde,
	OuterBinPitcairn,
	OuterBinTahiti
};

/*
 * AMD External ELF Binary
 */

class OuterBin
{
	/* Device Type */
	OuterBinDevice device;

	std::vector<std::unique_ptr<Data>> data_list;

	/* ELF file create internally.
	 * Private field. */
	ELFWriter::File writer;

	std::vector<std::unique_ptr<InnerBin>> inner_bin_list;

	std::vector<std::unique_ptr<Metadata>> metadata_list;

public:

	OuterBin();

	/* Getter */
	OuterBinDevice GetDevice() { return device; }

	InnerBin *GetInnerBin(unsigned int index) { return index < inner_bin_list.size() ?
			inner_bin_list[index].get() : nullptr; }
	Metadata *GetMetadata(unsigned int index) { return index < metadata_list.size() ?
			metadata_list[index].get() : nullptr; }
	Data *GetData(unsigned int index) { return index < data_list.size() ?
			data_list[index].get() : nullptr; }
	unsigned int GetDataCount() { return data_list.size(); }

	/* Setter */
	void SetDevice(OuterBinDevice device) { this->device = device; }

	void Generate(std::ostream& os);

	void AddData(Data *data);
	
	void AddKernel(InnerBin *inner_bin, Metadata *metadata);
};

} /* namespace si2bin */

#endif

