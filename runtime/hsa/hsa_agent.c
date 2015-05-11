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
	unsigned long long agent_ids[2];
	agent_ids[0] = 1;
	agent_ids[1] = 0;

	// Traverse all agent
	while(agent_ids[0] != 0)
	{
		ioctl(hsa_runtime->fd, NextAgent, agent_ids);
		if (agent_ids[0] != 0)
		{
			hsa_agent_t curr_agent;
			curr_agent.handle = agent_ids[0];
			hsa_status_t callback_status =
					callback(curr_agent, data);
			if (callback_status != HSA_STATUS_SUCCESS)
				return callback_status;
			agent_ids[1] = agent_ids[0];
		}
	}
	return HSA_STATUS_SUCCESS;
}


hsa_status_t HSA_API hsa_agent_get_info(hsa_agent_t agent,
                                        hsa_agent_info_t attribute,
                                        void *value)
{
	char args[4 + 8 + 4 + 4];
	memcpy(args + 4, &agent, 8);
	memcpy(args + 12, &attribute, 4);
	memcpy(args + 16, &value, 4);

	if (!hsa_runtime)
	{
		return HSA_STATUS_ERROR_NOT_INITIALIZED;
	}
	else
	{
		ioctl(hsa_runtime->fd, AgentGetInfo, args);
		return (hsa_status_t)args[0];
	}
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
