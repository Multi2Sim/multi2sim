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

hsa_status_t HSA_API
    hsa_signal_create(hsa_signal_value_t initial_value, uint32_t num_consumers,
                      const hsa_agent_t *consumers, hsa_signal_t *signal)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_signal_destroy(hsa_signal_t signal)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_signal_value_t HSA_API hsa_signal_load_acquire(hsa_signal_t signal)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_signal_value_t HSA_API hsa_signal_load_relaxed(hsa_signal_t signal)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


void HSA_API
    hsa_signal_store_relaxed(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_store_release(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


hsa_signal_value_t HSA_API
    hsa_signal_exchange_acq_rel(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API
    hsa_signal_exchange_acquire(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API
    hsa_signal_exchange_relaxed(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API
    hsa_signal_exchange_release(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API hsa_signal_cas_acq_rel(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API hsa_signal_cas_acquire(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API hsa_signal_cas_relaxed(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API hsa_signal_cas_release(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


void HSA_API
    hsa_signal_add_acq_rel(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_add_acquire(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_add_relaxed(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_add_release(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_subtract_acq_rel(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_subtract_acquire(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_subtract_relaxed(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_subtract_release(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_and_acq_rel(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_and_acquire(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_and_relaxed(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_and_release(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_or_acq_rel(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_or_acquire(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_or_relaxed(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_or_release(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_xor_acq_rel(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_xor_acquire(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_xor_relaxed(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


void HSA_API
    hsa_signal_xor_release(hsa_signal_t signal, hsa_signal_value_t value)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return;
}


hsa_signal_value_t HSA_API
    hsa_signal_wait_acquire(hsa_signal_t signal,
                            hsa_signal_condition_t condition,
                            hsa_signal_value_t compare_value,
                            uint64_t timeout_hint,
                            hsa_wait_state_t wait_state_hint)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}


hsa_signal_value_t HSA_API
    hsa_signal_wait_relaxed(hsa_signal_t signal,
                            hsa_signal_condition_t condition,
                            hsa_signal_value_t compare_value,
                            uint64_t timeout_hint,
                            hsa_wait_state_t wait_state_hint)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return 0;
}
