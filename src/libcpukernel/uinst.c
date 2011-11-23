/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <cpukernel.h>



/*
 * Public Functions
 */


struct x86_uinst_t *x86_uinst_create(void)
{
	struct x86_uinst_t *uinst;

	uinst = calloc(1, sizeof(struct x86_uinst_t));
	if (!uinst)
		fatal("%s: out of memory", __FUNCTION__);
	return uinst;
}


void x86_uinst_free(struct x86_uinst_t *uinst)
{
	free(uinst);
}
