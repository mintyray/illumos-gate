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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright 2017 Hayashi Naoyuki
 */

#include <sys/types.h>
#include <sys/mkdev.h>
#include <sys/regset.h>
#include <string.h>

#if defined(__amd64)
#include <sys/fp.h>
#include <ieeefp.h>
#endif

#include "P32ton.h"

dev_t
prexpldev(dev32_t d)
{
	if (d != (dev32_t)-1L)
		return (makedev((d >> NBITSMINOR32) & MAXMAJ32, d & MAXMIN32));

	return ((dev_t)PRNODEV);
}


dev32_t
prcmpldev(dev_t d)
{
#if defined(_LP64) && defined(_MULTI_DATAMODEL)
	if (d == PRNODEV) {
		return (PRNODEV32);
	} else {
		major_t maj = major(d);
		minor_t min = minor(d);

		if (maj == (major_t)PRNODEV || min == (minor_t)PRNODEV)
			return (PRNODEV32);

		return ((dev32_t)((maj << NBITSMINOR32) | min));
	}
#else
	return ((dev32_t)d);
#endif
}

#if defined(_LP64) && defined(_MULTI_DATAMODEL)
void
timestruc_32_to_n(const timestruc32_t *src, timestruc_t *dst)
{
	dst->tv_sec = (time_t)(uint32_t)src->tv_sec;
	dst->tv_nsec = (long)(uint32_t)src->tv_nsec;
}

void
stack_32_to_n(const stack32_t *src, stack_t *dst)
{
	dst->ss_sp = (caddr_t)(uintptr_t)src->ss_sp;
	dst->ss_size = src->ss_size;
	dst->ss_flags = src->ss_flags;
}

void
sigaction_32_to_n(const struct sigaction32 *src, struct sigaction *dst)
{
	(void) memset(dst, 0, sizeof (struct sigaction));
	dst->sa_flags = src->sa_flags;
	dst->sa_handler = (void (*)())(uintptr_t)src->sa_handler;
	(void) memcpy(&dst->sa_mask, &src->sa_mask, sizeof (dst->sa_mask));
}

void
siginfo_32_to_n(const siginfo32_t *src, siginfo_t *dst)
{
	(void) memset(dst, 0, sizeof (siginfo_t));

	/*
	 * The absolute minimum content is si_signo and si_code.
	 */
	dst->si_signo = src->si_signo;
	if ((dst->si_code = src->si_code) == SI_NOINFO)
		return;

	/*
	 * A siginfo generated by user level is structured
	 * differently from one generated by the kernel.
	 */
	if (SI_FROMUSER(src)) {
		dst->si_pid = src->si_pid;
		dst->si_ctid = src->si_ctid;
		dst->si_zoneid = src->si_zoneid;
		dst->si_uid = src->si_uid;
		if (SI_CANQUEUE(src->si_code)) {
			dst->si_value.sival_int =
			    (long)(uint32_t)src->si_value.sival_int;
		}
		return;
	}

	dst->si_errno = src->si_errno;

	switch (src->si_signo) {
	default:
		dst->si_pid = src->si_pid;
		dst->si_ctid = src->si_ctid;
		dst->si_zoneid = src->si_zoneid;
		dst->si_uid = src->si_uid;
		dst->si_value.sival_int =
		    (long)(uint32_t)src->si_value.sival_int;
		break;
	case SIGCLD:
		dst->si_pid = src->si_pid;
		dst->si_ctid = src->si_ctid;
		dst->si_zoneid = src->si_zoneid;
		dst->si_status = src->si_status;
		dst->si_stime = src->si_stime;
		dst->si_utime = src->si_utime;
		break;
	case SIGSEGV:
	case SIGBUS:
	case SIGILL:
	case SIGTRAP:
	case SIGFPE:
	case SIGEMT:
		dst->si_addr = (void *)(uintptr_t)src->si_addr;
		dst->si_trapno = src->si_trapno;
		dst->si_pc = (void *)(uintptr_t)src->si_pc;
		break;
	case SIGPOLL:
	case SIGXFSZ:
		dst->si_fd = src->si_fd;
		dst->si_band = src->si_band;
		break;
	case SIGPROF:
		dst->si_faddr = (void *)(uintptr_t)src->si_faddr;
		dst->si_tstamp.tv_sec = src->si_tstamp.tv_sec;
		dst->si_tstamp.tv_nsec = src->si_tstamp.tv_nsec;
		dst->si_syscall = src->si_syscall;
		dst->si_nsysarg = src->si_nsysarg;
		dst->si_fault = src->si_fault;
		break;
	}
}

