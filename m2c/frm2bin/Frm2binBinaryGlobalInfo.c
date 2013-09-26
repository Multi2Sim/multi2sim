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

#include "Frm2binBinaryGlobalInfo.h"

void Frm2binBinaryGlobalInfoCreate(Frm2binBinaryGlobalInfo *self)
{
	/* create a new list */
	self->info_list = new(List);
	/* just call parent's constructor */
	ELFWriterCreate(asELFWriter(self));
}

void Frm2binBinaryGlobalInfoDestroy(Frm2binBinaryGlobalInfo *self)
{
	/* delete the list */
	delete(self->info_list);
	/* just call parent's destructor */
	ELFWriterDestroy(asELFWriter(self));
}
