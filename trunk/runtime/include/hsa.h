////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2014 ADVANCED MICRO DEVICES, INC.
//
// AMD is granting you permission to use this software and documentation(if any)
// (collectively, the "Materials") pursuant to the terms and conditions of the
// Software License Agreement included with the Materials.If you do not have a
// copy of the Software License Agreement, contact your AMD representative for a
// copy.
//
// You agree that you will not reverse engineer or decompile the Materials, in
// whole or in part, except as allowed by applicable law.
//
// WARRANTY DISCLAIMER : THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND.AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON - INFRINGEMENT, THAT THE
// SOFTWARE WILL RUN UNINTERRUPTED OR ERROR - FREE OR WARRANTIES ARISING FROM
// CUSTOM OF TRADE OR COURSE OF USAGE.THE ENTIRE RISK ASSOCIATED WITH THE USE OF
// THE SOFTWARE IS ASSUMED BY YOU.Some jurisdictions do not allow the exclusion
// of implied warranties, so the above exclusion may not apply to You.
//
// LIMITATION OF LIABILITY AND INDEMNIFICATION : AMD AND ITS LICENSORS WILL NOT,
// UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
// INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
// THE SOFTWARE OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.In no event shall AMD's total
// liability to You for all damages, losses, and causes of action (whether in
// contract, tort (including negligence) or otherwise) exceed the amount of $100
// USD.  You agree to defend, indemnify and hold harmless AMD and its licensors,
// and any of their directors, officers, employees, affiliates or agents from
// and against any and all loss, damage, liability and other expenses (including
// reasonable attorneys' fees), resulting from Your use of the Software or
// violation of the terms and conditions of this Agreement.
//
// U.S.GOVERNMENT RESTRICTED RIGHTS : The Materials are provided with
// "RESTRICTED RIGHTS." Use, duplication, or disclosure by the Government is
// subject to the restrictions as set forth in FAR 52.227 - 14 and DFAR252.227 -
// 7013, et seq., or its successor.Use of the Materials by the Government
// constitutes acknowledgement of AMD's proprietary rights in them.
//
// EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
//                      stated in the Software License Agreement.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef HSA_RUNTIME_INC_HSA_H_
#define HSA_RUNTIME_INC_HSA_H_

#include "stdint.h"
#include "stddef.h"

// Calling convention and import macros
#define HSA_CALL

#ifndef HSA_IMPORT
#if defined(__GNUC__)
#define HSA_IMPORT
#elif defined(_MSC_VER)
#define HSA_IMPORT __declspec(dllimport)
#else
#error \
    "Your compiler is not recognized.  Add a library import macro to support " \
    "it or define HSA_API to the proper import property."
#endif
#endif

#undef HSA_API
#define HSA_API HSA_IMPORT HSA_CALL

// Structure alignment macro
#ifndef __ALIGNED__
#if defined(__GNUC__)
#define __ALIGNED__(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#define __ALIGNED__(x) __declspec(align(x))
#else
#error \
    "Your compiler is not recognized.  Add an alignment macro to support it "  \
    "or define __ALIGNED__(x) to the proper alignment property."
#endif
#endif

// Detect and set large model builds.
#undef HSA_LARGE_MODEL
#if defined(__LP64__) || defined(_M_X64)
#define HSA_LARGE_MODEL
#endif

//---------------------------------------------------------------------------//
//    Constants                                                              //
//---------------------------------------------------------------------------//

#define HSA_ARGUMENT_ALIGN_BYTES 16
#define HSA_QUEUE_ALIGN_BYTES 64
#define HSA_PACKET_ALIGN_BYTES 64

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

//---------------------------------------------------------------------------//
//    Enumerations and Basic Types                                           //
//---------------------------------------------------------------------------//

