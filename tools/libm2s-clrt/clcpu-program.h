#ifndef __CLCPU_PROGRAM_H__
#define __CLCPU_PROGRAM_H__

typedef void (*clcpu_function_t)(void);

enum clcpu_param_type_t
{
	CLCPU_TYPE_INVALID = 0,
	CLCPU_TYPE_CHAR = 1,
	CLCPU_TYPE_SHORT = 2,
	CLCPU_TYPE_INT = 3,
	CLCPU_TYPE_LONG = 4,
	CLCPU_TYPE_FLOAT = 5,
	CLCPU_TYPE_DOUBLE = 6,
	CLCPU_TYPE_POINTER = 7,
	CLCPU_TYPE_CHAR2 = 8,
	CLCPU_TYPE_CHAR3 = 9,
	CLCPU_TYPE_CHAR4 = 10,
	CLCPU_TYPE_CHAR8 = 11,
	CLCPU_TYPE_CHAR16 = 12,
	CLCPU_TYPE_SHORT2 = 13,
	CLCPU_TYPE_SHORT3 = 14,
	CLCPU_TYPE_SHORT4 = 15,
	CLCPU_TYPE_SHORT8 = 16,
	CLCPU_TYPE_SHORT16 = 17,
	CLCPU_TYPE_INT2 = 18,
	CLCPU_TYPE_INT3 = 19,
	CLCPU_TYPE_INT4 = 20,
	CLCPU_TYPE_INT8 = 21,
	CLCPU_TYPE_INT16 = 22,
	CLCPU_TYPE_LONG2 = 23,
	CLCPU_TYPE_LONG3 = 24,
	CLCPU_TYPE_LONG4 = 25,
	CLCPU_TYPE_LONG8 = 26,
	CLCPU_TYPE_LONG16 = 27,
	CLCPU_TYPE_FLOAT2 = 28,
	CLCPU_TYPE_FLOAT3 = 29,
	CLCPU_TYPE_FLOAT4 = 30,
	CLCPU_TYPE_FLOAT8 = 31,
	CLCPU_TYPE_FLOAT16 = 32,
	CLCPU_TYPE_DOUBLE2 = 33,
	CLCPU_TYPE_DOUBLE3 = 34,
	CLCPU_TYPE_DOUBLE4 = 35,
	CLCPU_TYPE_DOUBLE8 = 36,
	CLCPU_TYPE_DOUBLE16 = 37
};


/*
* Does not start with invalid as this is based on the ELF produced 
* by the AMD APP SDK.
*/

enum clcpu_memory_t
{
	CLRT_MEM_VALUE = 0,
	CLRT_MEM_LOCAL = 1,
	CLRT_MEM_CONSTANT = 2,
	CLRT_MEM_GLOBAL = 3
};


struct clcpu_reg_param_t
{
	int reg[4];
};


struct clcpu_parameter_t
{
	enum clcpu_param_type_t param_type;
	enum clcpu_memory_t mem_type;
	int is_set;
	int stack_offset;
	int is_stack;
	int reg_offset;
	int size;
};


struct clcpu_kernel_t
{
	clcpu_function_t function;
	size_t *metadata;
	unsigned int num_params;
	struct clcpu_parameter_t *param_info;
	int stack_param_words;
	size_t *stack_params;
	struct clcpu_reg_param_t *register_params;
	size_t local_reserved_bytes;
};

#endif
