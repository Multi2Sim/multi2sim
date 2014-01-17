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

#ifndef M2C_SI2BIN_TASK_H
#define M2C_SI2BIN_TASK_H

#include <iostream>

#include <lib/cpp/ELFWriter.h>


namespace si2bin
{

/* Forward declarations */
class Symbol;


class Task
{
	friend class Context;

	int offset;
	Symbol *symbol;
	
	ELFWriter::Buffer *buffer;
	
	Task(int offset, Symbol *symbol, ELFWriter::Buffer *buffer);

public:

	int GetOffset() { return offset; }
	Symbol *GetSymbol() { return symbol; }

	void Dump(std::ostream &os);
	friend std::ostream &operator<<(std::ostream &os, Task &task)
			{ task.Dump(os); return os; }

	void Process();
};


}  /* namespace si2bin */

#endif

