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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <src/driver/opengl/si-shader.h>

#include "fetch-shader.h"
#include "Wrapper.h"


struct si_fs_inst_t
{
	unsigned int size;
	void *data;
};

/*
 * Private Functions
 */

static struct si_fs_inst_t *si_fs_inst_create(SIInstFormat inst_fmt)
{
	struct si_fs_inst_t *inst;

	/* Allocate */
	inst = xcalloc(1, sizeof(struct si_fs_inst_t));

	/* Return */
	return inst;
}

static void si_fs_inst_free(struct si_fs_inst_t *inst)
{
	free(inst->data);
	free(inst);
}

static void si_fs_inst_as_smrd(struct si_fs_inst_t *inst)
{
	SIInstBytesSMRD *smrd;

	/* Allocate */
	smrd = xcalloc(1, sizeof(SIInstBytesSMRD));

	/* Set instruction */
	inst->data = smrd;
	inst->size = 4;
}

static void si_fs_inst_set_smrd(SIInstBytesSMRD *smrd,
	unsigned int op, unsigned int sdst, unsigned int sbase, unsigned int imm, unsigned int offset)
{
	smrd->enc = 0x18; /* 0b11000 */
	smrd->op = op;
	smrd->sdst = sdst;
	smrd->sbase = sbase >> 1; /* The LSB is ignored */
	smrd->imm = imm;
	smrd->offset = offset;
}

static void si_fs_inst_as_vop1(struct si_fs_inst_t *inst)
{
	SIInstBytesVOP1 *vop1;

	/* Allocate */
	vop1 = xcalloc(1, sizeof(SIInstBytesVOP1));

	/* Set instruction */
	inst->data = vop1;
	inst->size = 4;
}

static void si_fs_inst_set_vop1(SIInstBytesVOP1 *vop1,
	unsigned int vdst, unsigned int op, unsigned int src0)
{
	vop1->enc = 0x3F;
	vop1->vdst = vdst;
	vop1->op = op;
	vop1->src0 = src0;
}

static void si_fs_inst_as_vop2(struct si_fs_inst_t *inst)
{
	SIInstBytesVOP2 *vop2;

	/* Allocate */
	vop2 = xcalloc(1, sizeof(SIInstBytesVOP2));

	/* Set instruction */
	inst->data = vop2;
	inst->size = 4;
}

static void si_fs_inst_set_vop2(SIInstBytesVOP2 *vop2,
	unsigned int op, unsigned int vdst, unsigned int vsrc1, unsigned int ssrc0)
{
	vop2->enc = 0;
	vop2->op = op;
	vop2->vdst = vdst;
	vop2->vsrc1 = vsrc1;
	vop2->src0 = ssrc0;
}

static void si_fs_inst_as_sopp(struct si_fs_inst_t *inst)
{
	SIInstBytesSOPP *sopp;

	/* Allocate */
	sopp = xcalloc(1, sizeof(SIInstBytesSOPP));

	/* Set instruction */
	inst->data = sopp;
	inst->size = 4;
}

static void si_fs_inst_set_sopp(SIInstBytesSOPP *sopp,
	unsigned int op, unsigned int simm)
{
	sopp->enc = 0x17F; 
	sopp->op = op;
	sopp->simm16 = simm;
}

static void si_fs_inst_as_mtbuf(struct si_fs_inst_t *inst)
{
	SIInstBytesMTBUF *mtbuf;

	/* Allocate */
	mtbuf = xcalloc(1, sizeof(SIInstBytesMTBUF));

	/* Set instruction */
	inst->data = mtbuf;
	inst->size = 8;
}

static void si_fs_inst_set_mtbuf(SIInstBytesMTBUF *mtbuf,
	unsigned int soffset, unsigned int tfe, unsigned int slc, 
	unsigned int srsrc, unsigned int vdata, unsigned int vaddr,
	unsigned int nfmt, unsigned int dfmt, unsigned int op, 
	unsigned int addr64, unsigned int glc, unsigned int idxen, 
	unsigned int offen, unsigned int offset)
{
	mtbuf->soffset = soffset;
	mtbuf->tfe = tfe;
	mtbuf->slc = slc;
	mtbuf->srsrc = srsrc >> 2;
	mtbuf->vdata = vdata;
	mtbuf->vaddr = vaddr;
	mtbuf->enc = 0x3A;
	mtbuf->nfmt = nfmt;
	mtbuf->dfmt = dfmt;
	mtbuf->op = op;
	mtbuf->addr64 = addr64;
	mtbuf->glc = glc;
	mtbuf->idxen = idxen;
	mtbuf->offen = offen;
	mtbuf->offset = offset;
}

static void si_fs_inst_as_sop1(struct si_fs_inst_t *inst)
{
	SIInstBytesSOP1 *sop1;

	/* Allocate */
	sop1 = xcalloc(1, sizeof(SIInstBytesSOP1));

	/* Set instruction */
	inst->data = sop1;
	inst->size = 4;
}

static void si_fs_inst_set_sop1(SIInstBytesSOP1 *sop1,
	unsigned int sdst, unsigned int op, unsigned int ssrc0)
{
	sop1->enc = 0x17D;
	sop1->sdst = sdst;
	sop1->op = op;
	sop1->ssrc0 = ssrc0;
}

/*
 * Public Functions
 */

struct si_fetch_shader_t *si_fetch_shader_create(struct opengl_si_shader_t *shdr)
{
	struct si_fetch_shader_t *fs;
	struct si_fs_inst_t *fs_inst;
	int i;

	unsigned int isa_buf_size = 0;
	void *isa_data_ptr;