typedef enum hsa_status_s {
  HSA_STATUS_SUCCESS = 0,
  HSA_STATUS_INFO_BREAK = 0x1,
  HSA_EXT_STATUS_INFO_ALREADY_INITIALIZED = 0x4000,
  HSA_EXT_STATUS_INFO_UNRECOGNIZED_OPTIONS = 0x4001,
  HSA_STATUS_ERROR = 0x10000,
  HSA_STATUS_ERROR_INVALID_ARGUMENT = 0x10001,
  HSA_STATUS_ERROR_INVALID_QUEUE_CREATION = 0x10002,
  HSA_STATUS_ERROR_INVALID_ALLOCATION = 0x10003,
  HSA_STATUS_ERROR_INVALID_AGENT = 0x10004,
  HSA_STATUS_ERROR_INVALID_REGION = 0x10005,
  HSA_STATUS_ERROR_INVALID_SIGNAL = 0x10006,
  HSA_STATUS_ERROR_INVALID_QUEUE = 0x10007,
  HSA_STATUS_ERROR_OUT_OF_RESOURCES = 0x10008,
  HSA_STATUS_ERROR_INVALID_PACKET_FORMAT = 0x10009,
  HSA_STATUS_ERROR_RESOURCE_FREE = 0x1000A,
  HSA_STATUS_ERROR_NOT_INITIALIZED = 0x1000B,
  HSA_STATUS_ERROR_REFCOUNT_OVERFLOW = 0x1000C,
  HSA_EXT_STATUS_ERROR_DIRECTIVE_MISMATCH = 0x14000,
  HSA_EXT_STATUS_ERROR_IMAGE_FORMAT_UNSUPPORTED = 0x14001,
  HSA_EXT_STATUS_ERROR_IMAGE_SIZE_UNSUPPORTED = 0x14002
} hsa_status_t;

typedef enum hsa_packet_type_t {
  HSA_PACKET_TYPE_ALWAYS_RESERVED = 0,
  HSA_PACKET_TYPE_INVALID = 1,
  HSA_PACKET_TYPE_DISPATCH = 2,
  HSA_PACKET_TYPE_BARRIER = 3,
  HSA_PACKET_TYPE_AGENT_DISPATCH = 4
} hsa_packet_type_t;

typedef enum hsa_queue_type_t {
  HSA_QUEUE_TYPE_MULTI = 0,
  HSA_QUEUE_TYPE_SINGLE = 1
} hsa_queue_type_t;

typedef enum hsa_queue_feature_t {
  HSA_QUEUE_FEATURE_DISPATCH = 1,
  HSA_QUEUE_FEATURE_AGENT_DISPATCH = 2
} hsa_queue_feature_t;

typedef enum hsa_fence_scope_t {
  HSA_FENCE_SCOPE_NONE = 0,
  HSA_FENCE_SCOPE_COMPONENT = 1,
  HSA_FENCE_SCOPE_SYSTEM = 2
} hsa_fence_scope_t;

typedef enum hsa_wait_expectancy_t {
  HSA_WAIT_EXPECTANCY_SHORT,
  HSA_WAIT_EXPECTANCY_LONG,
  HSA_WAIT_EXPECTANCY_UNKNOWN
} hsa_wait_expectancy_t;

typedef enum hsa_signal_condition_t {
  HSA_EQ,
  HSA_NE,
  HSA_LT,
  HSA_GTE
} hsa_signal_condition_t;

typedef enum hsa_extension_t {
  HSA_EXT_START = 0,
  HSA_EXT_FINALIZER = HSA_EXT_START,
  HSA_EXT_LINKER = 1,
  HSA_EXT_IMAGES = 2,
  HSA_EXT_AMD_PROFILER = 3,
  HSA_SVEXT_START = 10000
} hsa_extension_t;

typedef enum {
	HSA_MACHINE_MODEL_SMALL = 0,
	HSA_MACHINE_MODEL_LARGE = 1
} hsa_machine_model_t;

typedef enum {
	HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT = 0,
	HSA_DEFAULT_FLOAT_ROUNDING_MODE_ZERO = 1,
	HSA_DEFAULT_FLOAT_ROUNDING_MODE_NEAR = 2
} hsa_default_float_rounding_mode_t;

typedef enum hsa_agent_feature_s {
  HSA_AGENT_FEATURE_DISPATCH = 1,
  HSA_AGENT_FEATURE_AGENT_DISPATCH = 2
} hsa_agent_feature_t;

