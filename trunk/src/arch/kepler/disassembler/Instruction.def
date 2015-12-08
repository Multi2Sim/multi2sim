/*
 * DEFINST format
 *      name: Instruction name
 *      fmt_str: String representing the format for dissasembly
 *      fmt: Instruction format
 *      category: Instruction category
 *              FP: Floating point instructions
 *              INT: Integer instructions
 *              MOV: Movement instructions
 *              LDST: Compute load/store instructions
 *              CTRL: Control instructions
 *              MISC: Miscellaneous instructions
 *      opcode: Instruction opcode
 */

/* INT catogory instructions:
 * IMAD, IMUL, IADD, ISETP, ISCADD 
 */


DEFINST(IMUL_A,
		"%pred IMUL%U32%U32%hi%s %dst %cc_dst, %srcA, %srcB;",
		0x01,
		0x03,
		0x00,
		0x07)

DEFINST(IMUL_B,
                "%pred IMUL%U32%U32%hi%s %dst %cc_dst, %srcA, %srcB;",
                0x02,
                0x01,
                0x0c,
                0x07)

DEFINST(ISCADD_A,
		"%pred ISCADD%s %dst%cc, %srcA, %srcB, %const;",
		0x01,
		0x03,
		0x00,
		0x03)

DEFINST(ISCADD_B,
                "%pred ISCADD%s %dst%cc, %srcA, %srcB, %const;",
                0x02,
		0x01,
		0x0c,
		0x03)
                

DEFINST(ISAD_A,
		"ISAD_A",
		0x01,
		0x02,
		0x03,
		0x03,
		0x02,
		0x01)
		
DEFINST(ISAD_B,
		"ISAD_B",
		0x02,
		0x01,
		0x03,
		0x03,
		0x02,
		0x01)

DEFINST(BFI_A,
		"BFI_A",
		0x01,
		0x02,
		0x03,
		0x03,
		0x03,
		0x00)
		
DEFINST(BFI_B,
		"BFI_B",
		0x02,
		0x01,
		0x03,
		0x03,
		0x03,
		0x00)

DEFINST(BFE_A,
		"%pred0 BFE_A %dst, %srcA, %imm32;",
		0x01,
		0x03,
		0x00,
		0x00)
		
DEFINST(BFE_B,
		"BFE_B",
		0x02,
		0x01,
		0x0c,
		0x00)
								
DEFINST(IMAD,
		"%pred IMAD%us%hi%po%sat%x%s %dst %cc_dst, %srcA, %srcB, %srcC;",
		0x02,
		0x02,
		0x02,
		0x00)
		
DEFINST(IMADSP_A,
		"IMADSP_A",
		0x02,
		0x01,
		0x02,
		0x01)
	
DEFINST(IMADSP_B,
		"IMADSP_B",
		0x01,
		0x02,
		0x02,
		0x01)
		
DEFINST(IADD_A,
                "%pred IADD%sat%x%s %dst %cc, %srcA, %srcB;",
                0x01,
                0x03,
                0x00,
                0x02)	
                	
DEFINST(IADD_B,
		"%pred IADD%sat%x%s %dst%cc, %srcA, %srcB;",
		0x02,
		0x01,
		0x0c,
		0x02)

DEFINST(IADD32I,
		"IADD32I",
		0x01,
		0x01,
		0x00)
		
DEFINST(ISET_A,
		"ISET_A",
		0x01,
		0x02,
		0x03,
		0x01,
		0x01)
		
DEFINST(ISET_B,
		"ISET_B",
		0x02,
		0x01,
		0x0b,
		0x01,
		0x01)
		
DEFINST(ISETP_A,
		"%pred0 ISETP%f%U32%x%and%s %pred1, %pred2, %srcA, %srcB, %pred3;",
		0x02,
		0x01,    
		0x0b,
		0x01,
		0x02)

DEFINST(ISETP_B,
		"%pred0 ISETP%f%U32%x%and%s %pred1, %pred2, %srcA, %srcB, %pred3;",
		0x01,
		0x02,    
		0x03,
		0x01,
		0x02)

DEFINST(LOP_A,
		"LOP_A",
		0x01,
		0x03,
		0x00,
		0x08)
		
DEFINST(LOP_B,
		"LOP_B",
		0x02,
		0x01,
		0x0c,
		0x08)
				
DEFINST(LOP32I,
		"LOP32I",
		0x00,
		0x01)
DEFINST(ICMP_A,
		"ICMP_A",
		0x01,
		0x02,
		0x03,
		0x01,
		0x00)
		
