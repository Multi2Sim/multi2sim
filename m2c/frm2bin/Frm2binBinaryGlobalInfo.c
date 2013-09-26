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
#include <stdio.h>

#include "Frm2binBinaryGlobalInfo.h"

void Frm2binBinaryGlobalInfoCreate(Frm2binBinaryGlobalInfo *self)
{
	/* just call parent's constructor */
	ObjectCreate(asObject(self));

	self->info_list = new(List);
	printf("creating info_list, %p", self->info_list);

}

void Frm2binBinaryGlobalInfoDestroy(Frm2binBinaryGlobalInfo *self)
{
	/* do nothing now */
	delete(self->info_list);

}