void
auxv_32_to_n(const auxv32_t *src, auxv_t *dst)
{
	/*
	 * This is a little sketchy: we have three types of values stored
	 * in an auxv (long, void *, and void (*)()) so the only sign-extension
	 * issue is with the long.  We could case on all possible AT_* types,
	 * but this seems silly since currently none of the types which use
	 * a_un.a_val actually use negative numbers as a value.  For this
	 * reason, it seems simpler to just do an unsigned expansion for now.
	 */
	dst->a_type = src->a_type;
	dst->a_un.a_ptr = (void *)(uintptr_t)src->a_un.a_ptr;
}

#if defined(__sparc)
void
rwindow_32_to_n(const struct rwindow32 *src, struct rwindow *dst)
{
	int i;

	for (i = 0; i < 8; i++) {
		dst->rw_local[i] = (uint64_t)(uint32_t)src->rw_local[i];
		dst->rw_in[i] = (uint64_t)(uint32_t)src->rw_in[i];
	}
}

void
gwindows_32_to_n(const gwindows32_t *src, gwindows_t *dst)
{
	int i;

	(void) memset(dst, 0, sizeof (gwindows_t));
	dst->wbcnt = src->wbcnt;

	for (i = 0; i < src->wbcnt; i++) {
		if (src->spbuf[i] != 0) {
			rwindow_32_to_n(&src->wbuf[i], &dst->wbuf[i]);
			dst->spbuf[i] = (greg_t *)(uintptr_t)src->spbuf[i];
		}
	}
}
#endif	/* __sparc */

void
prgregset_32_to_n(const prgreg32_t *src, prgreg_t *dst)
{
#ifdef __amd64
	(void) memset(dst, 0, NPRGREG * sizeof (prgreg_t));
	dst[REG_GS] = (uint32_t)src[GS];
	dst[REG_FS] = (uint32_t)src[FS];
	dst[REG_DS] = (uint32_t)src[DS];
	dst[REG_ES] = (uint32_t)src[ES];
	dst[REG_RDI] = (uint32_t)src[EDI];
	dst[REG_RSI] = (uint32_t)src[ESI];
	dst[REG_RBP] = (uint32_t)src[EBP];
	dst[REG_RBX] = (uint32_t)src[EBX];
	dst[REG_RDX] = (uint32_t)src[EDX];
	dst[REG_RCX] = (uint32_t)src[ECX];
	dst[REG_RAX] = (uint32_t)src[EAX];
	dst[REG_TRAPNO] = (uint32_t)src[TRAPNO];
	dst[REG_ERR] = (uint32_t)src[ERR];
	dst[REG_RIP] = (uint32_t)src[EIP];
	dst[REG_CS] = (uint32_t)src[CS];
	dst[REG_RFL] = (uint32_t)src[EFL];
	dst[REG_RSP] = (uint32_t)src[UESP];
	dst[REG_SS] = (uint32_t)src[SS];
#else
	int i;

	for (i = 0; i < NPRGREG; i++)
		dst[i] = (prgreg_t)(uint32_t)src[i];
#endif
}

