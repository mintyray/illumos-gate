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
 * Copyright 2017 Hayashi Naoyuki
 */

/*
 * aarch64 machine dependent and ELF file class dependent functions.
 * Contains routines for performing function binding and symbol relocations.
 */

#include	<stdio.h>
#include	<sys/elf.h>
#include	<sys/elf_aarch64.h>
#include	<sys/debug.h>
#include	<sys/mman.h>
#include	<dlfcn.h>
#include	<synch.h>
#include	<string.h>
#include	<debug.h>
#include	<reloc.h>
#include	<conv.h>
#include	"_rtld.h"
#include	"_audit.h"
#include	"_elf.h"
#include	"_inline_gen.h"
#include	"_inline_reloc.h"
#include	"msg.h"
#include	<unistd.h>
#include	<fcntl.h>

extern void elf_rtbndr(Rt_map *, ulong_t, caddr_t);

int
elf_mach_flags_check(Rej_desc *rej, Ehdr *ehdr)
{
	/*
	 * Check machine type and flags.
	 */
	if (ehdr->e_flags != 0) {
		rej->rej_type = SGS_REJ_BADFLAG;
		rej->rej_info = (uint_t)ehdr->e_flags;
		return (0);
	}
	return (1);
}

void
ldso_plt_init(Rt_map *lmp)
{
	/*
	 * There is no need to analyze ld.so because we don't map in any of
	 * its dependencies.  However we may map these dependencies in later
	 * (as if ld.so had dlopened them), so initialize the plt and the
	 * permission information.
	 */
	if (PLTGOT(lmp))
		elf_plt_init((PLTGOT(lmp)), (caddr_t)lmp);
}

static const uchar_t dyn_plt_template[] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

int	dyn_plt_ent_size = sizeof (dyn_plt_template) +
sizeof (uintptr_t) + sizeof (uintptr_t) + sizeof (ulong_t) +
sizeof (ulong_t) + sizeof (Sym);


/*
 * Function binding routine - invoked on the first call to a function through
 * the procedure linkage table;
 * passes first through an assembly language interface.
 *
 * Takes the offset into the relocation table of the associated
 * relocation entry and the address of the link map (rt_private_map struct)
 * for the entry.
 *
 * Returns the address of the function referenced after re-writing the PLT
 * entry to invoke the function directly.
 *
 * On error, causes process to terminate with a signal.
 */
