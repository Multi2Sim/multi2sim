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
#include "Frm2binBinaryGlobalInfoItem.h"

void Frm2binBinaryGlobalInfoCreate(Frm2binBinaryGlobalInfo *self)
{
	/* just call parent's constructor */
	ObjectCreate(asObject(self));

	self->info_list = new(List);

}

void Frm2binBinaryGlobalInfoDestroy(Frm2binBinaryGlobalInfo *self)
{
	/* delete all the kernelInfoItem */

	ListIterator *tmpIterator = new(ListIterator, self->info_list);
	Frm2binBinaryGlobalInfoItem *tmpInfoItem;

	/* iterate all kernel inside the list and deletet them */
	ListIteratorForEach(tmpIterator, tmpInfoItem, Frm2binBinaryGlobalInfoItem)
	{
		delete(tmpInfoItem);
	}

	/* delete the iterator */
	delete(tmpIterator);

	/* delete the infoItem list */
	delete(self->info_list);

}
