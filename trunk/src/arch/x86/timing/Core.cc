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

#include "CPU.h"
#include "Core.h"


namespace x86
{

Core::Core(const std::string &name, CPU *cpu, int id)
	:
	name(name), cpu(cpu), id(id)
{
	// The prefix for each core
	std::string prefix = name + "Thread";
	std::string thread_name;

	// Initialize ROB
	InitializeReorderBuffer();

	// Create threads
	for (int i = 0; i < CPU::getNumThreads(); i++)
	{
		thread_name = prefix + misc::fmt("%d", i);
		threads.emplace_back(new Thread(thread_name, this->cpu, this, i));
		thread_name = "";
	}
}


void Core::InitializeReorderBuffer()
{
	// Create empty reorder buffer
	reorder_buffer_total_size = CPU::getReorderBufferSize() * CPU::getNumThreads();
	for (int i = 0; i < reorder_buffer_total_size; i++)
		reorder_buffer.emplace_back(nullptr);
}


void Core::TrimReorderBuffer()
{
	// Local variable
	int idx;
	Uop *uop;

	// Trim head
	while (uop_count_in_rob > 0)
	{
		uop = reorder_buffer[reorder_buffer_head].get();
		if (uop != nullptr)
			break;
		reorder_buffer_head = reorder_buffer_head == reorder_buffer_total_size - 1 ?
				0 : reorder_buffer_head + 1;
		uop_count_in_rob--;
	}

	// Trim tail
	while (uop_count_in_rob > 0)
	{
		idx = reorder_buffer_tail ? reorder_buffer_tail - 1 : reorder_buffer_total_size - 1;
		uop = reorder_buffer[idx].get();
		if (uop)
			break;
		reorder_buffer_tail = idx;
		uop_count_in_rob--;
	}
}


bool Core::CanEnqueueInReorderBuffer(Uop *uop)
{
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		if (uop->getThread()->uop_count_in_rob < CPU::getReorderBufferSize())
			return true;
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		if (uop_count_in_rob < reorder_buffer_total_size)
			return true;
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	return false;
}


void Core::EnqueueInReorderBuffer(Uop *uop)
{
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		assert(uop->getThread()->uop_count_in_rob < CPU::getReorderBufferSize());
		assert(!reorder_buffer[uop->getThread()->reorder_buffer_tail]);
		reorder_buffer[uop->getThread()->reorder_buffer_tail].reset(uop);
		uop->getThread()->reorder_buffer_tail =
				uop->getThread()->reorder_buffer_tail == uop->getThread()->reorder_buffer_right_bound ?
				uop->getThread()->reorder_buffer_left_bound :
				uop->getThread()->reorder_buffer_tail + 1;
		uop->getThread()->uop_count_in_rob++;
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		assert(uop_count_in_rob < reorder_buffer_total_size);
		assert(!reorder_buffer[reorder_buffer_tail]);
		reorder_buffer[reorder_buffer_tail].reset(uop);
		reorder_buffer_tail = reorder_buffer_tail == reorder_buffer_total_size - 1 ?
				0 : reorder_buffer_tail + 1;
		uop_count_in_rob++;
		uop->getThread()->uop_count_in_rob++;
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	//Instruction is in the ROB
	uop->in_reorder_buffer = true;
}


bool Core::CanDequeueFromReorderBuffer(int thread_id_in_core)
{
	// Local variable declaration
	Uop *uop;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id_in_core);

	// Check according to ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		if (thread->uop_count_in_rob > 0)
			return true;

		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		if (uop_count_in_rob == 0)
			return false;

		uop = reorder_buffer[reorder_buffer_head].get();
		assert(uop->Exists());
		if (uop->thread == thread)
			return true;
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return false;
}

Uop *Core::getReorderBufferHead(int thread_id_in_core)
{
	// Local variable declaration
	Uop *uop;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id_in_core);

	// Get head according to ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		if (thread->uop_count_in_rob > 0)
		{
			uop = reorder_buffer[thread->reorder_buffer_head].get();
			return uop;
		}
		break;

	case CPU::ReorderBufferKindShared:

		// In core domain
		TrimReorderBuffer();
		if (thread->uop_count_in_rob == 0)
			return nullptr;

		for (int i = 0; i < uop_count_in_rob; i++)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->thread == thread)
				return uop;
		}
		throw misc::Panic("NO head found\n");
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return nullptr;
}

