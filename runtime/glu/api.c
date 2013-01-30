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

#include <stdio.h>
#include <unistd.h>

#include "../include/GL/glu.h"
#include "api.h"
#include "debug.h"



/*
 * Error Messages
 */

#define __GLU_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, glu_err_not_impl);


static char *glu_err_not_impl =
	"\tMulti2Sim provides partial support for the GLU library.\n"
	"\tTo request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

static char *glu_err_version =
	"\tYour guest application is using a version of the GLU runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tGLU runtime library ('libm2s-glu').\n";

static char *glu_err_native =
	"\tYou are trying to run natively an application using the Multi2Sim GLU\n"
	"\tlibrary implementation ('libm2s-glu'). Please run this program on top of\n"
	"\tMulti2Sim.\n";

/*static char *glu_err_version =
	"\tYour guest application is using a version of the GLU runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tOpengl runtime library ('libm2s-glu').\n";*/




/* Multi2Sim GLU Runtime required */
#define GLU_RUNTIME_VERSION_MAJOR	0
#define GLU_RUNTIME_VERSION_MINOR	669

struct glu_version_t
{
	int major;
	int minor;
};


void gluInit(int *argcp, char **argv)
{
	struct glu_version_t version;

	int ret;

	/* Runtime function 'init' */
	ret = syscall(GLU_SYSCALL_CODE, glu_call_init, &version);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call glu_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			glu_err_native);

	/* Check that exact major version matches */
	if (version.major != GLU_RUNTIME_VERSION_MAJOR
			|| version.minor < GLU_RUNTIME_VERSION_MINOR)
		fatal("incompatible GLU runtime versions.\n"
			"\tGuest library v. %d.%d / Host implementation v. %d.%d.\n"
			"%s", GLU_RUNTIME_VERSION_MAJOR, GLU_RUNTIME_VERSION_MINOR,
			version.major, version.minor, glu_err_version);

	/* Initialize */
	
}

const GLubyte * gluErrorString(GLenum  error)
 {
 	__GLU_NOT_IMPL__
 	return 0;
 }
