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

#include <climits>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "Frame.h"
#include "Module.h"
#include "System.h"


namespace mem
{


Module::Module(const std::string &name,
		Type type,
		int num_ports,
		int block_size,
		int data_latency)
		:
		name(name),
		type(type),
		block_size(block_size),
		data_latency(data_latency),
		num_ports(num_ports)
{
	// Create 'num_ports' in vector of ports
	ports.resize(num_ports);

	// Block size
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	log_block_size = misc::LogBase2(block_size);
}


bool Module::ServesAddress(unsigned address) const
{
	// Address bounds
	if (range_type == RangeBounds)
		return address >= range.bounds.low &&
				address <= range.bounds.high;

	// Interleaved addresses
	if (range_type == RangeInterleaved)
		return (address / range.interleaved.div) %
				range.interleaved.mod ==
				range.interleaved.eq;

	// Invalid
	throw misc::Panic("Invalid range type");
}


Module *Module::getLowModuleServingAddress(unsigned address) const
{
	// The address must be served by the current module
	assert(ServesAddress(address));

	// Main memory does not have a low module
	if (type == TypeMainMemory)
	{
		assert(!low_modules.size());
		return nullptr;
	}

	// Check which low module serves address
	Module *server_module = nullptr;
	for (Module *low_module : low_modules)
	{
		// Skip if this low module doesn't serve address
		if (!low_module->ServesAddress(address))
			continue;

		// Address served by more than one module
		if (server_module)
			throw Error(misc::fmt("%s: low modules '%s' "
					"and '%s' both serve address 0x%x",
					name.c_str(),
					server_module->getName().c_str(),
					low_module->getName().c_str(),
					address));

		// Assign serving module
		server_module = low_module;
	}

	// Error if no low module serves address
	if (!server_module)
		throw Error(misc::fmt("Module %s: no lower module "
				"serves address 0x%x",
				name.c_str(),
				address));

	// Return server module
	return server_module;
}


Module *Module::getOwner(int set_id, int way_id, int sub_block_id) const
{
	// Get directory entry
	assert(directory.get());
	Directory::Entry *entry = directory->getEntry(set_id,
			way_id,
			sub_block_id);

	// Get owner ID
	assert(entry);
	int owner = entry->getOwner();

	// No owner
	if (owner == Directory::NoOwner)
		return nullptr;
	
	// Get node
	net::Node *node = high_network->getNode(owner);
	assert(node);

	// Return owner module
	return (Module *) node->getUserData();
}


bool Module::canAccess(int address) const
{
	// There must be a free port
	assert(num_locked_ports <= num_ports);
	if (num_locked_ports == num_ports)
		return false;

	// If no MSHR is given, module can be accessed
	if (!mshr_size)
		return true;

	// Module can be accessed if number of non-coalesced in-flight accesses
	// is smaller than the MSHR size.
	int num_non_coalesced_accesses = accesses.size() -
			num_coalesced_accesses;
	return num_non_coalesced_accesses < mshr_size;
}


long long Module::Access(AccessType access_type,
		unsigned address,
		int *witness,
		esim::Event *return_event)
{
	// Create a new event frame
	auto frame = misc::new_shared<Frame>(
			Frame::getNewId(),
			this,
			address);
	frame->witness = witness;

	// Select initial event type
	esim::Event *event;
	switch (type)
	{

	case TypeCache:
	case TypeMainMemory:

		switch (access_type)
		{

		case AccessLoad:

			event = System::event_load;
			break;

		case AccessStore:

			event = System::event_store;
			break;

		case AccessNCStore:

			event = System::event_nc_store;
			break;

		default:

			throw misc::Panic("Invalid access type");
		}
		break;

	case TypeLocalMemory:

		switch (access_type)
		{

		case AccessLoad:

			event = System::event_local_load;
			break;

		case AccessStore:

			event = System::event_local_store;
			break;

		default:

			throw misc::Panic("Invalid access type");
		}
		break;

	default:

		throw misc::Panic("Invalid module type");
	}

	// Schedule event
	esim::Engine *esim_engine = esim::Engine::getInstance();
	esim_engine->Call(event, frame, return_event);

	// Return frame ID
	return frame->getId();
}


void Module::StartAccess(Frame *frame, AccessType access_type)
{
	// Record access type
	frame->access_type = access_type;

	// Insert in access list
	frame->accesses_iterator = accesses.insert(accesses.end(),
			frame);

	// Insert in write access list
	if (access_type == AccessStore)
		frame->write_accesses_iterator = write_accesses.insert(
				write_accesses.end(),
				frame);

	// Insert in hash table of block addresses
	unsigned block_address = frame->getAddress() >> log_block_size;
	in_flight_block_addresses.emplace(std::make_pair(block_address, frame));

	// Insert in set of access identifiers
	in_flight_access_ids.emplace(frame->getId());
}


void Module::FinishAccess(Frame *frame)
{
	// Remove from access list
	accesses.erase(frame->accesses_iterator);
	frame->accesses_iterator = accesses.end();

	// Remove from write access list
	assert(frame->access_type);
	if (frame->access_type == Module::AccessStore)
	{
		write_accesses.erase(frame->write_accesses_iterator);
		frame->write_accesses_iterator = write_accesses.end();
	}

	// Remove from hash table
	unsigned block_address = frame->getAddress() >> log_block_size;
	auto range = in_flight_block_addresses.equal_range(block_address);
	bool found = false;
	for (auto it = range.first; it != range.second; ++it)
	{
		Frame *other_frame = it->second;
		if (frame == other_frame)
		{
			found = true;
			in_flight_block_addresses.erase(it);
			break;
		}
	}

	// Must have been removed
	if (!found)
		throw misc::Panic("Frame not found");

	// Remove from set of in-flight access identifiers
	in_flight_access_ids.erase(frame->getId());

	// If this was a coalesced access, update counter
	if (frame->coalesced)
	{
		assert(num_coalesced_accesses > 0);
		num_coalesced_accesses--;
	}

	// When a frame finishes its access, we need to check each of the frames
	// in the access list and make sure that no remaining frames use the 
	// finished frame as a master frame. If they are, their master frame
	// pointer is resest to null. This is to prevent a situation where the
	// finishing frame makes a later access and adopts a master frame from
	// a frame in the access list which points to itself.
	for (auto it = accesses.begin(); it != accesses.end(); it++)
	{
		Frame *list_frame = *it;
		if (list_frame->master_frame == frame)
			list_frame->master_frame = nullptr;
	}

	// Wake up dependent accesses
	frame->queue.WakeupAll();
}


Frame *Module::getInFlightAddress(unsigned address,
		Frame *older_than_frame)
{
	// Look for address
	unsigned block_address = address >> log_block_size;
	auto range = in_flight_block_addresses.equal_range(block_address);
	for (auto it = range.first; it != range.second; ++it)
	{
		// Get frame
		Frame *frame = it->second;

		// This frame is not older than 'older_than_frame'
		if (older_than_frame && frame->getId() >=
				older_than_frame->getId())
			continue;

		// Block address matches
		assert(frame->getAddress() >> log_block_size ==
				address >> log_block_size);
		return frame;
	}

	// Not found
	return nullptr;
}


Frame *Module::getInFlightWrite(Frame *older_than_frame)
{
	// No 'older_than_frame' given, return youngest write, or nullptr if
	// there is no in-flight write.
	if (!older_than_frame)
		return write_accesses.size() ?
				write_accesses.back() :
				nullptr;
	
	// Search
	auto it = older_than_frame->accesses_iterator;
	while (it != accesses.begin())
	{
		// Rewind
		--it;

		// Check if it is a write
		Frame *frame = *it;
		if (frame->access_type == AccessStore)
			return frame;
	}

	// Not found
	return nullptr;
}


bool Module::isInFlightAddress(unsigned address)
{
	unsigned block_address = address >> log_block_size;
	auto it = in_flight_block_addresses.find(block_address);
	return it != in_flight_block_addresses.end();
}


bool Module::isInFlightAccess(long long id)
{
	auto it = in_flight_access_ids.find(id);
	return it != in_flight_access_ids.end();
}


void Module::Dump(std::ostream &os) const
{
	// First we have to calculate the largest string that will be presented
	// in the table, and create the size of elements of the table according
	// to this string. Initial element size is the string "Module <name>"
	unsigned element_size = name.length() + 7;

	// Get the number of sets and ways
	unsigned sets = cache->getNumSets();
	unsigned ways = cache->getNumWays();
	int sub_blocks = directory->getNumSubBlocks();

	// Also we create a list with integers for the maximum number of 
	// sharers per set
	std::vector<int> max_sharers;
	max_sharers.resize(cache->getNumSets());

	for (unsigned set = 0; set < sets; set++)
	{
		for (unsigned way = 0; way < ways; way++)
		{
			// Get the block
			Cache::Block *block = cache->getBlock(set,way);

			// If block state is invalid continue
			if (!block->getState())
				continue;

			// Start the element size with the tag of the block
			if (element_size < std::to_string(
					block->getTag()).length())
				element_size = std::to_string(
						block->getTag()).length();

			// Sharers field size
			unsigned entry_text_size = 0;
			
			// find modules that are sharers/owner of the 
			// directory entries. 
			for (int sub_block_id = 0; 
					sub_block_id < sub_blocks;
					sub_block_id++)
			{
				// Number of sharers per sub_block
				int sharers = 0;

				// Add the name of the sharer + two spaces
				for (auto &high_module : high_modules)
				{
					int index = getSharerIndex(high_module);
					if (directory->isSharer((int) set, (int) way,
							sub_block_id, index))
					{
						if (entry_text_size < high_module->
								getName().length())
							entry_text_size = high_module->
									getName().
									length();
						sharers++;
					}
				}

				// Increase the maximum number of sharers
				// per set if it is required
				if (max_sharers[set] < sharers)
					max_sharers[set] = sharers;
			}

			// Check if this string is bigger than the previous
			// maximum value
			if (element_size < entry_text_size)
				element_size = entry_text_size;

		}
	}

	// Based on the element size construct the dump table
	// No matter what, we would add extra space to the element size
	element_size += 2;

	// We start drawing the table based on the element size
	// ******************************************
	// * Module <name>			    *
	// ******************************************
	os << "(The Dump is best viewed when text "
			"wrapping is disabled)\n";
	os << std::string((ways * sub_blocks + 2)* element_size, '=') << 
			"\n";
	os << "| Module " << name.c_str() <<
			std::string((ways * sub_blocks + 2) * element_size - 10 -
			name.length(), ' ') << "|" << "\n";
	os << std::string((ways * sub_blocks + 2)* element_size, '=') << 
			"\n";

	// Here we start printing
	int pad_size = 0;

	// The first line is the number of ways. The first two elements 
	// in this row are empty.
	os << "|" << std::string(element_size * 2 - 2 , ' ') << "|";
	for (unsigned i = 0; i < ways; i++)
	{
		// Print the way index
		std::string way_id = "Way " + std::to_string(i);

		// Empty space excluding | and the way index
		pad_size = (sub_blocks * element_size) - 2 - way_id.length();
		os << "|" <<
				std::string(pad_size/2, ' ') <<
				way_id.c_str() <<
				std::string((sub_blocks * element_size) - 2 
				- way_id.length() - pad_size/2, ' ') <<
				"|";
	}

	// Separating the first row
	os <<"\n";
	os << std::string((sub_blocks * ways + 2)* element_size, '=') << 
			"\n";
	for (unsigned set = 0; set < sets; set++)
	{
		// The row information
		std::string set_id = "Set " + std::to_string(set);

		// Print the set index
		os << "| " <<
				std::left <<
				std::setw(element_size - 3) <<
				std::setfill(' ') <<
				set_id.c_str() <<
				"|";

		// The first row is the state (which is 5 letters)
		pad_size = element_size - 2 - 5;
		os << "|" <<
				std::string(pad_size/2, ' ') <<
				"State" <<
				std::string(element_size - 7 - 
				pad_size/2, ' ') <<
				"|";

		// Printing the state of the block
		for (unsigned way = 0; way < ways; way++)
		{
			// Get the block
			Cache::Block *block = cache->getBlock(set, way);

			// Print the state
			pad_size = sub_blocks * element_size - 3;
			os << "|" <<
					std::string(pad_size/2, ' ') <<
					Cache::BlockStateMap[block->getState()] <<
					std::string(sub_blocks * element_size - 3 - 
					pad_size/2, ' ') <<
					"|";
		}

		// Internal separator
		os << "\n" << "|" << std::string(element_size - 1, ' ')
				<< std::string(element_size * 
				(ways * sub_blocks + 1),'-') << "\n";

		// The next row is the tag but the first column is empty
		os << "|" << std::string(element_size - 2 , ' ') << "|";

		// The second column prints Tag(which is 3 words)
		pad_size = element_size - 2 - 3;
		os << "|" <<
				std::string(pad_size/2, ' ') <<
				"Tag" <<
				std::string(element_size - 5 - 
				pad_size/2, ' ') <<
				"|";

		// Printing the tag of the block
		for (unsigned way = 0; way < ways; way++)
		{
			// Get the block
			Cache::Block *block = cache->getBlock(set, way);

			// If the block is invalid print empty
			if (!block->getState())
			{
				os << "|" << std::string(sub_blocks * element_size - 2, ' ')
						<< "|";
			}
			else
			{
				// Conduct the tag in Hex 
				std::stringstream stream;
				stream << std::hex << block->getTag();
				std::string tag = "0x" + stream.str();

				// Print the tag
				pad_size = sub_blocks * element_size - 
						2 - tag.length();
				os << "|" <<
						std::string(pad_size/2, ' ') <<
						tag.c_str() <<
						std::string(sub_blocks * 
						element_size - 2 - 
						tag.length() - pad_size/2, 
						' ') <<
						"|";
			}
		}
		// Internal separator
		os << "\n" << "|" << std::string(element_size - 1, ' ')
				<< std::string(element_size * 
				(sub_blocks * ways + 1),'-') << "\n";

		// Start printing sharers of the sub-blocks one sharer
		// per line
		std::vector<int> printed_counter;
		printed_counter.resize(cache->getNumWays() * directory->
				getNumSubBlocks());

		// Minimum number of rows for sharer is 1
		int sharer_rows = 1;
		if (sharer_rows < max_sharers[set])
			sharer_rows = max_sharers[set];
		for (int sharer_row = 0; sharer_row < sharer_rows; 
				sharer_row++)
		{
			// The next row is the sharers but the first column is empty
			os << "|" << std::string(element_size - 2 , ' ') << "|";

			// The second column prints Sharers(which is 7 words)
			if (sharer_row == 0)
			{
				pad_size = element_size - 2 - 7;
				os << "|" <<
						std::string(pad_size/2, ' ') <<
						"Sharers" <<
					std::string(element_size - 9 - 
					pad_size/2, ' ') <<
					"|";
			}
			else
			{
				os << "|" << std::string(element_size - 2, ' ')
						<< "|";
			}

			// For each entry
			for (unsigned way = 0; way < ways; way++)
			{
				for (int sub_block = 0; sub_block < sub_blocks;
						sub_block++)
				{
					// If the sub_block doesn't have a sharer
					// print empty and continue
					if (!getNumSharers(set, way, sub_block))
					{
						os << "|" << std::string(
						element_size - 2, ' ') << "|";
						continue;
					}
					// Counting the modules that are 
					// sharers
					int counting = 0;
					for (auto &high_module : high_modules)
					{
						// Get the number of printed sharer
						int printed = printed_counter[
								way * directory->
								getNumSubBlocks() +
								sub_block];

						// Find a sharer module
						int index = getSharerIndex(high_module);
						if (directory->isSharer((int) set, (int) way,
								sub_block,
								index))
						{
							// If the sharer module is
							// already printed continue
							// otherwise print and
							// exit the loop for
							// this sub-block
							if (printed > counting)
							{
								counting++;
								continue;
							}
							else
							{
								// Increase
								// printed
								// counter
								printed_counter[
								way * directory->
								getNumSubBlocks() +
								sub_block]++;
								
								// Print
								int length = 
										high_module->
										getName().
										length();
								pad_size = element_size - 
										2 -
										length;
								os << "|" <<
								std::string(pad_size/2, ' ') <<
								high_module->getName() <<
								std::string(element_size - 2 - 
								length - pad_size/2, ' ') <<
								"|";
								break;
							}
						}
					}
				}
			}
			// Internal separator
			os << "\n";
		}
		// Internal separator
		os << "|" << std::string(element_size - 1, ' ')
				<< std::string(element_size * 
				(sub_blocks * ways + 1),'-') << "\n";

		// The next row is the owner but the first column is empty
		os << "|" << std::string(element_size - 2 , ' ') << "|";

		// The second column prints Owner(which is 5 letters)
		pad_size = element_size - 2 - 5;
		os << "|" <<
				std::string(pad_size/2, ' ') <<
				"Owner" <<
				std::string(element_size - 7 - 
				pad_size/2, ' ') <<
				"|";

		// For each entry
		for (unsigned way = 0; way < ways; way++)
		{
			for (int sub_block = 0; sub_block < sub_blocks;
					sub_block++)
			{
				// If the sub_block doesn't have a owner
				// print empty
				Module *owner = getOwner(set, way, sub_block);
				if (!owner)
				{
					os << "|" << std::string(
					element_size - 2, ' ') << "|";
				}
				else
				{
					// Print
					int length = owner->getName().length();
					pad_size = element_size - 2 - length;
					os << "|" <<
							std::string(pad_size/2,
							' ') <<
							owner->getName() <<
							std::string(element_size - 
							2 - length - 
							pad_size/2, ' ') <<
							"|";
				}
			}
		}
		// Set separator
		os << "\n" << "|" << std::string((2 + ways * sub_blocks) * 
				element_size - 2, '=') << "|"
				<< "\n";
	}
}


void Module::DumpReport(std::ostream &os) const
{
	// Dumping module's name
	os << misc::fmt("[ %s ]\n\n", name.c_str());

	// Dump the cache related information
	if (type == TypeCache)
	{
		os << misc::fmt("Sets = %d\n", cache->getNumSets());
		os << misc::fmt("Ways = %d\n", cache->getNumWays());
		os << "ReplacementPolicy = " <<
				cache->ReplacementPolicyMap.MapValue(
				cache->getReplacementPolicy()) << "\n";
		os << "WritePolicy = " << cache->WritePolicyMap.MapValue(
				cache->getWritePolicy()) << "\n";
	}

	// Dump the module information
	os << misc::fmt("BlockSize = %d\n", block_size);
	os << misc::fmt("DataLatency = %d\n", data_latency);
	os << misc::fmt("Ports = %d\n", num_ports);
	os << "\n";

	// Statistics - Accesses
	os << misc::fmt("Accesses = %lld\n", num_accesses);
	os << misc::fmt("CoalescedAccesses = %lld\n", num_coalesced_reads +
			num_coalesced_writes + num_coalesced_nc_writes);
	os << misc::fmt("RetriedAccesses = %lld\n", num_retry_accesses);
	os << misc::fmt("Evictions = %lld\n", num_evictions);

	// Statistics - Hits and misses
	long long int num_hits = num_read_hits + num_write_hits 
			+ num_nc_write_hits;
	os << misc::fmt("Hits = %lld\n", num_hits);
	os << misc::fmt("Misses = %lld\n", num_accesses - num_hits);
	os << misc::fmt("HitRatio = %.4g\n", num_accesses ? 
			(double) num_hits / num_accesses : 0.0);
	os << "\n";

	// Statistics breakdown - Reads
	os << misc::fmt("Reads = %lld\n", num_reads);
	os << misc::fmt("CoalescedReads = %lld\n", num_coalesced_reads);
	os << misc::fmt("ReadHits = %lld\n", num_read_hits);
	os << misc::fmt("ReadMisses = %lld\n", num_reads - num_read_hits);
	os << misc::fmt("ReadRetries = %lld\n", num_retry_reads);
	os << misc::fmt("ReadRetryHits = %lld\n", num_retry_read_hits);
	os << misc::fmt("ReadRetryMisses = %lld\n", num_retry_read_misses);
	os << misc::fmt("BlockingReads = %lld\n", num_blocking_reads);
	os << misc::fmt("NonBlockingReads = %lld\n", num_non_blocking_reads);
	os << "\n";

	// Statistics breakdown - Writes
	os << misc::fmt("Writes = %lld\n", num_writes);
	os << misc::fmt("CoalescedWrites = %lld\n", num_coalesced_writes);
	os << misc::fmt("WriteHits = %lld\n", num_write_hits);
	os << misc::fmt("WriteMisses = %lld\n", num_writes - num_write_hits);
	os << misc::fmt("WriteRetries = %lld\n", num_retry_writes);
	os << misc::fmt("WriteRetryHits = %lld\n", num_retry_write_hits);
	os << misc::fmt("WriteRetryMisses = %lld\n", num_retry_write_misses);
	os << misc::fmt("BlockingWrites = %lld\n", num_blocking_writes);
	os << misc::fmt("NonBlockingWrites = %lld\n", num_non_blocking_writes);
	os << "\n";

	// Statistics breakdown - Non-coherent Writes
	os << misc::fmt("NCWrites = %lld\n", num_nc_writes);
	os << misc::fmt("CoalescedNCWrites = %lld\n", num_coalesced_nc_writes);
	os << misc::fmt("NCWriteHits = %lld\n", num_nc_write_hits);
	os << misc::fmt("NCWriteMisses = %lld\n", 
			num_nc_writes - num_nc_write_hits);
	os << misc::fmt("NCWriteRetries = %lld\n", num_retry_nc_writes);
	os << misc::fmt("NCWriteRetryHits = %lld\n", num_retry_nc_write_hits);
	os << misc::fmt("NCWriteRetryMisses = %lld\n", 
			num_retry_nc_write_misses);
	os << misc::fmt("BlockingNCWrites = %lld\n", num_blocking_nc_writes);
	os << misc::fmt("NonBlockingNCWrites = %lld\n", 
			num_non_blocking_nc_writes);
	os << "\n";

	// Statistics - Conflicts
	os << misc::fmt("DirectoryEntryConflicts = %lld\n", 
			num_directory_entry_conflicts);
	os << misc::fmt("RetryDirectoryEntryConflicts = %lld\n",
			num_retry_directory_entry_conflicts);
	if (type == TypeCache)
		os << misc::fmt("ConflictInvalidation = %lld\n",
				num_conflict_invalidations);
	
	// Separating line between modules
	os << "\n\n";
}

 
void Module::DumpInFlightAddresses(std::ostream &os)
{
	esim::Engine *engine = esim::Engine::getInstance();
	os << misc::fmt("[%s] In-flight blocks in cycle %lld:\n",
			name.c_str(), engine->getCycle());
	for (auto &pair : in_flight_block_addresses)
	{
		
		unsigned block_address = pair.first;
		Frame *frame = pair.second;
		os << misc::fmt("\tkey (block_address) = 0x%x: "
				"id = %lld, "
				"address = 0x%x, "
				"block_address = 0x%x\n",
				block_address,
				frame->getId(),
				frame->getAddress(),
				frame->getAddress() >> log_block_size);
		frame->CheckMagic();
		if (block_address != frame->getAddress() >> log_block_size)
			throw misc::Panic("Invalid block address");
	}
}


Frame *Module::canCoalesce(AccessType access_type,
		unsigned address,
		Frame *older_than_frame)
{
	// Nothing if there is no in-flight access
	if (!accesses.size())
		return nullptr;

	// For efficiency, first check in the hash table of accesses
	// whether there is an access in flight to the same block.
	assert(access_type);
	if (!getInFlightAddress(address, older_than_frame))
		return nullptr;

	// Nothing if 'older_than_frame' is in the head of the in-flight
	// access list (i.e., there is nothing older).
	if (older_than_frame->accesses_iterator == accesses.begin())
		return nullptr;
	
	// Get iterator to youngest access older than 'older_than_frame', or an
	// iterator to the overall youngest access if 'older_than_frame' is
	// null.
	auto tail = older_than_frame ?
			older_than_frame->accesses_iterator :
			accesses.end();
	assert(tail != accesses.begin());
	--tail;

	// Coalesce depending on access type
	switch (access_type)
	{

	case AccessLoad:
	{
		auto it = tail;
		while (true)
		{
			// Only coalesce with groups of reads at the tail
			Frame *frame = *it;
			if (frame->access_type != AccessLoad)
				return nullptr;

			// Same block address, coalesce
			if (frame->getAddress() >> log_block_size ==
					address >> log_block_size)
			{
				assert(!frame->master_frame ||
						!frame->master_frame->master_frame);
				return frame->master_frame ?
						frame->master_frame :
						frame;
			}
			
			// Done when head reached
			if (it == accesses.begin())
				break;

			// Previous
			--it;
		}
		break;
	}

	case AccessStore:
	{
		// Only coalesce with last access if it is a write
		Frame *frame = *tail;
		if (frame->access_type != AccessStore)
			return nullptr;
		
		// Only if it is an access to the same block
		if (frame->getAddress() >> log_block_size !=
				address >> log_block_size)
			return nullptr;

		// Only if previous write has not started yet
		if (frame->port_locked)
			return nullptr;

		// Coalesce
		assert(!frame->master_frame ||
				!frame->master_frame->master_frame);
		return frame->master_frame ?
				frame->master_frame :
				frame;
	}

	case AccessNCStore:
	{
		// Only coalesce with last access if it is a non-coherent write
		Frame *frame = *tail;
		if (frame->access_type != AccessNCStore)
			return nullptr;

		// Only if it is an access to the same block
		if (frame->getAddress() >> log_block_size !=
				address >> log_block_size)
			return nullptr;

		// Only if previous write has not started yet
		if (frame->port_locked)
			return nullptr;

		// Coalesce
		assert(!frame->master_frame ||
				!frame->master_frame->master_frame);
		return frame->master_frame ?
				frame->master_frame :
				frame;
	}

	default:

		throw misc::Panic("Invalid access type");
	}

	// No access found
	return nullptr;
}


void Module::Coalesce(Frame *master_frame, Frame *frame)
{
	// Debug
	System::debug << misc::fmt("    "
			"A-%lld is coalesced with A-%lld "
			"on %s for 0x%x\n",
			frame->getId(),
			master_frame->getId(),
			name.c_str(),
			frame->getAddress());

	// Master frame must not have a parent. We only want one level of
	// coalesced accesses.
	assert(!master_frame->master_frame);

	// Access must have been recorded already, which sets the access
	// kind to a valid value.
	assert(frame->access_type);

	// Set slave frame as a coalesced access
	frame->coalesced = true;
	frame->master_frame = master_frame;
	assert(num_coalesced_accesses <= (int) accesses.size());

	// Record in-flight coalesced access in module
	num_coalesced_accesses++;
}


void Module::LockPort(Frame *frame, esim::Event *event)
{
	// No free port
	if (num_locked_ports >= num_ports)
	{
		// Event chain must not be suspended in a queue
		assert(!frame->isInQueue());

		// If the request to lock the port is down-up, give it priority
		// since it is possibly holding up a large portion of the memory
		// hierarchy.
		port_queue.Wait(event, frame->request_direction ==
				Frame::RequestDirectionDownUp);
		return;
	}

	// Get free port
	int port_index;
	Port *port = nullptr;
	for (port_index = 0; port_index < num_ports; port_index++)
	{
		port = &ports[port_index];
		if (!port->frame)
			break;
	}

	// Lock port
	assert(port && port_index < num_ports);
	port->frame = frame;
	frame->port = port;
	num_locked_ports++;

	// Debug
	esim::Engine *esim_engine = esim::Engine::getInstance();
	System::debug << misc::fmt("    "
			"A-%lld locks port %d on %s\n",
			frame->getId(),
			port_index,
			name.c_str());

	// Schedule event
	esim_engine->Next(event);
}


void Module::UnlockPort(Port *port, Frame *frame)
{
	// Checks
	assert(num_locked_ports > 0);
	assert(frame->port && port->frame);
	assert(frame->port == port && port->frame == frame);
	assert(frame->getModule() == this);

	// Unlock port
	frame->port = nullptr;
	port->frame = nullptr;
	num_locked_ports--;

	// Debug
	System::debug << misc::fmt("    "
			"A-%lld unlocks port on %s\n",
			frame->getId(),
			name.c_str());

	// Check if there was any access waiting for free port
	if (port_queue.isEmpty())
		return;

	// Lock port with an access waiting for a free port
	frame = misc::cast<Frame *>(port_queue.getHead());
	port->frame = frame;
	frame->port = port;
	num_locked_ports++;

	// Wake up access
	port_queue.WakeupOne();
	
	// Debug
	System::debug << misc::fmt("    "
			"A-%lld locks port on %s\n",
			frame->getId(),
			name.c_str());
}


bool Module::FindBlock(unsigned address,
		int &set,
		int &way,
		int &tag,
		Cache::BlockState &state)
{
	// A transient tag is considered a hit if the block is locked in the
	// corresponding directory.
	tag = address & ~cache->getBlockMask();
	if (range_type == RangeInterleaved)
	{
		int num_modules = range.interleaved.mod;
		set = ((tag >> cache->getLogBlockSize()) / num_modules)
				% cache->getNumSets();
	}
	else if (range_type == RangeBounds)
	{
		set = (tag >> cache->getLogBlockSize())
				% cache->getNumSets();
	}
	else 
	{
		throw misc::Panic("Invalid range type");
	}

	// Find way in set
	for (way = 0; way < (int) cache->getNumWays(); way++)
	{
		// Get block
		Cache::Block *block = cache->getBlock(set, way);
		state = block->getState();

		// Permanent tag available with state other than invalid
		if (block->getTag() == (unsigned) tag && state)
			return true;

		// Transient tag available while directory entry is locked.
		// This is considered a hit, regardless of the state of the
		// block.
		if (block->getTransientTag() == (unsigned) tag &&
				directory->isEntryLocked(set, way))
			return true;
	}

	// Miss
	way = 0;
	state = Cache::BlockInvalid;
	return false;
}


void Module::Flush(int *witness)
{
	// Get pointer to esim engine
	esim::Engine *esim_engine = esim::Engine::getInstance();

	// Create a new event frame
	auto new_frame = misc::new_shared<Frame>(
			Frame::getNewId(),
			this,
			0);
	new_frame->witness = witness;

	// Schedule event
	esim_engine->Call(System::event_flush, new_frame);
}


void Module::RecursiveFlush()
{
	// Iterate through low_modules
	for (Module *low_module : low_modules)
	{
		// Skips main memory as it should not be flushed
		if (low_module->getType() != Module::TypeMainMemory)
			low_module->RecursiveFlush();
		else
			low_module->FlushCache();
	}
}


void Module::FlushCache()
{
	// Get pointer to event frame
	Frame *frame = misc::cast<Frame *>(frame);
	
	// Set up variables
	unsigned tag;
	Cache::BlockState state;

	// Invalidate all blocks
	for (int set = 0; set < directory_num_sets; set++)
	{
		for (int way = 0; way < directory_num_ways; way++)
		{
			// Get block
			cache->getBlock(set, way, tag, state);

			// Ignore if block has already been invalidated
			if (state == Cache::BlockState::BlockInvalid)
				continue;

			// One more pending request
			frame->pending++;

			// Get pointer to engine
			esim::Engine *esim_engine = esim::Engine::getInstance();

			// Create new frame
			auto new_frame = misc::new_shared<Frame>(
					frame->getId(),
					this,
					frame->tag);
			new_frame->set = set;
			new_frame->way = way;
			new_frame->witness = frame->witness;

			// Set up event
			esim::Event *event = System::event_invalidate;

			// Schedule event
			esim_engine->Call(event,
				new_frame);
		}
	}
}


void Module::UpdateStats(Frame *frame)
{
	// Assert that the frame module is in fact the module
	assert(this == frame->getModule());

	// Record access type. I purposefully chose to record both hits and
	// misses separately here so that we can sanity check them against
	// the total number of accesses.
	if (frame->request_direction == Frame::RequestDirectionUpDown)
	{
		if (frame->read)
		{
			num_reads++;
			if (frame->retry)
				num_retry_reads++;
			if (frame->blocking)
				num_blocking_reads++;
			else
				num_non_blocking_reads++;
			if (frame->hit)
			{
				num_read_hits++;
				if (frame->retry)
					num_retry_read_hits++;
			}
			else
			{
				num_read_misses++;
				if (frame->retry)
					num_retry_read_misses++;
			}

		}
		else if (frame->nc_write)  // Must go after read
		{
			num_nc_writes++;
			if (frame->retry)
				num_retry_nc_writes++;
			if (frame->blocking)
				num_blocking_nc_writes++;
			else
				num_non_blocking_nc_writes++;
			if (frame->hit)
			{
				num_nc_write_hits++;
				if (frame->retry)
					num_retry_nc_write_hits++;
			}
			else
			{
				num_nc_write_misses++;
				if (frame->retry)
					num_retry_nc_write_misses++;
			}
		}
		else if (frame->write)
		{
			num_writes++;
			if (frame->retry)
				num_retry_writes++;
			if (frame->blocking)
				num_blocking_writes++;
			else
				num_non_blocking_writes++;
			if (frame->hit)
			{
				num_write_hits++;
				if (frame->retry)
					num_retry_write_hits++;
			}
			else
			{
				num_write_misses++;
				if (frame->retry)
					num_retry_write_misses++;
			}
		}
		else 
		{
			throw misc::Panic("Invalid memory operation type");
		}
	}
	else if (frame->request_direction == Frame::RequestDirectionDownUp)
	{
		assert(frame->hit);
		if (frame->write)
		{
			num_write_probes++;
			if (frame->retry)
				num_retry_write_probes++;
		}
		else if (frame->read)
		{
			num_read_probes++;
			if (frame->retry)
				num_retry_read_probes++;
		}
		else
		{
			throw misc::Panic("Invalid memory operation type");
		}
	}
	else
	{
		num_hlc_evictions++;
	}
}


int Module::getRetryLatency() const
{
	// To support a data latency of zero, we must ensure that at least
	// one of the following values is non-zero so that the modulo operation
	// will work.  Using two instead of one to avoid livelock situations.
	return random() % (data_latency + 2);
}


}  // namespace mem


