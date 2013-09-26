#include <lib/class/array.h>
#include <lib/class/elf-writer.h>
#include <lib/class/hash-table.h>
#include <lib/class/string.h>
#include <lib/class/list.h>
#include <lib/mhandle/mhandle.h>
//#include <lib/util/debug.h>
//#include <lib/util/hash-table.h>
//#include <lib/util/list.h>
//#include <lib/util/string.h>

#include "Frm2binBinary.h"
#include "Frm2binBinaryKernel.h"
#include "Frm2binBinaryGlobalInfo.h"

void Frm2binBinaryCreate(Frm2binBinary *self)
{
	/* just call parent's constructor */
	ELFWriterCreate(asELFWriter(self));

	/* create the kernel_list */
	self->kernel_list = new(List);

	/* create global_info object, only one */
	self->global_info = new(Frm2binBinaryGlobalInfo);

	/* initialize numKernel to 0 */
	self->numKernel = 0;
}

void Frm2binBinaryDestroy(Frm2binBinary *self)
{
	/* delete all items inside the kernel_list */
	// TO BE FINISHED

	/* delete the kernel_list */
	delete(self->kernel_list);

	/* delete the global_info */
	delete(self->global_info);


	/* just call parent's destructor */
	ELFWriterDestroy(asELFWriter(self));
}
