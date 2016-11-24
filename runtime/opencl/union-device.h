#ifndef __UNION_DEVICE_H__
#define __UNION_DEVICE_H__

#include "list.h"
#include "device.h"

struct opencl_union_device_t
{
	enum opencl_runtime_type_t type;  /* First field */

	struct opencl_device_t *parent;
	struct list_t *devices;
};

struct opencl_union_device_t *opencl_union_device_create(struct opencl_device_t *parent, struct list_t *devices);
void opencl_union_device_free(struct opencl_union_device_t *device);

#endif
