#ifndef __DEVICE_INTERFACE_H__
#define __DEVICE_INTERFACE_H__

#include "../include/CL/cl.h"


extern const char *opencl_device_full_profile;
extern const char *opencl_device_opencl_c_version;
extern const char *opencl_device_name;
extern const char *opencl_device_vendor;
extern const char *opencl_device_extensions;
extern const char *opencl_device_driver_version;


struct clrt_device_type_t;



/* The Device properties...known to both the device and OpenCL */
struct _cl_device_id
{
	cl_int address_bits;
	cl_bool available;
	cl_bool compiler_available;
	cl_device_fp_config double_fp_config;
	cl_bool endian_little;
	cl_bool error_correction_support;
	cl_device_exec_capabilities execution_capabilities;
	const char *extensions;
	cl_int global_mem_cache_size;
	cl_device_mem_cache_type global_mem_cache_type;
	cl_uint global_mem_cacheline_size;
	cl_int global_mem_size;
	cl_bool host_unified_memory;
	cl_bool image_support;
	cl_int image2d_max_height;
	cl_int image2d_max_width;
	cl_int image3d_max_depth;
	cl_int image3d_max_height;
	cl_int image3d_max_width;
	cl_ulong local_mem_size;
	cl_device_local_mem_type local_mem_type;
	cl_int max_clock_frequency;
	cl_int max_compute_units;
	cl_uint max_constant_args;
	cl_ulong max_constant_buffer_size;
	cl_ulong max_mem_alloc_size;
	size_t max_parameter_size;
	cl_uint max_read_image_args;
	cl_int max_samplers;
	size_t max_work_group_size;
	cl_uint max_work_item_dimensions;
	size_t max_work_item_sizes[3];
	cl_uint max_write_image_args;
	cl_uint mem_base_addr_align;
	cl_uint min_data_type_align_size;
	const char *name;
	const char *opencl_c_version;
	cl_uint vector_width_char;
	cl_uint vector_width_short;
	cl_uint vector_width_int;
	cl_uint vector_width_long;
	cl_uint vector_width_float;
	cl_uint vector_width_double;
	cl_uint vector_width_half;
	const char *profile;
	size_t profiling_timer_resolution;
	cl_command_queue_properties queue_properties;
	cl_device_fp_config single_fp_config;
	cl_device_type type;
	const char *vendor;
	cl_uint vendor_id;
	const char *version;

	/* Device-dependent implementation */
	void *device;

	/* Don't set in driver.  Will be set by framework */
	struct clrt_device_type_t *device_type;
};

/* Interface that each device driver must implement */

/* same style as clGetDeviceIDs, except this time 
 * it is the framework querying a driver */
typedef cl_int (*clrt_device_type_init_devices_t)(
	cl_uint num_entries, 
	cl_device_id *devices, 
	cl_uint *num_devices);

/* check if a binary blob is a valid program */
typedef cl_bool (*clrt_device_type_is_valid_binary_t)(
	size_t length,
	const unsigned char *binary);

/* create a kernel object who's parameters can be set */
typedef void *(*clrt_device_type_create_kernel_t)(
	void *handle, 
	const char *kernel_name, 
	cl_int *errcode_ret);

/* set the argument of a kernel */
typedef cl_int (*clrt_device_type_set_kernel_arg_t)(
	void *kernel, 
	cl_uint arg_index, 
	size_t arg_size, 
	const void *arg_value);

/* execute on a device */
typedef void (*clrt_device_exceute_ndrange_t)(
	void *device,
	void *kernel, 
	cl_uint work_dim, 
	const size_t *global_work_offset, 
	const size_t *global_work_size, 
	const size_t *local_work_size);

/* validate that a kernel has properly set parameters */
typedef cl_int (*clrt_device_check_kernel_t)(
	void *kernel);

/* destroy a kernel object */
typedef void (*clrt_device_kernel_destroy_t)(
	void *kernel);

struct clrt_device_type_t
{
	clrt_device_type_init_devices_t init_devices;	
	clrt_device_type_is_valid_binary_t valid_binary;
	clrt_device_type_create_kernel_t create_kernel;
	clrt_device_type_set_kernel_arg_t set_kernel_arg;
	clrt_device_exceute_ndrange_t execute_ndrange;
	clrt_device_check_kernel_t check_kernel;
	clrt_device_kernel_destroy_t kernel_destroy;
};

/* create a device type */
typedef struct clrt_device_type_t *(*clrt_device_type_create_t)(void);

/* Calls that the device can make to the OpenCL Runtime */

void *clrt_get_address_of_buffer_object(cl_mem buffer);

/* Executing in native mode (vs. simulation mode on Multi2Sim) */
extern int m2s_clrt_native_mode;



#endif