typedef enum hsa_device_type_s {
  HSA_DEVICE_TYPE_CPU = 0,
  HSA_DEVICE_TYPE_GPU = 1,
  HSA_DEVICE_TYPE_DSP = 2
} hsa_device_type_t;

typedef enum hsa_system_info_s {
  HSA_SYSTEM_INFO_VERSION_MAJOR = 0,
  HSA_SYSTEM_INFO_VERSION_MINOR,
  HSA_SYSTEM_INFO_TIMESTAMP,
  HSA_SYSTEM_INFO_TIMESTAMP_FREQUENCY,
  HSA_SYSTEM_INFO_SIGNAL_MAX_WAIT,
} hsa_system_info_t;

typedef enum hsa_agent_info_s {
  HSA_AGENT_INFO_NAME,
  HSA_AGENT_INFO_VENDOR_NAME,
  HSA_AGENT_INFO_FEATURE,
  HSA_AGENT_INFO_WAVEFRONT_SIZE,
  HSA_AGENT_INFO_WORKGROUP_MAX_DIM,
  HSA_AGENT_INFO_WORKGROUP_MAX_SIZE,
  HSA_AGENT_INFO_GRID_MAX_DIM,
  HSA_AGENT_INFO_GRID_MAX_SIZE,
  HSA_AGENT_INFO_FBARRIER_MAX_SIZE,
  HSA_AGENT_INFO_QUEUES_MAX,
  HSA_AGENT_INFO_QUEUE_MAX_SIZE,
  HSA_AGENT_INFO_QUEUE_TYPE,
  HSA_AGENT_INFO_NODE,
  HSA_AGENT_INFO_DEVICE,
  HSA_AGENT_INFO_CACHE_SIZE,
  HSA_EXT_AGENT_INFO_IMAGE1D_MAX_DIM,
  HSA_EXT_AGENT_INFO_IMAGE2D_MAX_DIM,
  HSA_EXT_AGENT_INFO_IMAGE3D_MAX_DIM,
  HSA_EXT_AGENT_INFO_IMAGE_ARRAY_MAX_SIZE,
  HSA_EXT_AGENT_INFO_IMAGE_RD_MAX,
  HSA_EXT_AGENT_INFO_IMAGE_RDWR_MAX,
  HSA_EXT_AGENT_INFO_SAMPLER_MAX,
  HSA_AGENT_INFO_COUNT
} hsa_agent_info_t;

typedef enum hsa_segment_s {
  HSA_SEGMENT_GLOBAL = 0,
  HSA_SEGMENT_PRIVATE,
  HSA_SEGMENT_GROUP,
  HSA_SEGMENT_KERNARG,
  HSA_SEGMENT_READONLY,
  HSA_SEGMENT_SPILL,
  HSA_SEGMENT_ARG
} hsa_segment_t;

typedef enum hsa_region_flag_s {
  HSA_REGION_FLAG_KERNARG = 1,
  HSA_REGION_FLAG_CACHED_L1 = 2,
  HSA_REGION_FLAG_CACHED_L2 = 4,
  HSA_REGION_FLAG_CACHED_L3 = 8,
  HSA_REGION_FLAG_CACHED_L4 = 16
} hsa_region_flag_t;

typedef enum hsa_region_info_s {
  HSA_REGION_INFO_BASE = 0,
  HSA_REGION_INFO_SIZE,
  HSA_REGION_INFO_AGENT,
  HSA_REGION_INFO_FLAGS,
  HSA_REGION_INFO_SEGMENT,
  HSA_REGION_INFO_ALLOC_MAX_SIZE,
  HSA_REGION_INFO_ALLOC_GRANULE,
  HSA_REGION_INFO_ALLOC_ALIGNMENT,
  HSA_REGION_INFO_BANDWIDTH,
  HSA_REGION_INFO_NODE,
  HSA_REGION_INFO_COUNT
} hsa_region_info_t;

#ifdef HSA_LARGE_MODEL
typedef int64_t hsa_signal_value_t;
#else
typedef int32_t hsa_signal_value_t;
#endif

typedef uint64_t hsa_signal_t;
typedef uint64_t hsa_agent_t;
typedef uint64_t hsa_region_t;

