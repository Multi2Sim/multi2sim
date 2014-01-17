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

#include <ctype.h>
#include <unistd.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "runtime.h"


static struct list_t *runtime_list;

static char *runtime_err_redirect_current =
	"\tThe dynamic linker of your application seems to be accessing a \n"
	"\truntime library. A copy of this runtime library has been found \n"
	"\tin the current working directory, and this copy will be used by\n"
	"\tthe linker. As a safer and portable option, please link your \n"
	"\tguest program statically with the corresponding runtime.\n";

static char *runtime_err_redirect_root =
	"\tThe dynamic linker of your application seems to be accessing a\n"
	"\truntime library. A copy of this runtime library has been found\n"
	"\tin '$(TOPDIR)/lib/.libs', while running Multi2Sim from its\n"
	"\tbuild tree root. As a safer and portable option, please link\n"
	"\tyour guest program statically with the corresponding runtime.\n";



/*
 * Private Functions
 */

static int runtime_file_matches_lib(char *file_name, char *lib_name)
{
	/* First, prefix 'lib' must come */
	if (!str_prefix(file_name, "lib"))
		return 0;
	file_name += 3;

	/* Then, we need to find the library name */
	if (!str_prefix(file_name, lib_name))
		return 0;
	file_name += strlen(lib_name);

	/* Then, '.so' */
	if (!str_prefix(file_name, ".so"))
		return 0;
	file_name += 3;

	/* Then a set of '.xx' with version numbers */
	while (*file_name)
	{
		/* Dot */
		if (*file_name != '.')
			return 0;
		file_name++;

		/* At least one digit */
		if (!isdigit(*file_name))
			return 0;
		file_name++;

		/* More digits */
		while (isdigit(*file_name))
			file_name++;
	}

	/* All tests passed - it's the library */
	return 1;
}


/* If Multi2Sim is running as an installed package, this function returns 0.
 * If it is running as part of the build tree, the executable is assumed to
 * be found in $(TOPDIR)/bin/m2s. Directory $(TOPDIR) is returned in 'buf',
 * a string buffer of 'size' bytes. */
static int get_build_tree_root(char *buf, int size)
{
	char exe_name[MAX_PATH_SIZE];
	char *file_name;
	int length;

	/* Get path of 'm2s' executable. */
	exe_name[0] = '\0';
	length = readlink("/proc/self/exe", exe_name, sizeof exe_name);
	if (length < 0 || length >= sizeof exe_name)
	{
		fprintf(stderr, "%s: error when calling 'readlink'", __FUNCTION__);
		exit(1);
	}
	exe_name[length] = '\0';

	/* Discard suffix '/m2s' */
	file_name = rindex(exe_name, '/');
	if (!file_name)
		return 0;
	*file_name = '\0';

	/* Discard suffix '/bin' */
	file_name = rindex(exe_name, '/');
	if (!file_name || strcmp(file_name, "/bin"))
		return 0;
	*file_name = '\0';

	/* The remaining path is $(TOPDIR) */
	snprintf(buf, size, "%s", exe_name);
	return 1;
}



/*
 * Public Functions
 */

void runtime_init(void)
{
	runtime_list = list_create();
}


void runtime_done(void)
{
	struct runtime_t *runtime;
	int index;

	/* Free runtimes */
	LIST_FOR_EACH(runtime_list, index)
	{
		/* Get runtime */
		runtime = list_get(runtime_list, index);

		/* If we have detected unresolved failed attempts to access this
		 * runtime, issue a warning here. */
		if (runtime->open_attempt)
			warning("failed attempts detected to access runtime library '%s'.\n"
					"\tMulti2Sim has detected that the dynamic linker of your guest program\n"
					"\thas attempted to access a runtime and not succeeded. Please make sure\n"
					"\tthat file 'lib%s.so' is present in any of these three locations:\n"
					"\t  - Current working directory.\n"
					"\t  - Multi2Sim build tree at multi2sim/lib/.libs\n"
					"\t  - Default library path, as listed in LD_LIBRARY_PATH environment\n"
					"\t    variable.\n"
					"\tA safer and portable alternative is linking your guest program statically\n"
					"\tto avoid library dependencies at run time.\n",
					runtime->lib_name, runtime->redirect_lib_name);

		/* Free runtime */
		free(runtime->name);
		free(runtime->lib_name);
		free(runtime->redirect_lib_name);
		free(runtime);
	}

	/* Free runtime list */
	list_free(runtime_list);
}


void runtime_register(char *name, char *lib_name, char *redirect_lib_name,
		int syscall_code, runtime_abi_func_t abi_func)
{
	struct runtime_t *runtime;

	/* Create runtime */
	runtime = xcalloc(1, sizeof(struct runtime_t));
	runtime->name = xstrdup(name);
	runtime->lib_name = xstrdup(lib_name);
	runtime->redirect_lib_name = xstrdup(redirect_lib_name);
	runtime->syscall_code = syscall_code;
	runtime->abi_func = abi_func;

	/* Add to list */
	list_add(runtime_list, runtime);
}


