#include <stdio.h>

enum si_inst_arg_type_t 
{
	si_inst_arg_invalid = 0,
	si_inst_arg_scalar_register,
	si_inst_arg_vector_register,
	si_inst_arg_register_range,
	si_inst_arg_mtype_register,
	si_inst_arg_special_register,
	si_inst_arg_literal,
	si_inst_arg_waitcnt,
	si_inst_arg_label,
	si_inst_arg_format	
};

enum si_inst_arg_special_register_type_t
{
	si_inst_arg_special_register_vcc = 0,
	si_inst_arg_special_register_scc
};

struct si_inst_arg_t 
{
	enum si_inst_arg_type_t type;
	
	union
	{
	
		struct
		{
			int id;
		} scalar_register;
		
		struct
		{
			int id_low;
			int id_high;
		} register_range;
		
		struct
		{
			int id;
		} 
		vector_register;
		
		struct
		{
			unsigned int val;
		} literal;
		
		struct
		{
			int vmcnt_active;
			int vmcnt_value;

			int lgkmcnt_active;
			int lgkmcnt_value;

			int expcnt_active;
			int expcnt_value;
		} wait_cnt;
		
		struct
		{
			int id;
		} mtype_register;
		
		struct
		{
			char *data_format;
			char *num_format;
			
			int offen;
			int offset;
		} format;
		
		struct
		{
			enum si_inst_arg_special_register_type_t type;
		} special_register;
		
	} value;
};

struct si_inst_arg_t *si_inst_arg_create(void);
void si_inst_arg_free(struct si_inst_arg_t *inst_arg);
void si_inst_arg_dump(struct si_inst_arg_t *inst_arg, FILE *f);

