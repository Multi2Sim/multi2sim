/*
 * Id.h
 *
 *  Created on: Oct 22, 2013
 *      Author: Chulian Zhang
 */

#ifndef FRM2BIN_ID_H
#define FRM2BIN_ID_H

#ifdef __cplusplus

#include <string>

namespace frm2bin
{

} /* namespace frm2bin */

#endif  /* __cplusplus */


/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif

struct frm_id_t
{
	char *name;
};

struct frm_id_t *frm_id_create(char *name);
void frm_id_free(struct frm_id_t *id);

#ifdef __cplusplus
}
#endif

#endif /* FRM2BIN_ID_H_ */
