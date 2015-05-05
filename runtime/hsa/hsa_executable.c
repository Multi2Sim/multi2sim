/*
 *  Multi2Sim
 *  Copyright (C) 2012  Yifan Sun (yifansun@coe.neu.edu)
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "debug.h"
#include "hsa.h"

hsa_status_t HSA_API hsa_executable_create(
		hsa_profile_t profile,
		hsa_executable_state_t executable_stat,
		const char *options,
		hsa_executable_t *executable)
{
	unsigned int args[8] = {0};
	memcpy(args + 1, &profile, 8);
	memcpy(args + 3, &executable_stat, 4);
	memcpy(args + 4, &options, 8);
	memcpy(args + 6, &executable, 8);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ExecutableCreate, args);
	return args[0];
}


hsa_status_t HSA_API hsa_executable_destroy(
    hsa_executable_t executable)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t hsa_executable_load_code_object(
	hsa_executable_t executable,
	hsa_agent_t agent,
	hsa_code_object_t code_object,
	const char *options)
{
	unsigned int args[9] = {0};
	memcpy(args + 1, &executable, 8);
	memcpy(args + 3, &agent, 8);
	memcpy(args + 5, &code_object, 8);
	memcpy(args + 7, &options, 8);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ExecutableLoadCodeObject, args);
	return args[0];
}


hsa_status_t hsa_executable_freeze(
	hsa_executable_t executable,
	const char *options)
{
	// Currently do nothing
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_executable_global_variable_define(
    hsa_executable_t executable,
    const char *variable_name,
    void *address)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_executable_agent_global_variable_define(
    hsa_executable_t executable,
    hsa_agent_t agent,
    const char *variable_name,
    void *address)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_executable_readonly_variable_define(
    hsa_executable_t executable,
    hsa_agent_t agent,
    const char *variable_name,
    void *address)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_executable_validate(
    hsa_executable_t executable,
    uint32_t* result)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API
    hsa_executable_get_symbol(hsa_executable_t executable,
                              const char *module_name,
                              const char *symbol_name,
                              hsa_agent_t agent,
                              int32_t call_convention,
                              hsa_executable_symbol_t *symbol)
{
	unsigned int args[12] = {0};
	memcpy(args + 1, &executable, 8);
	memcpy(args + 3, &module_name, 4);
	memcpy(args + 5, &symbol_name, 4);
	memcpy(args + 7, &agent, 8);
	memcpy(args + 9, &call_convention, 4);
	memcpy(args + 10, &symbol, 8);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ExecutableGetSymbol, args);
	return args[0];
}


hsa_status_t hsa_executable_symbol_get_info(
	hsa_executable_symbol_t executable_symbol,
	hsa_executable_symbol_info_t attribute,
	void *value)
{
	unsigned int args[6] = {0};
	memcpy(args + 1, &executable_symbol, 8);
	memcpy(args + 3, &attribute, 4);
	memcpy(args + 4, &value, 8);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ExecutableSymbolGetInfo, args);
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_executable_iterate_symbols(
    hsa_executable_t executable,
    hsa_status_t (*callback)(hsa_executable_t executable, hsa_executable_symbol_t symbol, void* data),
    void* data)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}
