/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "debug.h"
#include "clcpu.h"
#include "clcpu-program.h"
#include "clcpu-device.h"
#include "mhandle.h"

void clcpu_device_exceute_ndrange(
	void *dev,
	void *kern, 
	cl_uint work_dim, 
	const size_t *global_work_offset, 
	const size_t *global_work_size, 
	const size_t *local_work_size)
{
	int i;
	int j;
	int k;
	int num_groups[3];
	struct clcpu_device_t *device = dev;
	struct opencl_x86_kernel_t *kernel = kern;

	struct clcpu_execution_t *run;

	run = xmalloc(sizeof (struct clcpu_execution_t));

	/* copy over dimensions */
	run->dims = work_dim;
	run->global = global_work_size;
	run->local = local_work_size;


	run->num_groups = 1;
	for (i = 0; i < 3; i++)
	{
		assert(!(run->global[i] % run->local[i]));	
		
		num_groups[i] = run->global[i] / run->local[i];
		run->num_groups *= num_groups[i];
	}
	run->kernel = kernel;
	run->next_group = 0;
	run->group_starts = xmalloc(3 * sizeof (size_t) * run->num_groups);
	if(!run->group_starts)
		fatal("%s: out of memory", __FUNCTION__);
	
	pthread_mutex_init(&run->mutex, NULL);
	
	for (i = 0; i < num_groups[2]; i++)
		for (j = 0; j < num_groups[1]; j++)
			for (k = 0; k < num_groups[0]; k++)
			{
				size_t *group_start;

				group_start = run->group_starts + 3 * (i * num_groups[1] * num_groups[0] + j * num_groups[0] + k);
				group_start[0] = run->local[0] * k;
				group_start[1] = run->local[1] * j;
				group_start[2] = run->local[2] * i;
			}

	pthread_mutex_lock(&device->lock);

	device->num_kernels++;
	device->num_done = 0;
	device->exec = run;
	pthread_cond_broadcast(&device->ready);

	while (device->num_done != device->num_cores)
		pthread_cond_wait(&device->done, &device->lock);

	pthread_mutex_unlock(&device->lock);

	/* free the execution context */
	free(run->group_starts);
	pthread_mutex_destroy(&run->mutex);
	free(run);
}
