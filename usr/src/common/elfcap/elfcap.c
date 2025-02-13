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
 * Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2019, Joyent, Inc.
 * Copyright 2022 Oxide Computer Company
 */

/* LINTLIBRARY */

/*
 * String conversion routine for hardware capabilities types.
 */
#include	<strings.h>
#include	<stdio.h>
#include	<ctype.h>
#include	<sys/machelf.h>
#include	<sys/elf.h>
#include	<sys/auxv_SPARC.h>
#include	<sys/auxv_386.h>
#include	<sys/auxv_aarch64.h>
#include	<elfcap.h>

/*
 * Given a literal string, generate an initialization for an
 * elfcap_str_t value.
 */
#define	STRDESC(_str) { _str, sizeof (_str) - 1 }

/*
 * The items in the elfcap_desc_t arrays are required to be
 * ordered so that the array index is related to the
 * c_val field as:
 *
 *	array[ndx].c_val = 2^ndx
 *
 * meaning that
 *
 *	array[0].c_val = 2^0 = 1
 *	array[1].c_val = 2^1 = 2
 *	array[2].c_val = 2^2 = 4
 *	.
 *	.
 *	.
 *
 * Since 0 is not a valid value for the c_val field, we use it to
 * mark an array entry that is a placeholder. This can happen if there
 * is a hole in the assigned bits.
 *
 * The RESERVED_ELFCAP_DESC macro is used to reserve such holes.
 */
#define	RESERVED_ELFCAP_DESC { 0, { NULL, 0 }, { NULL, 0 }, { NULL, 0 } }

/*
 * Define separators for output string processing. This must be kept in
 * sync with the elfcap_fmt_t values in elfcap.h. If something is added here
 * that is longer than ELFCAP_FMT_PIPSPACE, please update elfcap_chk.c.
 */
static const elfcap_str_t format[] = {
	STRDESC(" "),			/* ELFCAP_FMT_SNGSPACE */
	STRDESC("  "),			/* ELFCAP_FMT_DBLSPACE */
	STRDESC(" | ")			/* ELFCAP_FMT_PIPSPACE */
};
#define	FORMAT_NELTS	(sizeof (format) / sizeof (format[0]))



/*
 * Define all known software capabilities in all the supported styles.
 * Order the capabilities by their numeric value. See SF1_SUNW_
 * values in sys/elf.h.
 */
static const elfcap_desc_t sf1[ELFCAP_NUM_SF1] = {
	{						/* 0x00000001 */
		SF1_SUNW_FPKNWN, STRDESC("SF1_SUNW_FPKNWN"),
		STRDESC("FPKNWN"), STRDESC("fpknwn")
	},
	{						/* 0x00000002 */
		SF1_SUNW_FPUSED, STRDESC("SF1_SUNW_FPUSED"),
		STRDESC("FPUSED"), STRDESC("fpused"),
	},
	{						/* 0x00000004 */
		SF1_SUNW_ADDR32, STRDESC("SF1_SUNW_ADDR32"),
		STRDESC("ADDR32"), STRDESC("addr32"),
	}
};



/*
 * Order the SPARC hardware capabilities to match their numeric value.  See
 * AV_SPARC_ values in sys/auxv_SPARC.h.
 */