void
prfpregset_32_to_n(const prfpregset32_t *src, prfpregset_t *dst)
{
#if defined(__sparc)
	int i;

	(void) memset(dst, 0, sizeof (prfpregset_t));

	for (i = 0; i < 32; i++)
		dst->pr_fr.pr_regs[i] = src->pr_fr.pr_regs[i];

	/*
	 * We deliberately do not convert pr_qcnt or pr_q because it is a long-
	 * standing /proc bug that this information is not exported, and another
	 * bug further caused these values to be returned as uninitialized data
	 * when the 64-bit kernel exported them for a 32-bit process with en=0.
	 */
	dst->pr_filler = src->pr_filler;
	dst->pr_fsr = src->pr_fsr;
	dst->pr_q_entrysize = src->pr_q_entrysize;
	dst->pr_en = src->pr_en;

#elif defined(__amd64)

	struct _fpstate32 *src32 = (struct _fpstate32 *)src;
	struct _fpchip_state *dst64 = (struct _fpchip_state *)dst;
	int i;

	(void) memcpy(dst64->st, src32->_st, sizeof (src32->_st));
	(void) memcpy(dst64->xmm, src32->xmm, sizeof (src32->xmm));
	(void) memset((caddr_t)dst64->xmm + sizeof (src32->xmm), 0,
	    sizeof (dst64->xmm) - sizeof (src32->xmm));
	dst64->cw = (uint16_t)src32->cw;
	dst64->sw = (uint16_t)src32->sw;
	dst64->fop = 0;
	dst64->rip = src32->ipoff;
	dst64->rdp = src32->dataoff;
	dst64->mxcsr = src32->mxcsr;
	dst64->mxcsr_mask = 0;
	dst64->status = src32->status;
	dst64->xstatus = src32->xstatus;

	/*
	 * Converting from the tag field to the compressed fctw is easy.
	 * If the two tag bits are 3, then the register is empty and we
	 * clear the bit in fctw. Otherwise we set the bit.
	 */

	dst64->fctw = 0;
	for (i = 0; i < 8; i++)
		if (((src32->tag >> (i * 2)) & 3) != 3)
			dst64->fctw |= 1 << i;
#else
#error "unrecognized ISA"
#endif
}

void
lwpstatus_32_to_n(const lwpstatus32_t *src, lwpstatus_t *dst)
{
	int i;

	dst->pr_flags = src->pr_flags;
	dst->pr_lwpid = src->pr_lwpid;
	dst->pr_why = src->pr_why;
	dst->pr_what = src->pr_what;
	dst->pr_cursig = src->pr_cursig;

	siginfo_32_to_n(&src->pr_info, &dst->pr_info);

	dst->pr_lwppend = src->pr_lwppend;
	dst->pr_lwphold = src->pr_lwphold;

	sigaction_32_to_n(&src->pr_action, &dst->pr_action);
	stack_32_to_n(&src->pr_altstack, &dst->pr_altstack);

	dst->pr_oldcontext = src->pr_oldcontext;
	dst->pr_syscall = src->pr_syscall;
	dst->pr_nsysarg = src->pr_nsysarg;
	dst->pr_errno = src->pr_errno;

	for (i = 0; i < PRSYSARGS; i++)
		dst->pr_sysarg[i] = (long)(uint32_t)src->pr_sysarg[i];

	dst->pr_rval1 = (long)(uint32_t)src->pr_rval1;
	dst->pr_rval2 = (long)(uint32_t)src->pr_rval2;

	(void) memcpy(&dst->pr_clname[0], &src->pr_clname[0], PRCLSZ);
	timestruc_32_to_n(&src->pr_tstamp, &dst->pr_tstamp);

	dst->pr_ustack = src->pr_ustack;
	dst->pr_instr = src->pr_instr;

	prgregset_32_to_n(src->pr_reg, dst->pr_reg);
	prfpregset_32_to_n(&src->pr_fpreg, &dst->pr_fpreg);
}

