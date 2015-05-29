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

hsa_status_t HSA_API hsa_queue_create(
		hsa_agent_t agent, uint32_t size, hsa_queue_type_t type,
		void (*callback)(hsa_status_t status, hsa_queue_t *source,
				void *data),
		void *data, uint32_t private_segment_size,
		uint32_t group_segment_size, hsa_queue_t **queue)
{
	// Set arguments
	char args[64] = {0};
	memcpy(args + 4, &agent, 8);
	memcpy(args + 12, &size, 4);
	memcpy(args + 16, &type, 4);
	memcpy(args + 20, &callback, 4);
	memcpy(args + 28, &data, 4);
	memcpy(args + 36, &private_segment_size, 4);
	memcpy(args + 40, &group_segment_size, 4);
	memcpy(args + 44, &queue, 4);
	unsigned int host_lang = 1;
	memcpy(args + 52, &host_lang, 4);

 	if (!hsa_runtime)
 	{
 		return HSA_STATUS_ERROR_NOT_INITIALIZED;
 	}
 	else
 	{
 		ioctl(hsa_runtime->fd, QueueCreate, args);
 		return (hsa_status_t)args[0];
 	}
}


hsa_status_t HSA_API
     hsa_soft_queue_create(hsa_region_t region, uint32_t size,
                           hsa_queue_type_t type, uint32_t features,
                           hsa_signal_t doorbell_signal, hsa_queue_t **queue)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}

hsa_status_t HSA_API hsa_queue_destroy(hsa_queue_t *queue)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_queue_inactivate(hsa_queue_t *queue)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


uint64_t HSA_API hsa_queue_load_read_index_acquire(const hsa_queue_t *queue)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API hsa_queue_load_read_index_relaxed(const hsa_queue_t *queue)
{
	unsigned long long args[2] = {0};
	memcpy(args + 1, &queue, 4);
	ioctl(hsa_runtime->fd, QueueLoadReadIndexRelaxed, args);
	return args[0];
}


uint64_t HSA_API hsa_queue_load_write_index_acquire(const hsa_queue_t *queue)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API hsa_queue_load_write_index_relaxed(const hsa_queue_t *queue)
{
	unsigned long long args[2] = {0};
	memcpy(args + 1, &queue, 4);
	ioctl(hsa_runtime->fd, QueueLoadWriteIndexRelaxed, args);
	return args[0];
}


void HSA_API hsa_queue_store_write_index_relaxed(const hsa_queue_t *queue,
                                                 uint64_t value)
{
	unsigned long long args[2] = {0};
	memcpy(args, &queue, 4);
	memcpy(args + 1, &value, 8);
	ioctl(hsa_runtime->fd, QueueStoreWriteIndexRelaxed, args);
	return;
}


void HSA_API hsa_queue_store_write_index_release(const hsa_queue_t *queue,
                                                 uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


uint64_t HSA_API hsa_queue_cas_write_index_acq_rel(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API hsa_queue_cas_write_index_acquire(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API hsa_queue_cas_write_index_relaxed(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API hsa_queue_cas_write_index_release(const hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API
    hsa_queue_add_write_index_acq_rel(const hsa_queue_t *queue, uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API
    hsa_queue_add_write_index_acquire(const hsa_queue_t *queue, uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API
    hsa_queue_add_write_index_relaxed(const hsa_queue_t *queue, uint64_t value)
{
	unsigned long long args[3] = {0};
	memcpy(args + 1, &queue, 4);
	memcpy(args + 2, &value, 8);
	ioctl(hsa_runtime->fd, QueueAddWriteIndexRelaxed, args);
	return args[0];
}


uint64_t HSA_API
    hsa_queue_add_write_index_release(const hsa_queue_t *queue, uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


void HSA_API hsa_queue_store_read_index_relaxed(const hsa_queue_t *queue,
                                                uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API hsa_queue_store_read_index_release(const hsa_queue_t *queue,
                                                uint64_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}
