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

#ifndef DRIVER_GLUT_GLUT_H
#define DRIVER_GLUT_GLUT_H

#include <pthread.h>

#include <driver/common/driver.h>


/*
 * Class 'GlutDriver'
 */

CLASS_BEGIN(GlutDriver, Driver)

CLASS_END(GlutDriver)


void GlutDriverCreate(GlutDriver *self, X86Emu *emu);
void GlutDriverDestroy(GlutDriver *self);



/*
 * Public
 */

#define glut_debug(...) debug(glut_debug_category, __VA_ARGS__)
extern int glut_debug_category;

extern pthread_mutex_t glut_mutex;

int GlutDriverCall(X86Context *ctx);


#endif