void
pstatus_32_to_n(const pstatus32_t *src, pstatus_t *dst)
{
	dst->pr_flags = src->pr_flags;
	dst->pr_nlwp = src->pr_nlwp;
	dst->pr_nzomb = src->pr_nzomb;
	dst->pr_pid = src->pr_pid;
	dst->pr_ppid = src->pr_ppid;
	dst->pr_pgid = src->pr_pgid;
	dst->pr_sid = src->pr_sid;
	dst->pr_taskid = src->pr_taskid;
	dst->pr_projid = src->pr_projid;
	dst->pr_zoneid = src->pr_zoneid;
	dst->pr_aslwpid = src->pr_aslwpid;
	dst->pr_agentid = src->pr_agentid;
	dst->pr_sigpend = src->pr_sigpend;
	dst->pr_brkbase = src->pr_brkbase;
	dst->pr_brksize = src->pr_brksize;
	dst->pr_stkbase = src->pr_stkbase;
	dst->pr_stksize = src->pr_stksize;

	timestruc_32_to_n(&src->pr_utime, &dst->pr_utime);
	timestruc_32_to_n(&src->pr_stime, &dst->pr_stime);
	timestruc_32_to_n(&src->pr_cutime, &dst->pr_cutime);
	timestruc_32_to_n(&src->pr_cstime, &dst->pr_cstime);

	dst->pr_sigtrace = src->pr_sigtrace;
	dst->pr_flttrace = src->pr_flttrace;
	dst->pr_sysentry = src->pr_sysentry;
	dst->pr_sysexit = src->pr_sysexit;
	dst->pr_dmodel = src->pr_dmodel;

	lwpstatus_32_to_n(&src->pr_lwp, &dst->pr_lwp);
}

void
lwpsinfo_32_to_n(const lwpsinfo32_t *src, lwpsinfo_t *dst)
{
	dst->pr_flag = src->pr_flag;
	dst->pr_lwpid = src->pr_lwpid;
	dst->pr_addr = src->pr_addr;
	dst->pr_wchan = src->pr_wchan;
	dst->pr_stype = src->pr_stype;
	dst->pr_state = src->pr_state;
	dst->pr_sname = src->pr_sname;
	dst->pr_nice = src->pr_nice;
	dst->pr_syscall = src->pr_syscall;
	dst->pr_oldpri = src->pr_oldpri;
	dst->pr_cpu = src->pr_cpu;
	dst->pr_pri = src->pr_pri;
	dst->pr_pctcpu = src->pr_pctcpu;

	timestruc_32_to_n(&src->pr_start, &dst->pr_start);
	timestruc_32_to_n(&src->pr_time, &dst->pr_time);

	(void) memcpy(&dst->pr_clname[0], &src->pr_clname[0], PRCLSZ);
	(void) memcpy(&dst->pr_name[0], &src->pr_name[0], PRFNSZ);

	dst->pr_onpro = src->pr_onpro;
	dst->pr_bindpro = src->pr_bindpro;
	dst->pr_bindpset = src->pr_bindpset;
	dst->pr_lgrp = src->pr_lgrp;
}

void
psinfo_32_to_n(const psinfo32_t *src, psinfo_t *dst)
{
	dst->pr_flag = src->pr_flag;
	dst->pr_nlwp = src->pr_nlwp;
	dst->pr_nzomb = src->pr_nzomb;
	dst->pr_pid = src->pr_pid;
	dst->pr_pgid = src->pr_pgid;
	dst->pr_sid = src->pr_sid;
	dst->pr_taskid = src->pr_taskid;
	dst->pr_projid = src->pr_projid;
	dst->pr_zoneid = src->pr_zoneid;
	dst->pr_uid = src->pr_uid;
	dst->pr_euid = src->pr_euid;
	dst->pr_gid = src->pr_gid;
	dst->pr_egid = src->pr_egid;
	dst->pr_addr = src->pr_addr;
	dst->pr_size = src->pr_size;
	dst->pr_rssize = src->pr_rssize;

	dst->pr_ttydev = prexpldev(src->pr_ttydev);

	dst->pr_pctcpu = src->pr_pctcpu;
	dst->pr_pctmem = src->pr_pctmem;

	timestruc_32_to_n(&src->pr_start, &dst->pr_start);
	timestruc_32_to_n(&src->pr_time, &dst->pr_time);
	timestruc_32_to_n(&src->pr_ctime, &dst->pr_ctime);

	(void) memcpy(&dst->pr_fname[0], &src->pr_fname[0], PRFNSZ);
	(void) memcpy(&dst->pr_psargs[0], &src->pr_psargs[0], PRARGSZ);

	dst->pr_wstat = src->pr_wstat;
	dst->pr_argc = src->pr_argc;
	dst->pr_argv = src->pr_argv;
	dst->pr_envp = src->pr_envp;
	dst->pr_dmodel = src->pr_dmodel;

	lwpsinfo_32_to_n(&src->pr_lwp, &dst->pr_lwp);
}

