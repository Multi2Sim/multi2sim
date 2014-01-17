#include <lib/class/array.h>
#include <lib/class/elf-writer.h>
#include <lib/class/hash-table.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
//#include <lib/util/debug.h>
//#include <lib/util/hash-table.h>
//#include <lib/util/list.h>
//#include <lib/util/string.h>

#include "Frm2binBinaryKernel.h"
#include "Frm2binBinaryKernelInfo.h"

void Frm2binBinaryKernelCreate(Frm2binBinaryKernel *self)
{
	/* create string for the kernel name */
	// this string will be created inside parser.y
	// when we see a new label for kernel function

	/* just call parent's constructor */
	ELFWriterCreate(asELFWriter(self));

	/* create a Frm2binBinaryKernelInfo object */
	self->kInfo = new(Frm2binBinaryKernelInfo);

	/* initialize argument size to zero */
	self->argSize = 0;
}

void Frm2binBinaryKernelDestroy(Frm2binBinaryKernel *self)
{
	/* delete string for the name */
	delete(self->name);

	/* delete the kernelInfo */
	delete(self->kInfo);

}
