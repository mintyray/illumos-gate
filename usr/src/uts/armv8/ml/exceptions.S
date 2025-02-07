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
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright 2017 Hayashi Naoyuki
 */

#include <sys/asm_linkage.h>
#include <sys/errno.h>
#include <sys/privregs.h>

#include "assym.h"

#define	SYSENT_SHIFT	5
#if (1 << SYSENT_SHIFT) != SYSENT_SIZE
#error	"SYSENT_SHIFT does not correspond to size of sysent structure"
#endif

#define CPUP_THREADP(cp_reg, tp_reg)		\
	   mrs tp_reg, tpidr_el1;		\
	   ldr cp_reg, [tp_reg, #T_CPU]

#define CPUP(cp_reg)			\
	   mrs cp_reg, tpidr_el1;	\
	   ldr cp_reg, [cp_reg, #T_CPU]

#define THREADP(tp_reg)			\
	   mrs tp_reg, tpidr_el1


	.balign	2048
	ENTRY(exception_vector)
	/*
	 * From Current Exception level with SP_EL0
	 */
	.balign	0x80
from_current_el_sp0_sync:
0:	b	0b

	.balign	0x80
from_current_el_sp0_irq:
0:	b	0b

	.balign	0x80
from_current_el_sp0_fiq:
0:	b	0b

	.balign	0x80
from_current_el_sp0_error:
0:	b	0b


	/*
	 * From Current Exception level with SP_ELx
	 */
	.balign	0x80
from_current_el_sync:
	clrex
	__SAVE_REGS
	__SAVE_FRAME
	b	from_current_el_sync_handler

	.balign	0x80
from_current_el_irq:
	clrex
	__SAVE_REGS
	__SAVE_FRAME
	b	irq_handler

	.balign	0x80
from_current_el_fiq:
0:	b	0b

	.balign	0x80
from_current_el_error:
0:	b	0b


	/*
	 * From Lower Exception level using aarch64
	 */
	.balign	0x80
from_lower_el_aarch64_sync:
	clrex
	__SAVE_REGS
	__TERMINATE_FRAME
	mrs	x1, esr_el1
	lsr	w0, w1, #ESR_EC_SHIFT	// w0 <- EC, no need to mask
	cmp	w0, #T_SVC
	b.eq	svc_handler

	mrs	x2, far_el1
	mov	x3, sp

	bl	trap
	b	user_rtt

	.balign	0x80
from_lower_el_aarch64_irq:
	clrex
	__SAVE_REGS
	__TERMINATE_FRAME
	b	irq_handler

	.balign	0x80
from_lower_el_aarch64_fiq:
0:	b	0b

	.balign	0x80
from_lower_el_aarch64_error:
0:	b	0b


	/*
	 * From Lower Exception level using aarch32
	 */
	.balign	0x80
from_lower_el_aarch32_sync:
0:	b	0b

	.balign	0x80
from_lower_el_aarch32_irq:
0:	b	0b

	.balign	0x80
from_lower_el_aarch32_fiq:
0:	b	0b

	.balign	0x80
from_lower_el_aarch32_error:
0:	b	0b

	.balign	0x80
	SET_SIZE(exception_vector)

	ENTRY(from_current_el_sync_handler)
	mrs	x1, esr_el1
	lsr	w0, w1, #ESR_EC_SHIFT	// w0 <- EC, no need to mask.
	mrs	x2, far_el1
	mov	x3, sp
	mov	x29, sp
	bl	trap
	ALTENTRY(dtrace_invop_callsite)
	cbz	x0, 1f
	b	_sys_rtt
1:
	__RESTORE_REGS
	eret
	SET_SIZE(from_current_el_sync_handler)

	/*
	 * System call Handler From AArch64 EL0
	 */
	ENTRY(svc_handler)
	// w0: ESR_EL1.EC
	// w1: ESR_EL1

	mov	x29, sp

	and	w20, w1, #ESR_ISS_MASK	// w20 <- syscall number
	cbnz	w20, 1f
	mov	w20, w9

	// XXXARM: #15/0x8000 needs to be symbolic
1:	tbnz	w20, #15, _fasttrap	// if (w20 & 0x8000) goto _fasttrap
	CPUP(x0)

	// cpu_stats.sys.syscall++
	ldr	x9, [x0, #CPU_STATS_SYS_SYSCALL]
	add	x9, x9, #1
	str	x9, [x0, #CPU_STATS_SYS_SYSCALL]

	THREADP(x21)	// x21 <- thread
	ldr	x1, [x21, #T_LWP]	// x1 <- lwp

	// lwp->lwp_state = LWP_SYS
	mov	w10, #LWP_SYS
	strb	w10, [x1, #LWP_STATE]

	// lwp->lwp_ru.sysc++
	ldr	x9, [x1, #LWP_RU_SYSC]
	add	x9, x9, #1
	str	x9, [x1, #LWP_RU_SYSC]

	msr	DAIFClr, #DAIF_SETCLEAR_IRQ

	// syscall_mstate(LMS_USER, LMS_SYSTEM);
	mov	w0, #LMS_USER
	mov	w1, #LMS_SYSTEM
	bl	syscall_mstate

	strh	w20, [x21, #T_SYSNUM]
	ldrb	w9, [x21, #T_PRE_SYS]
	cbnz	w9, _syscall_pre

_syscall_invoke:
	ldr	x1, =sysent
	cmp	w20, #NSYSCALL
	b.hs	_syscall_ill

	add	x20, x1, x20, lsl #SYSENT_SHIFT	// x20 <- sysent
	ldr	x16, [x20, #SY_CALLC]
	ldp	x0, x1, [sp, #REGOFF_X0]
	ldp	x2, x3, [sp, #REGOFF_X2]
	ldp	x4, x5, [sp, #REGOFF_X4]
	ldp	x6, x7, [sp, #REGOFF_X6]
	blr	x16
	mov	x1, xzr

	ldrh	w9, [x20, #SY_FLAGS]
	ands	w9, w9, #SE_32RVAL2
	b.eq	2f
	lsr	x1, x0, #32
	mov	w0, w0
	mov	w1, w1
	sxtw	x0, w0
	sxtw	x1, w1
2:
	mov	x22, x0
	mov	x23, x1

	// syscall_mstate(LMS_SYSTEM, LMS_USER);
	mov	w0, #LMS_SYSTEM
	mov	w1, #LMS_USER
	bl	syscall_mstate

	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	ldr	w9, [x21, #T_POST_SYS_AST]
	cbnz	w9, _syscall_post

	ldr	x2, [sp, #REGOFF_SPSR]
	bic	x2, x2, #PSR_C
	str	x2, [sp, #REGOFF_SPSR]

	ldr	x2, [x21, #T_LWP]	// x2 <- lwp
	mov	w10, #LWP_USER
	strb	w10, [x2, #LWP_STATE]
	stp	x22, x23, [sp, #REGOFF_X0]
	strh	wzr, [x21, #T_SYSNUM]

	__RESTORE_REGS
	eret

_user_rtt:
	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	ldrb	w9, [x21, #T_ASTFLAG]
	cbnz	w9, 3f

	__RESTORE_REGS
	eret

3:
	mov	x0, #T_AST
	mov	x1, #0
	mov	x2, #0
	mov	x3, sp

	bl	trap
	b	_user_rtt

_syscall_pre:
	bl	pre_syscall
	mov	x22, x0
	mov	x23, xzr
	cbnz	w0, _syscall_post_call
	ldrh	w20, [x21, #T_SYSNUM]
	b	_syscall_invoke

_syscall_ill:
	bl	nosys
	mov	x22, x0
	mov	x23, xzr
	b	_syscall_post_call

_syscall_post:
	msr	DAIFClr, #DAIF_SETCLEAR_IRQ

	// syscall_mstate(LMS_USER, LMS_SYSTEM);
	mov	w0, #LMS_USER
	mov	w1, #LMS_SYSTEM
	bl	syscall_mstate
_syscall_post_call:
	mov	x0, x22
	mov	x1, x23
	bl	post_syscall

	// syscall_mstate(LMS_SYSTEM, LMS_USER);
	mov	w0, #LMS_SYSTEM
	mov	w1, #LMS_USER
	bl	syscall_mstate
	b	_sys_rtt

	ALTENTRY(user_rtt)
	THREADP(x21)
	b	_user_rtt
	SET_SIZE(_user_rtt)

_fasttrap:
	// XXXARM: #14 needs to be symbolic, and 0x7fff
	tbnz	w20, #14, .L_dtrace_pid	// if (w20 & 0x4000) goto .L_dtrace_pid
	and	w20, w20, #0x7fff
	cmp	w20, #T_LASTFAST
	b.cc	1f
	mov	w20, #0
1:
	ldr	x8, =fasttrap_table
	lsl	w20, w20, #3
	ldr	x9, [x8, x20]

	ldp	x0, x1, [sp, #REGOFF_X0]
	ldp	x2, x3, [sp, #REGOFF_X2]

	msr	DAIFClr, #DAIF_SETCLEAR_IRQ
	blr	x9
	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	ldp	x17, x18, [sp, #REGOFF_PC]
	msr	elr_el1, x17
	msr	spsr_el1, x18
	ldr	x29, [sp, #REGOFF_X29]
	ldp	x30, x16, [sp, #REGOFF_X30]
	msr	sp_el0, x16
	ldr	x20, [sp, #REGOFF_X20]
	add	sp, sp, #REG_FRAME
	eret
.L_dtrace_pid:
	adr	x30, user_rtt
	msr	DAIFClr, #DAIF_SETCLEAR_IRQ
	mov	x0, sp
	mov	w1, w20
	b	dtrace_user_probe
	SET_SIZE(svc_handler)

	ENTRY(getlgrp)
	CPUP_THREADP(x3, x2)

	ldr	x3, [x2, #T_LPL]
	ldr	w1, [x3, #LPL_LGRPID]	/* x1 = t->t_lpl->lpl_lgrpid */
	ldr	x3, [x2, #T_CPU]
	ldr	w0, [x3, #CPU_ID]	/* x0 = t->t_cpu->cpu_id */
	ret
	SET_SIZE(getlgrp)

	ENTRY(fasttrap_gethrestime)
	stp	x29, x30, [sp, #-(8*2)]!
	sub	sp, sp, #TIMESPEC_SIZE

	mov	x0, sp
	bl	gethrestime

	ldr	x0, [sp, #TV_SEC]
	ldr	x1, [sp, #TV_NSEC]

	add	sp, sp, #TIMESPEC_SIZE
	ldp	x29, x30, [sp], #(8*2)
	ret
	SET_SIZE(fasttrap_gethrestime)

	ENTRY(_sys_rtt)
	ldr	x2, [sp, #REGOFF_SPSR]
	and	x2, x2, #PSR_M_MASK
	cmp	x2, #PSR_M_EL0t
	b.eq	user_rtt

	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	CPUP(x21)
	ldrb	w0, [x21, #CPU_KPRUNRUN]
	cbnz	w0, _sys_rtt_preempt

_sys_rtt_preempt_ret:
	ldr	x0, [sp, #REGOFF_PC]

	ldr	x1, = mutex_owner_running_critical_start
	ldr	x2, = mutex_owner_running_critical_size
	ldr	x2, [x2]
	sub	x3, x0, x1
	cmp	x3, x2
	b.lo	2f
1:
	__RESTORE_REGS
	eret

2:	str	x1, [sp, #REGOFF_PC]
	b	1b

_sys_rtt_preempt:
	THREADP(x20)
	ldrb	w0, [x20, #T_PREEMPT_LK]
	cbnz	w0, _sys_rtt_preempt_ret
	mov	w0, #1
	strb	w0, [x20, #T_PREEMPT_LK]

	msr	DAIFClr, #DAIF_SETCLEAR_IRQ
	bl	kpreempt
	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	strb	wzr, [x20, #T_PREEMPT_LK]
	b	_sys_rtt_preempt_ret
	SET_SIZE(_sys_rtt)

	/*
	 * IRQ Handler
	 */
	ENTRY(irq_handler)
	CPUP(x19)				// x19 <- CPU
	ldr	w20, [x19, #CPU_PRI]		// x20 <- old pri

	ldr	x21, =gic_cpuif			// x21 <- gic_cpuif
	ldr	x21, [x21]
	ldr	w23, [x21, #GIC_CPUIF_IAR]	// x23 <- IAR, acknowledge receipt
	and	w22, w23, #GICC_IAR_INTID_MASK	// x22 <- irq
	// On receipt of a "special" interrupt, just return
	// See ARM® Generic Interrupt Controller Architecture Specification
	//    GIC architecture version 3.0 and version 4.0 §2.2.1 Special INTIDs
	//
	// XXXARM: This code needs changing to support LPIs in GICv3 and above
	cmp	w22, #GIC_INTID_MIN_SPECIAL
	b.ge	_sys_rtt

	// irq mask (Clear Enable)
	mov	w0, w22
	bl	gic_mask_level_irq

	// write the full GICC_IAR back to GICC_EOIR to acknowledge end of interrupt
	str	w23, [x21, #GIC_CPUIF_EOIR]

	// set pri
	mov	w0, w22
	bl	setlvl
	cbnz	w0, 1f

	// If ipl == 0
	mov	w0, w22
	bl	gic_unmask_level_irq
	b	check_softint

	// Else (ipl != 0)
1:
	mov	w21, w0				// x21 <- new pri
	str	w21, [x19, #CPU_PRI]

	cmp	w21, #LOCK_LEVEL
	b.le	intr_thread

	//	x19: CPU
	//	x20: old pri
	//	x21: new pri
	//	x22: irq

	mov	x0, x19
	mov	w1, w21
	mov	w2, w20
	mov	x3, sp
	bl	hilevel_intr_prolog
	cbnz	x0, 2f

	mov	x23, sp				// x23 <- old sp
	ldr	x0, [x19, #CPU_INTR_STACK]
	mov	sp, x0
2:

	/* Dispatch interrupt handler. */
	msr	DAIFClr, #DAIF_SETCLEAR_IRQ
	mov	w0, w22
	bl	av_dispatch_autovect
	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	mov	x0, x19
	mov	w1, w21
	mov	w2, w20
	mov	w3, w22
	bl	hilevel_intr_epilog
	cbnz	x0, check_softint

	mov	sp, x23
	b	check_softint

intr_thread:
	//	x19: CPU
	//	x20: old pri
	//	x21: new pri
	//	x22: irq

	mov	x0, x19
	mov	x1, sp
	mov	w2, w21
	bl	intr_thread_prolog
	mov	x23, sp				// x23 <- old sp
	mov	sp, x0

	/* Dispatch interrupt handler. */
	msr	DAIFClr, #DAIF_SETCLEAR_IRQ
	mov	w0, w22
	bl	av_dispatch_autovect
	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	mov	x0, x19
	mov	x1, x22
	mov	x2, x20
	bl	intr_thread_epilog

	mov	sp, x23

check_softint:
	ldr	w22, [x19, #CPU_SOFTINFO]
	cbnz	w22, dosoftint
	b	_sys_rtt

dosoftint:
	//	x19: CPU
	//	x20: old pri
	//	x21: new pri
	//	x22: st_pending

	mov	x0, x19
	mov	x1, sp
	mov	w2, w22
	mov	w3, w20
	bl	dosoftint_prolog
	cbz	x0, _sys_rtt

	mov	x23, sp				// x23 <- old sp
	mov	sp, x0

	msr	DAIFClr, #DAIF_SETCLEAR_IRQ
	THREADP(x0)
	ldrb	w0, [x0, #T_PIL]
	bl	av_dispatch_softvect
	msr	DAIFSet, #DAIF_SETCLEAR_IRQ

	mov	x0, x19
	mov	w1, w20
	bl	dosoftint_epilog

	mov	sp, x23
	b	check_softint
	SET_SIZE(irq_handler)


	ENTRY(lwp_rtt_initial)
	THREADP(x0)
	ldr	x9, [x0, #T_STACK]			// switch stack
	mov	sp, x9
1:
	bl	__dtrace_probe___proc_start
	.pushsection .probepoint, "aw"
	.xword 1b
	.popsection

	b	1f

	ALTENTRY(lwp_rtt)
	THREADP(x0)
	ldr	x9, [x0, #T_STACK]			// switch stack
	mov	sp, x9
1:
	bl	__dtrace_probe___proc_lwp__start
	.pushsection .probepoint, "aw"
	.xword 1b
	.popsection
	bl	dtrace_systrace_rtt

	ldr	x0, [sp, #REGOFF_X0]
	ldr	x1, [sp, #REGOFF_X1]
	adr	x30, user_rtt
	b	post_syscall
	SET_SIZE(lwp_rtt)
	SET_SIZE(lwp_rtt_initial)

	.data
	.globl t1stack
	.type t1stack, @object
	.size t1stack, DEFAULTSTKSZ
	.align MMU_PAGESHIFT
t1stack:
	.zero DEFAULTSTKSZ
