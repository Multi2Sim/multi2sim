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

hsa_status_t HSA_API hsa_code_object_serialize(
    hsa_code_object_t code_object,
    hsa_status_t (*alloc_callback)(size_t size, hsa_callback_data_t data, void **address),
    hsa_callback_data_t callback_data,
    const char *options,
    void **serialized_code_object,
    size_t *serialized_code_object_size)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_code_object_deserialize(
    void *serialized_code_object,
    size_t serialized_code_object_size,
    const char *options,
    hsa_code_object_t *code_object)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_code_object_destroy(
    hsa_code_object_t code_object)
{
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_code_object_get_info(
    hsa_code_object_t code_object,
    hsa_code_object_info_t attribute,
    void *value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_code_object_get_symbol(
	hsa_code_object_t code_object,
	const char *symbol_name,
	hsa_code_symbol_t *symbol)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_code_symbol_get_info(
    hsa_code_symbol_t code_symbol,
    hsa_code_symbol_info_t attribute,
    void *value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_code_object_iterate_symbols(
    hsa_code_object_t code_object,
    hsa_status_t (*callback)(hsa_code_object_t code_object, hsa_code_symbol_t symbol, void* data),
    void* data)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}
