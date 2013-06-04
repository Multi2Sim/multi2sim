/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

/* C++ includes */
#include <iostream>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Function.h>
#include <llvm/LLVMContext.h>
#include <llvm/Pass.h>
#include <llvm/PassManager.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/system_error.h>


/* C includes */
extern "C"
{

#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <clcc/llvm2si/basic-block.h>
#include <clcc/llvm2si/function.h>
#include <clcc/si2bin/arg.h>
#include <clcc/si2bin/inst.h>

#include "translate.h"
}


using namespace llvm;
using namespace std;

/* Private variables */
static FILE *llvm2si_outf;


/*
 * Private Functions
 */

static int llvm2si_translate_sreg_uav = 0;  /* s[0:1] */
static int llvm2si_translate_sreg_cb0 = 2;  /* s[2:5] */
static int llvm2si_translate_sreg_cb1 = 6;  /* s[6:9] */
static int llvm2si_translate_sreg_wgid = 10;  /* s[10:12] */
static int llvm2si_translate_sreg_lsize = 13;  /* s[13:15] */
static int llvm2si_translate_sreg_offs = 16;  /* s[16:18] */
static int llvm2si_translate_sreg_uav10 = 19;  /* s[19:22] */
static int llvm2si_translate_sreg_uav11 = 23;  /* s[23:26] */

static int llvm2si_translate_vreg_lid = 0;  /* v[0:2] */
static int llvm2si_translate_vreg_gid = 3;  /* v[3:5] */


/* Emit initialization Southern Islands assembly. This code is present in every
 * Southern Islands binary, regardless of its content.
 */
static void llvm2si_translate_emit_header(struct llvm2si_basic_block_t *basic_block)
{
	struct si2bin_inst_t *inst;
	struct list_t *arg_list;
	int index;

	/* Obtain local size in s[lsize:lsize+2].
	 *
	 * s_buffer_load_dword s[lsize], s[cb0:cb0+3], 0x04
	 * s_buffer_load_dword s[lsize+1], s[cb0:cb0+3], 0x05
	 * s_buffer_load_dword s[lsize+2], s[cb0:cb0+3], 0x06
	 * */
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				llvm2si_translate_sreg_lsize + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				llvm2si_translate_sreg_cb0, llvm2si_translate_sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(4 + index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		llvm2si_basic_block_add(basic_block, inst);
	}

	/* Obtain global offset in s[offs:offs+2].
	 *
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x18
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x19
	 * s_buffer_load_dword s[offs], s[cb0:cb0+3], 0x1a
	 */
	for (index = 0; index < 3; index++)
	{
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_scalar_register(
				llvm2si_translate_sreg_offs + index));
		list_add(arg_list, si2bin_arg_create_scalar_register_series(
				llvm2si_translate_sreg_cb0, llvm2si_translate_sreg_cb0 + 3));
		list_add(arg_list, si2bin_arg_create_literal(0x18 + index));
		inst = si2bin_inst_create(SI_INST_S_BUFFER_LOAD_DWORD, arg_list);
		llvm2si_basic_block_add(basic_block, inst);
	}

	/* Calculate global ID in dimensions [0:2] and store it in v[3:5].
	 *
	 * v_mov_b32 v[gid+dim], s[lsize+dim]
	 * v_mul_i32_i24 v[gid+dim], s[wgid+dim], v[gid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], v[lid+dim]
	 * v_add_i32 v[gid+dim], vcc, v[gid+dim], s[offs+dim]
	 */
	for (index = 0; index < 3; index++)
	{
		/* v_mov_b32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				llvm2si_translate_sreg_lsize + index));
		inst = si2bin_inst_create(SI_INST_V_MOV_B32, arg_list);
		llvm2si_basic_block_add(basic_block, inst);

		/* v_mul_i32_i24 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				llvm2si_translate_sreg_wgid + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_gid + index));
		inst = si2bin_inst_create(SI_INST_V_MUL_I32_I24, arg_list);
		llvm2si_basic_block_add(basic_block, inst);

		/* v_add_i32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_lid + index));
		inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
		llvm2si_basic_block_add(basic_block, inst);

		/* v_add_i32 */
		arg_list = list_create();
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_gid + index));
		list_add(arg_list, si2bin_arg_create_special_register(si_inst_special_reg_vcc));
		list_add(arg_list, si2bin_arg_create_scalar_register(
				llvm2si_translate_sreg_offs + index));
		list_add(arg_list, si2bin_arg_create_vector_register(
				llvm2si_translate_vreg_gid + index));
		inst = si2bin_inst_create(SI_INST_V_ADD_I32, arg_list);
		llvm2si_basic_block_add(basic_block, inst);
	}

	/* Load UAVs. UAV10 is used for private memory an stored in s[uav10:uav10+3].
	 * UAV11 is used for global memory and stored in s[uav11:uav11+3].
	 *
	 * s_load_dwordx4 s[uav10:uav10+3], s[uav:uav+1], 0x50
	 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			llvm2si_translate_sreg_uav10, llvm2si_translate_sreg_uav10 + 3));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			llvm2si_translate_sreg_uav, llvm2si_translate_sreg_uav + 1));
	list_add(arg_list, si2bin_arg_create_literal(0x50));
	inst = si2bin_inst_create(SI_INST_S_LOAD_DWORDX4, arg_list);
	llvm2si_basic_block_add(basic_block, inst);

	/* s_load_dwordx4 s[uav11:uav11+3], s[uav:uav+1], 0x58 */
	arg_list = list_create();
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			llvm2si_translate_sreg_uav11, llvm2si_translate_sreg_uav11 + 3));
	list_add(arg_list, si2bin_arg_create_scalar_register_series(
			llvm2si_translate_sreg_uav, llvm2si_translate_sreg_uav + 1));
	list_add(arg_list, si2bin_arg_create_literal(0x58));
	inst = si2bin_inst_create(SI_INST_S_LOAD_DWORDX4, arg_list);
	llvm2si_basic_block_add(basic_block, inst);
}



