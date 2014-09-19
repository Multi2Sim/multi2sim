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

#include <arch/hsa/driver/Driver.h>

#include "Emu.h"
#include "RuntimeInterceptor.h"


namespace HSA
{

// Singleton instance
std::unique_ptr<RuntimeInterceptor> RuntimeInterceptor::instance;

misc::StringMap RuntimeInterceptor::function_name_to_call_map =
{
#define STR(x) #x
#define DEFCALL(name, code, func) {STR(func), code},
#include <arch/hsa/driver/Driver.def>
#undef DEFCALL
#undef STR
};

RuntimeInterceptor::RuntimeInterceptor()
{

}

RuntimeInterceptor::~RuntimeInterceptor()
{
}

RuntimeInterceptor *RuntimeInterceptor::getInstance()
{
	if (instance.get())
		return instance.get();
	instance.reset(new RuntimeInterceptor());
	return instance.get();
}

bool RuntimeInterceptor::Intercept(const std::string &function_name)
{
	// Translate from the function name to the ABI call number
	bool error;
	int call_number = 0;
	call_number = function_name_to_call_map.MapString(function_name, error);
	if(error)
		return false;

	// Redirect the runtime function call to drivers ABI call
	Driver::getInstance()->Call(call_number,
			Emu::getInstance()->getMemory(),
			0);

	// Dump interception information into debug log
	Emu::isa_debug << misc::fmt("Runtime function %s intercepted, "
			"executing ABI call %d. \n",
			function_name.c_str(), call_number);

	return true;
}

} /* namespace HSA */