//---------------------------------------------------------------------------//
//    Structures                                                             //
//---------------------------------------------------------------------------//

typedef __ALIGNED__(HSA_QUEUE_ALIGN_BYTES) struct hsa_queue_s {
  hsa_queue_type_t queue_type;
  uint32_t queue_features;
  uint64_t base_address;
  hsa_signal_t doorbell_signal;
  uint32_t size;
  uint32_t id;
  uint64_t service_queue;
} hsa_queue_t;

// typedef struct hsa_packet_header_s {
//	hsa_packet_type_t type : 8;
//	uint16_t barrier : 1;
//	hsa_fence_scope_t acquire_fence_scope : 2;
//	hsa_fence_scope_t release_fence_scope : 2;
//	uint16_t reserved : 3;
//} hsa_packet_header_t;

typedef struct hsa_packet_header_s {
  uint16_t type : 8;
  uint16_t barrier : 1;
  uint16_t acquire_fence_scope : 2;
  uint16_t release_fence_scope : 2;
  uint16_t reserved : 3;
} hsa_packet_header_t;

#define HSA_PACKET_ALIGN __ALIGNED__(HSA_PACKET_ALIGN_BYTES)
typedef HSA_PACKET_ALIGN struct hsa_dispatch_packet_s {
  hsa_packet_header_t header;
  uint16_t dimensions : 2;
  uint16_t reserved : 14;
  uint16_t workgroup_size_x;
  uint16_t workgroup_size_y;
  uint16_t workgroup_size_z;
  uint16_t reserved2;
  uint32_t grid_size_x;
  uint32_t grid_size_y;
  uint32_t grid_size_z;
  uint32_t private_segment_size;
  uint32_t group_segment_size;
  uint64_t kernel_object_address;
  uint64_t kernarg_address;
  uint64_t reserved3;
  hsa_signal_t completion_signal;
} hsa_dispatch_packet_t;

typedef HSA_PACKET_ALIGN struct hsa_agent_dispatch_packet_s {
  hsa_packet_header_t header;
  uint16_t type;
  uint32_t reserved2;
  uint64_t return_address;
  uint64_t arg[4];
  uint64_t reserved3;
  hsa_signal_t completion_signal;
} hsa_agent_dispatch_packet_t;

typedef HSA_PACKET_ALIGN struct hsa_barrier_packet_s {
  hsa_packet_header_t header;
  uint16_t reserved2;
  uint32_t reserved3;
  hsa_signal_t dep_signal[5];
  uint64_t reserved4;
  hsa_signal_t completion_signal;
} hsa_barrier_packet_t;

//---------------------------------------------------------------------------//
//    Common Definitions                                                     //
//---------------------------------------------------------------------------//

typedef uint8_t hsa_powertwo8_t;

typedef enum hsa_powertwo_t {
  HSA_POWERTWO_1 = 0,
  HSA_POWERTWO_2 = 1,
  HSA_POWERTWO_4 = 2,
  HSA_POWERTWO_8 = 3,
  HSA_POWERTWO_16 = 4,
  HSA_POWERTWO_32 = 5,
  HSA_POWERTWO_64 = 6,
  HSA_POWERTWO_128 = 7,
  HSA_POWERTWO_256 = 8
} hsa_powertwo_t;

typedef struct hsa_dim3_s {
  uint32_t x;
  uint32_t y;
  uint32_t z;
} hsa_dim3_t;

enum hsa_dim_t {
  HSA_DIM_X = 0,
  HSA_DIM_Y = 1,
  HSA_DIM_Z = 2,
};

typedef enum {
  HSA_PROFILE_BASE = 0,
  HSA_PROFILE_FULL = 1
} hsa_profile_t;

typedef struct hsa_code_object_s {
  uint64_t handle;
} hsa_code_object_t;

typedef struct hsa_executable_s {
  uint64_t handle;
} hsa_executable_t;

typedef struct hsa_executable_symbol_s {
  uint64_t handle;
} hsa_executable_symbol_t;

typedef enum {
  HSA_EXECUTABLE_STATE_UNFROZEN = 0,
  HSA_EXECUTABLE_STATE_FROZEN = 1
} hsa_executable_state_t;

