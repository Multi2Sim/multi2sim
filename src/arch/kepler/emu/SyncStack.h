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

#ifndef ARCH_KEPLER_EMU_SYNCSTACK_H
#define ARCH_KEPLER_EMU_SYNCSTACK_H

#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>

namespace Kepler
{

enum EntryType
{
        NOTCONTROL = 0,
        BRA = 1,
        SSY,
        PBK,
        PCNT
};

enum MaskType
{
        NOTMASK = 0,
        BRK = 1,
        CONT,
        EXIT
};

struct SyncStackEntry
{

        // acitve masks to be restored
        unsigned int active_thread_mask;

        // entry type. Used in individual type operation.
        EntryType entry_type;

        SyncStackEntry()
        {
        	active_thread_mask = unsigned(-1);
        	entry_type = NOTCONTROL;
        }
        SyncStackEntry(unsigned rec, unsigned am, EntryType et)
        {
        	active_thread_mask = am;
        	entry_type = et;
        }
};

// A result value used by popSyncStack, with a bool to validate it.
struct PopResult
{
	bool find;
	unsigned active_mask;

	PopResult():find(false), active_mask(0x0){}

	PopResult(bool find, unsigned active_mask)
	{
		this->find = find;
		this->active_mask = active_mask;
	}
};

// Main class of synchronization stack
class SyncStack
{
        typedef std::unique_ptr<SyncStackEntry> handler;
        typedef std::list<handler>::iterator list_it;

        // Modeled as a list, instead of a stack, recording the active mask
        // and entry type.
        // Each element of it is mapped with a reconvergence pc in hash table.
        std::list<handler> sync_stack;

        // a hash table to check before every instruction
        // if the current pc is recorded as a reconvergence pc.
        // every pair is combined with a reconvergence pc
        // and an iterator to a specific list element of sync_stack
        std::unordered_multimap<unsigned, list_it> hash_table;

public:

        SyncStack(){}

        /// push the reconvergence pc, active mask and entry type
        /// into stack
        void pushSyncStack(unsigned, unsigned, EntryType);

        /// FIXME push an empty entry into stack, for testing.
        void pushSyncStack();

        /// search the address in hash table, "or" all active masks found
        /// return it with a bool to indicate find or not
        /// and delete the found element both in hash table and list
        PopResult popSyncStack(unsigned);

        /// Used by BRK and EXIT.
        /// The former sets the specific bit of
        /// all active masks in stack before a PBK entry
        /// The later sets the bit of all entries in stack.
        /// \param The number of bit to be set
        void MaskSyncStack(unsigned, MaskType);

        unsigned checkSyncStack(unsigned);

        void Dump(std::ostream &os = std::cout) const;

        /// Dump synchronization stack into output stream
        /*
        friend std::ostream &operator<<(std::ostream &os,
			const SyncStack &sync_stack) {
        	sync_stack.Dump(os);
        	return os;
        }
        */

};

}	// namespace

#endif

