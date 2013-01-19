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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_API_H
#define ARCH_SOUTHERN_ISLANDS_EMU_OPENCL_API_H


struct si_opencl_image_format_t
{
	unsigned int image_channel_order;
	unsigned int image_channel_data_type;
};

/* Forward declaration */
struct x86_ctx_t;

/* Function tables */
extern char *si_opencl_func_name[];
extern int si_opencl_func_argc[];

/* Debugging */
#define si_opencl_debug(...) debug(si_opencl_debug_category, __VA_ARGS__)
extern int si_opencl_debug_category;

/* Some constants */
#define SI_OPENCL_FUNC_FIRST  1000
#define SI_OPENCL_FUNC_LAST  1073
#define SI_OPENCL_FUNC_COUNT  (si_OPENCL_FUNC_LAST - si_OPENCL_FUNC_FIRST + 1)
#define SI_OPENCL_MAX_ARGS  14

/* Common entry point for Evergreen/Southern Islands.
 * FIXME - hacky thing to be removed soon. */
int x86_opencl_call(struct x86_ctx_t *ctx);

int si_opencl_api_run(struct x86_ctx_t *ctx);

int si_opencl_api_read_args(struct x86_ctx_t *ctx, int *argc_ptr,
		void *argv_ptr, int argv_size);
void si_opencl_api_return(struct x86_ctx_t *ctx, int value);

#endif