	struct opengl_si_enc_dict_vertex_shader_t *enc_dict;
	enc_dict = (struct opengl_si_enc_dict_vertex_shader_t *)shdr->bin->enc_dict;
	unsigned int vgpr_used = enc_dict->meta->u32NumVgprs;
	unsigned int input_count = enc_dict->meta->numVsInSemantics;
	unsigned int input_vs_semantic_vgpr_start;
	unsigned int input_format;
	unsigned int usage_index;

	/*  Instruction TBUFF_LOAD_XXX needs to right shirf it >> 2 and it's always start > 16 */
	unsigned int sgpr_buf_desc_start;
	sgpr_buf_desc_start = 16;

	/* Allocate */
	fs = xcalloc(1, sizeof(struct si_fetch_shader_t));
	fs->isa_list = list_create();

	/* FIXME: temporary solution */
	for (i = 0; i < input_count; ++i)
	{
		/* s_load_dword4 s[sgpr_buf_desc_start ~ +3], s[userElement[VertexTableStartSGPR] ~+1], offset = 32 >> 2 * usage_index */
		usage_index = enc_dict->meta->vsInSemantics[i].usageIdx;
		fs_inst = si_fs_inst_create(SIInstFormatSMRD);
		si_fs_inst_as_smrd(fs_inst);
		si_fs_inst_set_smrd(fs_inst->data, 0x2, sgpr_buf_desc_start + 4 * i, 0x4, 0x1, 0x8 * usage_index);
		list_add(fs->isa_list, fs_inst);	
	}

	/* 
	 * Use next available VGPR, this VGPR is later used to initialize 4 
	 * VGPRs by TBUFFER_LOAD_FORMAT_XXX instruction 
	 * v0 stores the workitem id, which == vertex id for vertex shader 
	 * s0 content is unknown, FIXME
	 */
	/* v_add_i32 vgpr_used, vcc, s0, v0 */
	fs_inst = si_fs_inst_create(SIInstFormatVOP2);
	si_fs_inst_as_vop2(fs_inst);
	si_fs_inst_set_vop2(fs_inst->data, 0x25, vgpr_used, 0x0, 0x0);
	list_add(fs->isa_list, fs_inst);

	/* Create s_waitcnt instruction and add to instruction list */
	fs_inst = si_fs_inst_create(SIInstFormatSOPP);
	si_fs_inst_as_sopp(fs_inst);
	si_fs_inst_set_sopp(fs_inst->data, 0xC, 0x7F);
	list_add(fs->isa_list, fs_inst);

	/* Create t_buffer_load instruction and add to instruction list */
	for (i = 0; i < input_count; ++i)
	{
		input_vs_semantic_vgpr_start = enc_dict->meta->vsInSemantics[i].dataVgpr;
		input_format = 0xe; /* FIXME: should depends on the input, hardcoded to 32_32_32_32 for now */
		fs_inst = si_fs_inst_create(SIInstFormatMTBUF);
		si_fs_inst_as_mtbuf(fs_inst);
		si_fs_inst_set_mtbuf(fs_inst->data, 
			0x0, 0x0, 0x0, 
			sgpr_buf_desc_start + 4 * i, input_vs_semantic_vgpr_start, vgpr_used, 
			0x7, input_format, 0x3, 
			0x0, 0x0, 0x1, 
			0x0, 0x0);
		list_add(fs->isa_list, fs_inst);		
	}

	/* Create s_waitcnt instruction and add to instruction list */
	fs_inst = si_fs_inst_create(SIInstFormatSOPP);
	si_fs_inst_as_sopp(fs_inst);
	si_fs_inst_set_sopp(fs_inst->data, 0xC, 0x7F);
	list_add(fs->isa_list, fs_inst);

	/* FIXME: what's the purpose of this instruction? Keep it for now */
	/* Load v0 with s0 + v[vgpr_used] */
	fs_inst = si_fs_inst_create(SIInstFormatVOP1);
	si_fs_inst_as_vop1(fs_inst);
	si_fs_inst_set_vop1(fs_inst->data, 0x0, 0x1, vgpr_used);
	list_add(fs->isa_list, fs_inst);

	/* Jump back to Vertex Shader */
	fs_inst = si_fs_inst_create(SIInstFormatSOP1);
	si_fs_inst_as_sop1(fs_inst);
	si_fs_inst_set_sop1(fs_inst->data, 0x2, 0x21, 0x2);
	list_add(fs->isa_list, fs_inst);

	/* Calculate ISA buffer size and allocate */
	LIST_FOR_EACH(fs->isa_list, i)
	{
		fs_inst = list_get(fs->isa_list, i);
		if (!fs_inst || !fs_inst->size)
			fatal("Instruction size = 0!");
		else
			isa_buf_size += fs_inst->size;
	}

	fs->isa = xcalloc(1, isa_buf_size);
	fs->size = isa_buf_size;

	/* Copy instructions to buffer */
	isa_data_ptr = fs->isa;
	LIST_FOR_EACH(fs->isa_list, i)
	{
		fs_inst = list_get(fs->isa_list, i);
		if (!fs_inst->size)
			fatal("Empty Instruction!");
		memcpy(isa_data_ptr, fs_inst->data, fs_inst->size);
		isa_data_ptr += fs_inst->size;
	}

	/* Return */
	return fs;
}

void si_fetch_shader_free(struct si_fetch_shader_t *fs)
{	
	int i;

	/* Free */
	LIST_FOR_EACH(fs->isa_list, i)
		si_fs_inst_free(list_get(fs->isa_list, i));
	list_free(fs->isa_list);
	free(fs->isa);
	free(fs);
}

