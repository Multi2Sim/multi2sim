/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_EMU_H
#define ARCH_SOUTHERN_ISLANDS_EMU_EMU_H

#include <stdio.h>

#include <driver/opencl-old/southern-islands/kernel.h>

#define SI_EMU_UAV_TABLE_START 0
#define SI_EMU_MAX_NUM_UAVS 16
#define SI_EMU_UAV_TABLE_SIZE (SI_EMU_MAX_NUM_UAVS * 32)
#define SI_EMU_CONSTANT_BUFFER_TABLE_START (SI_EMU_UAV_TABLE_START + SI_EMU_UAV_TABLE_SIZE)
#define SI_EMU_CONSTANT_BUFFER_TABLE_SIZE 1024
#define SI_EMU_CONSTANT_MEMORY_START (SI_EMU_CONSTANT_BUFFER_TABLE_START + SI_EMU_CONSTANT_BUFFER_TABLE_SIZE)
#define SI_EMU_CONSTANT_BUFFER_SIZE 1024
#define SI_EMU_MAX_CONSTANT_BUFFERS 16 
#define SI_EMU_CONSTANT_MEMORY_SIZE (SI_EMU_CONSTANT_BUFFER_SIZE * SI_EMU_MAX_CONSTANT_BUFFERS)
#define SI_EMU_GLOBAL_MEMORY_START (SI_EMU_CONSTANT_MEMORY_START + SI_EMU_CONSTANT_MEMORY_SIZE)

#define SI_EMU_CALC_CB_ADDR(cb_num) (SI_EMU_CONSTANT_MEMORY_START + SI_EMU_CONSTANT_BUFFER_SIZE*cb_num)


extern int si_emu_num_mapped_const_buffers;

enum si_buf_desc_data_fmt_t
{
	SI_BUF_DESC_DATA_FMT_INVALID = 0,
	SI_BUF_DESC_DATA_FMT_8,
	SI_BUF_DESC_DATA_FMT_16,
	SI_BUF_DESC_DATA_FMT_8_8,
	SI_BUF_DESC_DATA_FMT_32,
	SI_BUF_DESC_DATA_FMT_16_16,
	SI_BUF_DESC_DATA_FMT_10_11_11,
	SI_BUF_DESC_DATA_FMT_10_10_10_2,
	SI_BUF_DESC_DATA_FMT_2_10_10_10,
	SI_BUF_DESC_DATA_FMT_8_8_8_8,
	SI_BUF_DESC_DATA_FMT_32_32,
	SI_BUF_DESC_DATA_FMT_16_16_16_16,
	SI_BUF_DESC_DATA_FMT_32_32_32,
	SI_BUF_DESC_DATA_FMT_32_32_32_32
};

enum si_buf_desc_num_fmt_t
{
	SI_BUF_DESC_NUM_FMT_UNORM = 0,
	SI_BUF_DESC_NUM_FMT_SNORM,
	SI_BUF_DESC_NUM_FMT_USCALED,
	SI_BUF_DESC_NUM_FMT_SSCALED,
	SI_BUF_DESC_NUM_FMT_UINT,
	SI_BUF_DESC_NUM_FMT_SINT,
	SI_BUF_DESC_NUM_FMT_SNORM_OGL,
	SI_BUF_DESC_NUM_FMT_FLOAT
};

/* Table 8.5 in SI documentation */
struct si_buffer_desc_t
{
	unsigned long long base_addr : 48;   /*    [47:0] */
	unsigned int stride          : 14;   /*   [61:48] */
	unsigned int cache_swizzle   : 1;    /*       62  */
	unsigned int swizzle_enable  : 1;    /*       63  */
	unsigned int num_records     : 32;   /*   [95:64] */
	unsigned int dst_sel_x       : 3;    /*   [98:96] */
	unsigned int dst_sel_y       : 3;    /*  [101:99] */
	unsigned int dst_sel_z       : 3;    /* [104:102] */
	unsigned int dst_sel_w       : 3;    /* [107:105] */
	unsigned int num_format      : 3;    /* [110:108] */
	unsigned int data_format     : 4;    /* [114:111] */
	unsigned int elem_size       : 2;    /* [116:115] */
	unsigned int index_stride    : 2;    /* [118:117] */
	unsigned int add_tid_enable  : 1;    /*      119  */
	unsigned int reserved        : 1;    /*      120  */
	unsigned int hash_enable     : 1;    /*      121  */
	unsigned int heap            : 1;    /*      122  */
	unsigned int unused          : 3;    /* [125:123] */
	unsigned int type            : 2;    /* [127:126] */
};

