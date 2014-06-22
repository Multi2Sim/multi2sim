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


#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/regs.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <memory/memory.h>

#include "glew.h"


static char *glew_err_code =
	"\tAn invalid function code was generated by your application in a GLEW system\n"
	"\tcall. Probably, this means that your application is using an incompatible\n"
	"\tversion of the Multi2Sim GLEW runtime library ('libm2s-glew'). Please\n"
	"\trecompile your application and try again.\n";


/* Debug */
int glew_debug_category;


/*
 * GLEW calls
 */

/* List of GLEW runtime calls */
enum glew_call_t
{
	glew_call_invalid = 0,
#define X86_GLEW_DEFINE_CALL(name, code) glew_call_##name = code,
#include "glew.dat"
#undef X86_GLEW_DEFINE_CALL
	glew_call_count
};


/* List of GLEW runtime call names */
char *glew_call_name[glew_call_count + 1] =
{
	NULL,
#define X86_GLEW_DEFINE_CALL(name, code) #name,
#include "glew.dat"
#undef X86_GLEW_DEFINE_CALL
	NULL
};


/* Forward declarations of GLEW runtime functions */
#define X86_GLEW_DEFINE_CALL(name, code) \
	static int glew_abi_##name(X86Context *ctx);
#include "glew.dat"
#undef X86_GLEW_DEFINE_CALL

/* List of GLEW runtime functions */
typedef int (*glew_abi_func_t)(X86Context *ctx);
static glew_abi_func_t glew_abi_table[glew_call_count + 1] =
{
	NULL,
#define X86_GLEW_DEFINE_CALL(name, code) glew_abi_##name,
#include "glew.dat"
#undef X86_GLEW_DEFINE_CALL
	NULL
};





/*
 * Class 'GlewDriver'
 */

void GlewDriverCreate(GlewDriver *self, X86Emu *emu)
{
	/* Parent */
	DriverCreate(asDriver(self), emu);

	/* Assign driver to host emulator */
	emu->glew_driver = self;

	/* Debug */
	glew_debug("Initializing Glew...\n");
}


void GlewDriverDestroy(GlewDriver *self)
{
	glew_debug("Finalizing Glew...\n");
}




/*
 * GLEW global functions
 */

int GlewDriverCall(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int code;
	int ret;

	/* Function code */
	code = regs->ebx;
	if (code <= glew_call_invalid || code >= glew_call_count)
		fatal("%s: invalid GLEW function (code %d).\n%s",
			__FUNCTION__, code, glew_err_code);

	/* Debug */
	glew_debug("GLEW runtime call '%s' (code %d)\n",
		glew_call_name[code], code);

	/* Call GLEW function */
	assert(glew_abi_table[code]);
	ret = glew_abi_table[code](ctx);

	/* Return value */
	return ret;
}




/*
 * GLEW call #1 - init
 *
 * @param struct glew_version_t *version;
 *	Structure where the version of the GLEW runtime implementation will be
 *	dumped. To succeed, the major version should match in the runtime
 *	library (guest) and runtime implementation (host), whereas the minor
 *	version should be equal or higher in the implementation (host).
 *
 *	Features should be added to the GLEW runtime (guest and host) using the
 *	following rules:
 *	1)  If the guest library requires a new feature from the host
 *	    implementation, the feature is added to the host, and the minor
 *	    version is updated to the current Multi2Sim SVN revision both in
 *	    host and guest.
 *          All previous services provided by the host should remain available
 *          and backward-compatible. Executing a newer library on the older
 *          simulator will fail, but an older library on the newer simulator
 *          will succeed.
 *      2)  If a new feature is added that affects older services of the host
 *          implementation breaking backward compatibility, the major version is
 *          increased by 1 in the host and guest code.
 *          Executing a library with a different (lower or higher) major version
 *          than the host implementation will fail.
 *
 * @return
 *	The runtime implementation version is return in argument 'version'.
 *	The return value is always 0.
 */

#define X86_GLEW_RUNTIME_VERSION_MAJOR	0
#define X86_GLEW_RUNTIME_VERSION_MINOR	669

struct glew_version_t
{
	int major;
	int minor;
};

static int glew_abi_init(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int version_ptr;
	struct glew_version_t version;

	/* Arguments */
	version_ptr = regs->ecx;
	glew_debug("\tversion_ptr=0x%x\n", version_ptr);

	/* Return version */
	assert(sizeof(struct glew_version_t) == 8);
	version.major = X86_GLEW_RUNTIME_VERSION_MAJOR;
	version.minor = X86_GLEW_RUNTIME_VERSION_MINOR;
	mem_write(mem, version_ptr, sizeof version, &version);
	glew_debug("\tGLEW Runtime host implementation v. %d.%d\n", version.major, version.minor);

	/* Return success */
	return 0;
}

