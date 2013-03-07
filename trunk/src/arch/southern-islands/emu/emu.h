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

/* UAV table */
#define SI_EMU_MAX_NUM_UAVS 16
#define SI_EMU_UAV_TABLE_ENTRY_SIZE 32
#define SI_EMU_UAV_TABLE_SIZE (SI_EMU_MAX_NUM_UAVS * SI_EMU_UAV_TABLE_ENTRY_SIZE)

/* Constant buffer table */
#define SI_EMU_MAX_NUM_CONST_BUFS 16
#define SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE 16
#define SI_EMU_CONST_BUF_TABLE_SIZE (SI_EMU_MAX_NUM_CONST_BUFS * SI_EMU_CONST_BUF_TABLE_ENTRY_SIZE)

#define SI_EMU_CONST_BUF_0_SIZE 160  /* Defined in Metadata.pdf */
#define SI_EMU_CONST_BUF_1_SIZE 1024 /* FIXME */

/* Resource table */
#define SI_EMU_MAX_NUM_RESOURCES 16
#define SI_EMU_RESOURCE_TABLE_ENTRY_SIZE 32
#define SI_EMU_RESOURCE_TABLE_SIZE (SI_EMU_MAX_NUM_RESOURCES * SI_EMU_RESOURCE_TABLE_ENTRY_SIZE)


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
	SI_BUF_DESC_NUM_FMT_INVALID = -1,  /* Not part of SI spec */
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
}__attribute__((packed));

/* Table 8.11 in SI documentation */
#if 0
struct si_image_desc_128_t
{
	unsigned long long base_addr : 40;   /*    [39:0] */
	unsigned int mid_lod         : 12;   /*   [51:40] */
	unsigned int data_fmt        : 6;    /*   [57:52] */
	unsigned int num_fmt         : 4;    /*   [61:58] */
	unsigned int                 : 2;    /*   [63:62] */
	unsigned int width           : 14;   /*   [77:64] */
	unsigned int height          : 14;   /*   [91:78] */
	unsigned int perf_mod        : 3;    /*   [94:92] */
	unsigned int interlaced      : 1;    /*       95  */
	unsigned int dst_sel_x       : 3;    /*   [98:96] */
	unsigned int dst_sel_y       : 3;    /*  [101:99] */
	unsigned int dst_sel_z       : 3;    /* [104:102] */
	unsigned int dst_sel_w       : 3;    /* [107:105] */
	unsigned int base_level      : 4;    /* [111:108] */
	unsigned int last_level      : 4;    /* [115:112] */
	unsigned int tiling_idx      : 5;    /* [120:116] */
	unsigned int pow2pad         : 1;    /*      121  */
	unsigned int                 : 2;    /* [123:122] */
	unsigned int type            : 4;    /* [127:124] */
}__attribute__((packed));
#endif

/* Table 8.11 in SI documentation */
struct si_image_desc_t
{
	unsigned long long base_addr : 40;   /*    [39:0] */
	unsigned int mid_lod         : 12;   /*   [51:40] */
	unsigned int data_fmt        : 6;    /*   [57:52] */
	unsigned int num_fmt         : 4;    /*   [61:58] */
	unsigned int                 : 2;    /*   [63:62] */
	unsigned int width           : 14;   /*   [77:64] */
	unsigned int height          : 14;   /*   [91:78] */
	unsigned int perf_mod        : 3;    /*   [94:92] */
	unsigned int interlaced      : 1;    /*       95  */
	unsigned int dst_sel_x       : 3;    /*   [98:96] */
	unsigned int dst_sel_y       : 3;    /*  [101:99] */
	unsigned int dst_sel_z       : 3;    /* [104:102] */
	unsigned int dst_sel_w       : 3;    /* [107:105] */
	unsigned int base_level      : 4;    /* [111:108] */
	unsigned int last_level      : 4;    /* [115:112] */
	unsigned int tiling_idx      : 5;    /* [120:116] */
	unsigned int pow2pad         : 1;    /*      121  */
	unsigned int                 : 2;    /* [123:122] */
	unsigned int type            : 4;    /* [127:124] */
	unsigned int depth           : 13;   /* [140:128] */
	unsigned int pitch           : 14;   /* [154:141] */
	unsigned int                 : 5;    /* [159:155] */
	unsigned int base_array      : 13;   /* [172:160] */
	unsigned int last_array      : 13;   /* [185:173] */
	unsigned int                 : 6;    /* [191:186] */
	unsigned int min_lod_warn    : 12;   /* [203:192] */
	unsigned long long           : 52;   /* [255:204] */
}__attribute__((packed));

/* Table 8.12 in SI documentation */
struct si_sampler_desc_t
{
	unsigned int clamp_x            : 3;    /*     [2:0] */
	unsigned int clamp_y            : 3;    /*     [5:3] */
	unsigned int clamp_z            : 3;    /*     [8:6] */
	unsigned int max_aniso_ratio    : 3;    /*    [11:9] */
	unsigned int depth_cmp_func     : 3;    /*   [14:12] */
	unsigned int force_unnorm       : 1;    /*       15  */
	unsigned int aniso_thresh       : 3;    /*   [18:16] */
	unsigned int mc_coord_trunc     : 1;    /*       19  */
	unsigned int force_degamma      : 1;    /*       20  */
	unsigned int aniso_bias         : 6;    /*   [26:21] */
	unsigned int trunc_coord        : 1;    /*       27  */
	unsigned int disable_cube_wrap  : 1;    /*       28  */
	unsigned int filter_mode        : 2;    /*   [30:29] */ 
	unsigned int                    : 1;    /*       31  */ 
	unsigned int min_lod            : 12;   /*   [43:32] */ 
	unsigned int max_lod            : 12;   /*   [55:44] */ 
	unsigned int perf_mip           : 4;    /*   [59:56] */ 
	unsigned int perf_z             : 4;    /*   [63:60] */ 
	unsigned int lod_bias           : 14;   /*   [77:64] */ 
	unsigned int lod_bias_sec       : 6;    /*   [83:78] */ 
	unsigned int xy_mag_filter      : 2;    /*   [85:84] */ 
	unsigned int xy_min_filter      : 2;    /*   [87:86] */ 
	unsigned int z_filter           : 2;    /*   [89:88] */ 
	unsigned int mip_filter         : 2;    /*   [91:90] */ 
	unsigned int mip_point_preclamp : 1;    /*       92  */ 
	unsigned int disable_lsb_cell   : 1;    /*       93  */ 
	unsigned int                    : 2;    /*   [95:94] */ 
	unsigned int border_color_ptr   : 12;   /*  [107:96] */
	unsigned int                    : 18;   /* [125:108] */
	unsigned int border_color_type  : 2;    /* [127:126] */
}__attribute__((packed));

/* Pointers get stored in 2 consecutive 32-bit registers */
struct si_mem_ptr_t
{
	unsigned long long addr : 48;
	unsigned int unused     : 16;
}__attribute__((packed));

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

#endif