static const elfcap_desc_t hw1_sparc[ELFCAP_NUM_HW1_SPARC] = {
	{						/* 0x00000001 */
		AV_SPARC_MUL32, STRDESC("AV_SPARC_MUL32"),
		STRDESC("MUL32"), STRDESC("mul32"),
	},
	{						/* 0x00000002 */
		AV_SPARC_DIV32, STRDESC("AV_SPARC_DIV32"),
		STRDESC("DIV32"), STRDESC("div32"),
	},
	{						/* 0x00000004 */
		AV_SPARC_FSMULD, STRDESC("AV_SPARC_FSMULD"),
		STRDESC("FSMULD"), STRDESC("fsmuld"),
	},
	{						/* 0x00000008 */
		AV_SPARC_V8PLUS, STRDESC("AV_SPARC_V8PLUS"),
		STRDESC("V8PLUS"), STRDESC("v8plus"),
	},
	{						/* 0x00000010 */
		AV_SPARC_POPC, STRDESC("AV_SPARC_POPC"),
		STRDESC("POPC"), STRDESC("popc"),
	},
	{						/* 0x00000020 */
		AV_SPARC_VIS, STRDESC("AV_SPARC_VIS"),
		STRDESC("VIS"), STRDESC("vis"),
	},
	{						/* 0x00000040 */
		AV_SPARC_VIS2, STRDESC("AV_SPARC_VIS2"),
		STRDESC("VIS2"), STRDESC("vis2"),
	},
	{						/* 0x00000080 */
		AV_SPARC_ASI_BLK_INIT, STRDESC("AV_SPARC_ASI_BLK_INIT"),
		STRDESC("ASI_BLK_INIT"), STRDESC("asi_blk_init"),
	},
	{						/* 0x00000100 */
		AV_SPARC_FMAF, STRDESC("AV_SPARC_FMAF"),
		STRDESC("FMAF"), STRDESC("fmaf"),
	},
	RESERVED_ELFCAP_DESC,				/* 0x00000200 */
	{						/* 0x00000400 */
		AV_SPARC_VIS3, STRDESC("AV_SPARC_VIS3"),
		STRDESC("VIS3"), STRDESC("vis3"),
	},
	{						/* 0x00000800 */
		AV_SPARC_HPC, STRDESC("AV_SPARC_HPC"),
		STRDESC("HPC"), STRDESC("hpc"),
	},
	{						/* 0x00001000 */
		AV_SPARC_RANDOM, STRDESC("AV_SPARC_RANDOM"),
		STRDESC("RANDOM"), STRDESC("random"),
	},
	{						/* 0x00002000 */
		AV_SPARC_TRANS, STRDESC("AV_SPARC_TRANS"),
		STRDESC("TRANS"), STRDESC("trans"),
	},
	{						/* 0x00004000 */
		AV_SPARC_FJFMAU, STRDESC("AV_SPARC_FJFMAU"),
		STRDESC("FJFMAU"), STRDESC("fjfmau"),
	},
	{						/* 0x00008000 */
		AV_SPARC_IMA, STRDESC("AV_SPARC_IMA"),
		STRDESC("IMA"), STRDESC("ima"),
	},
	{						/* 0x00010000 */
		AV_SPARC_ASI_CACHE_SPARING,
		STRDESC("AV_SPARC_ASI_CACHE_SPARING"),
		STRDESC("CSPARE"), STRDESC("cspare"),
	},
	{						/* 0x00020000 */
		AV_SPARC_PAUSE,
		STRDESC("AV_SPARC_PAUSE"),
		STRDESC("PAUSE"), STRDESC("pause"),
	},
	{						/* 0x00040000 */
		AV_SPARC_CBCOND,
		STRDESC("AV_SPARC_CBCOND"),
		STRDESC("CBCOND"), STRDESC("cbcond"),
	},
	{						/* 0x00080000 */
		AV_SPARC_AES,
		STRDESC("AV_SPARC_AES"),
		STRDESC("AES"), STRDESC("aes"),
	},
	{						/* 0x00100000 */
		AV_SPARC_DES,
		STRDESC("AV_SPARC_DES"),
		STRDESC("DES"), STRDESC("des"),
	},
	{						/* 0x00200000 */
		AV_SPARC_KASUMI,
		STRDESC("AV_SPARC_KASUMI"),
		STRDESC("KASUMI"), STRDESC("kasumi"),
	},
	{						/* 0x00400000 */
		AV_SPARC_CAMELLIA,
		STRDESC("AV_SPARC_CAMELLIA"),
		STRDESC("CAMELLIA"), STRDESC("camellia"),
	},
	{						/* 0x00800000 */
		AV_SPARC_MD5,
		STRDESC("AV_SPARC_MD5"),
		STRDESC("MD5"), STRDESC("md5"),
	},
	{						/* 0x01000000 */
		AV_SPARC_SHA1,
		STRDESC("AV_SPARC_SHA1"),
		STRDESC("SHA1"), STRDESC("sha1"),
	},
	{						/* 0x02000000 */
		AV_SPARC_SHA256,
		STRDESC("AV_SPARC_SHA256"),
		STRDESC("SHA256"), STRDESC("sha256"),
	},
	{						/* 0x04000000 */
		AV_SPARC_SHA512,
		STRDESC("AV_SPARC_SHA512"),
		STRDESC("SHA512"), STRDESC("sha512"),
	},
	{						/* 0x08000000 */
		AV_SPARC_MPMUL,
		STRDESC("AV_SPARC_MPMUL"),
		STRDESC("MPMUL"), STRDESC("mpmul"),
	},
	{						/* 0x10000000 */
		AV_SPARC_MONT,
		STRDESC("AV_SPARC_MONT"),
		STRDESC("MONT"), STRDESC("mont"),
	},
	{						/* 0x20000000 */
		AV_SPARC_CRC32C,
		STRDESC("AV_SPARC_CRC32C"),
		STRDESC("CRC32C"), STRDESC("crc32c"),
	}
};



/*
 * Order the Intel hardware capabilities to match their numeric value.  See
 * AV_386_ values in sys/auxv_386.h.
 */
