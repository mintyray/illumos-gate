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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#include <sys/types.h>
#include <sys/ucontext.h>

#include <setjmp.h>
#include <string.h>
#include <ucontext.h>
#include <upanic.h>

extern void _siglongjmp(sigjmp_buf, int) __NORETURN;

#pragma weak _siglongjmp = siglongjmp
void
siglongjmp(sigjmp_buf env, int val)
{
	const char *msg = "siglongjmp(): setcontext() returned";
	ucontext_t *ucp = (ucontext_t *)env;

	if (val)
		ucp->uc_mcontext.gregs[REG_X0] = val;
	else
		ucp->uc_mcontext.gregs[REG_X0] = 1;

	/*
	 * While unlikely, it is possible that setcontext() may fail for some
	 * reason. If that happens, we will kill the process.
	 */
	(void) setcontext(ucp);
	upanic(msg, strlen(msg) + 1);
}
