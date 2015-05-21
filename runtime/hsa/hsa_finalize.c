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

hsa_status_t HSA_API hsa_ext_program_create(
	hsa_machine_model_t machine_model,
	hsa_profile_t profile,
	hsa_default_float_rounding_mode_t default_float_rounding_mode,
	const char *options,
	hsa_ext_program_t *program)
{
	unsigned int args[8] = {0};
	memcpy(args + 1, &machine_model, 4);
	memcpy(args + 2, &profile, 4);
	memcpy(args + 3, &default_float_rounding_mode, 4);
	memcpy(args + 4, &options, 4);
	memcpy(args + 6, &program, 4);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ProgramCreate, args);
	return args[0];
}


hsa_status_t HSA_API hsa_ext_program_destroy(
    hsa_ext_program_t program)
{
	unsigned int args[3] = {0};
	memcpy(args + 1, &program, 8);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ProgramDestroy, args);
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_ext_program_add_module(
	hsa_ext_program_t program,
	hsa_ext_module_t module)
{
	unsigned int args[5] = {0};
	memcpy(args + 1, &program, 8);
	memcpy(args + 3, &module, 8);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ProgramAddModule, args);
	return args[0];
}


hsa_status_t HSA_API hsa_ext_program_iterate_modules(
    hsa_ext_program_t program,
    hsa_status_t (*callback)(hsa_ext_program_t program, hsa_ext_module_t module,
                             void* data),
    void* data)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_ext_program_get_info(
    hsa_ext_program_t program,
    hsa_ext_program_info_t attribute,
    void *value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_ext_program_finalize(
	hsa_ext_program_t program,
	hsa_isa_t isa,
	int32_t call_convention,
	hsa_ext_control_directives_t control_directives,
	const char *options,
	hsa_code_object_type_t code_object_type,
	hsa_code_object_t *code_object )
{
	unsigned int args[57] = {0};
	memcpy(args + 1, &program, 8);
	memcpy(args + 3, &isa, 8);
	memcpy(args + 5, &call_convention, 4);
	memcpy(args + 6, &control_directives, 144);
	memcpy(args + 42, &options, 8);
	memcpy(args + 44, &code_object_type, 4);
	memcpy(args + 45, &code_object, 8);
	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	ioctl(hsa_runtime->fd, ProgramFinalize, args);
	return args[0];
}

