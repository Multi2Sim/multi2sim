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

#include "Frm2binBinaryGlobalInfoItem.h"

void Frm2binBinaryGlobalInfoItemCreate(Frm2binBinaryGlobalInfoItem *self)
{
	/* just call parent's constructor */
	ObjectCreate(asObject(self));

	/* set values to member variables to default value */

	/* id_A, fixed value */
	self->id_A = 0x00081204;

	/* MinStackSize, not known, asfermi
	 * just set it to zero */
	self->MinStackSize = 0;

	/* id_B, fixed value */
	self->id_B = 0x00081104;

	/* FrameSize, not known, asfermi
	 * just set it to zero */
	self->FrameSize = 0;
}

void Frm2binBinaryGlobalInfoItemDestroy(Frm2binBinaryGlobalInfoItem *self)
{
	/* do nothing now */
}