static const elfcap_desc_t hw1_386[ELFCAP_NUM_HW1_386] = {
	{						/* 0x00000001 */
		AV_386_FPU, STRDESC("AV_386_FPU"),
		STRDESC("FPU"), STRDESC("fpu"),
	},
	{						/* 0x00000002 */
		AV_386_TSC, STRDESC("AV_386_TSC"),
		STRDESC("TSC"), STRDESC("tsc"),
	},
	{						/* 0x00000004 */
		AV_386_CX8, STRDESC("AV_386_CX8"),
		STRDESC("CX8"), STRDESC("cx8"),
	},
	{						/* 0x00000008 */
		AV_386_SEP, STRDESC("AV_386_SEP"),
		STRDESC("SEP"), STRDESC("sep"),
	},
	{						/* 0x00000010 */
		AV_386_AMD_SYSC, STRDESC("AV_386_AMD_SYSC"),
		STRDESC("AMD_SYSC"), STRDESC("amd_sysc"),
	},
	{						/* 0x00000020 */
		AV_386_CMOV, STRDESC("AV_386_CMOV"),
		STRDESC("CMOV"), STRDESC("cmov"),
	},
	{						/* 0x00000040 */
		AV_386_MMX, STRDESC("AV_386_MMX"),
		STRDESC("MMX"), STRDESC("mmx"),
	},
	{						/* 0x00000080 */
		AV_386_AMD_MMX, STRDESC("AV_386_AMD_MMX"),
		STRDESC("AMD_MMX"), STRDESC("amd_mmx"),
	},
	{						/* 0x00000100 */
		AV_386_AMD_3DNow, STRDESC("AV_386_AMD_3DNow"),
		STRDESC("AMD_3DNow"), STRDESC("amd_3dnow"),
	},
	{						/* 0x00000200 */
		AV_386_AMD_3DNowx, STRDESC("AV_386_AMD_3DNowx"),
		STRDESC("AMD_3DNowx"), STRDESC("amd_3dnowx"),
	},
	{						/* 0x00000400 */
		AV_386_FXSR, STRDESC("AV_386_FXSR"),
		STRDESC("FXSR"), STRDESC("fxsr"),
	},
	{						/* 0x00000800 */
		AV_386_SSE, STRDESC("AV_386_SSE"),
		STRDESC("SSE"), STRDESC("sse"),
	},
	{						/* 0x00001000 */
		AV_386_SSE2, STRDESC("AV_386_SSE2"),
		STRDESC("SSE2"), STRDESC("sse2"),
	},
	/* 0x02000 withdrawn - do not assign */
	{						/* 0x00004000 */
		AV_386_SSE3, STRDESC("AV_386_SSE3"),
		STRDESC("SSE3"), STRDESC("sse3"),
	},
	/* 0x08000 withdrawn - do not assign */
	{						/* 0x00010000 */
		AV_386_CX16, STRDESC("AV_386_CX16"),
		STRDESC("CX16"), STRDESC("cx16"),
	},
	{						/* 0x00020000 */
		AV_386_AHF, STRDESC("AV_386_AHF"),
		STRDESC("AHF"), STRDESC("ahf"),
	},
	{						/* 0x00040000 */
		AV_386_TSCP, STRDESC("AV_386_TSCP"),
		STRDESC("TSCP"), STRDESC("tscp"),
	},
	{						/* 0x00080000 */
		AV_386_AMD_SSE4A, STRDESC("AV_386_AMD_SSE4A"),
		STRDESC("AMD_SSE4A"), STRDESC("amd_sse4a"),
	},
	{						/* 0x00100000 */
		AV_386_POPCNT, STRDESC("AV_386_POPCNT"),
		STRDESC("POPCNT"), STRDESC("popcnt"),
	},
	{						/* 0x00200000 */
		AV_386_AMD_LZCNT, STRDESC("AV_386_AMD_LZCNT"),
		STRDESC("AMD_LZCNT"), STRDESC("amd_lzcnt"),
	},
	{						/* 0x00400000 */
		AV_386_SSSE3, STRDESC("AV_386_SSSE3"),
		STRDESC("SSSE3"), STRDESC("ssse3"),
	},
	{						/* 0x00800000 */
		AV_386_SSE4_1, STRDESC("AV_386_SSE4_1"),
		STRDESC("SSE4.1"), STRDESC("sse4.1"),
	},
	{						/* 0x01000000 */
		AV_386_SSE4_2, STRDESC("AV_386_SSE4_2"),
		STRDESC("SSE4.2"), STRDESC("sse4.2"),
	},
	{						/* 0x02000000 */
		AV_386_MOVBE, STRDESC("AV_386_MOVBE"),
		STRDESC("MOVBE"), STRDESC("movbe"),
	},
	{						/* 0x04000000 */
		AV_386_AES, STRDESC("AV_386_AES"),
		STRDESC("AES"), STRDESC("aes"),
	},
	{						/* 0x08000000 */
		AV_386_PCLMULQDQ, STRDESC("AV_386_PCLMULQDQ"),
		STRDESC("PCLMULQDQ"), STRDESC("pclmulqdq"),
	},
	{						/* 0x10000000 */
		AV_386_XSAVE, STRDESC("AV_386_XSAVE"),
		STRDESC("XSAVE"), STRDESC("xsave"),
	},
	{						/* 0x20000000 */
		AV_386_AVX, STRDESC("AV_386_AVX"),
		STRDESC("AVX"), STRDESC("avx"),
	},
	{						/* 0x40000000 */
		AV_386_VMX, STRDESC("AV_386_VMX"),
		STRDESC("VMX"), STRDESC("vmx"),
	},
	{						/* 0x80000000 */
		AV_386_AMD_SVM, STRDESC("AV_386_AMD_SVM"),
		STRDESC("AMD_SVM"), STRDESC("amd_svm"),
	}
};

