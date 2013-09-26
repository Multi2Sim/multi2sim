#include <lib/class/array.h>
#include <lib/class/elf-writer.h>
#include <lib/class/hash-table.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
//#include <lib/util/debug.h>
//#include <lib/util/hash-table.h>
#include <lib/util/list.h>
//#include <lib/util/string.h>

#include "Frm2binBinaryKernelInfo.h"

void Frm2binBinaryKernelInfoCreate(Frm2binBinaryKernelInfo *self)
{
	/* create string for the kernel name */
	// this string will be created inside parser.y
	// when we see a new label for kernel function

	/* just call parent's constructor */
	ELFWriterCreate(asELFWriter(self));

	/* set default value to some of the members */
	self->paramCbank.id = 0x00080a04;

	/* create the list for kparam_info */
	self->kparam_info_list = list_create();
}

void Frm2binBinaryKernelInfoDestroy(Frm2binBinaryKernelInfo *self)
{
	/* delete the kparam_info list */
	list_free(self->kparam_info_list);

}
