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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_KDI_REGS_H
#define	_SYS_KDI_REGS_H

#ifndef _ASM
#include <sys/types.h>
#include <sys/regset.h>
#include <sys/privregs.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define	KDI_NCRUMBS	5

#define	KDI_CPU_STATE_NONE		0
#define	KDI_CPU_STATE_MASTER		1
#define	KDI_CPU_STATE_SLAVE		2

#define	KDIREG_DRCTL_WPALLEN_MASK	0x000000ff
#define	KDIREG_DRSTAT_RESERVED		0xffff0ff0
#define	KDIREG_DRCTL_RESERVED		0x00000700

#define	KDI_MSR_READ		0x1	/* read during entry (unlimited) */
#define	KDI_MSR_WRITE		0x2	/* write during exit (unlimited) */
#define	KDI_MSR_WRITEDELAY	0x4	/* write after last branch (<= 1) */
#define	KDI_MSR_CLEARENTRY	0x3	/* clear before 1st branch (<= 1) */

#ifndef _ASM

/*
 * We maintain a ring buffer of bread crumbs for debugging purposes.  The
 * current buffer pointer is advanced along the ring with each intercepted
 * trap (debugger entry, invalid memory access, fault during step, etc).
 */
typedef struct kdi_crumb {
	greg_t krm_cpu_state;	/* This CPU's state at last entry */
	greg_t krm_pc;		/* Instruction pointer at trap */
	greg_t krm_sp;		/* Stack pointer at trap */
	greg_t krm_trapno;	/* The last trap number */
	greg_t krm_flag;	/* KAIF_CRUMB_F_* */
} kdi_crumb_t;

#define	KDI_MAXWPIDX	3

/*
 * Storage for %dr0-3, %dr6, and %dr7.
 */
typedef struct kdi_drreg {
	greg_t			dr_ctl;
	greg_t			dr_stat;
	greg_t			dr_addr[KDI_MAXWPIDX + 1];
} kdi_drreg_t;

typedef struct kdi_msr {
	uint_t		msr_num;
	uint_t		msr_type;
	union {
		uint64_t *_msr_valp;
		uint64_t _msr_val;
	} _u;
} kdi_msr_t;

#define	kdi_msr_val	_u._msr_val
#define	kdi_msr_valp	_u._msr_valp

/*
 * Data structure used to hold all of the state for a given CPU.
 */
typedef struct kdi_cpusave {
	greg_t			*krs_gregs;	/* saved registers */

	uint_t			krs_cpu_state;	/* KDI_CPU_STATE_* mstr/slv */
	uint_t			krs_cpu_flushed; /* Have caches been flushed? */
	uint_t			krs_cpu_id;	/* this CPU's ID */

	/* Bread crumb ring buffer */
	ulong_t			krs_curcrumbidx; /* Current krs_crumbs idx */
	kdi_crumb_t		*krs_curcrumb;	/* Pointer to current crumb */
	kdi_crumb_t		krs_crumbs[KDI_NCRUMBS]; /* Crumbs */
} kdi_cpusave_t;

#endif /* !_ASM */

#ifdef __cplusplus
}
#endif

/*
 * A modified version of struct regs layout.
 * XXXARM: Not actually yet modified.  I don't know what this should look like.
 */

#define	KDIREG_SAVFP	0
#define	KDIREG_SAVPC	1
#define	KDIREG_X0	2
#define	KDIREG_X1	3
#define	KDIREG_X2	4
#define	KDIREG_X3	5
#define	KDIREG_X4	6
#define	KDIREG_X5	7
#define	KDIREG_X6	8
#define	KDIREG_X7	9
#define	KDIREG_X8	10
#define	KDIREG_X9	11
#define	KDIREG_X10	12
#define	KDIREG_X11	13
#define	KDIREG_X12	14
#define	KDIREG_X13	15
#define	KDIREG_X14	16
#define	KDIREG_X15	17
#define	KDIREG_X16	18
#define	KDIREG_X17	19
#define	KDIREG_X18	20
#define	KDIREG_X19	21
#define	KDIREG_X20	22
#define	KDIREG_X21	23
#define	KDIREG_X22	24
#define	KDIREG_X23	25
#define	KDIREG_X24	26
#define	KDIREG_X25	27
#define	KDIREG_X26	28
#define	KDIREG_X27	29
#define	KDIREG_X28	30
#define	KDIREG_X29	31
#define	KDIREG_FP	KDIREG_X29
#define	KDIREG_X30	32
#define	KDIREG_LR	KDIREG_X30
#define	KDIREG_SP	33
#define	KDIREG_PC	34
#define	KDIREG_PSR	35
#define	KDIREG_TP	36

#define	KDIREG_NGREG (KDIREG_TP + 1)


#endif /* _SYS_KDI_REGS_H */