static const elfcap_desc_t hw2_386[ELFCAP_NUM_HW2_386] = {
	{						/* 0x00000001 */
		AV_386_2_F16C, STRDESC("AV_386_2_F16C"),
		STRDESC("F16C"), STRDESC("f16c"),
	},
	{						/* 0x00000002 */
		AV_386_2_RDRAND, STRDESC("AV_386_2_RDRAND"),
		STRDESC("RDRAND"), STRDESC("rdrand"),
	},
	{						/* 0x00000004 */
		AV_386_2_BMI1, STRDESC("AV_386_2_BMI1"),
		STRDESC("BMI1"), STRDESC("bmi1"),
	},
	{						/* 0x00000008 */
		AV_386_2_BMI2, STRDESC("AV_386_2_BMI2"),
		STRDESC("BMI2"), STRDESC("bmi2"),
	},
	{						/* 0x00000010 */
		AV_386_2_FMA, STRDESC("AV_386_2_FMA"),
		STRDESC("FMA"), STRDESC("fma"),
	},
	{						/* 0x00000020 */
		AV_386_2_AVX2, STRDESC("AV_386_2_AVX2"),
		STRDESC("AVX2"), STRDESC("avx2"),
	},
	{						/* 0x00000040 */
		AV_386_2_ADX, STRDESC("AV_386_2_ADX"),
		STRDESC("ADX"), STRDESC("adx"),
	},
	{						/* 0x00000080 */
		AV_386_2_RDSEED, STRDESC("AV_386_2_RDSEED"),
		STRDESC("RDSEED"), STRDESC("rdseed"),
	},
	{						/* 0x00000100 */
		AV_386_2_AVX512F, STRDESC("AV_386_2_AVX512F"),
		STRDESC("AVX512F"), STRDESC("avx512f"),
	},
	{						/* 0x00000200 */
		AV_386_2_AVX512DQ, STRDESC("AV_386_2_AVX512DQ"),
		STRDESC("AVX512DQ"), STRDESC("avx512dq"),
	},
	{						/* 0x00000400 */
		AV_386_2_AVX512IFMA, STRDESC("AV_386_2_AVX512IFMA"),
		STRDESC("AVX512IFMA"), STRDESC("avx512ifma"),
	},
	{						/* 0x00000800 */
		AV_386_2_AVX512PF, STRDESC("AV_386_2_AVX512PF"),
		STRDESC("AVX512PF"), STRDESC("avx512pf"),
	},
	{						/* 0x00001000 */
		AV_386_2_AVX512ER, STRDESC("AV_386_2_AVX512ER"),
		STRDESC("AVX512ER"), STRDESC("avx512er"),
	},
	{						/* 0x00002000 */
		AV_386_2_AVX512CD, STRDESC("AV_386_2_AVX512CD"),
		STRDESC("AVX512CD"), STRDESC("avx512cd"),
	},
	{						/* 0x00004000 */
		AV_386_2_AVX512BW, STRDESC("AV_386_2_AVX512BW"),
		STRDESC("AVX512BW"), STRDESC("avx512bw"),
	},
	{						/* 0x00008000 */
		AV_386_2_AVX512VL, STRDESC("AV_386_2_AVX512VL"),
		STRDESC("AVX512VL"), STRDESC("avx512vl"),
	},
	{						/* 0x00010000 */
		AV_386_2_AVX512VBMI, STRDESC("AV_386_2_AVX512VBMI"),
		STRDESC("AVX512VBMI"), STRDESC("avx512vbmi"),
	},
	{						/* 0x00020000 */
		AV_386_2_AVX512VPOPCDQ, STRDESC("AV_386_2_AVX512_VPOPCDQ"),
		STRDESC("AVX512_VPOPCDQ"), STRDESC("avx512_vpopcntdq"),
	},
	{						/* 0x00040000 */
		AV_386_2_AVX512_4NNIW, STRDESC("AV_386_2_AVX512_4NNIW"),
		STRDESC("AVX512_4NNIW"), STRDESC("avx512_4nniw"),
	},
	{						/* 0x00080000 */
		AV_386_2_AVX512_4FMAPS, STRDESC("AV_386_2_AVX512_4FMAPS"),
		STRDESC("AVX512_4FMAPS"), STRDESC("avx512_4fmaps"),
	},
	{						/* 0x00100000 */
		AV_386_2_SHA, STRDESC("AV_386_2_SHA"),
		STRDESC("SHA"), STRDESC("sha"),
	},
	{						/* 0x00200000 */
		AV_386_2_FSGSBASE, STRDESC("AV_386_2_FSGSBASE"),
		STRDESC("FSGSBASE"), STRDESC("fsgsbase")
	},
	{						/* 0x00400000 */
		AV_386_2_CLFLUSHOPT, STRDESC("AV_386_2_CLFLUSHOPT"),
		STRDESC("CLFLUSHOPT"), STRDESC("clflushopt")
	},
	{						/* 0x00800000 */
		AV_386_2_CLWB, STRDESC("AV_386_2_CLWB"),
		STRDESC("CLWB"), STRDESC("clwb")
	},
	{						/* 0x01000000 */
		AV_386_2_MONITORX, STRDESC("AV_386_2_MONITORX"),
		STRDESC("MONITORX"), STRDESC("monitorx")
	},
	{						/* 0x02000000 */
		AV_386_2_CLZERO, STRDESC("AV_386_2_CLZERO"),
		STRDESC("CLZERO"), STRDESC("clzero")
	},
	{						/* 0x04000000 */
		AV_386_2_AVX512_VNNI, STRDESC("AV_386_2_AVX512_VNNI"),
		STRDESC("AVX512_VNNI"), STRDESC("avx512_vnni")
	},
	{						/* 0x08000000 */
		AV_386_2_VPCLMULQDQ, STRDESC("AV_386_2_VPCLMULQDQ"),
		STRDESC("VPCLMULQDQ"), STRDESC("vpclmulqdq")
	},
	{						/* 0x10000000 */
		AV_386_2_VAES, STRDESC("AV_386_2_VAES"),
		STRDESC("VAES"), STRDESC("vaes")
	},
	{						/* 0x20000000 */
		AV_386_2_GFNI, STRDESC("AV_386_2_GFNI"),
		STRDESC("GFNI"), STRDESC("gfni")
	},
	{						/* 0x40000000 */
		AV_386_2_AVX512_VP2INT, STRDESC("AV_386_2_AVX512_VP2INT"),
		STRDESC("AVX512_VP2INT"), STRDESC("avx512_vp2int")
	},
	{						/* 0x80000000 */
		AV_386_2_AVX512_BITALG, STRDESC("AV_386_2_AVX512_BITALG"),
		STRDESC("AVX512_BITALG"), STRDESC("avx512_bitalg")
	}
};

static const elfcap_desc_t hw3_386[ELFCAP_NUM_HW3_386] = {
	{						/* 0x00000001 */
		AV_386_3_AVX512_VBMI2, STRDESC("AV_386_3_AVX512_VBMI2"),
		STRDESC("AVX512_VBMI2"), STRDESC("avx512_vbmi2")
	},
	{						/* 0x00000002 */
		AV_386_3_AVX512_BF16, STRDESC("AV_386_3_AVX512_BF16"),
		STRDESC("AVX512_BF16"), STRDESC("avx512_bf16")
	}
};

/*
 * Order the AArch64 hardware capabilities to match their numeric value.  See
 * AV_AARCH64_ values in sys/auxv_aarch64.h.
 */
