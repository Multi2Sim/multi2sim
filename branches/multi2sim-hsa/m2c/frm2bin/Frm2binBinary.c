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

	/* initialize numKernel to 0 */
	self->numKernel = 0;

	/* create global_info */
	self->global_info = new(Frm2binBinaryGlobalInfo);
}

void Frm2binBinaryDestroy(Frm2binBinary *self)
{
	/* delete all items inside the kernel_list */

	ListIterator *tmpIterator = new(ListIterator, self->kernel_list);
	Frm2binBinaryKernel *tmpKernel;

	/* iterate all kernel inside the list and deletet them */
	ListIteratorForEach(tmpIterator, tmpKernel, Frm2binBinaryKernel)
	{
		delete(tmpKernel);
	}

	/* delete teh iterator */
	delete(tmpIterator);

	/* delete the kernel_list */
	delete(self->kernel_list);

	/* delete global_info */
	delete(self->global_info);

}
