/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2017 Hayashi Naoyuki
 * Copyright 2022 Michael van der Westhuizen
 */

#ifndef	_ASM_CONTROLREGS_H
#define	_ASM_CONTROLREGS_H

#include <sys/types.h>

#ifdef	__cplusplus
extern "C" {
#endif

static inline uint64_t
read_actlr(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, actlr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_actlr(uint64_t reg)
{
	__asm__ __volatile__("msr actlr_el1, %0"::"r"(reg):"memory");
}

static inline void
write_mair(uint64_t reg)
{
	__asm__ __volatile__("msr mair_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_mair(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, mair_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_tcr(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, tcr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_tcr(uint64_t reg)
{
	__asm__ __volatile__("msr tcr_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_ttbr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, ttbr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_ttbr0(uint64_t reg)
{
	__asm__ __volatile__("msr ttbr0_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_ttbr1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, ttbr1_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_ttbr1(uint64_t reg)
{
	__asm__ __volatile__("msr ttbr1_el1, %0"::"r"(reg):"memory");
}

static inline void
write_sctlr(uint64_t reg)
{
	__asm__ __volatile__("msr sctlr_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_sctlr(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, sctlr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
tlbi_allis(void)
{
	__asm__ __volatile__("tlbi vmalle1is":::"memory");
}

static inline void
tlbi_mva(uint64_t addr)
{
	/*
	 * TLB Invalidate by VA, All ASID, EL1, Inner Shareable
	 *	VAA (global/non-global, any level)
	 */
	__asm__ __volatile__("tlbi vaae1is, %0"
	    ::"r"((addr >> 12) & ((1ul << 44) - 1)):"memory");
}

static inline uint64_t
read_revidr(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, revidr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_cntkctl(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, cntkctl_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_cntkctl(uint64_t reg)
{
	__asm__ __volatile__("msr cntkctl_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_cntpct(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, cntpct_el0":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_cntfrq(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, cntfrq_el0":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_cntp_ctl(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, cntp_ctl_el0":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_cntp_ctl(uint64_t reg)
{
	__asm__ __volatile__("msr cntp_ctl_el0, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_cntp_cval(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, cntp_cval_el0":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_cntp_cval(uint64_t reg)
{
	__asm__ __volatile__("msr cntp_cval_el0, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_cntp_tval(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, cntp_tval_el0":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_cntp_tval(uint64_t reg)
{
	__asm__ __volatile__("msr cntp_tval_el0, %0"::"r"(reg):"memory");
}

static inline void
write_tpidr_el1(uint64_t reg)
{
	__asm__ __volatile__("msr tpidr_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_tpidr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, tpidr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_tpidr_el0(uint64_t reg)
{
	__asm__ __volatile__("msr tpidr_el0, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_tpidr_el0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, tpidr_el0":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_s1e1r(uint64_t reg)
{
	__asm__ __volatile__("at s1e1r, %0"::"r"(reg):"memory");
}

static inline void
write_s1e1w(uint64_t reg)
{
	__asm__ __volatile__("at s1e1w, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_par_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, par_el1":"=r"(reg)::"memory");
	return (reg);
}

#define	dmb(_option_) do {				\
	__asm__ __volatile__("dmb " #_option_ :::"memory");	\
} while (0)

#define	dsb(_option_) do {				\
	__asm__ __volatile__("dsb " #_option_ :::"memory");	\
} while (0)

static inline void
isb(void)
{
	__asm__ __volatile__("isb":::"memory");
}

static inline void
flush_data_cache_by_sw(uint64_t reg)
{
	__asm__ __volatile__("dc cisw, %0"::"r"(reg):"memory");
}

static inline void
clean_data_cache_by_sw(uint64_t reg)
{
	__asm__ __volatile__("dc csw, %0"::"r"(reg):"memory");
}

static inline void
invalidate_data_cache_by_sw(uint64_t reg)
{
	__asm__ __volatile__("dc isw, %0"::"r"(reg):"memory");
}

static inline void
flush_data_cache(uint64_t addr)
{
	__asm__ __volatile__("dc civac, %0"::"r"(addr):"memory");
}

static inline void
clean_data_cache_poc(uint64_t addr)
{
	__asm__ __volatile__("dc cvac, %0"::"r"(addr):"memory");
}

static inline void
clean_data_cache_pou(uint64_t addr)
{
	__asm__ __volatile__("dc cvau, %0"::"r"(addr):"memory");
}

static inline void
invalidate_data_cache(uint64_t addr)
{
	__asm__ __volatile__("dc ivac, %0"::"r"(addr):"memory");
}

static inline void
data_cache_zero(uint64_t addr)
{
	__asm__ __volatile__("dc zva, %0"::"r"(addr):"memory");
}

static inline void
invalidate_instruction_cache(uint64_t addr)
{
	__asm__ __volatile__("ic ivau, %0"::"r"(addr):"memory");
}

static inline void
invalidate_instruction_cache_all(void)
{
	__asm__ __volatile__("ic iallu":::"memory");
}

static inline void
invalidate_instruction_cache_allis(void)
{
	__asm__ __volatile__("ic ialluis":::"memory");
}

static inline uint64_t
read_ccsidr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, ccsidr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_clidr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, clidr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_csselr_el1(uint64_t reg)
{
	__asm__ __volatile__("msr csselr_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_csselr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, csselr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_ctr_el0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, ctr_el0":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_mpidr(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, mpidr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_midr(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, midr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_cpacr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, cpacr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_cpacr_el1(uint64_t reg)
{
	__asm__ __volatile__("msr cpacr_el1, %0"::"r"(reg):"memory");
}

/*
 * DAIF bits.
 *
 * Arm® Architecture Registers for A-profile architecture pp. 410
 *  DAIF, Interrupt Mask Bits
 *
 * These are mnemonic, but daifset/daifclear "registers" do the shifting for
 * you. daif does not.
 */

/* Bits shifted, as for DAIFset and DAIFclear */
#define	DAIF_SETCLEAR_FIQ	(1 << 0) /* Mask FIQs */
#define	DAIF_SETCLEAR_IRQ	(1 << 1) /* Mask IRQs */
#define	DAIF_SETCLEAR_SERROR	(1 << 2) /* Mask SErrors */
#define	DAIF_SETCLEAR_DEBUG	(1 << 3) /* Mask Debug exceptions */
#define	DAIF_SETCLEAR_ALL	0xf

#define	DAIF_SHIFT	 6

/* Unshifted, their positions in the true DAIF  */
#define	DAIF_FIQ	(1 << 6)
#define	DAIF_IRQ	(1 << 7)
#define	DAIF_SERROR	(1 << 8)
#define	DAIF_DEBUG	(1 << 9)


static inline uint64_t
read_daif(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, daif":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_daif(uint64_t reg)
{
	__asm__ __volatile__("msr daif, %0"::"r"(reg):"memory");
}

static inline void
set_daif(uint64_t val)
{
	__asm__ __volatile__("msr DAIFSet, %0"::"I"(val):"memory");
}

static inline void
clear_daif(uint64_t val)
{
	__asm__ __volatile__("msr DAIFClr, %0"::"I"(val):"memory");
}

static inline void
write_vbar(uint64_t reg)
{
	__asm__ __volatile__("msr vbar_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_CurrentEL(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, CurrentEL":"=r"(reg)::"memory");
	return (reg);
}

static inline uint32_t
read_fpsr(void)
{
	uint32_t reg;
	__asm__ __volatile__("mrs %0, fpsr":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_fpsr(uint32_t reg)
{
	__asm__ __volatile__("msr fpsr, %0":"=r"(reg)::"memory");
}

static inline uint32_t
read_fpcr(void)
{
	uint32_t reg;
	__asm__ __volatile__("mrs %0, fpcr":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_fpcr(uint32_t reg)
{
	__asm__ __volatile__("msr fpcr, %0":"=r"(reg)::"memory");
}

static inline uint64_t
read_cpuactlr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, s3_1_c15_c2_0":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_cpuactlr_el1(uint64_t reg)
{
	__asm__ __volatile__("msr s3_1_c15_c2_0, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_cpuectlr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, s3_1_c15_c2_1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_cpuectlr_el1(uint64_t reg)
{
	__asm__ __volatile__("msr s3_1_c15_c2_1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_l2actlr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, s3_1_c15_c0_0":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_l2actlr_el1(uint64_t reg)
{
	__asm__ __volatile__("msr s3_1_c15_c0_0, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_id_aa64pfr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64pfr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64pfr1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64pfr1_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64afr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64afr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64afr1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64afr1_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64dfr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64dfr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64dfr1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64dfr1_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64isar0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64isar0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64isar1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64isar1_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64isar2(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64isar2_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64mmfr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64mmfr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64mmfr1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64mmfr1_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64mmfr2(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64mmfr2_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64mmfr3(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64mmfr3_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64zfr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64zfr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_id_aa64smfr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, id_aa64smfr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_pmmir(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, pmmir_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_mvfr0(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, mvfr0_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_mvfr1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, mvfr1_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_mvfr2(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, mvfr2_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_mdscr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, mdscr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline void
write_mdscr_el1(uint64_t reg)
{
	__asm__ __volatile__("msr mdscr_el1, %0"::"r"(reg):"memory");
}

static inline void
write_oslar_el1(uint64_t reg)
{
	__asm__ __volatile__("msr oslar_el1, %0"::"r"(reg):"memory");
}

static inline uint64_t
read_oslsr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, oslsr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_esr_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, esr_el1":"=r"(reg)::"memory");
	return (reg);
}

static inline uint64_t
read_far_el1(void)
{
	uint64_t reg;
	__asm__ __volatile__("mrs %0, far_el1":"=r"(reg)::"memory");
	return (reg);
}
#ifdef __cplusplus
}
#endif

#endif	/* _ASM_CONTROLREGS_H */