static const elfcap_desc_t hw1_aarch64[ELFCAP_NUM_HW1_AARCH64] = {
	{						/* 0x00000001 */
		AV_AARCH64_FP, STRDESC("AV_AARCH64_FP"),
		STRDESC("FP"), STRDESC("fp"),
	},
	{						/* 0x00000002 */
		AV_AARCH64_ADVSIMD, STRDESC("AV_AARCH64_ADVSIMD"),
		STRDESC("ADVSIMD"), STRDESC("advsimd"),
	},
	{						/* 0x00000004 */
		AV_AARCH64_SVE, STRDESC("AV_AARCH64_SVE"),
		STRDESC("SVE"), STRDESC("sve"),
	},
	{						/* 0x00000008 */
		AV_AARCH64_CRC32, STRDESC("AV_AARCH64_CRC32"),
		STRDESC("CRC32"), STRDESC("crc32"),
	},
	{						/* 0x00000010 */
		AV_AARCH64_SB, STRDESC("AV_AARCH64_SB"),
		STRDESC("SB"), STRDESC("sb"),
	},
	{						/* 0x00000020 */
		AV_AARCH64_SSBS, STRDESC("AV_AARCH64_SSBS"),
		STRDESC("SSBS"), STRDESC("ssbs"),
	},
	{						/* 0x00000040 */
		AV_AARCH64_DGH, STRDESC("AV_AARCH64_DGH"),
		STRDESC("DGH"), STRDESC("dgh"),
	},
	{						/* 0x00000080 */
		AV_AARCH64_AES, STRDESC("AV_AARCH64_AES"),
		STRDESC("AES"), STRDESC("aes"),
	},
	{						/* 0x00000100 */
		AV_AARCH64_PMULL, STRDESC("AV_AARCH64_PMULL"),
		STRDESC("PMULL"), STRDESC("pmull"),
	},
	{						/* 0x00000200 */
		AV_AARCH64_SHA1, STRDESC("AV_AARCH64_SHA1"),
		STRDESC("SHA1"), STRDESC("sha1"),
	},
	{						/* 0x00000400 */
		AV_AARCH64_SHA256, STRDESC("AV_AARCH64_SHA256"),
		STRDESC("SHA256"), STRDESC("sha256"),
	},
	{						/* 0x00000800 */
		AV_AARCH64_SHA512, STRDESC("AV_AARCH64_SHA512"),
		STRDESC("SHA512"), STRDESC("sha512"),
	},
	{						/* 0x00001000 */
		AV_AARCH64_SHA3, STRDESC("AV_AARCH64_SHA3"),
		STRDESC("SHA3"), STRDESC("sha3"),
	},
	{						/* 0x00002000 */
		AV_AARCH64_SM3, STRDESC("AV_AARCH64_SM3"),
		STRDESC("SM3"), STRDESC("sm3"),
	},
	{						/* 0x00004000 */
		AV_AARCH64_SM4, STRDESC("AV_AARCH64_SM4"),
		STRDESC("SM4"), STRDESC("sm4"),
	},
	{						/* 0x00008000 */
		AV_AARCH64_LSE, STRDESC("AV_AARCH64_LSE"),
		STRDESC("LSE"), STRDESC("lse"),
	},
	{						/* 0x00010000 */
		AV_AARCH64_RDM, STRDESC("AV_AARCH64_RDM"),
		STRDESC("RDM"), STRDESC("rdm"),
	},
	{						/* 0x00020000 */
		AV_AARCH64_FP16, STRDESC("AV_AARCH64_FP16"),
		STRDESC("FP16"), STRDESC("fp16"),
	},
	{						/* 0x00040000 */
		AV_AARCH64_DOTPROD, STRDESC("AV_AARCH64_DOTPROD"),
		STRDESC("DOTPROD"), STRDESC("dotprod"),
	},
	{						/* 0x00080000 */
		AV_AARCH64_FHM, STRDESC("AV_AARCH64_FHM"),
		STRDESC("FHM"), STRDESC("fhm"),
	},
	{						/* 0x00100000 */
		AV_AARCH64_DCPOP, STRDESC("AV_AARCH64_DCPOP"),
		STRDESC("DCPOP"), STRDESC("dcpop"),
	},
	{						/* 0x00200000 */
		AV_AARCH64_F32MM, STRDESC("AV_AARCH64_F32MM"),
		STRDESC("F32MM"), STRDESC("f32mm"),
	},
	{						/* 0x00400000 */
		AV_AARCH64_F64MM, STRDESC("AV_AARCH64_F64MM"),
		STRDESC("F64MM"), STRDESC("f64mm"),
	},
	{						/* 0x00800000 */
		AV_AARCH64_DCPODP, STRDESC("AV_AARCH64_DCPODP"),
		STRDESC("DCPODP"), STRDESC("dcpodp"),
	},
	{						/* 0x01000000 */
		AV_AARCH64_BF16, STRDESC("AV_AARCH64_BF16"),
		STRDESC("BF16"), STRDESC("bf16"),
	},
	{						/* 0x02000000 */
		AV_AARCH64_I8MM, STRDESC("AV_AARCH64_I8MM"),
		STRDESC("I8MM"), STRDESC("i8mm"),
	},
	{						/* 0x04000000 */
		AV_AARCH64_FCMA, STRDESC("AV_AARCH64_FCMA"),
		STRDESC("FCMA"), STRDESC("fcma"),
	},
	{						/* 0x08000000 */
		AV_AARCH64_JSCVT, STRDESC("AV_AARCH64_JSCVT"),
		STRDESC("JSCVT"), STRDESC("jscvt"),
	},
	{						/* 0x10000000 */
		AV_AARCH64_LRCPC, STRDESC("AV_AARCH64_LRCPC"),
		STRDESC("LRCPC"), STRDESC("lrcpc"),
	},
	{						/* 0x20000000 */
		AV_AARCH64_PACA, STRDESC("AV_AARCH64_PACA"),
		STRDESC("paca"), STRDESC("paca"),
	},
	{						/* 0x40000000 */
		AV_AARCH64_PACG, STRDESC("AV_AARCH64_PACG"),
		STRDESC("pacg"), STRDESC("pacg"),
	},
	{						/* 0x80000000 */
		AV_AARCH64_DIT, STRDESC("AV_AARCH64_DIT"),
		STRDESC("dit"), STRDESC("dit"),
	},
};