void
timestruc_n_to_32(const timestruc_t *src, timestruc32_t *dst)
{
	dst->tv_sec = (time32_t)src->tv_sec;
	dst->tv_nsec = (int32_t)src->tv_nsec;
}

void
stack_n_to_32(const stack_t *src, stack32_t *dst)
{
	dst->ss_sp = (caddr32_t)(uintptr_t)src->ss_sp;
	dst->ss_size = src->ss_size;
	dst->ss_flags = src->ss_flags;
}

void
sigaction_n_to_32(const struct sigaction *src, struct sigaction32 *dst)
{
	(void) memset(dst, 0, sizeof (struct sigaction32));
	dst->sa_flags = src->sa_flags;
	dst->sa_handler = (caddr32_t)(uintptr_t)src->sa_handler;
	(void) memcpy(&dst->sa_mask, &src->sa_mask, sizeof (dst->sa_mask));
}

void
siginfo_n_to_32(const siginfo_t *src, siginfo32_t *dst)
{
	(void) memset(dst, 0, sizeof (siginfo32_t));

	/*
	 * The absolute minimum content is si_signo and si_code.
	 */
	dst->si_signo = src->si_signo;
	if ((dst->si_code = src->si_code) == SI_NOINFO)
		return;

	/*
	 * A siginfo generated by user level is structured
	 * differently from one generated by the kernel.
	 */
	if (SI_FROMUSER(src)) {
		dst->si_pid = src->si_pid;
		dst->si_ctid = src->si_ctid;
		dst->si_zoneid = src->si_zoneid;
		dst->si_uid = src->si_uid;
		if (SI_CANQUEUE(src->si_code)) {
			dst->si_value.sival_int =
			    (int32_t)src->si_value.sival_int;
		}
		return;
	}

	dst->si_errno = src->si_errno;

	switch (src->si_signo) {
	default:
		dst->si_pid = src->si_pid;
		dst->si_ctid = src->si_ctid;
		dst->si_zoneid = src->si_zoneid;
		dst->si_uid = src->si_uid;
		dst->si_value.sival_int =
		    (int32_t)src->si_value.sival_int;
		break;
	case SIGCLD:
		dst->si_pid = src->si_pid;
		dst->si_ctid = src->si_ctid;
		dst->si_zoneid = src->si_zoneid;
		dst->si_status = src->si_status;
		dst->si_stime = src->si_stime;
		dst->si_utime = src->si_utime;
		break;
	case SIGSEGV:
	case SIGBUS:
	case SIGILL:
	case SIGTRAP:
	case SIGFPE:
	case SIGEMT:
		dst->si_addr = (caddr32_t)(uintptr_t)src->si_addr;
		dst->si_trapno = src->si_trapno;
		dst->si_pc = (caddr32_t)(uintptr_t)src->si_pc;
		break;
	case SIGPOLL:
	case SIGXFSZ:
		dst->si_fd = src->si_fd;
		dst->si_band = src->si_band;
		break;
	case SIGPROF:
		dst->si_faddr = (caddr32_t)(uintptr_t)src->si_faddr;
		dst->si_tstamp.tv_sec = src->si_tstamp.tv_sec;
		dst->si_tstamp.tv_nsec = src->si_tstamp.tv_nsec;
		dst->si_syscall = src->si_syscall;
		dst->si_nsysarg = src->si_nsysarg;
		dst->si_fault = src->si_fault;
		break;
	}
}

