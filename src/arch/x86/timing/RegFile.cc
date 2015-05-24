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

#include "RegFile.h"

namespace x86
{

misc::StringMap RegFile::KindMap =
{
	{"Shared", KindShared},
	{"Private", KindPrivate}
};

RegFile::Kind RegFile::kind;
int RegFile::int_size;
int RegFile::fp_size;
int RegFile::xmm_size;
int RegFile::int_local_size;
int RegFile::fp_local_size;
int RegFile::xmm_local_size;


RegFile::RegFile()
{

}


void RegFile::ParseConfiguration(const std::string &section,
				misc::IniFile &config)
{
	kind = (Kind)config.ReadEnum(section, "RfKind",
			KindMap, KindPrivate);
	int_size = config.ReadInt(section, "RfIntSize", 80);
	fp_size = config.ReadInt(section, "RfFpSize", 40);
	xmm_size = config.ReadInt(section, "RfXmmSize", 40);

	int threads_num = config.ReadInt("General", "Threads", 1);

	if (int_size < RegFileMinINTSize)
		misc::fatal("rf_int_size must be at least %d", RegFileMinINTSize);
	if (fp_size < RegFileMinFPSize)
		misc::fatal("rf_fp_size must be at least %d", RegFileMinFPSize);
	if (xmm_size < RegFileMinXMMSize)
		misc::fatal("rf_xmm_size must be at least %d", RegFileMinXMMSize);

	if (kind == KindPrivate)
	{
		int_local_size = int_size;
		fp_local_size = fp_size;
		xmm_local_size = xmm_size;
	}
	else
	{
		int_local_size = int_size * threads_num;
		fp_local_size = fp_size * threads_num;
		xmm_local_size = xmm_size * threads_num;
	}
}

}
