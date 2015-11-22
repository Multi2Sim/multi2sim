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
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint32_t profile;
		uint32_t executable_stat;
		uint32_t options;
		uint32_t executable;
	} data;
	data.profile = profile;
	data.executable_stat = executable_stat;
	data.options = (uint32_t)options;
	data.executable = (uint32_t)executable;
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ExecutableCreate, &data);
	return data.status;
}


hsa_status_t HSA_API hsa_executable_destroy(
    hsa_executable_t executable)
{
	// FIXME(Yifan): destory the executable from memory
	return HSA_STATUS_SUCCESS;
}


hsa_status_t hsa_executable_load_code_object(
	hsa_executable_t executable,
	hsa_agent_t agent,
	hsa_code_object_t code_object,
	const char *options)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t executable;
		uint64_t agent;
		uint64_t code_object;
		uint32_t options;
	} data;
	data.executable = executable.handle;
	data.agent = agent.handle;
	data.code_object = code_object.handle;
	data.options = (uint32_t)options;

	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ExecutableLoadCodeObject, &data);
	return data.status;
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
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t executable;
		uint32_t module_name;
		uint32_t symbol_name;
		uint64_t agent;
		int32_t call_convention;
		uint32_t symbol;
	} data;

	data.executable = executable.handle;
	data.module_name = (uint32_t)module_name;
	data.symbol_name = (uint32_t)symbol_name;
	data.agent = agent.handle;
	data.call_convention = call_convention;
	data.symbol = (uint32_t)symbol;
	if (!hsa_runtime)
	{
		// FIXME: This is a problem. Temporarily changed for Histogram
		// return HSA_STATUS_ERROR_NOT_INITIALIZED;
		hsa_init();
	}
	ioctl(hsa_runtime->fd, ExecutableGetSymbol, &data);
	return data.status;
}


hsa_status_t hsa_executable_symbol_get_info(
	hsa_executable_symbol_t executable_symbol,
	hsa_executable_symbol_info_t attribute,
	void *value)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t executable_symbol;
		uint32_t attribute;
		uint32_t value;
	} data;
	data.executable_symbol = executable_symbol.handle;
	data.attribute = attribute;
	data.value = (uint32_t)value;
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ExecutableSymbolGetInfo, &data);
	return data.status;
}


hsa_status_t HSA_API hsa_executable_iterate_symbols(
    hsa_executable_t executable,
    hsa_status_t (*callback)(hsa_executable_t executable, hsa_executable_symbol_t symbol, void* data),
    void* data)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}
