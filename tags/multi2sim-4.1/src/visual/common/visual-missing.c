/*
 *  Multi2Sim Tools
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "visual.h"


void visual_run(char *file_name)
{
	fatal("operation not available.\n"
		"\tMulti2Sim was compiled without support for GTK applications. When you ran\n"
		"\tthe './configure' script, no GTK support was detected in your system, so\n"
		"\tthe simulator compilation did not include the visualization tool. To\n"
		"\tinstall it, please follow these steps:\n"
		"\t  1) Install the development packages for GTK 3.0. Under Debian-based\n"
		"\t     Linux distributions, this package is listed as 'libgtk-3-dev'.\n"
		"\t  2) Re-run the './configure' script\n"
		"\t  3) Recompile the simulator: make clean && make\n");
}