/*
 * Class for LLVM Pass
 */

class llvm2si_translate_pass_t : public FunctionPass
{
public:
	char ID;
	llvm2si_translate_pass_t();
	virtual bool runOnFunction(Function &f);
};


llvm2si_translate_pass_t::llvm2si_translate_pass_t() : FunctionPass(ID)
{
}


bool llvm2si_translate_pass_t::runOnFunction(Function &f)
{
	struct llvm2si_function_t *function;
	struct llvm2si_basic_block_t *basic_block;
	const char *name;

	/* Skip special functions */
	name = f.getName().data();
	if (!strcmp(name, "get_work_dim")
			|| !strcmp(name, "get_global_size")
			|| !strcmp(name, "get_global_id")
			|| !strcmp(name, "get_local_size")
			|| !strcmp(name, "get_local_id")
			|| !strcmp(name, "get_num_groups")
			|| !strcmp(name, "get_group_id")
			|| !strcmp(name, "get_global_offset"))
		return false;

	/* Create function and entry basic block */
	function = llvm2si_function_create(f.getName().data());
	basic_block = llvm2si_basic_block_create("entry");
	llvm2si_function_add(function, basic_block);

	/* Emit function header in entry basic block */
	llvm2si_translate_emit_header(basic_block);

	/* Basic blocks in function */
	for (Function::iterator bb = f.begin(), bb_end = f.end();
			bb != bb_end; bb++)
	{
		outs() << "\tBasic block '" << bb->getName() << "'\n";
		for (BasicBlock::iterator inst = bb->begin(), inst_end = bb->end();
				inst != inst_end; inst++)
		{
			outs() << "\t\t" << *inst << "\n";
		}
	}

	/* Free function. This takes care of freeing all basic blocks and
	 * instructions added to the function. */
	llvm2si_function_dump(function, llvm2si_outf);
	llvm2si_function_free(function);

	/* Module not modified, return false */
	return false;
}



/*
 * Public Functions
 */

extern "C" void llvm2si_translate(char *source_file, char *output_file)
{
	OwningPtr<MemoryBuffer> buffer;
	LLVMContext context;
	Module *module;
	string err_msg;
	
	/* Read bitcode in 'source_file' */
	if (MemoryBuffer::getFile(source_file, buffer))
	{
		cerr << "fatal: " << source_file << ": error loading file\n";
		exit(1);
	}

	/* Load module */
	module = ParseBitcodeFile(buffer.get(), context, &err_msg);
	if (!module)
	{
		cerr << "fatal: " << source_file << ": " << err_msg << "\n";
		exit(1);
	}
	
	/* Open output file */
	llvm2si_outf = fopen(output_file, "w");
	if (!llvm2si_outf)
		fatal("%s: cannot open output file", output_file);

	/* Apply pass */
	PassManager pm;
	pm.add(new llvm2si_translate_pass_t());
	pm.run(*module);

	/* Close output file */
	fclose(llvm2si_outf);
}