ulong_t
elf_bndr(Rt_map *lmp, ulong_t reloff)
{
	Rt_map		*nlmp, *llmp;
	ulong_t		addr, pltndx, symval, rsymndx;
	char		*name;
	Rela		*rptr;
	Sym		*rsym, *nsym;
	uint_t		binfo, sb_flags = 0, dbg_class;
	Slookup		sl;
	Sresult		sr;
	int		entry, lmflags;
	Lm_list		*lml;

	pltndx = reloff / (ulong_t)RELENT(lmp);

	/*
	 * For compatibility with libthread (TI_VERSION 1) we track the entry
	 * value.  A zero value indicates we have recursed into ld.so.1 to
	 * further process a locking request.  Under this recursion we disable
	 * tsort and cleanup activities.
	 */
	entry = enter(0);

	lml = LIST(lmp);
	if ((lmflags = lml->lm_flags) & LML_FLG_RTLDLM) {
		dbg_class = dbg_desc->d_class;
		dbg_desc->d_class = 0;
	}

	/*
	 * Perform some basic sanity checks.  If we didn't get a load map or
	 * the relocation offset is invalid then its possible someone has walked
	 * over the .got entries or jumped to plt0 out of the blue.
	 */
	if ((!lmp) && (pltndx <=
	    (ulong_t)PLTRELSZ(lmp) / (ulong_t)RELENT(lmp))) {
		rtldexit(lml, 1);
	}
	reloff = pltndx * (ulong_t)RELENT(lmp);

	/*
	 * Use relocation entry to get symbol table entry and symbol name.
	 */
	addr = (ulong_t)JMPREL(lmp);
	rptr = (Rela *)(addr + reloff);
	rsymndx = ELF_R_SYM(rptr->r_info);
	rsym = (Sym *)((ulong_t)SYMTAB(lmp) + (rsymndx * SYMENT(lmp)));
	name = (char *)(STRTAB(lmp) + rsym->st_name);

	/*
	 * Determine the last link-map of this list, this'll be the starting
	 * point for any tsort() processing.
	 */
	llmp = lml->lm_tail;

	/*
	 * Find definition for symbol.  Initialize the symbol lookup, and
	 * symbol result, data structures.
	 */
	SLOOKUP_INIT(sl, name, lmp, lml->lm_head, ld_entry_cnt, 0,
	    rsymndx, rsym, 0, LKUP_DEFT);
	SRESULT_INIT(sr, name);

	if (lookup_sym(&sl, &sr, &binfo, NULL) == 0) {
		rtldexit(lml, 1);
	}

	name = (char *)sr.sr_name;
	nlmp = sr.sr_dmap;
	nsym = sr.sr_sym;

	symval = nsym->st_value;

	if (!(FLAGS(nlmp) & FLG_RT_FIXED) &&
	    (nsym->st_shndx != SHN_ABS))
		symval += ADDR(nlmp);
	if ((lmp != nlmp) && ((FLAGS1(nlmp) & FL1_RT_NOINIFIN) == 0)) {
		/*
		 * Record that this new link map is now bound to the caller.
		 */
		if (bind_one(lmp, nlmp, BND_REFER) == 0)
			rtldexit(lml, 1);
	}

	if ((lml->lm_tflags | AFLAGS(lmp) | AFLAGS(nlmp)) &
	    LML_TFLG_AUD_SYMBIND) {
		uint_t	symndx = (((uintptr_t)nsym -
		    (uintptr_t)SYMTAB(nlmp)) / SYMENT(nlmp));
		symval = audit_symbind(lmp, nlmp, nsym, symndx, symval,
		    &sb_flags);
	}

	if (!(rtld_flags & RT_FL_NOBIND)) {
		addr = rptr->r_offset;
		if (!(FLAGS(lmp) & FLG_RT_FIXED))
			addr += ADDR(lmp);
		/*
		 * Write standard PLT entry to jump directly
		 * to newly bound function.
		 */
		*(ulong_t *)addr = symval;
	}

	/*
	 * Complete any processing for newly loaded objects.  Note we don't
	 * know exactly where any new objects are loaded (we know the object
	 * that supplied the symbol, but others may have been loaded lazily as
	 * we searched for the symbol), so sorting starts from the last
	 * link-map know on entry to this routine.
	 */
	if (entry)
		load_completion(llmp);

	/*
	 * Some operations like dldump() or dlopen()'ing a relocatable object
	 * result in objects being loaded on rtld's link-map, make sure these
	 * objects are initialized also.
	 */
	if ((LIST(nlmp)->lm_flags & LML_FLG_RTLDLM) && LIST(nlmp)->lm_init)
		load_completion(nlmp);

	/*
	 * Make sure the object to which we've bound has had it's .init fired.
	 * Cleanup before return to user code.
	 */
	if (entry) {
		is_dep_init(nlmp, lmp);
		leave(lml, 0);
	}

	if (lmflags & LML_FLG_RTLDLM)
		dbg_desc->d_class = dbg_class;

	return (symval);
}

/*
 * Read and process the relocations for one link object, we assume all
 * relocation sections for loadable segments are stored contiguously in
 * the file.
 */