DEFINST(ICMP_B,
		"ICMP_B",
		0x02,
		0x01,
		0x03,
		0x01,
		0x00)
				
		
/* Control catogory instructions:
 * EXIT, BRA
 */

DEFINST(EXIT,
		"%pred EXIT%keeprefcount%cc;",
		0x00,
		0x00,
		0x30)

DEFINST(BRA,
		"%pred BRA%u%LMT%cc %tgt;",
		0x00,
		0x00,
		0x24)


/* Movement catogory instructions:
 * MOV, MOV32I
 */

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>.FIX
DEFINST(MOV_A,
		"%pred MOV%s %dst, %src, %constant(sometimes)???;",
		0x02,
		0x03,
		0x0c,
		0x13)

DEFINST(MOV_B,
		"%pred MOV%s %dst, %srcB;",
		0x02,
		0x01,
		0x0c,
		0x13)

DEFINST(MOV32I,
		"%pred MOV32I%s %dst, %imm32;",
		0x02,
		0x01,
		0x0e,
		0x01,
		0x06,
		0x04,
		0x00)
		
DEFINST(SEL_A,
		"SEL_A",
		0x01,
		0x03,
		0x00,
		0x14)	
		
DEFINST(SEL_B,
		"SEL_B",
		0x02,
		0x01,
		0x0c,
		0x14)
		
DEFINST(I2F_A,
		"I2F_A",
		0x01,
		0x03,
		0x00,
		0x17)
			
DEFINST(I2F_B,
		"I2F_B",
		0x02,
		0x01,
		0x0c,
		0x17)
		
DEFINST(I2I_A,
		"I2I_A",
		0x01,
		0x03,
		0x00,
		0x18)
		
DEFINST(I2I_B,
		"I2I_B",
		0x02,
		0x01,
		0x0c,
		0x18)

DEFINST(F2I_A,
		"F2I_A",
		0x01,
		0x03,
		0x00,
		0x16)

DEFINST(F2I_B,
		"F2I_B",
		0x02,
		0x01,
		0x0c,
		0x16)
		
DEFINST(F2F_A,
		"F2F_A",
		0x01,
		0x03,
		0x00,
		0x15)

DEFINST(F2F_B,
		"F2F_B",
		0x02,
		0x01,
		0x0c,
		0x15)
				
/* Load/Store catogory instructions:
 * LD, ST
 */

DEFINST(LD,
		"%pred0 LD%e%cg%u8%s %dst, [%srcA]%offset;",
		0x00,
		0x06)

DEFINST(LDS,
		"%pred0 LDS %u8 %dst, [%srcA+0x%offset2];",
		0x02,
		0x01,
		0x0f,
		0x01,
		0x07,
		0x00,
		0x02,
		0x01)

DEFINST(LDC,  
        "%pred0 LDC %u8 %dst,[%srcA + 0x%offset2]; ",
		0x02,
		0x01,
		0x0f,
		0x01,
		0x07,
		0x01,
		0x02)
		
DEFINST(ST,
		"%pred0 ST%e%cg%u8%s [%srcA]%offset, %dst;",
		0x00,
		0x07)


DEFINST(STS,
		"%pred0 STS !!! ADD ARGS !!!;",
		0x02,
        0x01,
        0x0f,
        0x01,
        0x07,
        0x00,
		0x02,
		0x03)

/* Floating Point catogory instructions:
 * DADD
 */

DEFINST(DADD,
		"%pred0 DADD%rm%s %dst, %srcA, %srcB;",
		0x02,
		0x03,
		0x0c,
		0x0e)

DEFINST(FMUL,
		"%pred0 FMUL %dst, %srcA, %srcB;",
		0x02,
		0x03,
		0x0c,
		0x0d)

DEFINST(FADD_A,
		"FADD_A",
		0x01,
		0x03,
		0x00,
		0x0b)
			
DEFINST(FADD_B,
		"%pred0 FADD_B %dst, %srcA, %srcB;",
		0x02,
		0x03,
		0x0c,
		0x0b)
	
DEFINST(MUFU,
		"MUFU",
		0x02,
		0x01,
		0x00,
		0x02,
		0x00,
		0x01,
		0x00)
		
DEFINST(FFMA_A,
		"%pred0 FFMA_A %dst, %srcA, %srcB, %srcC;",
		0x01,
		0x02,
		0x01,
		0x01)
		
DEFINST(FFMA_B,
		"%pred0 FFMA_B %dst, %srcA, %srcB, %srcC;",
		0x02,
		0x01,
		0x01,
		0x01)

