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

namespace FRM2BIN
{
/* Class definition for Id */
class Id
{
	/* private members */
	std::string name;

	/* public function */
public:
	/* constructor */
	Id(std::string name_string);

	/* get name of the ID */
	std::string GetName() {return name ;}
};

} /* namespace FRM2BIN */

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