static const elfcap_desc_t hw2_aarch64[ELFCAP_NUM_HW2_AARCH64] = {
	{						/* 0x00000001 */
		AV_AARCH64_2_FLAGM, STRDESC("AV_AARCH64_2_FLAGM"),
		STRDESC("FLAGM"), STRDESC("flagm"),
	},
	{						/* 0x00000002 */
		AV_AARCH64_2_FRINTTS, STRDESC("AV_AARCH64_2_FRINTTS"),
		STRDESC("FRINTTS"), STRDESC("frintts"),
	},
	{						/* 0x00000004 */
		AV_AARCH64_2_BTI, STRDESC("AV_AARCH64_2_BTI"),
		STRDESC("BTI"), STRDESC("bti"),
	},
	{						/* 0x00000008 */
		AV_AARCH64_2_RNG, STRDESC("AV_AARCH64_2_RNG"),
		STRDESC("RNG"), STRDESC("rng"),
	},
	{						/* 0x00000010 */
		AV_AARCH64_2_MTE, STRDESC("AV_AARCH64_2_MTE"),
		STRDESC("MTE"), STRDESC("mte"),
	},
	{						/* 0x00000020 */
		AV_AARCH64_2_MTE3, STRDESC("AV_AARCH64_2_MTE3"),
		STRDESC("MTE3"), STRDESC("mte3"),
	},
	{						/* 0x00000040 */
		AV_AARCH64_2_ECV, STRDESC("AV_AARCH64_2_ECV"),
		STRDESC("ECV"), STRDESC("ecv"),
	},
	{						/* 0x00000080 */
		AV_AARCH64_2_AFP, STRDESC("AV_AARCH64_2_AFP"),
		STRDESC("AFP"), STRDESC("afp"),
	},
	{						/* 0x00000100 */
		AV_AARCH64_2_RPRES, STRDESC("AV_AARCH64_2_RPRES"),
		STRDESC("RPRES"), STRDESC("rpres"),
	},
	{						/* 0x00000200 */
		AV_AARCH64_2_RPRES, STRDESC("AV_AARCH64_2_RPRES"),
		STRDESC("RPRES"), STRDESC("rpres"),
	},
	{						/* 0x00000400 */
		AV_AARCH64_2_LD64B, STRDESC("AV_AARCH64_2_LD64B"),
		STRDESC("LD64B"), STRDESC("ld64b"),
	},
	{						/* 0x00000800 */
		AV_AARCH64_2_ST64BV, STRDESC("AV_AARCH64_2_ST64BV"),
		STRDESC("ST64BV"), STRDESC("st64bv"),
	},
	{						/* 0x00001000 */
		AV_AARCH64_2_ST64BV0, STRDESC("AV_AARCH64_2_ST64BV0"),
		STRDESC("ST64BV0"), STRDESC("st64bv0"),
	},
	{						/* 0x00002000 */
		AV_AARCH64_2_WFXT, STRDESC("AV_AARCH64_2_WFXT"),
		STRDESC("WFXT"), STRDESC("wxft"),
	},
	{						/* 0x00004000 */
		AV_AARCH64_2_MOPS, STRDESC("AV_AARCH64_2_MOPS"),
		STRDESC("MOPS"), STRDESC("mops"),
	},
	{						/* 0x00008000 */
		AV_AARCH64_2_HBC, STRDESC("AV_AARCH64_2_HBC"),
		STRDESC("HBC"), STRDESC("hbc"),
	},
	{						/* 0x00010000 */
		AV_AARCH64_2_CMOW, STRDESC("AV_AARCH64_2_CMOW"),
		STRDESC("CMOW"), STRDESC("cmow"),
	},
	{						/* 0x00020000 */
		AV_AARCH64_2_SVE2, STRDESC("AV_AARCH64_2_SVE2"),
		STRDESC("SVE2"), STRDESC("sve2"),
	},
	{						/* 0x00040000 */
		AV_AARCH64_2_SVE2_AES, STRDESC("AV_AARCH64_2_SVE2_AES"),
		STRDESC("SVE2_AES"), STRDESC("sve2_aes"),
	},
	{						/* 0x00080000 */
		AV_AARCH64_2_SVE2_BITPERM, STRDESC("AV_AARCH64_2_SVE2_BITPERM"),
		STRDESC("SVE2_BITPERM"), STRDESC("sve2_bitperm"),
	},
	{						/* 0x00100000 */
		AV_AARCH64_2_SVE2_PMULL128,
		STRDESC("AV_AARCH64_2_SVE2_PMULL128"),
		STRDESC("SVE2_PMULL128"), STRDESC("sve2_pmull128"),
	},
	{						/* 0x00200000 */
		AV_AARCH64_2_SVE2_SHA3, STRDESC("AV_AARCH64_2_SVE2_SHA3"),
		STRDESC("SVE2_SHA3"), STRDESC("sve2_sha3"),
	},
	{						/* 0x00400000 */
		AV_AARCH64_2_SVE2_SM4, STRDESC("AV_AARCH64_2_SVE2_SM4"),
		STRDESC("SVE2_SM4"), STRDESC("sve2_sm4"),
	},
	{						/* 0x00800000 */
		AV_AARCH64_2_TME, STRDESC("AV_AARCH64_2_TME"),
		STRDESC("SVE2_SM4"), STRDESC("sve2_sm4"),
	},
	{						/* 0x01000000 */
		AV_AARCH64_2_SME, STRDESC("AV_AARCH64_2_SME"),
		STRDESC("SME"), STRDESC("sme"),
	},
	{						/* 0x02000000 */
		AV_AARCH64_2_SME_FA64, STRDESC("AV_AARCH64_2_SME_FA64"),
		STRDESC("SME_FA64"), STRDESC("sme_fa64"),
	},
	{						/* 0x04000000 */
		AV_AARCH64_2_EBF16, STRDESC("AV_AARCH64_2_EBF16"),
		STRDESC("EBF16"), STRDESC("ebf16"),
	},
	{						/* 0x08000000 */
		AV_AARCH64_2_SME_F64F64, STRDESC("AV_AARCH64_2_SME_F64F64"),
		STRDESC("SME_F64F64"), STRDESC("sme_f64f64"),
	},
	{						/* 0x08000000 */
		AV_AARCH64_2_SME_I16I64, STRDESC("AV_AARCH64_2_SME_I16I64"),
		STRDESC("SME_i16i64"), STRDESC("sme_i16i64"),
	},
};


