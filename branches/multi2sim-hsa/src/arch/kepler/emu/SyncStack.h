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

enum SyncStackEntryType
{
        SyncStackEntryNotControl,
        SyncStackEntryBRA,
        SyncStackEntrySSY,
        SyncStackEntryPBK,
        SyncStackEntryPCNT,
        SyncStackEntryCAL
};

enum SyncStackMaskType
{
        SyncStackMaskNotMask,
        SyncStackMaskBRK,
        SyncStackMaskCONT,
        SyncStackMaskEXIT
};


// Main class of synchronization stack
class SyncStack
{
        class SyncStackEntry
        {
                // reconvergence pc
                unsigned reconv_pc;

                // acitve masks to be restored
                unsigned int active_thread_mask;

                // entry type. Used in individual type operation.
                SyncStackEntryType entry_type;

        public:

                SyncStackEntry()
                {
                		reconv_pc = 0u;
                		active_thread_mask = unsigned(-1);
                		entry_type = SyncStackEntryNotControl;
                }

                SyncStackEntry(unsigned rec, unsigned am, SyncStackEntryType et)
                {
                    	reconv_pc = rec;
                    	active_thread_mask = am;
                    	entry_type = et;
                }

                unsigned getActiveThreadMask() { return active_thread_mask; }

                unsigned getEntryType() { return entry_type; }

                unsigned getRenconvPC() { return reconv_pc; }

                void setActiveThreadMask(unsigned am)
                {
                		active_thread_mask = am;
                }

                /// It compares the entry's type with param.
                /// return true if type matches.
                bool compareEntryType(SyncStackEntryType type)
                {
                        return type == entry_type;
                }

                /// clear the corresponding bit in active thread mask
                void MaskBit(unsigned id_in_warp)
                {
                        active_thread_mask &= ~(1u << id_in_warp);
                }

                /// Set the corresponding bit in active thread mask
                void SetBit(unsigned id_in_warp)
                {
                        active_thread_mask |= (1u << id_in_warp);
                }
        };

        // Modeled as a list, instead of a stack, recording the active mask
        // and entry type.
        // Each element of it is mapped with a reconvergence pc in hash table.
        // This list is used to keep track of pushing order of reconvergence pcs.
        // It is easy to find all entries before a PBK entry,
        // once BRK wants to mask a specific bit of them all.
        std::list<std::unique_ptr<SyncStackEntry>> sync_stack;

        // It is a hash table used to check before every instruction
        // whether the current pc is recorded as a reconvergence pc.
        // Every pair is combined with a reconvergence pc
        // and an iterator to a specific list element of sync_stack
        std::unordered_multimap<unsigned, std::list<std::unique_ptr<SyncStackEntry>>::iterator> hash_table;

        // number of thread in warp
        unsigned thread_count;

        // current active mask
        unsigned active_mask;

        // A temp but must be stored entry of sync_stack
        // used by BRA to set individual bit of active mask
        unsigned temp_mask;

public:

        SyncStack(unsigned thread_count)
        {
        	this->thread_count = thread_count;
        	temp_mask = 0;
        	active_mask = (thread_count < 32) ?
        			 ((1u << thread_count) - 1) : unsigned(-1);
        }

        /// Get temp mask
        unsigned getTempMask() const { return temp_mask; }

        /// Get acitive mask
        unsigned getActiveMask() const { return active_mask; }

        /// Set active mask
        void setActiveMask(unsigned am) { active_mask = am; }

        /// Set temp mask bit
        /// \param num
        /// the bit number to be set
        void setTempMaskBit(unsigned num)
        {
        	temp_mask |= 1u << num;
        }

        /// Initial (or reset) temp_entry
        void resetTempMask()
        {
        	temp_mask = 0u;
        }

        /// Get number of 1s in active mask
        unsigned getActiveMaskBitCount() const;

        /// Get the number of 1s in temp mask
        unsigned getTempMaskBitCount() const;

        /// Clear specific bit of current active mask
        void clearActiveMaskBit(unsigned id) { active_mask &= ~(1u << id); }

        /// push the reconvergence pc, active mask and entry type
        /// into stack
        void push(unsigned reconv_pc,
                        unsigned active_mask,
                        SyncStackEntryType entry_type
                        );

        /// FIXME push an empty entry into stack, for testing.
        void push();

        /// This function searches the current pc in hash table,
        /// and "or" all active masks found.
        /// Then it deletes all found elements both in hash table and list
        /// return a bool to indicate whether pc is find or not.
        /// \param address the current pc to be searched
        /// \param active_mask the returned active mask
        /// \return whether or not this pc is found in hash table
        bool pop(unsigned address, unsigned& active_mask);

        /// Used by BRK, CONT and EXIT.
        /// In BRK case, it clears the specific bit of
        /// all active masks in stack before a PBK entry
        /// In EXIT case, it clears the bit of all entries in stack.
        /// And in CONT case, it finds the first PCNT entry in stack,
        /// and sets the specific bit.
        /// \param id_in_warp The number of bit to be set
        /// \param type instruction type
        void mask(unsigned id_in_warp, SyncStackMaskType type);

        /// Used by BRA, BRK, CONT
        /// When no thread or all threads take the instruction,
        /// pc is updated to a specific value.
        /// This function pops all entries with addresses
        /// between target pc and current pc.
        /// You do not have worried about which pc is larger.
        /// This function will detect that.
        /// \return true if pop at least one element.
        bool popTillTarget(unsigned target_pc, unsigned current_pc);

        /// Used by BRK, check if all threads taken the loop have break.
        /// \param address Return the first PBK reconvergence pc
        bool checkBRK(unsigned& address);

        /// Used by CONT, check if all threads taken the loop have continued.
        /// \param address Return the first PCNT reconvergence pc
        bool checkCONT(unsigned& address);

        /// Used by RET, check if the sync stack top has a CAL entry
        /// \param address Return the first CAL entry's pc
        /// \param address Return the first CAL entry's active mask
        bool checkCAL(unsigned& address, unsigned& active_mask);

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

}        // namespace

#endif