int
elf_reloc(Rt_map *lmp, uint_t plt, int *in_nfavl, APlist **textrel)
{
	ulong_t		relbgn, relend, relsiz, basebgn;
	ulong_t		pltbgn, pltend;
	ulong_t		roffset, rsymndx, psymndx = 0;
	ulong_t		dsymndx;
	Word		rtype;
	long		reladd, value, pvalue;
	Sym		*symref, *psymref, *symdef, *psymdef;
	Syminfo		*sip;
	char		*name, *pname;
	Rt_map		*_lmp, *plmp;
	int		ret = 1, noplt = 0;
	int		relacount = RELACOUNT(lmp), plthint = 0;
	Rela		*rel;
	uint_t		binfo, pbinfo;
	APlist		*bound = NULL;
	int		i;

	/*
	 * Although only necessary for lazy binding, initialize the first
	 * global offset entry to go to elf_rtbndr().  dbx(1) seems
	 * to find this useful.
	 */
	if ((plt == 0) && PLTGOT(lmp)) {
		mmapobj_result_t	*mpp;

		/*
		 * Make sure the segment is writable.
		 */
		if ((((mpp =
		    find_segment((caddr_t)PLTGOT(lmp), lmp)) != NULL) &&
		    ((mpp->mr_prot & PROT_WRITE) == 0)) &&
		    ((set_prot(lmp, mpp, 1) == 0) ||
		    (aplist_append(textrel, mpp, AL_CNT_TEXTREL) == NULL)))
			return (0);

		elf_plt_init(PLTGOT(lmp), (caddr_t)lmp);
	}

	/*
	 * Initialize the plt start and end addresses.
	 */
	if ((pltbgn = (ulong_t)JMPREL(lmp)) != 0)
		pltend = pltbgn + (ulong_t)(PLTRELSZ(lmp));

	relsiz = (ulong_t)(RELENT(lmp));
	basebgn = ADDR(lmp);

	if (PLTRELSZ(lmp))
		plthint = PLTRELSZ(lmp) / relsiz;

	/*
	 * If we've been called upon to promote an RTLD_LAZY object to an
	 * RTLD_NOW then we're only interested in scaning the .plt table.
	 * An uninitialized .plt is the case where the associated got entry
	 * points back to the plt itself.  Determine the range of the real .plt
	 * entries using the _PROCEDURE_LINKAGE_TABLE_ symbol.
	 */
	if (plt) {
		relbgn = pltbgn;
		relend = pltend;
	} else {
		/*
		 * The relocation sections appear to the run-time linker as a
		 * single table.  Determine the address of the beginning and end
		 * of this table.  There are two different interpretations of
		 * the ABI at this point:
		 *
		 *   o	The REL table and its associated RELSZ indicate the
		 *	concatenation of *all* relocation sections (this is the
		 *	model our link-editor constructs).
		 *
		 *   o	The REL table and its associated RELSZ indicate the
		 *	concatenation of all *but* the .plt relocations.  These
		 *	relocations are specified individually by the JMPREL and
		 *	PLTRELSZ entries.
		 *
		 * Determine from our knowledege of the relocation range and
		 * .plt range, the range of the total relocation table.  Note
		 * that one other ABI assumption seems to be that the .plt
		 * relocations always follow any other relocations, the
		 * following range checking drops that assumption.
		 */
		relbgn = (ulong_t)(REL(lmp));
		relend = relbgn + (ulong_t)(RELSZ(lmp));
		if (pltbgn) {
			if (!relbgn || (relbgn > pltbgn))
				relbgn = pltbgn;
			if (!relbgn || (relend < pltend))
				relend = pltend;
		}
	}
	if (!relbgn || (relbgn == relend)) {
		DBG_CALL(Dbg_reloc_run(lmp, 0, plt, DBG_REL_NONE));
		return (1);
	}
	DBG_CALL(Dbg_reloc_run(lmp, M_REL_SHT_TYPE, plt, DBG_REL_START));

	/*
	 * If we're processing a dynamic executable in lazy mode there is no
	 * need to scan the .rel.plt table, however if we're processing a shared
	 * object in lazy mode the .got addresses associated to each .plt must
	 * be relocated to reflect the location of the shared object.
	 */
	if (pltbgn && ((MODE(lmp) & RTLD_NOW) == 0) &&
	    (FLAGS(lmp) & FLG_RT_FIXED))
		noplt = 1;

	sip = SYMINFO(lmp);
	/*
	 * Loop through relocations.
	 */
	while (relbgn < relend) {
		mmapobj_result_t	*mpp;
		uint_t			sb_flags = 0;

		rtype = ELF_R_TYPE(((Rela *)relbgn)->r_info, M_MACH);

		/*
		 * If this is a RELATIVE relocation in a shared object (the
		 * common case), and if we are not debugging, then jump into a
		 * tighter relocation loop (elf_reloc_relative).
		 */
		if ((rtype == R_AARCH64_RELATIVE) &&
		    ((FLAGS(lmp) & FLG_RT_FIXED) == 0) && (DBG_ENABLED == 0)) {
			if (relacount) {
				relbgn = elf_reloc_relative_count(relbgn,
				    relacount, relsiz, basebgn, lmp,
				    textrel, 0);
				relacount = 0;
			} else {
				relbgn = elf_reloc_relative(relbgn, relend,
				    relsiz, basebgn, lmp, textrel, 0);
			}
			if (relbgn >= relend)
				break;
			rtype = ELF_R_TYPE(((Rela *)relbgn)->r_info, M_MACH);
		}

		roffset = ((Rela *)relbgn)->r_offset;

		/*
		 * If this is a shared object, add the base address to offset.
		 */
		if (!(FLAGS(lmp) & FLG_RT_FIXED)) {
			/*
			 * If we're processing lazy bindings, we have to step
			 * through the plt entries and add the base address
			 * to the corresponding got entry.
			 */
			if (plthint && (plt == 0) &&
			    (rtype == R_AARCH64_JUMP_SLOT) &&
			    ((MODE(lmp) & RTLD_NOW) == 0)) {
				relbgn = elf_reloc_relative_count(relbgn,
				    plthint, relsiz, basebgn, lmp, textrel, 1);
				plthint = 0;
				continue;
			}
			roffset += basebgn;
		}

		reladd = (long)(((Rela *)relbgn)->r_addend);
		rsymndx = ELF_R_SYM(((Rela *)relbgn)->r_info);
		rel = (Rela *)relbgn;
		relbgn += relsiz;

		/*
		 * Optimizations.
		 */
		if (rtype == R_AARCH64_NONE)
			continue;
		if (noplt && ((ulong_t)rel >= pltbgn) &&
		    ((ulong_t)rel < pltend)) {
			relbgn = pltend;
			continue;
		}

		/*
		 * If this relocation is not against part of the image
		 * mapped into memory we skip it.
		 */
		if ((mpp = find_segment((caddr_t)roffset, lmp)) == NULL) {
			elf_reloc_bad(lmp, (void *)rel, rtype, roffset,
			    rsymndx);
			continue;
		}

		binfo = 0;
		/*
		 * If a symbol index is specified then get the symbol table
		 * entry, locate the symbol definition, and determine its
		 * address.
		 */
		if (rsymndx) {
			/*
			 * If a Syminfo section is provided, determine if this
			 * symbol is deferred, and if so, skip this relocation.
			 */
			if (sip && is_sym_deferred((ulong_t)rel, basebgn, lmp,
			    textrel, sip, rsymndx))
				continue;

			/*
			 * Get the local symbol table entry.
			 */
			symref = (Sym *)((ulong_t)SYMTAB(lmp) +
			    (rsymndx * SYMENT(lmp)));

			/*
			 * If this is a local symbol, just use the base address.
			 * (we should have no local relocations in the
			 * executable).
			 */
			if (ELF_ST_BIND(symref->st_info) == STB_LOCAL) {
				value = basebgn;
				name = NULL;

				/*
				 * Special case TLS relocations.
				 */
				if (rtype == R_AARCH64_TLS_DTPMOD) {
					value = TLSMODID(lmp);
				} else if (rtype == R_AARCH64_TLS_TPREL) {
					if ((value = elf_static_tls(lmp, symref,
					    rel, rtype, 0, roffset, 0)) == 0) {
						ret = 0;
						break;
					}
				}
			} else {
				/*
				 * If the symbol index is equal to the previous
				 * symbol index relocation we processed then
				 * reuse the previous values. (Note that there
				 * have been cases where a relocation exists
				 * against a copy relocation symbol, our ld(1)
				 * should optimize this away, but make sure we
				 * don't use the same symbol information should
				 * this case exist).
				 */
				if ((rsymndx == psymndx) &&
				    (rtype != R_AARCH64_COPY)) {
					/* LINTED */
					if (psymdef == 0) {
						DBG_CALL(Dbg_bind_weak(lmp,
						    (Addr)roffset, (Addr)
						    (roffset - basebgn), name));
						continue;
					}
					/* LINTED */
					value = pvalue;
					/* LINTED */
					name = pname;
					/* LINTED */
					symdef = psymdef;
					/* LINTED */
					symref = psymref;
					/* LINTED */
					_lmp = plmp;
					/* LINTED */
					binfo = pbinfo;

					if ((LIST(_lmp)->lm_tflags |
					    AFLAGS(_lmp)) &
					    LML_TFLG_AUD_SYMBIND) {
						value = audit_symbind(lmp, _lmp,
						    /* LINTED */
						    symdef, dsymndx, value,
						    &sb_flags);
					}
				} else {
					Slookup		sl;
					Sresult		sr;

					/*
					 * Lookup the symbol definition.
					 * Initialize the symbol lookup, and
					 * symbol result, data structure.
					 */
					name = (char *)(STRTAB(lmp) +
					    symref->st_name);

					SLOOKUP_INIT(sl, name, lmp, 0,
					    ld_entry_cnt, 0, rsymndx, symref,
					    rtype, LKUP_STDRELOC);
					SRESULT_INIT(sr, name);
					symdef = NULL;

					if (lookup_sym(&sl, &sr, &binfo,
					    in_nfavl)) {
						name = (char *)sr.sr_name;
						_lmp = sr.sr_dmap;
						symdef = sr.sr_sym;
					}

					/*
					 * If a copy bound to itself, we're in
					 * trouble.
					 */
					if (rtype == M_R_COPY)
						ASSERT(lmp != _lmp);

					/*
					 * If the symbol is not found and the
					 * reference was not to a weak symbol,
					 * report an error.  Weak references
					 * may be unresolved.
					 */
					/* BEGIN CSTYLED */
					if (symdef == 0) {
					    if (sl.sl_bind != STB_WEAK) {
						if (elf_reloc_error(lmp, name,
						    rel, binfo))
							continue;

						ret = 0;
						break;

					    } else {
						psymndx = rsymndx;
						psymdef = 0;

						DBG_CALL(Dbg_bind_weak(lmp,
						    (Addr)roffset, (Addr)
						    (roffset - basebgn), name));
						continue;
					    }
					}
					/* END CSTYLED */

					/*
					 * If symbol was found in an object
					 * other than the referencing object
					 * then record the binding.
					 */
					if ((lmp != _lmp) && ((FLAGS1(_lmp) &
					    FL1_RT_NOINIFIN) == 0)) {
						if (aplist_test(&bound, _lmp,
						    AL_CNT_RELBIND) == 0) {
							ret = 0;
							break;
						}
					}

					/*
					 * Calculate the location of definition;
					 * symbol value plus base address of
					 * containing shared object.
					 */
					if (IS_SIZE(rtype))
						value = symdef->st_size;
					else
						value = symdef->st_value;

					if (!(FLAGS(_lmp) & FLG_RT_FIXED) &&
					    !(IS_SIZE(rtype)) &&
					    (symdef->st_shndx != SHN_ABS) &&
					    (ELF_ST_TYPE(symdef->st_info) !=
					    STT_TLS))
						value += ADDR(_lmp);

					/*
					 * Retain this symbol index and the
					 * value in case it can be used for the
					 * subsequent relocations.
					 */
					if (rtype != R_AARCH64_COPY) {
						psymndx = rsymndx;
						pvalue = value;
						pname = name;
						psymdef = symdef;
						psymref = symref;
						plmp = _lmp;
						pbinfo = binfo;
					}
					if ((LIST(_lmp)->lm_tflags |
					    AFLAGS(_lmp)) &
					    LML_TFLG_AUD_SYMBIND) {
						dsymndx = (((uintptr_t)symdef -
						    (uintptr_t)SYMTAB(_lmp)) /
						    SYMENT(_lmp));
						value = audit_symbind(lmp, _lmp,
						    symdef, dsymndx, value,
						    &sb_flags);
					}
				}

				/*
				 * If relocation is PC-relative, subtract
				 * offset address.
				 */
				if (IS_PC_RELATIVE(rtype))
					value -= roffset;

				/*
				 * Special case TLS relocations
				 */
				if (rtype == R_AARCH64_TLS_DTPMOD) {
					value = TLSMODID(_lmp);
				} else if (rtype == R_AARCH64_TLS_TPREL) {
					if ((value = elf_static_tls(_lmp,
					    symdef, rel, rtype, name, roffset,
					    value)) == 0) {
						ret = 0;
						break;
					}
				}
			}
		} else {
			if (rtype == R_AARCH64_TLS_DTPMOD) {
				value = TLSMODID(_lmp);
			} else {
				value = basebgn;
			}
			name = NULL;
		}

		DBG_CALL(Dbg_reloc_in(LIST(lmp), ELF_DBG_RTLD, M_MACH,
		    M_REL_SHT_TYPE, rel, NULL, 0, name));

		/*
		 * Make sure the segment is writable.
		 */
		if (((mpp->mr_prot & PROT_WRITE) == 0) &&
		    ((set_prot(lmp, mpp, 1) == 0) ||
		    (aplist_append(textrel, mpp, AL_CNT_TEXTREL) == NULL))) {
			ret = 0;
			break;
		}

		/*
		 * Call relocation routine to perform required relocation.
		 */
		switch (rtype) {
		case R_AARCH64_COPY:
			if (elf_copy_reloc(name, symref, lmp, (void *)roffset,
			    symdef, _lmp, (const void *)value) == 0)
				ret = 0;
			break;
		case R_AARCH64_JUMP_SLOT:
			if (!(FLAGS(lmp) & FLG_RT_FIXED)) {
				DBG_CALL(Dbg_reloc_apply_val(LIST(lmp),
				    ELF_DBG_RTLD, (Xword)roffset,
				    (Xword)value));
				*(ulong_t *)roffset = value;
			}
			break;
		default:
			value += reladd;
			/*
			 * Write the relocation out.
			 */
			if (do_reloc_rtld(rtype, (uchar_t *)roffset,
			    (Xword *)&value, name, NAME(lmp), LIST(lmp)) == 0)
				ret = 0;

			DBG_CALL(Dbg_reloc_apply_val(LIST(lmp), ELF_DBG_RTLD,
			    (Xword)roffset, (Xword)value));

		}

		if ((ret == 0) &&
		    ((LIST(lmp)->lm_flags & LML_FLG_TRC_WARN) == 0))
			break;

		if (binfo) {
			DBG_CALL(Dbg_bind_global(lmp, (Addr)roffset,
			    (Off)(roffset - basebgn), (Xword)(-1), PLT_T_FULL,
			    _lmp, (Addr)value, symdef->st_value, name, binfo));
		}
	}

	return (relocate_finish(lmp, bound, ret));
}

