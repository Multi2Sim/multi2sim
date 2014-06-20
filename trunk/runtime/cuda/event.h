/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef RUNTIME_CUDA_EVENT_H
#define RUNTIME_CUDA_EVENT_H


#include "api.h"

struct CUevent_st
{
	unsigned id;

	CUstream stream;

	clock_t t;

	/* Flags */
	unsigned to_be_recorded;
	unsigned recorded;
	unsigned flags;
};

CUevent cuda_event_create(unsigned flags);
void cuda_event_free(CUevent event);
void cuda_event_record(CUevent event);


#endif

