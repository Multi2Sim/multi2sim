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

#ifndef X86_ARCH_TIMING_REG_FILE_H
#define X86_ARCH_TIMING_REG_FILE_H

#include <lib/util/class.h>

#include "uop.h"


/* Forward declarations */
struct config_t;
struct x86_uop_t;


/*
 * Public
 */

#define X86_REG_FILE_MIN_INT_SIZE  (x86_dep_int_count + X86_UINST_MAX_ODEPS)
#define X86_REG_FILE_MIN_FP_SIZE  (x86_dep_fp_count + X86_UINST_MAX_ODEPS)
#define X86_REG_FILE_MIN_XMM_SIZE  (x86_dep_xmm_count + X86_UINST_MAX_ODEPS)

extern char *x86_reg_file_kind_map[];
extern enum x86_reg_file_kind_t
{
	x86_reg_file_kind_shared = 0,
	x86_reg_file_kind_private
} x86_reg_file_kind;

extern int x86_reg_file_int_size;
extern int x86_reg_file_fp_size;
extern int x86_reg_file_xmm_size;

extern int x86_reg_file_int_local_size;
extern int x86_reg_file_fp_local_size;
extern int x86_reg_file_xmm_local_size;




/*
 * Public
 */

void X86ReadRegFileConfig(struct config_t *config);



/*
 * Class 'X86Core'
 */

void X86ThreadInitRegFile(X86Thread *self);
void X86ThreadFreeRegFile(X86Thread *self);
void X86ThreadDumpRegFile(X86Thread *self, FILE *f);

int X86ThreadRequestIntReg(X86Thread *self);
int X86ThreadRequestFPReg(X86Thread *self);
int X86ThreadRequestXMMReg(X86Thread *self);

int X86ThreadCanRenameUop(X86Thread *self, struct x86_uop_t *uop);
void X86ThreadRenameUop(X86Thread *self, struct x86_uop_t *uop);

int X86ThreadIsUopReady(X86Thread *self, struct x86_uop_t *uop);
void X86ThreadIsUopListReady(X86Thread *self, struct linked_list_t *uop_list);

void X86ThreadWriteUop(X86Thread *self, struct x86_uop_t *uop);
void X86ThreadUndoUop(X86Thread *self, struct x86_uop_t *uop);
void X86ThreadCommitUop(X86Thread *self, struct x86_uop_t *uop);

void X86ThreadCheckRegFile(X86Thread *self);




/*
 * Object 'x86_reg_file_t'
 */

struct x86_phreg_t
{
	int pending;  /* not completed (bit) */
	int busy;  /* number of mapped logical registers */
};

struct x86_reg_file_t
{
	/* Integer registers */
	int int_rat[x86_dep_int_count];
	struct x86_phreg_t *int_phreg;
	int int_phreg_count;
	int *int_free_phreg;
	int int_free_phreg_count;

	/* FP registers */
	int fp_top_of_stack;  /* Value between 0 and 7 */
	int fp_rat[x86_dep_fp_count];
	struct x86_phreg_t *fp_phreg;
	int fp_phreg_count;
	int *fp_free_phreg;
	int fp_free_phreg_count;

	/* XMM registers */
	int xmm_rat[x86_dep_xmm_count];
	struct x86_phreg_t *xmm_phreg;
	int xmm_phreg_count;
	int *xmm_free_phreg;
	int xmm_free_phreg_count;
};

struct x86_reg_file_t *x86_reg_file_create(int int_size, int fp_size, int xmm_size);
void x86_reg_file_free(struct x86_reg_file_t *reg_file);


#endif

