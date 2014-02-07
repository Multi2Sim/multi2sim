/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "Emu.h"
#include "Grid.h"
#include "machine.h"
#include "warp.h"
#include "thread-block.h"

namespace Kepler
{
/*
 * Class 'KplEmu'
 */

KplEmu::KplEmu(Asm *as)
{

}

void KplEmu::Dump(std::ostream &os = std::cout) const
{

}


void KplEmu::KplEmuDumpSummary(std::ostream &os = std::cout)
{
	/* Call parent */

}


int KplEmu::KplEmuRun()
{
	return true;
}


void KplEmu::KplEmuConstMemWrite(unsigned addr, void *value_ptr)
{
	mem_write(this->const_mem, addr, sizeof(unsigned), value_ptr);
}


void KplEmu::KplEmuConstMemRead(unsigned addr, void *value_ptr)
{
	mem_read(this->const_mem, addr, sizeof(unsigned), value_ptr);
}


/*
 * Non-Class Stuff
 */
/*
long long kpl_emu_max_cycles;
long long kpl_emu_max_inst;
int kpl_emu_max_functions;

const int kpl_emu_warp_size = 32;
*/
}

