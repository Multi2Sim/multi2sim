/*
 * Id.cc
 *
 *  Created on: Oct 22, 2013
 *      Author: Chulian Zhang
 */

#include "Id.h"

using namespace frm2bin;


/*
 * C Wrapper
 */

#include <lib/mhandle/mhandle.h>
#include "Id.h"


struct frm_id_t *frm_id_create(char *name)
{
	return (frm_id_t *)new std::string(name);
}


void frm_id_free(struct frm_id_t *id)
{
	delete ((std::string *)id);
}
