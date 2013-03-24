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

#include "color-channel.h"


void opengl_clamped_float_to_color_channel(GLfloat *src, GLchan* dst)
{
	dst[0] = (GLchan) src[0] * CHAN_MAX;
	dst[1] = (GLchan) src[1] * CHAN_MAX;
	dst[2] = (GLchan) src[2] * CHAN_MAX;
	dst[3] = (GLchan) src[3] * CHAN_MAX;
}
