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

#include "../include/GL/glew.h"
#include "api.h"
#include "debug.h"



/*
 * Error Messages
 */

#define __GLEW_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, glew_err_not_impl);


static char *glew_err_not_impl =
	"\tMulti2Sim provides partial support for the GLEW library.\n"
	"\tTo request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

static char *glew_err_version =
	"\tYour guest application is using a version of the GLEW runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tGLEW runtime library ('libm2s-glew').\n";

static char *glew_err_native =
	"\tYou are trying to run natively an application using the Multi2Sim GLEW\n"
	"\tlibrary implementation ('libm2s-glew'). Please run this program on top of\n"
	"\tMulti2Sim.\n";

/*static char *glew_err_version =
	"\tYour guest application is using a version of the GLEW runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tOpengl runtime library ('libm2s-glew').\n";*/




/* Multi2Sim GLEW Runtime required */
#define GLEW_RUNTIME_VERSION_MAJOR	0
#define GLEW_RUNTIME_VERSION_MINOR	669

struct glew_version_t
{
	int major;
	int minor;
};


GLenum glewInit(int *argcp, char **argv)
{
	struct glew_version_t version;

	int ret;

	/* Runtime function 'init' */
	ret = syscall(GLEW_SYSCALL_CODE, glew_call_init, &version);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call glew_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			glew_err_native);

	/* Check that exact major version matches */
	if (version.major != GLEW_RUNTIME_VERSION_MAJOR
			|| version.minor < GLEW_RUNTIME_VERSION_MINOR)
		fatal("incompatible GLEW runtime versions.\n"
			"\tGuest library v. %d.%d / Host implementation v. %d.%d.\n"
			"%s", GLEW_RUNTIME_VERSION_MAJOR, GLEW_RUNTIME_VERSION_MINOR,
			version.major, version.minor, glew_err_version);

	/* Initialize */
	
	/* Return */
	return 0;
}

 GLboolean glewIsSupported (const char* name)
 {
 	__GLEW_NOT_IMPL__
 	return 0;
 }