int runtime_redirect(char *path, char *redirect_path, int redirect_path_size)
{
	char current_dir[MAX_PATH_SIZE];
	char build_tree_root[MAX_PATH_SIZE];
	char buf[MAX_PATH_SIZE];
	char *file_name;

	struct runtime_t *runtime;

	int index;

	/* If the path is empty, no redirection. */
	if (!*path)
		return 0;

	/* Get file name from path, making sure that the value passed to 'path'
	 * is a full path. */
	file_name = rindex(path, '/');
	if (!file_name)
		panic("%s: value in 'path' must be a full path", __FUNCTION__);
	file_name++;  /* Discard slash */

	/* First, detect attempts to open the target runtime directly
	 * (e.g., "libm2s-opencl.so"). This happens if the guest program
	 * was linked dynamically with the Multi2Sim runtime instead of
	 * the original library (e.g., "libOpenCL.so"). */
	/* For each runtime registered, detect attempts to open either the original
	 * library name (e.g., 'libOpenCL.so'), or directly the Multi2Sim runtime
	 * library provided as a replacement (e.g., 'libm2s-opencl.so'). In either
	 * case, look for occurrences of it in:
	 *   - The original path requested by the dynamic linker, which will
	 *     explore all library paths listed in environment variable
	 *     LD_LIBRARY_PATH.
	 *   - The current working directory.
	 *   - Multi2Sim's build tree at $(TOPDIR)/lib/.libs, which is where
	 *     libtool places the library when running 'make'.
	 */
	LIST_FOR_EACH(runtime_list, index)
	{
		int matches_lib;
		int matches_redirect_lib;

		/* Get runtime */
		runtime = list_get(runtime_list, index);

		/* Check if path matches the original runtime name (e.g., 'libOpenCL.so'),
		 * or the runtime it should be redirected to (e.g., 'libm2s-opencl.so'). */
		matches_lib = runtime_file_matches_lib(file_name, runtime->lib_name);
		matches_redirect_lib = runtime_file_matches_lib(file_name, runtime->redirect_lib_name);
		if (!matches_lib && !matches_redirect_lib)
			continue;

		/* If a Multi2Sim runtime was given directly, try first to open the
		 * original path provided. */
		if (matches_redirect_lib && !access(path, R_OK))
		{
			runtime->open_attempt = 0;
			return 0;  /* No redirection */
		}

		/* If this failed, try to open Multi2Sim runtime in current path. */
		if (!getcwd(current_dir, sizeof current_dir))
			fatal("%s: buffer too small", __FUNCTION__);
		snprintf(buf, sizeof buf, "%s/lib%s.so", current_dir,
				runtime->redirect_lib_name);
		if (!access(buf, R_OK))
		{
			snprintf(redirect_path, redirect_path_size, "%s", buf);
			runtime->open_attempt = 0;
			warning("path '%s' has been redirected to\n"
					"\tpath '%s' while loading runtime '%s'.\n%s",
					path, redirect_path, runtime->lib_name,
					runtime_err_redirect_current);
			return 1;  /* Redirection happened */
		}

		/* Try to open the file in '$(TOPDIR)/lib/.libs' directory */
		if (get_build_tree_root(build_tree_root, sizeof build_tree_root))
		{
			snprintf(buf, sizeof buf, "%s/lib/.libs/lib%s.so",
					build_tree_root, runtime->redirect_lib_name);
			if (!access(buf, R_OK))
			{
				snprintf(redirect_path, redirect_path_size, "%s", buf);
				runtime->open_attempt = 0;
				warning("path '%s' has been redirected to\n"
						"\tpath '%s' while loading runtime '%s'.\n%s",
						path, redirect_path, runtime->lib_name,
						runtime_err_redirect_root);
				return 1;
			}
		}

		/* All attempts to locate the library have failed so far. Record
		 * this in order to give a detailed report at the end of the simulation. */
		runtime->open_attempt = 1;
	}

	return 0;
}


struct runtime_t *runtime_get_from_syscall_code(int syscall_code)
{
	struct runtime_t *runtime;
	int index;

	/* Find a runtime associated with the system call code and run its
	 * corresponding ABI function. */
	LIST_FOR_EACH(runtime_list, index)
	{
		runtime = list_get(runtime_list, index);
		if (runtime->syscall_code == syscall_code)
			return runtime;
	}

	/* No match found for system call code */
	return NULL;
}


int runtime_abi_call(struct runtime_t *runtime, void *user_data)
{
	return runtime->abi_func(user_data);
}

