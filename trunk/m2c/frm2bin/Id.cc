/*
 * Id.cc
 *
 *  Created on: Oct 22, 2013
 *      Author: Chulian Zhang
 */

#include "Id.h"

using namespace FRM2BIN;

/* constructor */
Id::Id(std::string name_string)
{
	/* set the initial name to empty string */
	name = name_string;
}


/*
 * C Wrapper
 */

#include <lib/mhandle/mhandle.h>
#include "Id.h"


struct frm_id_t *frm_id_create(char *name)
{
	return (frm_id_t *)new Id(std::string(name));
}


void frm_id_free(struct frm_id_t *id)
{
	delete ((Id *)id);
}
