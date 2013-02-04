#include <stdio.h>

//Contains enumeration for supported
//op-codes of S.I. architecture.
//0 is deafaulted to invalid code.

enum si_inst_opcode_t
{
	si_inst_opcode_invalid = 0,
	si_inst_opcode_s_mov_b32,
	si_inst_opcode_s_waitcnt, 
	si_inst_opcode_v_cvt_f32_u32,
	si_inst_opcode_v_rcp_f32,
	si_inst_opcode_v_mul_f32,
	si_inst_opcode_s_buffer_load_dwordx2,
	si_inst_opcode_v_cvt_u32_f32,
	si_inst_opcode_v_mul_lo_u32,
	si_inst_opcode_v_mul_hi_u32,
	si_inst_opcode_s_buffer_load_dword,
	si_inst_opcode_v_sub_i32,
	si_inst_opcode_v_cmp_ne_i32,
	si_inst_opcode_v_cndmask_b32,
	si_inst_opcode_s_min_u32,
	si_inst_opcode_v_add_i32,
	si_inst_opcode_v_mov_b32,
	si_inst_opcode_v_mul_i32_i24,
	si_inst_opcode_s_load_dwordx4,
	si_inst_opcode_v_mul_lo_i32,
	si_inst_opcode_v_cmp_ge_u32,
	si_inst_opcode_s_and_b64,
	si_inst_opcode_v_lshlrev_b32,
	si_inst_opcode_v_addc_u32,
	si_inst_opcode_tbuffer_store_format_x,
	si_inst_opcode_s_cbranch_vccz,
	si_inst_opcode_s_mul_i32,
	si_inst_opcode_s_lshl_b32,
	si_inst_opcode_v_readfirstlane_b32,
	si_inst_opcode_tbuffer_load_format_x,
	si_inst_opcode_ds_write_b32,
	si_inst_opcode_s_barrier,
	si_inst_opcode_s_cmp_eq_i32,
	si_inst_opcode_s_cbranch_scc1,
	si_inst_opcode_ds_read_b32,
	si_inst_opcode_s_add_i32,
	si_inst_opcode_v_mac_f32,
	si_inst_opcode_s_branch,
	si_inst_opcode_s_endpgm
};

struct si_inst_t
{
	enum si_inst_opcode_t opcode;
	struct list_t *arg_list;
};

/* Returns a newly created si_inst_t object
 * with the op-code corresponding to the
 * 'inst_str' instruction.                 */
struct si_inst_t *si_inst_create(char *inst_str);

void si_inst_free(struct si_inst_t *inst);
void si_inst_dump(struct si_inst_t *inst, FILE *f);

/* This function generates the assembly code for instruction 'inst' into
 * buffer 'inst_bytes. The returned value is the number of
 *  bytes of the generated instruction, or 0 if the inst
 * could not be decoded */
int si_inst_code_gen(struct si_inst_t *inst, unsigned long long *inst_bytes);
