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

#ifndef DRIVER_GLUT_FRAME_BUFFER_H
#define DRIVER_GLUT_FRAME_BUFFER_H


void glut_frame_buffer_init(void);
void glut_frame_buffer_done(void);

void glut_frame_buffer_clear(void);
void glut_frame_buffer_pixel(int x, int y, int color);

void glut_frame_buffer_resize(int width, int height);
void glut_frame_buffer_get_size(int *width, int *height);

void glut_frame_buffer_flush_request(void);
void glut_frame_buffer_flush_if_requested(void);


#endif

