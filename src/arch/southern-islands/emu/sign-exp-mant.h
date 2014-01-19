#ifndef SIGN_EXP_MANT 
#define SIGN_EXP_MANT
/*
 * Author: Nick Materise
 * Date Created: Tue Dec 31 15:57:18 EST 2013
 * 
 */ 

#include <stdint.h>
#include <inttypes.h>

/* IEEE 754 Standards */

/* double precision bits */
#define DOUBLE_BITS 64

/* exponent macros*/
#define EXP_MASK 0x7FF0000000000000

/* number of mantissa bits */
#define MANT_WIDTH 52
#define MANT_MASK 0x000FFFFFFFFFFFFF


/* union containing hi,lo,double */
union double_si_uint32
{
	double as_double;
	unsigned int as_reg[2];
	unsigned long long as_uint;
};

/* Function to extract sign, exponent, mantissa*/
void extract_sign_exp_mant(
			double s0,
			double s1, 
			unsigned long long *sign, 
			unsigned long long *exp0, 
			unsigned long long *mant,
			union double_si_uint32 *s0_reg,
			union double_si_uint32 *s1_reg);

/* Function to concatenate two 32-bit registers as 64-bit register */
void concat_32_to_64_reg(
			unsigned int s0, 
			unsigned int s1, 
			unsigned long long *dst);

#endif