typedef struct hsa_isa_s {
	uint64_t handle;
} hsa_isa_t;

typedef enum {
	HSA_CODE_OBJECT_TYPE_PROGRAM = 0
} hsa_code_object_type_t;

typedef struct hsa_code_symbol_s {
	uint64_t handle;
} hsa_code_symbol_t;

typedef enum {
	HSA_EXECUTABLE_SYMBOL_INFO_TYPE = 0,
	HSA_EXECUTABLE_SYMBOL_INFO_NAME_LENGTH = 1,
	HSA_EXECUTABLE_SYMBOL_INFO_NAME = 2,
	HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME_LENGTH = 3,
	HSA_EXECUTABLE_SYMBOL_INFO_MODULE_NAME = 4,
	HSA_EXECUTABLE_SYMBOL_INFO_AGENT = 20,
	HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS = 21,
	HSA_EXECUTABLE_SYMBOL_INFO_LINKAGE = 5,
	HSA_EXECUTABLE_SYMBOL_INFO_IS_DEFINITION = 17,
	HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALLOCATION = 6,
	HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SEGMENT = 7,
	HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ALIGNMENT = 8,
	HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_SIZE = 9,
	HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_IS_CONST = 10,
	HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT = 22,
	HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE = 11,
	HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_ALIGNMENT = 12,
	HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE = 13,
	HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE = 14,
	HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_DYNAMIC_CALLSTACK = 15,
	HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_OBJECT = 23,
	HSA_EXECUTABLE_SYMBOL_INFO_INDIRECT_FUNCTION_CALL_CONVENTION = 16
} hsa_executable_symbol_info_t;

typedef struct hsa_runtime_caller_s { uint64_t caller; } hsa_runtime_caller_t;

typedef hsa_status_t (*hsa_runtime_alloc_data_callback_t)(
    hsa_runtime_caller_t caller, size_t byte_size, void **address);

//---------------------------------------------------------------------------//
//  Init/Shutdown routines                                                   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API hsa_init();
hsa_status_t HSA_API hsa_shut_down();

//---------------------------------------------------------------------------//
//  System                                                                   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API
    hsa_system_get_info(hsa_system_info_t attribute, void *value);

//---------------------------------------------------------------------------//
//  Agent                                                                    //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API
    hsa_iterate_agents(hsa_status_t (*callback)(hsa_agent_t agent, void *data),
                       void *data);
hsa_status_t HSA_API hsa_agent_get_info(hsa_agent_t agent,
                                        hsa_agent_info_t attribute,
                                        void *value);

//---------------------------------------------------------------------------//
//  Queues                                                                   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API
    hsa_queue_create(hsa_agent_t agent, size_t size, hsa_queue_type_t type,
                     void (*callback)(hsa_status_t status, hsa_queue_t *queue),
                     const hsa_queue_t *service_queue, hsa_queue_t **queue);
hsa_status_t HSA_API hsa_queue_destroy(hsa_queue_t *queue);
hsa_status_t HSA_API hsa_queue_inactivate(hsa_queue_t *queue);

uint64_t HSA_API hsa_queue_load_read_index_acquire(hsa_queue_t *queue);
uint64_t HSA_API hsa_queue_load_read_index_relaxed(hsa_queue_t *queue);

uint64_t HSA_API hsa_queue_load_write_index_acquire(hsa_queue_t *queue);
uint64_t HSA_API hsa_queue_load_write_index_relaxed(hsa_queue_t *queue);

void HSA_API
    hsa_queue_store_write_index_relaxed(hsa_queue_t *queue, uint64_t value);
void HSA_API
    hsa_queue_store_write_index_release(hsa_queue_t *queue, uint64_t value);

uint64_t HSA_API hsa_queue_cas_write_index_acq_rel(hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value);
uint64_t HSA_API hsa_queue_cas_write_index_acquire(hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value);
uint64_t HSA_API hsa_queue_cas_write_index_relaxed(hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value);
uint64_t HSA_API hsa_queue_cas_write_index_release(hsa_queue_t *queue,
                                                   uint64_t expected,
                                                   uint64_t value);