void
auxv_n_to_32(const auxv_t *src, auxv32_t *dst)
{
	dst->a_type = src->a_type;
	dst->a_un.a_ptr = (caddr32_t)(uintptr_t)src->a_un.a_ptr;
}

void
prgregset_n_to_32(const prgreg_t *src, prgreg32_t *dst)
{
#ifdef __amd64
	(void) memset(dst, 0, NPRGREG32 * sizeof (prgreg32_t));
	dst[GS] = src[REG_GS];
	dst[FS] = src[REG_FS];
	dst[DS] = src[REG_DS];
	dst[ES] = src[REG_ES];
	dst[EDI] = src[REG_RDI];
	dst[ESI] = src[REG_RSI];
	dst[EBP] = src[REG_RBP];
	dst[EBX] = src[REG_RBX];
	dst[EDX] = src[REG_RDX];
	dst[ECX] = src[REG_RCX];
	dst[EAX] = src[REG_RAX];
	dst[TRAPNO] = src[REG_TRAPNO];
	dst[ERR] = src[REG_ERR];
	dst[EIP] = src[REG_RIP];
	dst[CS] = src[REG_CS];
	dst[EFL] = src[REG_RFL];
	dst[UESP] = src[REG_RSP];
	dst[SS] = src[REG_SS];
#else
	int i;

	for (i = 0; i < NPRGREG; i++)
		dst[i] = (prgreg32_t)src[i];
#endif
}

void
prfpregset_n_to_32(const prfpregset_t *src, prfpregset32_t *dst)
{
#if defined(__sparc)
	int i;

	(void) memset(dst, 0, sizeof (prfpregset32_t));

	for (i = 0; i < 32; i++)
		dst->pr_fr.pr_regs[i] = src->pr_fr.pr_regs[i];

	dst->pr_filler = src->pr_filler;
	dst->pr_fsr = src->pr_fsr;
	dst->pr_q_entrysize = src->pr_q_entrysize;
	dst->pr_en = src->pr_en;

#elif defined(__amd64)

	struct _fpstate32 *dst32 = (struct _fpstate32 *)dst;
	struct _fpchip_state *src64 = (struct _fpchip_state *)src;
	uint32_t top;
	int i;

	(void) memcpy(dst32->_st, src64->st, sizeof (dst32->_st));
	(void) memcpy(dst32->xmm, src64->xmm, sizeof (dst32->xmm));
	dst32->cw = src64->cw;
	dst32->sw = src64->sw;
	dst32->ipoff = (unsigned int)src64->rip;
	dst32->cssel = 0;
	dst32->dataoff = (unsigned int)src64->rdp;
	dst32->datasel = 0;
	dst32->status = src64->status;
	dst32->mxcsr = src64->mxcsr;
	dst32->xstatus = src64->xstatus;

	/*
	 * AMD64 stores the tag in a compressed form. It is
	 * necessary to extract the original 2-bit tag value.
	 * See AMD64 Architecture Programmer's Manual Volume 2:
	 * System Programming, Chapter 11.
	 */

	top = (src64->sw & FPS_TOP) >> 11;
	dst32->tag = 0;
	for (i = 0; i < 8; i++) {
		/*
		 * Recall that we need to use the current TOP-of-stack value to
		 * associate the _st[] index back to a physical register number,
		 * since tag word indices are physical register numbers.  Then
		 * to get the tag value, we shift over two bits for each tag
		 * index, and then grab the bottom two bits.
		 */
		uint_t tag_index = (i + top) & 7;
		uint_t tag_fctw = (src64->fctw >> tag_index) & 1;
		uint_t tag_value;
		uint_t exp;

		/*
		 * Union for overlaying _fpreg structure on to quad-precision
		 * floating-point value (long double).
		 */
		union {
			struct _fpreg reg;
			long double ld;
		} fpru;

		fpru.ld = src64->st[i].__fpr_pad._q;
		exp = fpru.reg.exponent & 0x7fff;

		if (tag_fctw == 0) {
			tag_value = 3; /* empty */
		} else if (exp == 0) {
			if (fpru.reg.significand[0] == 0 &&
			    fpru.reg.significand[1] == 0 &&
			    fpru.reg.significand[2] == 0 &&
			    fpru.reg.significand[3] == 0)
				tag_value = 1; /* zero */
			else
				tag_value = 2; /* special: denormal */
		} else if (exp == 0x7fff) {
			tag_value = 2; /* special: infinity or NaN */
		} else if (fpru.reg.significand[3] & 0x8000) {
			tag_value = 0; /* valid */
		} else {
			tag_value = 2; /* special: unnormal */
		}
		dst32->tag |= tag_value << (tag_index * 2);
	}
#else
#error "unrecognized ISA"
#endif
}

