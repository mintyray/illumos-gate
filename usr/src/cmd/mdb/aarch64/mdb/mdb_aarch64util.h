/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

/* Copyright 2022 Richard Lowe */

#ifndef _MDB_AARCH64UTIL_H
#define	_MDB_AARCH64UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mdb/mdb_kreg.h>

typedef uint32_t mdb_instr_t;

int mdb_aarch64_next(mdb_tgt_t *, uintptr_t *, kreg_t, mdb_instr_t);

#ifdef __cplusplus
}
#endif

#endif /* _MDB_AARCH64UTIL_H */
