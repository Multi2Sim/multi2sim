/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <fstream>
#include <iostream>

#include "Debug.h"


namespace misc
{


Debug::Debug()
{
	os = nullptr;
	active = false;
}

Debug::~Debug()
{
	Flush();
	Close();
}


void Debug::Close()
{
	if (os && os != &std::cout && os != &std::cerr)
		delete os;
	os = nullptr;
}


void Debug::setPath(const std::string &path)
{
	// Release previous output stream
	Close();
	this->path = path;

	// Empty file
	if (path.empty())
		return;

	// File is standard output
	if (path == "stdout")
		os = &std::cout;
	else if (path == "stderr")
		os = &std::cerr;
	else
		os = new std::ofstream(path.c_str());

	// Create new output stream
	if (!*os)
	{
		std::cerr << "fatal: cannot open " << path << '\n';
		exit(1);
	}

	//activate the debug
	active = true;
}


void Debug::Flush()
{
	if (os)
		os->flush();
}



}  // namespace misc