void
lwpstatus_n_to_32(const lwpstatus_t *src, lwpstatus32_t *dst)
{
	int i;

	dst->pr_flags = src->pr_flags;
	dst->pr_lwpid = src->pr_lwpid;
	dst->pr_why = src->pr_why;
	dst->pr_what = src->pr_what;
	dst->pr_cursig = src->pr_cursig;

	siginfo_n_to_32(&src->pr_info, &dst->pr_info);

	dst->pr_lwppend = src->pr_lwppend;
	dst->pr_lwphold = src->pr_lwphold;

	sigaction_n_to_32(&src->pr_action, &dst->pr_action);
	stack_n_to_32(&src->pr_altstack, &dst->pr_altstack);

	dst->pr_oldcontext = (caddr32_t)src->pr_oldcontext;
	dst->pr_syscall = src->pr_syscall;
	dst->pr_nsysarg = src->pr_nsysarg;
	dst->pr_errno = src->pr_errno;

	for (i = 0; i < PRSYSARGS; i++)
		dst->pr_sysarg[i] = (int32_t)src->pr_sysarg[i];

	dst->pr_rval1 = (int32_t)src->pr_rval1;
	dst->pr_rval2 = (int32_t)src->pr_rval2;

	(void) memcpy(&dst->pr_clname[0], &src->pr_clname[0], PRCLSZ);
	timestruc_n_to_32(&src->pr_tstamp, &dst->pr_tstamp);

	dst->pr_ustack = (caddr32_t)src->pr_ustack;
	dst->pr_instr = src->pr_instr;

	prgregset_n_to_32(src->pr_reg, dst->pr_reg);
	prfpregset_n_to_32(&src->pr_fpreg, &dst->pr_fpreg);
}

void
pstatus_n_to_32(const pstatus_t *src, pstatus32_t *dst)
{
	dst->pr_flags = src->pr_flags;
	dst->pr_nlwp = src->pr_nlwp;
	dst->pr_nzomb = src->pr_nzomb;
	dst->pr_pid = (pid32_t)src->pr_pid;
	dst->pr_ppid = (pid32_t)src->pr_ppid;
	dst->pr_pgid = (pid32_t)src->pr_pgid;
	dst->pr_sid = (pid32_t)src->pr_sid;
	dst->pr_taskid = (id32_t)src->pr_taskid;
	dst->pr_projid = (id32_t)src->pr_projid;
	dst->pr_zoneid = (id32_t)src->pr_zoneid;
	dst->pr_aslwpid = (id32_t)src->pr_aslwpid;
	dst->pr_agentid = (id32_t)src->pr_agentid;
	dst->pr_sigpend = src->pr_sigpend;
	dst->pr_brkbase = (caddr32_t)src->pr_brkbase;
	dst->pr_brksize = (size32_t)src->pr_brksize;
	dst->pr_stkbase = (caddr32_t)src->pr_stkbase;
	dst->pr_stksize = (size32_t)src->pr_stksize;

	timestruc_n_to_32(&src->pr_utime, &dst->pr_utime);
	timestruc_n_to_32(&src->pr_stime, &dst->pr_stime);
	timestruc_n_to_32(&src->pr_cutime, &dst->pr_cutime);
	timestruc_n_to_32(&src->pr_cstime, &dst->pr_cstime);

	dst->pr_sigtrace = src->pr_sigtrace;
	dst->pr_flttrace = src->pr_flttrace;
	dst->pr_sysentry = src->pr_sysentry;
	dst->pr_sysexit = src->pr_sysexit;
	dst->pr_dmodel = src->pr_dmodel;

	lwpstatus_n_to_32(&src->pr_lwp, &dst->pr_lwp);
}

