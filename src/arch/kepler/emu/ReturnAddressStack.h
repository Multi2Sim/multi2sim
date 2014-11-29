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

#ifndef SRC_ARCH_KEPLER_EMU_RETURNADDRESSSTACK_H_
#define SRC_ARCH_KEPLER_EMU_RETURNADDRESSSTACK_H_

#include <cassert>

#include "SyncStack.h"

namespace Kepler
{

// Main class of ReturnAddressStack
class ReturnAddressStack
{
        class ReturnAddressStackEntry
        {

        		// Counter, used for debug
        		unsigned counter;

                // Return Address
                unsigned return_address;

                // Active mask
                unsigned active_mask;

                // SyncStack handler
                std::unique_ptr<SyncStack> sync_stack;

        public:

                ReturnAddressStackEntry(unsigned ret, unsigned active_mask,
                		std::unique_ptr<SyncStack>& ss, unsigned counter)
                {
                	this->active_mask = active_mask;
                	this->counter = counter;
                	return_address = ret;
                	sync_stack = std::move(ss);
                }

                ReturnAddressStackEntry(ReturnAddressStackEntry &entry)
                {
                	this->active_mask = active_mask;
                	this->counter = entry.getCounter();
                	this->return_address = entry.getReturnAddress();
                	this->sync_stack = std::move(*(entry.getSyncStack()));
                }

                std::unique_ptr<SyncStack>* getSyncStack() { return &sync_stack; }

                /// Get return address
                unsigned getReturnAddress() { return return_address; }

                /// Get active mask
                unsigned getActiveMask() { return active_mask; }

                /// Get counter
                unsigned getCounter() { return counter; }
        };


        // A counter recording every sync stack "id"
     	static unsigned common_counter;

        // Modeled the stack as a list, recording the return address of CAL
        // and the sync stack of all previous contexts.
        std::list<std::unique_ptr<ReturnAddressStackEntry>> ret_stack;

public:


        /// Get sync stack of top entry
        std::unique_ptr<SyncStack>* getTopSyncStack() const
		{
        	assert(ret_stack.size());
        	return ret_stack.begin()->get()->getSyncStack();
		}

        /// Get return address of top entry
        unsigned getActiveMask() const
        {
        	assert(ret_stack.size());
        	return ret_stack.begin()->get()->getActiveMask();
        }

        /// Get return address of top entry
        unsigned getReturnAddress() const
        {
        	assert(ret_stack.size());
        	return ret_stack.begin()->get()->getReturnAddress();
        }

        /// push the return address and sync stack handler
        /// into return addrress stack
        /// \param ret_addr the return address to be stored
        /// \param ss the sync stack's handler to be stored
        void push(unsigned ret_addr, unsigned am, std::unique_ptr<SyncStack>& ss);

        /// This function pops the top entry of the return address stack
        /// \return false if return address stack is empty before or after popping.
        bool pop();

        void Dump(std::ostream &os = std::cout) const;

        /// Dump synchronization stack into output stream
        /*
        friend std::ostream &operator<<(std::ostream &os,
                        const ReturnAddressStack &sync_stack) {
                sync_stack.Dump(os);
                return os;
        }
        */

};

}        // namespace

#endif
