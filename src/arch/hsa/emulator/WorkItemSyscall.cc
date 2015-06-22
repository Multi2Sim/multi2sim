/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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
#include <unistd.h>

#include "WorkItem.h"


namespace HSA
{

WorkItem::ExecuteSyscallFn WorkItem::execute_syscall_fn[SyscallCodeCount + 1] =
{
	nullptr,
#define DEFSYSCALL(name, code) &WorkItem::ExecuteSyscall_##name,
#include "Syscall.def"
#undef DEFSYSCALL
	nullptr
};


void WorkItem::ExecuteSyscall_print_int()
{
	// Get current inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Get file descriptor
	comm::FileDescriptor *desc =
			ProgramLoader::getInstance()->
			getFileTable()->getFileDescriptor(1);

	// Convert guest file descriptor to host file descriptor, default to
	// be stdout (1).
	int host_fd = 1;
	if (desc)
	{
		host_fd = desc->getHostIndex();
	}

	// Do action according to type
	switch (inst.getType())
	{
	case BRIG_TYPE_U32:
		{
			// Signed or unsigned, marked in operand 3
			int isSigned = getOperandValue<unsigned int>(3);

			// Retrieve and dump the integer
			if (isSigned)
			{
				// Get the integer to output
				int src1 = getOperandValue<int>(2);

				// Output to file
				std::string str = misc::fmt("%d", src1);
				write(host_fd, str.c_str(), str.size());
			}
			else
			{
				// Get the integer to output
				unsigned int src1 =
						getOperandValue
						<unsigned int>(2);

				// Output to file
				std::string str = misc::fmt("%u", src1);
				write(host_fd, str.c_str(), str.size());
			}

		}
		break;
	case BRIG_TYPE_U64:
		{
			// Signed or unsigned, marked in operand 3
			unsigned long long isSigned =
					getOperandValue<unsigned long long>(3);

			// Retrieve and dump the integer
			if (isSigned)
			{
				// Get the integer to output
				long long src1 = getOperandValue<long long>(2);
				//std::cout << "long long " << src1 << "\n";

				// Output to file
				std::string str = misc::fmt("%lld", src1);
				write(host_fd, str.c_str(), str.size());
			}
			else
			{
				// Get the integer to output
				unsigned long long src1 =
						getOperandValue
						<unsigned long long>(2);
				//std::cout << "unsigned long long " << src1 << "\n";

				// Output to file
				std::string str = misc::fmt("%llu", src1);
				write(host_fd, str.c_str(), str.size());
			}
		}
		break;
	}

}


void WorkItem::ExecuteSyscall_print_float()
{
	// Get current inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Get file descriptor
	comm::FileDescriptor *desc =
			ProgramLoader::getInstance()->
			getFileTable()->getFileDescriptor(1);

	// Convert guest file descriptor to host file descriptor, default to
	// be stdout (1).
	int host_fd = 1;
	if (desc)
	{
		host_fd = desc->getHostIndex();
	}

	// Do action according to type
	switch (inst.getType())
	{
	case BRIG_TYPE_U32:
		{
			// Get the float to output
			float src1 = getOperandValue<float>(2);

			// Output to file
			std::string str = misc::fmt("%f", src1);
			write(host_fd, str.c_str(), str.size());

		}
		break;
	case BRIG_TYPE_U64:
		throw Error(misc::fmt("Type U64 not supported for "
				"print_float"));
		break;
	}
}


void WorkItem::ExecuteSyscall_print_double()
{
	// Get current inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Get file descriptor
	comm::FileDescriptor *desc =
			ProgramLoader::getInstance()->
			getFileTable()->getFileDescriptor(1);

	// Convert guest file descriptor to host file descriptor, default to
	// be stdout (1).
	int host_fd = 1;
	if (desc)
	{
		host_fd = desc->getHostIndex();
	}

	// Do action according to type
	switch (inst.getType())
	{
	case BRIG_TYPE_U64:
		{
			// Get the float to output
			double src1 = getOperandValue<double>(2);

			// Output to file
			std::string str = misc::fmt("%f", src1);
			write(host_fd, str.c_str(), str.size());

		}
		break;
	case BRIG_TYPE_U32:
		throw Error(misc::fmt("Type U32 not supported for "
				"print_double"));
		break;
	}
}


void WorkItem::ExecuteSyscall_print_string()
{
}


void WorkItem::ExecuteSyscall_read_int()
{
}


void WorkItem::ExecuteSyscall_read_float()
{
}


void WorkItem::ExecuteSyscall_read_double()
{
}


void WorkItem::ExecuteSyscall_read_string()
{
}


void WorkItem::ExecuteSyscall_sbrk()
{
}


void WorkItem::ExecuteSyscall_exit()
{
}


void WorkItem::ExecuteSyscall_print_character()
{
	// Get current inst
	StackFrame *stack_top = stack.back().get();
	BrigInstEntry inst(stack_top->getPc(),
			ProgramLoader::getInstance()->getBinary());

	// Get file descriptor
	comm::FileDescriptor *desc =
			ProgramLoader::getInstance()->
			getFileTable()->getFileDescriptor(1);

	// Convert guest file descriptor to host file descriptor, default to
	// be stdout (1).
	int host_fd = 1;
	if (desc)
	{
		host_fd = desc->getHostIndex();
	}

	// Do action according to type
	switch (inst.getType())
	{
	case BRIG_TYPE_U32:
		{
			// Get the float to output
			char src1 = getOperandValue<char>(2);

			// Output to file
			std::string str = misc::fmt("%c", src1);
			write(host_fd, str.c_str(), str.size());

		}
		break;
	case BRIG_TYPE_U64:
		throw Error(misc::fmt("Type U64 not supported for "
				"print_character"));
		break;
	}
}


void WorkItem::ExecuteSyscall_read_character()
{
}


void WorkItem::ExecuteSyscall_open()
{
}


void WorkItem::ExecuteSyscall_read()
{
}


void WorkItem::ExecuteSyscall_write()
{
}


void WorkItem::ExecuteSyscall_close()
{
}


void WorkItem::ExecuteSyscall_exit2()
{
}


}  // namespace HSA