DEFINST(FSET_A,
		"FSET_A",
		0x01,
		0x02,
		0x00)
		
DEFINST(FSET_B,
		"FSET_B",
		0x02,
		0x01,
		0x08)		
				
/* Misc catogory instructions:
 * S2R, NOP
 */

DEFINST(NOP,
		"%pred NOP;",
		0x02,
		0x02,
		0x00,
		0x02,
		0x00,
		0x01,
		0x06)

DEFINST(S2R,
		"%pred0 S2R%s %dst, %src_spec;",
		0x02,
		0x02,
		0x00,
		0x02,
		0x00,
		0x01,
		0x09)
		
DEFINST(PSETP,
		"%pred0 PSETP%and%and %pred1, %pred2, %pred4, %pred5, %pred3;",
		0x02,
		0x02,
		0x00,
		0x02,
		0x00,
		0x01,
		0x02)		

DEFINST(SHF,
                "%pred0 SHF !!! Add arguments !!!;",
                0x01,
                0x02,
                0x03,
                0x03,
                0x03,
                0x01)


DEFINST(BAR,
                "%pred0 BAR !!! Add arguments !!!;",
                0x02,
                0x02,
                0x00,
                0x02,
                0x00,
                0x01,
		0x05)

/* Other (place later) instructions:
 * 
 */
 
 

DEFINST(BPT,
		"BPT",
		0x00,
		0x00,
		0x00)

DEFINST(JMX,
		"JMX",
		0x00,
		0x00,
		0x20)

DEFINST(JMP,
		"JMP",
		0x00,
		0x00,
		0x21)

DEFINST(JCAL,
		"JCAL",
		0x00,
		0x00,
		0x22)

DEFINST(BRX,
		"BRX",
		0x00,
		0x00,
		0x25)

DEFINST(CAL,
		"CAL",
		0x00,
		0x00,
		0x26)

DEFINST(PRET,
		"PRET",
		0x00,
		0x00,
		0x27)

DEFINST(PLONGJMP,
		"PLONGJMP",
		0x00,
		0x00,
		0x28)

DEFINST(SSY,
		"SSY",
		0x00,
		0x00,
		0x29)

DEFINST(PBK,
		"PBK",
		0x00,
		0x00,
		0x2a)

DEFINST(PCNT,
		"PCNT",
		0x00,
		0x00,
		0x2b)

DEFINST(GETCRSPTR,
		"GETCRSPTR",
		0x00,
		0x00,
		0x2c)

DEFINST(GETLMEMBASE,
		"GETLMEMBASE",
		0x00,
		0x00,
		0x2d)

DEFINST(SETCRSPTR,
		"SETCRSPTR",
		0x00,
		0x00,
		0x2e)

DEFINST(SETLMEMBASE,
		"SETLMEMBASE",
		0x00,
		0x00,
		0x2f)

DEFINST(LONGJMP,
		"LONGJMP",
		0x00,
		0x00,
		0x31)

DEFINST(RET,
		"RET",
		0x00,
		0x00,
		0x32)

DEFINST(KIL,
		"KIL",
		0x00,
		0x00,
		0x33)

DEFINST(BRK,
		"BRK",
		0x00,
		0x00,
		0x34)

DEFINST(CONT,
		"CONT",
		0x00,
		0x00,
		0x35)

DEFINST(RTT,
		"RTT",
		0x00,
		0x00,
		0x36)

DEFINST(SAM,
		"SAM",
		0x00,
		0x00,
		0x37)

DEFINST(RAM,
		"RAM",
		0x00,
		0x00,
		0x38)

DEFINST(IDE,
		"IDE",
		0x00,
		0x00,
		0x39)

DEFINST(FADD32I,
		"FADD32I",
		0x00,
		0x02)

DEFINST(FFMA32I,
		"FFMA32I",
		0x00,
		0x03)

DEFINST(IMAD32I,
		"IMAD32I",
		0x00,
		0x04)

DEFINST(ISCADD32I,
		"IASCADD32I",
		0x00,
		0x05)
		
DEFINST(SHL_A,
		"SHL_A",
		0x01,
		0x03,
		0x00,
		0x09)

DEFINST(SHL_B,
		"SHL_B",
		0x02,
		0x01,
		0x0c,
		0x09)		
		
DEFINST(SHR_A,
		"SHR_A",
		0x01,
		0x03,
		0x00,
		0x05)
		
DEFINST(SHR_B,
		"SHR_B",
		0x02,
		0x01,
		0x0c,
		0x05)