void
lwpsinfo_n_to_32(const lwpsinfo_t *src, lwpsinfo32_t *dst)
{
	dst->pr_flag = src->pr_flag;
	dst->pr_lwpid = (id32_t)src->pr_lwpid;
	dst->pr_addr = (caddr32_t)src->pr_addr;
	dst->pr_wchan = (caddr32_t)src->pr_wchan;
	dst->pr_stype = src->pr_stype;
	dst->pr_state = src->pr_state;
	dst->pr_sname = src->pr_sname;
	dst->pr_nice = src->pr_nice;
	dst->pr_syscall = src->pr_syscall;
	dst->pr_oldpri = src->pr_oldpri;
	dst->pr_cpu = src->pr_cpu;
	dst->pr_pri = src->pr_pri;
	dst->pr_pctcpu = src->pr_pctcpu;

	timestruc_n_to_32(&src->pr_start, &dst->pr_start);
	timestruc_n_to_32(&src->pr_time, &dst->pr_time);

	(void) memcpy(&dst->pr_clname[0], &src->pr_clname[0], PRCLSZ);
	(void) memcpy(&dst->pr_name[0], &src->pr_name[0], PRFNSZ);

	dst->pr_onpro = src->pr_onpro;
	dst->pr_bindpro = src->pr_bindpro;
	dst->pr_bindpset = src->pr_bindpset;
	dst->pr_lgrp = src->pr_lgrp;
}

void
psinfo_n_to_32(const psinfo_t *src, psinfo32_t *dst)
{
	dst->pr_flag = src->pr_flag;
	dst->pr_nlwp = src->pr_nlwp;
	dst->pr_nzomb = src->pr_nzomb;
	dst->pr_pid = (pid32_t)src->pr_pid;
	dst->pr_pgid = (pid32_t)src->pr_pgid;
	dst->pr_sid = (pid32_t)src->pr_sid;
	dst->pr_taskid = (id32_t)src->pr_taskid;
	dst->pr_projid = (id32_t)src->pr_projid;
	dst->pr_zoneid = (id32_t)src->pr_zoneid;
	dst->pr_uid = (uid32_t)src->pr_uid;
	dst->pr_euid = (uid32_t)src->pr_euid;
	dst->pr_gid = (gid32_t)src->pr_gid;
	dst->pr_egid = (gid32_t)src->pr_egid;
	dst->pr_addr = (caddr32_t)src->pr_addr;
	dst->pr_size = (size32_t)src->pr_size;
	dst->pr_rssize = (size32_t)src->pr_rssize;

	dst->pr_ttydev = prcmpldev(src->pr_ttydev);

	dst->pr_pctcpu = src->pr_pctcpu;
	dst->pr_pctmem = src->pr_pctmem;

	timestruc_n_to_32(&src->pr_start, &dst->pr_start);
	timestruc_n_to_32(&src->pr_time, &dst->pr_time);
	timestruc_n_to_32(&src->pr_ctime, &dst->pr_ctime);

	(void) memcpy(&dst->pr_fname[0], &src->pr_fname[0], PRFNSZ);
	(void) memcpy(&dst->pr_psargs[0], &src->pr_psargs[0], PRARGSZ);

	dst->pr_wstat = src->pr_wstat;
	dst->pr_argc = src->pr_argc;
	dst->pr_argv = (caddr32_t)src->pr_argv;
	dst->pr_envp = (caddr32_t)src->pr_envp;
	dst->pr_dmodel = src->pr_dmodel;

	lwpsinfo_n_to_32(&src->pr_lwp, &dst->pr_lwp);
}


#endif	/* _LP64 */