/*
 * Initialize the first few got entries so that function calls go to
 * elf_rtbndr:
 */
void
elf_plt_init(void *got, caddr_t l)
{
	uint64_t *_got = (uint64_t *)got;
	_got[1] = (uint64_t)l;
	_got[2] = (uint64_t)elf_rtbndr;
}

/*
 * Plt writing interface to allow debugging initialization to be generic.
 */
Pltbindtype
elf_plt_write(uintptr_t addr, uintptr_t vaddr, void *rptr, uintptr_t symval,
    Xword pltndx)
{
	Rela		*rel = (Rela*)rptr;
	uintptr_t	pltaddr;

	pltaddr = addr + rel->r_offset;
	*(ulong_t *)pltaddr = (ulong_t)symval + rel->r_addend;
	DBG_CALL(pltcntfull++);
	return (PLT_T_FULL);
}

const char *
_conv_reloc_type(uint_t rel)
{
	static Conv_inv_buf_t inv_buf;
	return (conv_reloc_aarch64_type(rel, 0, &inv_buf));
}

/*
 * XXXARM: To keep chkmsg() happy:
 * MSG_INTL(MSG_REL_NOFIT)
 * MSG_INTL(MSG_REL_PLTREF)
 * MSG_INTL(MSG_REL_VALNONALIGN)
 * MSG_ORIG(MSG_SPECFIL_DYNPLT)
 * MSG_ORIG(MSG_SYM_ELFPLTTRACE)
 * MSG_ORIG(MSG_SYM_LADYNDATA)
 * MSG_ORIG(MSG_SYM_PLT)
 */
