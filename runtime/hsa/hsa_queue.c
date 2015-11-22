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
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t agent;
		uint32_t size;
		uint32_t type;
		uint32_t callback;
		uint32_t data;
		uint32_t private_segment_size;
		uint32_t group_segment_size;
		uint32_t queue;
	} info;
	info.agent = agent.handle;
	info.size = size;
	info.type = type;
	info.callback = (uint32_t)callback;
	info.data = (uint32_t)data;
	info.private_segment_size = private_segment_size;
	info.group_segment_size = group_segment_size;
	info.queue = (uint32_t)queue;

 	if (!hsa_runtime)
 	{
 		return HSA_STATUS_ERROR_NOT_INITIALIZED;
 	}

	ioctl(hsa_runtime->fd, QueueCreate, &info);
	return info.status;
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
	struct __attribute__((packed))
	{
		uint64_t value;
		uint32_t queue;
	} data;
	data.queue = (uint32_t)queue;
	if (!hsa_runtime)
	{
		fprintf(stderr, "HSA runtime not initialized");
	}
	ioctl(hsa_runtime->fd, QueueLoadReadIndexRelaxed, &data);
	return data.value;
}


uint64_t HSA_API hsa_queue_load_write_index_acquire(const hsa_queue_t *queue)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


uint64_t HSA_API hsa_queue_load_write_index_relaxed(const hsa_queue_t *queue)
{
	struct __attribute__((packed))
	{
		uint64_t value;
		uint32_t queue;
	} data;
	data.queue = (uint32_t)queue;
	if (!hsa_runtime)
	{
		fprintf(stderr, "HSA runtime not initialized");
	}
	ioctl(hsa_runtime->fd, QueueLoadWriteIndexRelaxed, &data);
	return data.value;
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