uint64_t HSA_API
    hsa_queue_add_write_index_acq_rel(hsa_queue_t *queue, uint64_t value);
uint64_t HSA_API
    hsa_queue_add_write_index_acquire(hsa_queue_t *queue, uint64_t value);
uint64_t HSA_API
    hsa_queue_add_write_index_relaxed(hsa_queue_t *queue, uint64_t value);
uint64_t HSA_API
    hsa_queue_add_write_index_release(hsa_queue_t *queue, uint64_t value);

void HSA_API
    hsa_queue_store_read_index_relaxed(hsa_queue_t *queue, uint64_t value);
void HSA_API
    hsa_queue_store_read_index_release(hsa_queue_t *queue, uint64_t value);

//---------------------------------------------------------------------------//
//  Memory                                                                   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API hsa_agent_iterate_regions(
    hsa_agent_t agent,
    hsa_status_t (*callback)(hsa_region_t region, void *data), void *data);
hsa_status_t HSA_API hsa_region_get_info(hsa_region_t region,
                                         hsa_region_info_t attribute,
                                         void *value);

hsa_status_t HSA_API hsa_memory_register(void *address, size_t size);
hsa_status_t HSA_API hsa_memory_deregister(void *address, size_t size);

hsa_status_t HSA_API
    hsa_memory_allocate(hsa_region_t region, size_t size, void **ptr);
hsa_status_t HSA_API hsa_memory_free(void *ptr);

//---------------------------------------------------------------------------//
//  Signals                                                                  //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API
    hsa_signal_create(hsa_signal_value_t initial_value, uint32_t num_consumers,
                      const hsa_agent_t *consumers, hsa_signal_t *signal);
hsa_status_t HSA_API hsa_signal_destroy(hsa_signal_t signal);

hsa_signal_value_t HSA_API hsa_signal_load_relaxed(hsa_signal_t signal);
hsa_signal_value_t HSA_API hsa_signal_load_acquire(hsa_signal_t signal);