/*
 * Concatenate a token to the string buffer.  This can be a capabilities token
 * or a separator token.
 */
static elfcap_err_t
token(char **ostr, size_t *olen, const elfcap_str_t *nstr)
{
	if (*olen < nstr->s_len)
		return (ELFCAP_ERR_BUFOVFL);

	(void) strcat(*ostr, nstr->s_str);
	*ostr += nstr->s_len;
	*olen -= nstr->s_len;

	return (ELFCAP_ERR_NONE);
}

static elfcap_err_t
get_str_desc(elfcap_style_t style, const elfcap_desc_t *cdp,
    const elfcap_str_t **ret_str)
{
	switch (ELFCAP_STYLE_MASK(style)) {
	case ELFCAP_STYLE_FULL:
		*ret_str = &cdp->c_full;
		break;
	case ELFCAP_STYLE_UC:
		*ret_str = &cdp->c_uc;
		break;
	case ELFCAP_STYLE_LC:
		*ret_str = &cdp->c_lc;
		break;
	default:
		return (ELFCAP_ERR_INVSTYLE);
	}

	return (ELFCAP_ERR_NONE);
}


/*
 * Expand a capabilities value into the strings defined in the associated
 * capabilities descriptor.
 */
static elfcap_err_t
expand(elfcap_style_t style, elfcap_mask_t val, const elfcap_desc_t *cdp,
    uint_t cnum, char *str, size_t slen, elfcap_fmt_t fmt)
{
	uint_t			cnt;
	int			follow = 0, err;
	const elfcap_str_t	*nstr;

	if (val == 0)
		return (ELFCAP_ERR_NONE);

	for (cnt = cnum; cnt > 0; cnt--) {
		uint_t mask = cdp[cnt - 1].c_val;

		if ((val & mask) != 0) {
			if (follow++ && ((err = token(&str, &slen,
			    &format[fmt])) != ELFCAP_ERR_NONE))
				return (err);

			err = get_str_desc(style, &cdp[cnt - 1], &nstr);
			if (err != ELFCAP_ERR_NONE)
				return (err);
			if ((err = token(&str, &slen, nstr)) != ELFCAP_ERR_NONE)
				return (err);

			val = val & ~mask;
		}
	}

	/*
	 * If there are any unknown bits remaining display the numeric value.
	 */
	if (val) {
		if (follow && ((err = token(&str, &slen, &format[fmt])) !=
		    ELFCAP_ERR_NONE))
			return (err);

		(void) snprintf(str, slen, "0x%x", val);
	}
	return (ELFCAP_ERR_NONE);
}

/*
 * Expand a CA_SUNW_HW_1 value.
 */
elfcap_err_t
elfcap_hw1_to_str(elfcap_style_t style, elfcap_mask_t val, char *str,
    size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	/*
	 * Initialize the string buffer, and validate the format request.
	 */
	*str = '\0';
	if ((fmt < 0) || (fmt >= FORMAT_NELTS))
		return (ELFCAP_ERR_INVFMT);

	if ((mach == EM_386) || (mach == EM_AMD64))
		return (expand(style, val, hw1_386, ELFCAP_NUM_HW1_386,
		    str, len, fmt));

	if (mach == EM_AARCH64)
		return (expand(style, val, hw1_aarch64, ELFCAP_NUM_HW1_AARCH64,
		    str, len, fmt));

	if ((mach == EM_SPARC) || (mach == EM_SPARC32PLUS) ||
	    (mach == EM_SPARCV9))
		return (expand(style, val, hw1_sparc, ELFCAP_NUM_HW1_SPARC,
		    str, len, fmt));

	return (ELFCAP_ERR_UNKMACH);
}

/*
 * Expand a CA_SUNW_HW_2 value.
 */
elfcap_err_t
elfcap_hw2_to_str(elfcap_style_t style, elfcap_mask_t val, char *str,
    size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	/*
	 * Initialize the string buffer, and validate the format request.
	 */
	*str = '\0';
	if ((fmt < 0) || (fmt >= FORMAT_NELTS))
		return (ELFCAP_ERR_INVFMT);

	if ((mach == EM_386) || (mach == EM_AMD64))
		return (expand(style, val, hw2_386, ELFCAP_NUM_HW2_386,
		    str, len, fmt));

	if (mach == EM_AARCH64)
		return (expand(style, val, hw2_aarch64, ELFCAP_NUM_HW2_AARCH64,
		    str, len, fmt));

	return (expand(style, val, NULL, 0, str, len, fmt));
}

/*
 * Expand a CA_SUNW_HW_3 value.
 */
elfcap_err_t
elfcap_hw3_to_str(elfcap_style_t style, elfcap_mask_t val, char *str,
    size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	/*
	 * Initialize the string buffer, and validate the format request.
	 */
	*str = '\0';
	if ((fmt < 0) || (fmt >= FORMAT_NELTS))
		return (ELFCAP_ERR_INVFMT);

	if ((mach == EM_386) || (mach == EM_IA_64) || (mach == EM_AMD64))
		return (expand(style, val, &hw3_386[0], ELFCAP_NUM_HW3_386,
		    str, len, fmt));

	return (expand(style, val, NULL, 0, str, len, fmt));
}

