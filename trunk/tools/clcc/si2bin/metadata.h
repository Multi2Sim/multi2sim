#ifndef TOOLS_CLCC_SI2BIN_METADATA_H
#define TOOLS_CLCC_SI2BIN_METADATA_H

/* Forward Declarations */
struct list_t;

/* FIXME - get rid of this whole file, integrated with si2bin_inner_bin_t */

/*
 * Arg_val Object
 */

/* FIXME - this goes to arch/southern-islands/asm/arg.c|h,
 * object named si_arg_t
 * NOTE: make sure that there's not an existing si_arg_t already */

struct si2bin_arg_val_t
{
	char *type;
	int num_elem;
	int offset;
};

struct si2bin_arg_val_t *si2bin_arg_val_create(char *type, int num_elem, int offset);
void si2bin_arg_val_free(struct si2bin_arg_val_t *arg_val);

/*
 * Arg_ptr Object
 */

struct si2bin_arg_ptr_t
{
	char *type;
	int offset;
	char *mem_type;
};

struct si2bin_arg_ptr_t *si2bin_arg_ptr_create(char *type, int offset, char *mem_type);
void si2bin_arg_ptr_free(struct si2bin_arg_ptr_t *arg_ptr);



/*
 * Metadata Object
 */



struct si2bin_metadata_t
{
	
	char *name;

	struct list_t *const_buffer_list;
	
	int uav_table_ptr;

	
	struct list_t *arg_val_list;

	struct list_t *arg_ptr_list;


	int offset;

};

struct si2bin_metadata_t *si2bin_metadata_create(char *name);
void si2bin_metadata_free(struct si2bin_metadata_t *metadata);

void si2bin_metadata_add_arg_val(struct si2bin_metadata_t *metadata, 
	struct si2bin_arg_val_t *arg_val);

void si2bin_metadata_add_arg_ptr(struct si2bin_metadata_t *metadata, 
	struct si2bin_arg_ptr_t *arg_ptr);

#endif