void Core::RemoveReorderBufferHead(int thread_id_in_core)
{
	// Local variable declaration
	Uop *uop = nullptr;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id_in_core);

	// Remove Uop according to the ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		// In thread domain
		assert(thread->uop_count_in_rob > 0);
		uop = reorder_buffer[thread->reorder_buffer_head].get();
		assert(uop->Exists());
		assert(uop->thread == thread);
		reorder_buffer[thread->reorder_buffer_head].reset(nullptr);
		thread->reorder_buffer_head =
				thread->reorder_buffer_head == thread->reorder_buffer_right_bound ?
						thread->reorder_buffer_left_bound :
						thread->reorder_buffer_head + 1;
		thread->uop_count_in_rob--;
		break;

	case CPU::ReorderBufferKindShared:
		TrimReorderBuffer();
		assert(thread->uop_count_in_rob);
		for (int i = 0; i < uop_count_in_rob; i++)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->thread == thread)
			{
				reorder_buffer[idx].reset(nullptr);;
				thread->uop_count_in_rob--;
				break;
			}
		}
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	// Free instruction
	uop->in_reorder_buffer = false;

	// FIXME
	// x86_uop_free_if_not_queued(uop);
}

Uop *Core::getReorderBufferTail(int thread_id_in_core)
{
	// Local variable declaration
	Uop *uop;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id_in_core);

	// Get tail according to ROB kind
	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		if (thread->uop_count_in_rob > 0)
		{
			idx = thread->reorder_buffer_tail == thread->reorder_buffer_left_bound ?
					thread->reorder_buffer_right_bound :
					thread->reorder_buffer_tail - 1;
			uop = reorder_buffer[idx].get();
			return uop;
		}
		break;

	case CPU::ReorderBufferKindShared:

		TrimReorderBuffer();
		if (!thread->uop_count_in_rob)
			return nullptr;
		for (int i = uop_count_in_rob - 1; i >= 0; i--)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->thread == thread)
				return uop;
		}
		throw misc::Panic("reorder_buffer_tail: no tail found");
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return nullptr;
}

void Core::RemoveReorderBufferTail(int thread_id_in_core)
{
	// Local variable declaration
	Uop *uop = nullptr;
	int idx;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id_in_core);

	// Remove tail according to ROB kind
	switch (CPU::getReorderBufferKind())
	{

	case CPU::ReorderBufferKindPrivate:

		assert(thread->uop_count_in_rob > 0);
		idx = thread->reorder_buffer_tail == thread->reorder_buffer_left_bound ?
				thread->reorder_buffer_right_bound : thread->reorder_buffer_tail - 1;
		uop = reorder_buffer[idx].get();
		assert(uop->Exists());
		assert(uop->thread == thread);
		reorder_buffer[idx].reset(nullptr);;
		thread->reorder_buffer_tail = idx;
		thread->uop_count_in_rob--;
		break;

	case CPU::ReorderBufferKindShared:

		TrimReorderBuffer();
		assert(thread->uop_count_in_rob);
		for (int i = uop_count_in_rob - 1; i >= 0; i--)
		{
			idx = (reorder_buffer_head + i) % reorder_buffer_total_size;
			uop = reorder_buffer[idx].get();
			if (uop && uop->thread == thread)
			{
				reorder_buffer[idx].reset(nullptr);;
				thread->uop_count_in_rob--;
				break;
			}
		}
		break;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}

	// Free instruction
	uop->in_reorder_buffer = false;

	// FIXME
	//x86_uop_free_if_not_queued(uop);
}

Uop *Core::getReorderBufferEntry(int index, int thread_id_in_core)
{
	// Local declaration
	Uop *uop;

	// Get the corresponding thread
	Thread *thread = getThread(thread_id_in_core);

	// Check that index is in bounds
	if (index < 0 || index >= thread->uop_count_in_rob)
		return nullptr;

	switch (CPU::getReorderBufferKind())
	{
	case CPU::ReorderBufferKindPrivate:

		index += thread->reorder_buffer_head;
		if (index > thread->reorder_buffer_right_bound)
			index = index - thread->reorder_buffer_right_bound + thread->reorder_buffer_left_bound - 1;
		uop = reorder_buffer[index].get();
		assert(uop);
		return uop;

	case CPU::ReorderBufferKindShared:

		TrimReorderBuffer();
		index = (reorder_buffer_head + index) % reorder_buffer_total_size;
		uop = reorder_buffer[index].get();
		assert(uop);
		return uop;

	default:

		throw misc::Panic("The reorder buffer kind is invalid\n");
	}
	return nullptr;
}


}
