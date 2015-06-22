/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cassert>
#include <unordered_map>

#include <lib/cpp/Error.h>

#include "SyncStack.h"

namespace Kepler
{

unsigned SyncStack::getTempMaskBitCount() const
{
        unsigned num = 0;

        for (unsigned i = 0; i < thread_count; i++)
                num += (temp_mask >> i) & 1u;

        assert(num <= thread_count);

        return num;
}


unsigned SyncStack::getActiveMaskBitCount() const
{
        unsigned num = 0;

        for (unsigned i = 0; i < thread_count; i++)
                num += (active_mask >> i) & 1u;

        assert(num <= thread_count);

        return num;
}


void SyncStack::push(unsigned reconv_pc,
						unsigned active_mask,
						SyncStackEntryType entry_type)
{
	std::list<std::list<std::unique_ptr<SyncStackEntry>>::iterator> no_bra;

	SyncStackEntry* entry = new SyncStackEntry(reconv_pc,
									active_mask,
									entry_type);

	std::unique_ptr<SyncStackEntry> temp(entry);
	sync_stack.push_front(std::move(temp));
	hash_table.emplace(reconv_pc, sync_stack.begin());

	// We assume that two entries, except BRA,
	// cannot have the same reconv_pc
	auto range = hash_table.equal_range(reconv_pc);
	for (auto i = range.first; i != range.second; ++i)
	{
		if (!i->second->get()->compareEntryType(SyncStackEntryBRA)
				&& !i->second->get()->compareEntryType(SyncStackEntryCAL))
			no_bra.push_back(i->second);
	}

	assert(no_bra.size() < 2);
}


void SyncStack::push()
{
	push(0x0, 0x0, SyncStackEntryNotControl);
}


bool SyncStack::pop(unsigned address, unsigned& active_mask)
{
	bool ret = false;

	auto range = hash_table.equal_range(address);

	active_mask = 0;

	ret = range.first != range.second;

	for (auto i = range.first; i != range.second; ++i)
	{
		active_mask |= i->second->get()->getActiveThreadMask();
		sync_stack.erase(i->second);
	}

	hash_table.erase(range.first, range.second);

	return ret;
}

bool SyncStack::popTillTarget(unsigned target_pc, unsigned current_pc)
{
	bool ret = false;

	unsigned smaller_pc = target_pc > current_pc ? current_pc : target_pc;
	unsigned larger_pc = target_pc <= current_pc ? current_pc : target_pc;

	std::list<unsigned> address;
	for (auto t = hash_table.begin(); t != hash_table.end(); ++t)
	{

		if (t->first < larger_pc && t->first > smaller_pc)
		{
			ret = true;
			sync_stack.erase(t->second);
			address.push_back(t->first);
		}
	}

	for (auto t = address.begin(); t != address.end(); ++t)
	{
		hash_table.erase(*t);
	}

	return ret;
}


void SyncStack::mask(unsigned id_in_warp, SyncStackMaskType type)
{
	bool find = false;

	for(auto i = sync_stack.begin(); i != sync_stack.end(); ++i)
	{
			if (i->get()->compareEntryType(SyncStackEntryPBK)
					&& type == SyncStackMaskBRK)
			{
				find = true;
				break;
			}
			else
			{
				if (i->get()->compareEntryType(SyncStackEntryPCNT)
						&& type == SyncStackMaskCONT)
				{
					i->get()->SetBit(id_in_warp);
					find = true;
					break;
				}
				else
				{
					i->get()->MaskBit(id_in_warp);
				}
			}

			// clear the bit in every entry before a PBK
	}

	if (!find && type != SyncStackMaskEXIT )
		throw misc::Panic("Coding error, invalid mask type\n");
}


bool SyncStack::checkBRK(unsigned& address)
{
	unsigned active_PBK = 0;
	unsigned active = 0;
	bool find_PBK = false;

	for (auto i = sync_stack.begin(); i != sync_stack.end(); ++i)
	{
		if (i->get()->compareEntryType(SyncStackEntryPBK))
		{
			// check if all threads in loop are broken.
			active_PBK = i->get()->getActiveThreadMask();
			address = i->get()->getRenconvPC();
			find_PBK = true;
			break;
		}

		active |= i->get()->getActiveThreadMask();
	}

	if (!find_PBK)
		misc::Panic("No PBK is found in sync stack.\n");

	return active_PBK == (active_PBK & ~active);
}


bool SyncStack::checkCONT(unsigned& address)
{
	unsigned active_PBK = 0;
	unsigned active_PCNT = 0;
	unsigned active = 0;
	bool find_PCNT = false;
	bool find_PBK = false;

	for (auto i = sync_stack.begin(); i != sync_stack.end(); ++i)
	{

		if (i->get()->compareEntryType(SyncStackEntryPCNT))
		{
			active_PCNT = i->get()->getActiveThreadMask();

			if (!find_PCNT)
			{
                find_PCNT = true;
                address = i->get()->getRenconvPC();
			}
			else
			{
                misc::Panic("Two PCNTs are found without PBK in sync stack.\n");
			}
		}
		else if (i->get()->compareEntryType(SyncStackEntryPBK))
		{
			active_PBK = i->get()->getActiveThreadMask();
			find_PBK = true;
			break;
		}

		active |= i->get()->getActiveThreadMask();

	}

	if(!find_PCNT || !find_PBK)
		misc::Panic("Cannot find PCNT PCNT or PBK in sync stack.\n");

	// return true, if every thread in warp is either "continued" or "broken".
	// A thread is regarded broken when all corresponding bits in entries
	// above the first PBK entry is 0 (in real case, include BRK and EXIT).
	// A thread is continued when the bit in first PCNT active mask is 1.
	return ((~active & active_PBK) ^ active_PCNT) == active_PBK;
}


bool SyncStack::checkCAL(unsigned& address, unsigned& active_mask)
{

	bool ret = false;

	for (auto i = sync_stack.begin(); i != sync_stack.end(); ++i)
	{

		if (i->get()->compareEntryType(SyncStackEntryCAL))
		{
			active_mask = i->get()->getActiveThreadMask();
			address = i->get()->getRenconvPC();
			ret = true;
		}
	}


	return ret;
}


void SyncStack::Dump(std::ostream &os) const
{
	os << "Reconv_pc	" << "active mask	"
				<< "entry type"<< std::endl;

	for (auto &pair : hash_table)
	{
		os << std:: hex << pair.first << "		";
		os << pair.second->get()->getActiveThreadMask()
				<< "		" << pair.second->get()->getEntryType()
				<< std::endl;
	}

	if (sync_stack.size() != hash_table.size())
		throw misc::Panic("Error mapping list and hash table\n");
}

} // namespace
