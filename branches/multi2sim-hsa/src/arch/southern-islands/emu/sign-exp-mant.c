/*
 * Mantissa, exponent, and sign 
 * extraction for double precision 
 * floating point numbers
 *
 * Author: Nick Materise
 * Date Created: Mon Dec 30 21:08:40 EST 2013
 * 
 */

/* See header for macros definitions */
#include "sign-exp-mant.h"

/* Function to extract sign, exponent, mantissa*/
void extract_sign_exp_mant(
			double s0,
			double s1, 
			unsigned long long *sign, 
			unsigned long long *exp0, 
			unsigned long long *mant,
			union double_si_uint32 *s0_reg,
			union double_si_uint32 *s1_reg)
{
	/* pointer to input_val */
	unsigned long long *input_ptr_s0 = (unsigned long long*)(&s0);
	unsigned long long *input_ptr_s1 = (unsigned long long*)(&s1);	

	/* split into two 32-bit integers */
	unsigned int input_s0_lo = (unsigned int)(*input_ptr_s0 >> (unsigned long long)32);
	unsigned int input_s0_hi = (unsigned int)(*input_ptr_s0 << (unsigned long long)32);
	
	unsigned int input_s1_lo = (unsigned int)(*input_ptr_s1 >> (unsigned long long)32);
	unsigned int input_s1_hi = (unsigned int)(*input_ptr_s1 << (unsigned long long)32);
	
	/* union double to two unsigned int */
	s0_reg->as_double = s0;
	s0_reg->as_reg[0] = input_s0_lo;
	s0_reg->as_reg[1] = input_s0_hi;
	
	s1_reg->as_double = s1;
	s1_reg->as_reg[0] = input_s1_lo;
	s1_reg->as_reg[1] = input_s1_hi;
	
	
	/* extract sign */
	*sign = (unsigned long long)(*input_ptr_s1 >> (unsigned long long)(DOUBLE_BITS - 1));
	
	/* grab exponent */
	*exp0 = (unsigned long long)(*input_ptr_s1 & (unsigned long long)EXP_MASK);
	*exp0 >>= (unsigned long long)(MANT_WIDTH - 1);

	/* select mantissa */
	*mant = (unsigned long long)(*input_ptr_s1 & (unsigned long long)(MANT_MASK));
}

/* function to concatenate two 32-bit registers as 64-bit */
void concat_32_to_64_reg(
			unsigned int s0, 
			unsigned int s1, 
			unsigned long long *dst)
{
	/* left shift s0 and OR with s1 */
	*dst = (unsigned long long)(((unsigned long long)s0 << (unsigned long long)32) 
						| (unsigned long long)s1);
}

