#include <string.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>


#include "metadata.h"





struct si2bin_arg_val_t *si2bin_arg_val_create(char *type, int num_elem, int offset)
{
	struct si2bin_arg_val_t *arg_val;

	/* Initialize */
	arg_val = xcalloc(1, sizeof(struct si2bin_arg_val_t));

	
	if (strcmp(type, "i32") &&
		strcmp(type, "float"))
	{
		fatal("Unrecognized value type");
	}

	arg_val->type = xstrdup(type);
	arg_val->offset = offset;
	arg_val->num_elem = num_elem;

	return arg_val;
}

void si2bin_arg_val_free(struct si2bin_arg_val_t *arg_val)
{
	free(arg_val->type);
	free(arg_val);
}

struct si2bin_arg_ptr_t *si2bin_arg_ptr_create(char *type, int offset, char *mem_type)
{
	struct si2bin_arg_ptr_t *arg_ptr;

	/* Initialize */
	arg_ptr = xcalloc(1, sizeof(struct si2bin_arg_ptr_t));


	if (strcmp(type, "i32") &&
		strcmp(type, "float"))
	{
		fatal("Unrecognized pointer type");
	}

	arg_ptr->type = xstrdup(type);
	arg_ptr->offset = offset;
	arg_ptr->mem_type = xstrdup(mem_type);
	
	return arg_ptr;
}


void si2bin_arg_ptr_free(struct si2bin_arg_ptr_t *arg_ptr)
{
	free(arg_ptr->type);
	free(arg_ptr->mem_type);
	free(arg_ptr);
}


struct si2bin_metadata_t *si2bin_metadata_create(char *name)
{
	struct si2bin_metadata_t *metadata;

	/* Initialize */
	metadata = xcalloc(1, sizeof(struct si2bin_metadata_t));
	metadata->name = xstrdup(name);

	metadata->const_buffer_list = list_create();
	metadata->arg_val_list = list_create();
	metadata->arg_ptr_list = list_create();
	
	return metadata;
}

void si2bin_metadata_free(struct si2bin_metadata_t *metadata)
{
	list_free(metadata->const_buffer_list);
	list_free(metadata->arg_val_list);
	list_free(metadata->arg_ptr_list);
	free(metadata->name);
	free(metadata);
}

void si2bin_metadata_add_arg_val(struct si2bin_metadata_t *metadata,
	struct si2bin_arg_val_t *arg_val)
{
	list_add(metadata->arg_val_list, arg_val);
	metadata->offset += arg_val->offset;
}

void si2bin_metadata_add_arg_ptr(struct si2bin_metadata_t *metadata,
	struct si2bin_arg_ptr_t *arg_ptr)
{
	list_add(metadata->arg_ptr_list, arg_ptr);
	metadata->offset += arg_ptr->offset;
}
