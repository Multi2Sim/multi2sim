

#include <cassert>
#include <unordered_map>

#include <lib/cpp/Error.h>

#include "SyncStack.h"

namespace Kepler
{

void SyncStack::pushSyncStack(unsigned reconv_pc,
							unsigned active_mask,
							EntryType entry_type)
{
	std::list<list_it> no_bra;

	SyncStackEntry* entry = new SyncStackEntry(reconv_pc,
									active_mask,
									entry_type);
	handler temp(entry);
	sync_stack.push_front(std::move(temp));
	hash_table.emplace(reconv_pc, sync_stack.begin());

	// We assume that two entries, except BRA,
	// cannot have the same reconv_pc
	auto range = hash_table.equal_range(reconv_pc);
	for (auto i = range.first; i != range.second; ++i)
	{
		if (i->second->get()->entry_type != BRA)
			no_bra.push_back(i->second);
	}

	assert(no_bra.size() < 2);
}


void SyncStack::pushSyncStack()
{
	pushSyncStack(0x0, 0x0, NOTCONTROL);
}


PopResult SyncStack::popSyncStack(unsigned address)
{
	PopResult ret;

	auto range = hash_table.equal_range(address);

	ret.find = range.first != range.second;

	for (auto i = range.first; i != range.second; ++i)
	{
		ret.active_mask |= i->second->get()->active_thread_mask;
		sync_stack.erase(i->second);
	}

	hash_table.erase(range.first, range.second);

	return ret;
}


void SyncStack::MaskSyncStack(unsigned id_in_warp, MaskType type)
{
	for(auto i = sync_stack.begin(); i != sync_stack.end(); ++i)
	{
			if (i->get()->entry_type == PBK && type == BRK)
			{
                return;
			}
			else
			{
				if (i->get()->entry_type == PCNT && type == CONT)
				{
					i->get()->active_thread_mask |= (1u << id_in_warp);
					return;
				}
			}

			// clear the bit in every entry before a PBK
			i->get()->active_thread_mask &= ~(0x1u << id_in_warp);
	}

	if (type != EXIT && sync_stack.size())
		throw misc::Panic("Coding error, invalid mask type\n");
}


void SyncStack::Dump(std::ostream &os) const
{
	os << "Reconv_pc	" << "active mask	"
				<< "entry type"<< std::endl;

	for (auto &pair : hash_table)
	{
		os << std:: hex << pair.first << "		";
		os << pair.second->get()->active_thread_mask
				<< "		" << pair.second->get()->entry_type << std::endl;
	}

	if (sync_stack.size() != hash_table.size())
		throw misc::Panic("Error mapping list and hash table\n");
}

} // namespace