void HSA_API
    hsa_signal_store_relaxed(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_store_release(hsa_signal_t signal, hsa_signal_value_t value);

hsa_signal_value_t HSA_API
    hsa_signal_wait_relaxed(hsa_signal_t signal,
                            hsa_signal_condition_t condition,
                            hsa_signal_value_t compare_value,
                            uint64_t timeout_hint,
                            hsa_wait_expectancy_t wait_expectancy_hint);
hsa_signal_value_t HSA_API
    hsa_signal_wait_acquire(hsa_signal_t signal,
                            hsa_signal_condition_t condition,
                            hsa_signal_value_t compare_value,
                            uint64_t timeout_hint,
                            hsa_wait_expectancy_t wait_expectancy_hint);

void HSA_API
    hsa_signal_and_relaxed(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_and_acquire(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_and_release(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_and_acq_rel(hsa_signal_t signal, hsa_signal_value_t value);

void HSA_API
    hsa_signal_or_relaxed(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_or_acquire(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_or_release(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_or_acq_rel(hsa_signal_t signal, hsa_signal_value_t value);

void HSA_API
    hsa_signal_xor_relaxed(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_xor_acquire(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_xor_release(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_xor_acq_rel(hsa_signal_t signal, hsa_signal_value_t value);

void HSA_API
    hsa_signal_add_relaxed(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_add_acquire(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_add_release(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_add_acq_rel(hsa_signal_t signal, hsa_signal_value_t value);

void HSA_API
    hsa_signal_subtract_relaxed(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_subtract_acquire(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_subtract_release(hsa_signal_t signal, hsa_signal_value_t value);
void HSA_API
    hsa_signal_subtract_acq_rel(hsa_signal_t signal, hsa_signal_value_t value);

hsa_signal_value_t HSA_API
    hsa_signal_exchange_relaxed(hsa_signal_t signal, hsa_signal_value_t value);
hsa_signal_value_t HSA_API
    hsa_signal_exchange_acquire(hsa_signal_t signal, hsa_signal_value_t value);
hsa_signal_value_t HSA_API
    hsa_signal_exchange_release(hsa_signal_t signal, hsa_signal_value_t value);
hsa_signal_value_t HSA_API
    hsa_signal_exchange_acq_rel(hsa_signal_t signal, hsa_signal_value_t value);

hsa_signal_value_t HSA_API hsa_signal_cas_relaxed(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value);
hsa_signal_value_t HSA_API hsa_signal_cas_acquire(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value);
hsa_signal_value_t HSA_API hsa_signal_cas_release(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value);
hsa_signal_value_t HSA_API hsa_signal_cas_acq_rel(hsa_signal_t signal,
                                                  hsa_signal_value_t expected,
                                                  hsa_signal_value_t value);

//---------------------------------------------------------------------------//
//  Errors                                                                   //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API
    hsa_status_string(hsa_status_t status, const char **status_string);

//---------------------------------------------------------------------------//
//  Code                                                                     //
//---------------------------------------------------------------------------//

hsa_status_t HSA_API
    hsa_executable_create(hsa_profile_t profile,
		          hsa_executable_state_t executable_stat,
		          const char *options,
		          hsa_executable_t *executable);

hsa_status_t HSA_API
    hsa_executable_get_symbol(hsa_executable_t executable,
                              const char *module_name,
                              const char *symbol_name,
                              hsa_agent_t agent,
                              int32_t call_convention,
                              hsa_executable_symbol_t *symbol);

hsa_status_t hsa_code_object_get_symbol(
	hsa_code_object_t code_object,
	const char *symbol_name,
	hsa_code_symbol_t *symbol);

hsa_status_t hsa_executable_load_code_object(
	hsa_executable_t executable,
	hsa_agent_t agent,
	hsa_code_object_t code_object,
	const char *options);

hsa_status_t hsa_executable_freeze(
	hsa_executable_t executable,
	const char *options);

hsa_status_t hsa_executable_get_symbol(
	hsa_executable_t executable,
	const char *module_name,
	const char *symbol_name,
	hsa_agent_t agent,
	int32_t call_convention,
	hsa_executable_symbol_t *symbol);

hsa_status_t hsa_executable_symbol_get_info(
	hsa_executable_symbol_t executable_symbol,
	hsa_executable_symbol_info_t attribute,
	void *value);

//---------------------------------------------------------------------------//
//  Extensions - NOT YET IMPLEMENTED!                                        //
//---------------------------------------------------------------------------//
hsa_status_t HSA_API
	hsa_extension_query(hsa_extension_t extension, int* result);
//hsa_status_t HSA_API hsa_vendor_extension_query(hsa_extension_t extension,
//												void* extension_structure,
//												int* result);

typedef struct hsa_ext_program_s {
	uint64_t handle;
} hsa_ext_program_t;

typedef char *hsa_ext_module_t;

typedef enum {
	HSA_EXT_FINALIZER_CALL_CONVENTION_AUTO = -1
}hsa_ext_finalizer_call_convention_t;

typedef struct hsa_ext_control_directives_s {
	uint64_t control_directives_mask ;
	uint16_t break_exceptions_mask ;
	uint16_t detect_exceptions_mask ;
	uint32_t max_dynamic_group_size;
	uint64_t max_flat_grid_size;
	uint32_t max_flat_workgroup_size;
	uint32_t reserved1;
	uint64_t required_grid_size[3];
	hsa_dim3_t required_workgroup_size;
	uint8_t required_dim;
	uint8_t reserved2[75];
} hsa_ext_control_directives_t;


hsa_status_t HSA_API hsa_ext_program_create(
	hsa_machine_model_t machine_model,
	hsa_profile_t profile,
	hsa_default_float_rounding_mode_t default_float_rounding_mode,
	const char *options,
	hsa_ext_program_t *program);

hsa_status_t HSA_API hsa_ext_program_add_module(
	hsa_ext_program_t program,
	hsa_ext_module_t module);

hsa_status_t HSA_API hsa_ext_program_finalize(
	hsa_ext_program_t program,
	hsa_isa_t isa,
	int32_t call_convention,
	hsa_ext_control_directives_t control_directives,
	const char *options,
	hsa_code_object_type_t code_object_type,
	hsa_code_object_t *code_object );




#ifdef __cplusplus
}  // end extern "C" block
#endif

#endif  // header guard