/* Pointers get stored in 2 consecutive 32-bit registers */
struct si_mem_ptr_t
{
	unsigned long long addr : 48;
	unsigned int unused     : 16;
};

struct si_emu_t
{
	/* Timer */
	struct m2s_timer_t *timer;

	/* OpenCL objects */
	struct si_opencl_repo_t *opencl_repo;
	struct si_opencl_platform_t *opencl_platform;
	struct si_opencl_device_t *opencl_device;

	/* List of ND-Ranges */
	struct si_ndrange_t *ndrange_list_head;
	struct si_ndrange_t *ndrange_list_tail;
	int ndrange_list_count;
	int ndrange_list_max;

	/* List of pending ND-Ranges */
	struct si_ndrange_t *pending_ndrange_list_head;
	struct si_ndrange_t *pending_ndrange_list_tail;
	int pending_ndrange_list_count;
	int pending_ndrange_list_max;

	/* List of running ND-Ranges */
	struct si_ndrange_t *running_ndrange_list_head;
	struct si_ndrange_t *running_ndrange_list_tail;
	int running_ndrange_list_count;
	int running_ndrange_list_max;

	/* List of finished ND-Ranges */
	struct si_ndrange_t *finished_ndrange_list_head;
	struct si_ndrange_t *finished_ndrange_list_tail;
	int finished_ndrange_list_count;
	int finished_ndrange_list_max;

	/* Global memory */
	struct mem_t *global_mem;
	unsigned int global_mem_top;

	/* List of set UAVs */
	int valid_uav_list[SI_EMU_MAX_NUM_UAVS];

	/* Statistics */
	int ndrange_count;  /* Number of OpenCL kernels executed */
	long long inst_count;  /* Number of instructions executed by a WF */
	long long scalar_alu_inst_count;  /* Scalar ALU instructions executed */
	long long scalar_mem_inst_count;  /* Scalar mem instructions executed */
	long long branch_inst_count;  /* Branch instructions executed */
	long long vector_alu_inst_count;  /* Vector ALU instructions executed */
	long long lds_inst_count;  /* LDS instructions executed */
	long long vector_mem_inst_count;  /* Vector mem instructions executed */
};

/* Forward declaration */
struct x86_ctx_t;

extern enum arch_sim_kind_t si_emu_sim_kind;

extern long long si_emu_max_cycles;
extern long long si_emu_max_inst;
extern int si_emu_max_kernels;

extern char *si_emu_opencl_binary_name;
extern char *si_emu_opengl_binary_name;
extern char *si_emu_report_file_name;
extern FILE *si_emu_report_file;

extern int si_emu_wavefront_size;

extern char *si_err_opencl_note;
extern char *si_err_opencl_param_note;

extern struct si_emu_t *si_emu;
extern struct arch_t *si_emu_arch;

void si_emu_init(void);
void si_emu_done(void);

void si_emu_dump_summary(FILE *f);

int si_emu_run(void);

void si_emu_disasm(char *path);
void si_emu_opengl_disasm(char *path, int opengl_shader_index);

struct si_buffer_desc_t si_emu_create_buffer_desc(
	struct si_opencl_kernel_arg_t *arg);
void si_emu_insert_into_uav_table(struct si_buffer_desc_t buf_desc,
	struct si_opencl_kernel_arg_t *arg);
void si_emu_set_uav_table_entry(int uav, unsigned int addr);
struct si_buffer_desc_t si_emu_get_uav_table_entry(int uav);
unsigned int si_emu_get_uav_base_addr(int uav);
#endif

