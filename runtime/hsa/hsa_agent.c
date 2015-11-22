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

hsa_status_t HSA_API hsa_iterate_agents(
		hsa_status_t (*callback)(hsa_agent_t agent, void *data),
		void *data)
{
	struct __attribute__ ((packed))
	{
		uint64_t current_agent_id;
		uint64_t next_agent_id;
		bool has_next;
	} agents;
	agents.current_agent_id = 0;
	agents.has_next = true;

	// Traverse all agent
	while(agents.has_next)
	{
		ioctl(hsa_runtime->fd, NextAgent, &agents);
		if (agents.has_next)
		{
			hsa_agent_t agent;
			agent.handle = agents.next_agent_id;
			hsa_status_t callback_status =
					callback(agent, data);
			if (callback_status != HSA_STATUS_SUCCESS)
				return callback_status;
			agents.current_agent_id = agents.next_agent_id;
		}
	}
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_agent_get_info(hsa_agent_t agent,
                                        hsa_agent_info_t attribute,
                                        void *value)
{
	struct __attribute__ ((packed))
	{
		uint32_t status;
		uint64_t agent;
		uint32_t attribute;
		uint32_t value;
	} data;
	data.agent = agent.handle;
	data.attribute = attribute;
	data.value = (uint32_t)value;

	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}

	ioctl(hsa_runtime->fd, AgentGetInfo, &data);
	return data.status;
}


hsa_status_t HSA_API hsa_agent_get_exception_policies(hsa_agent_t agent,
                                                      hsa_profile_t profile,
                                                      uint16_t *mask)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API
    hsa_agent_extension_supported(uint16_t extension, hsa_agent_t agent,
                                  uint16_t version_major,
                                  uint16_t version_minor, bool *result)
{
	__HSA_RUNTIME_NOT_IMPLEMENTED__
	return HSA_STATUS_SUCCESS;
}
