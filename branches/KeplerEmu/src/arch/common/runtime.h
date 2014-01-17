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

#ifndef ARCH_COMMON_RUNTIME_H
#define ARCH_COMMON_RUNTIME_H

/* Function acting as ABI between the runtime and the underlying driver.
 * The function takes a generic pointer as an argument, and returns an integer
 * value. */
typedef int (*runtime_abi_func_t)(void *user_data);

/* Runtime type */
struct runtime_t
{
	/* Runtime name, just used for debug info. */
	char *name;

	/* Name of the library to intercept during the execution of the
	 * 'open' system call. */
	char *lib_name;

	/* Name of the library to redirect it to, searching either in the
	 * source tree or Multi2Sim installation path. */
	char *redirect_lib_name;

	/* Record here failed attempts to access this runtime. This is used
	 * to issue a warning to the user at the end of the simulation. */
	int open_attempt;

	/* System call code used for the driver ABI. When the runtime needs
	 * to communicate with the driver, it is done through this system call. */
	int syscall_code;

	/* Function called when the system call 'syscall_code' is intercepted. */
	runtime_abi_func_t abi_func;
};


/* Initialization/finalization of runtimes management. */
void runtime_init(void);
void runtime_done(void);

/* Register a runtime library, i.e., user-space library code linked with the
 * guest program running on Multi2Sim. When a system call 'open' accesses a file
 * with name 'lib_name' (e.g., "OpenCL"), it redirects it to
 * 'redirect_lib_name' (e.g., "m2s-opencl").
 * Argument 'syscall_code' is the special unique system call used to communicate
 * the runtime with the driver (ABI). Argument 'abi_func' is the function to
 * be called when this system call is encountered. */
void runtime_register(char *name, char *lib_name, char *redirect_lib_name,
		int syscall_code, runtime_abi_func_t abi_func);

/* Function used in the 'open' system call to redirect a runtime. If 'path' is a
 * registered runtime, the function returns True and writes the redirection path
 * into 'redirect_path' of size 'redirect_path_size'. If 'path' is not a
 * registered runtime, the function returns False. */
int runtime_redirect(char *path, char *redirect_path, int redirect_path_size);

/* Return the runtime associated with a given system call code. If no runtime
 * has been registered with this code, return NULL. */
struct runtime_t *runtime_get_from_syscall_code(int syscall_code);

/* Run the ABI function associated with a runtime. The function returns the
 * same value returned by the ABI function itself. */
int runtime_abi_call(struct runtime_t *runtime, void *user_data);

#endif