/*
 * Expand a CA_SUNW_SF_1 value.  Note, that at present these capabilities are
 * common across all platforms.  The use of "mach" is therefore redundant, but
 * is retained for compatibility with the interface of elfcap_hw1_to_str(), and
 * possible future expansion.
 */
elfcap_err_t
/* ARGSUSED4 */
elfcap_sf1_to_str(elfcap_style_t style, elfcap_mask_t val, char *str,
    size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	/*
	 * Initialize the string buffer, and validate the format request.
	 */
	*str = '\0';
	if ((fmt < 0) || (fmt >= FORMAT_NELTS))
		return (ELFCAP_ERR_INVFMT);

	return (expand(style, val, &sf1[0], ELFCAP_NUM_SF1, str, len, fmt));
}

/*
 * Given a capability tag type and value, map it to a string representation.
 */
elfcap_err_t
elfcap_tag_to_str(elfcap_style_t style, uint64_t tag, elfcap_mask_t val,
    char *str, size_t len, elfcap_fmt_t fmt, ushort_t mach)
{
	switch (tag) {
	case CA_SUNW_HW_1:
		return (elfcap_hw1_to_str(style, val, str, len, fmt, mach));

	case CA_SUNW_SF_1:
		return (elfcap_sf1_to_str(style, val, str, len, fmt, mach));

	case CA_SUNW_HW_2:
		return (elfcap_hw2_to_str(style, val, str, len, fmt, mach));

	case CA_SUNW_HW_3:
		return (elfcap_hw3_to_str(style, val, str, len, fmt, mach));

	}

	return (ELFCAP_ERR_UNKTAG);
}

/*
 * Determine a capabilities value from a capabilities string.
 */
static elfcap_mask_t
value(elfcap_style_t style, const char *str, const elfcap_desc_t *cdp,
    uint_t cnum)
{
	const elfcap_str_t	*nstr;
	uint_t	num;
	int	err;

	for (num = 0; num < cnum; num++) {
		/*
		 * Skip "reserved" bits. These are unassigned bits in the
		 * middle of the assigned range.
		 */
		if (cdp[num].c_val == 0)
			continue;

		if ((err = get_str_desc(style, &cdp[num], &nstr)) != 0)
			return (err);
		if (style & ELFCAP_STYLE_F_ICMP) {
			if (strcasecmp(str, nstr->s_str) == 0)
				return (cdp[num].c_val);
		} else {
			if (strcmp(str, nstr->s_str) == 0)
				return (cdp[num].c_val);
		}
	}

	return (0);
}

elfcap_mask_t
elfcap_sf1_from_str(elfcap_style_t style, const char *str, ushort_t mach)
{
	return (value(style, str, &sf1[0], ELFCAP_NUM_SF1));
}

elfcap_mask_t
elfcap_hw1_from_str(elfcap_style_t style, const char *str, ushort_t mach)
{
	if ((mach == EM_386) || (mach == EM_AMD64))
		return (value(style, str, hw1_386, ELFCAP_NUM_HW1_386));

	if (mach == EM_AARCH64)
		return (value(style, str, hw1_aarch64, ELFCAP_NUM_HW1_AARCH64));

	if ((mach == EM_SPARC) || (mach == EM_SPARC32PLUS) ||
	    (mach == EM_SPARCV9))
		return (value(style, str, hw1_sparc, ELFCAP_NUM_HW1_SPARC));

	return (0);
}

elfcap_mask_t
elfcap_hw2_from_str(elfcap_style_t style, const char *str, ushort_t mach)
{
	if ((mach == EM_386) || (mach == EM_IA_64) || (mach == EM_AMD64))
		return (value(style, str, &hw2_386[0], ELFCAP_NUM_HW2_386));

	return (0);
}

elfcap_mask_t
elfcap_hw3_from_str(elfcap_style_t style, const char *str, ushort_t mach)
{
	if ((mach == EM_386) || (mach == EM_IA_64) || (mach == EM_AMD64))
		return (value(style, str, &hw3_386[0], ELFCAP_NUM_HW3_386));

	return (0);
}


/*
 * Given a capability tag type and value, return the capabilities values
 * contained in the string.
 */
elfcap_mask_t
elfcap_tag_from_str(elfcap_style_t style, uint64_t tag, const char *str,
    ushort_t mach)
{
	switch (tag) {
	case CA_SUNW_HW_1:
		return (elfcap_hw1_from_str(style, str, mach));

	case CA_SUNW_SF_1:
		return (elfcap_sf1_from_str(style, str, mach));

	case CA_SUNW_HW_2:
		return (elfcap_hw2_from_str(style, str, mach));

	case CA_SUNW_HW_3:
		return (elfcap_hw3_from_str(style, str, mach));
	}

	return (0);
}

/*
 * These functions allow the caller to get direct access to the
 * cap descriptors.
 */
const elfcap_str_t *
elfcap_getdesc_formats(void)
{
	return (format);
}

const elfcap_desc_t *
elfcap_getdesc_hw1_sparc(void)
{
	return (hw1_sparc);
}

const elfcap_desc_t *
elfcap_getdesc_hw1_386(void)
{
	return (hw1_386);
}

const elfcap_desc_t *
elfcap_getdesc_hw1_aarch64(void)
{
	return (hw1_aarch64);
}

const elfcap_desc_t *
elfcap_getdesc_sf1(void)
{
	return (sf1);
}

const elfcap_desc_t *
elfcap_getdesc_hw2_386(void)
{
	return (hw2_386);
}

const elfcap_desc_t *
elfcap_getdesc_hw2_aarch64(void)
{
	return (hw2_aarch64);
}

const elfcap_desc_t *
elfcap_getdesc_hw3_386(void)
{
	return (hw3_386);
}
