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
 * autod_xdr.c
 *
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * This file can not be automatically generated by rpcgen from
 * autofs_prot.x because of the xdr routines that provide readdir
 * support, and my own implementation of xdr_autofs_netbuf().
 */

#include <sys/vfs.h>
#include <sys/sysmacros.h>		/* includes roundup() */
#include <string.h>
#include <rpcsvc/autofs_prot.h>
#include <nfs/nfs4.h>
#include <rpcsvc/nfs4_prot.h>
#include <rpc/xdr.h>
#include <stdlib.h>
#include <strings.h>

bool_t
xdr_autofs_stat(register XDR *xdrs, autofs_stat *objp)
{
	if (!xdr_enum(xdrs, (enum_t *)objp))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_autofs_action(register XDR *xdrs, autofs_action *objp)
{
	if (!xdr_enum(xdrs, (enum_t *)objp))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_linka(register XDR *xdrs, linka *objp)
{
	if (!xdr_string(xdrs, &objp->dir, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->link, AUTOFS_MAXPATHLEN))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_autofs_netbuf(xdrs, objp)
	XDR *xdrs;
	struct netbuf *objp;
{
	bool_t dummy;

	if (!xdr_u_int(xdrs, &objp->maxlen)) {
		return (FALSE);
	}
	dummy = xdr_bytes(xdrs, (char **)&(objp->buf),
			(uint_t *)&(objp->len), objp->maxlen);
	return (dummy);
}

bool_t
xdr_autofs_args(register XDR *xdrs, autofs_args *objp)
{
	if (!xdr_autofs_netbuf(xdrs, &objp->addr))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->path, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->opts, AUTOFS_MAXOPTSLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->map, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->subdir, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->key, AUTOFS_MAXCOMPONENTLEN))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->mount_to))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->rpc_to))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->direct))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_mounta(register XDR *xdrs, struct mounta *objp)
{
	if (!xdr_string(xdrs, &objp->spec, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->dir, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->flags))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->fstype, AUTOFS_MAXCOMPONENTLEN))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->dataptr, sizeof (autofs_args),
	    (xdrproc_t)xdr_autofs_args))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->datalen))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->optptr, AUTOFS_MAXOPTSLEN))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->optlen))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_action_list_entry(register XDR *xdrs, action_list_entry *objp)
{
	if (!xdr_autofs_action(xdrs, &objp->action))
		return (FALSE);
	switch (objp->action) {
	case AUTOFS_MOUNT_RQ:
		if (!xdr_mounta(xdrs, &objp->action_list_entry_u.mounta))
			return (FALSE);
		break;
	case AUTOFS_LINK_RQ:
		if (!xdr_linka(xdrs, &objp->action_list_entry_u.linka))
			return (FALSE);
		break;
	}
	return (TRUE);
}

bool_t
xdr_action_list(XDR *xdrs, action_list *objp)
{
	action_list *tmp_action_list;
	bool_t more_data = TRUE;
	bool_t first_objp = TRUE;

	if (xdrs->x_op == XDR_DECODE) {
		while (more_data) {
			if (!xdr_action_list_entry(xdrs, &objp->action))
				return (FALSE);
			if (!xdr_bool(xdrs, &more_data))
				return (FALSE);
			if (!more_data) {
				objp->next = NULL;
				break;
			}
			if (objp->next == NULL) {
				objp->next = (action_list *)
				    mem_alloc(sizeof (action_list));
				if (objp->next == NULL)
					return (FALSE);
				bzero(objp->next, sizeof (action_list));
			}
			objp = objp->next;
		}
	} else if (xdrs->x_op == XDR_ENCODE) {
		while (more_data) {
			if (!xdr_action_list_entry(xdrs, &objp->action))
				return (FALSE);
			objp = objp->next;
			if (objp == NULL)
				more_data = FALSE;
			if (!xdr_bool(xdrs, &more_data))
				return (FALSE);
		}
	} else {
		while (more_data) {
			if (!xdr_action_list_entry(xdrs, &objp->action))
				return (FALSE);
			tmp_action_list = objp;
			objp = objp->next;
			if (objp == NULL)
				more_data = FALSE;
			if (!first_objp)
				mem_free(tmp_action_list, sizeof (action_list));
			else
				first_objp = FALSE;
		}
	}
	return (TRUE);
}

bool_t
xdr_umntrequest(register XDR *xdrs, umntrequest *objp)
{
	if (!xdr_bool_t(xdrs, &objp->isdirect))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->mntresource, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->mntpnt, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->fstype, AUTOFS_MAXCOMPONENTLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->mntopts, AUTOFS_MAXOPTSLEN))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->next, sizeof (umntrequest),
	    (xdrproc_t)xdr_umntrequest))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_umntres(register XDR *xdrs, umntres *objp)
{
	if (!xdr_int(xdrs, &objp->status))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_autofs_res(xdrs, objp)
	register XDR *xdrs;
	autofs_res *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_autofs_lookupargs(xdrs, objp)
	register XDR *xdrs;
	autofs_lookupargs *objp;
{
	if (!xdr_string(xdrs, &objp->map, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->path, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->name, AUTOFS_MAXCOMPONENTLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->subdir, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->opts, AUTOFS_MAXOPTSLEN))
		return (FALSE);
	if (!xdr_bool_t(xdrs, &objp->isdirect))
		return (FALSE);
	if (!xdr_u_int(xdrs, (uint_t *)&objp->uid))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_mount_result_type(xdrs, objp)
	register XDR *xdrs;
	mount_result_type *objp;
{
	if (!xdr_autofs_stat(xdrs, &objp->status))
		return (FALSE);
	switch (objp->status) {
	case AUTOFS_ACTION:
		if (!xdr_pointer(xdrs,
		    (char **)&objp->mount_result_type_u.list,
		    sizeof (action_list), (xdrproc_t)xdr_action_list))
			return (FALSE);
		break;
	case AUTOFS_DONE:
		if (!xdr_int(xdrs, &objp->mount_result_type_u.error))
			return (FALSE);
		break;
	}
	return (TRUE);
}

bool_t
xdr_autofs_mountres(xdrs, objp)
	register XDR *xdrs;
	autofs_mountres *objp;
{
	if (!xdr_mount_result_type(xdrs, &objp->mr_type))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->mr_verbose))
		return (FALSE);
	return (TRUE);
}
bool_t
xdr_lookup_result_type(xdrs, objp)
	register XDR *xdrs;
	lookup_result_type *objp;
{
	if (!xdr_autofs_action(xdrs, &objp->action))
		return (FALSE);
	switch (objp->action) {
	case AUTOFS_LINK_RQ:
		if (!xdr_linka(xdrs, &objp->lookup_result_type_u.lt_linka))
			return (FALSE);
		break;
	}
	return (TRUE);
}

bool_t
xdr_autofs_lookupres(xdrs, objp)
	register XDR *xdrs;
	autofs_lookupres *objp;
{
	if (!xdr_autofs_res(xdrs, &objp->lu_res))
		return (FALSE);
	if (!xdr_lookup_result_type(xdrs, &objp->lu_type))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->lu_verbose))
		return (FALSE);
	return (TRUE);
}

/*
 * ******************************************************
 * Readdir XDR support
 * ******************************************************
 */

bool_t
xdr_autofs_rddirargs(xdrs, objp)
	register XDR *xdrs;
	autofs_rddirargs *objp;
{
	if (!xdr_string(xdrs, &objp->rda_map, AUTOFS_MAXPATHLEN))
		return (FALSE);
	if (!xdr_u_int(xdrs, &objp->rda_offset))
		return (FALSE);
	if (!xdr_u_int(xdrs, &objp->rda_count))
		return (FALSE);
	if (!xdr_u_int(xdrs, (uint_t *)&objp->uid))
		return (FALSE);
	return (TRUE);
}

/*
 * Directory read reply:
 * union (enum autofs_res) {
 *	AUTOFS_OK: entlist;
 *		 boolean eof;
 *	default:
 * }
 *
 * Directory entries
 *	struct  direct {
 *		off_t   d_off;			* offset of next entry *
 *		u_long  d_fileno;		* inode number of entry *
 *		u_short d_reclen;		* length of this record *
 *		u_short d_namlen;		* length of string in d_name *
 *		char    d_name[MAXNAMLEN + 1];	* name no longer than this *
 *	};
 * are on the wire as:
 * union entlist (boolean valid) {
 * 	TRUE:	struct otw_dirent;
 *		u_long nxtoffset;
 *		union entlist;
 *	FALSE:
 * }
 * where otw_dirent is:
 * 	struct dirent {
 *		u_long	de_fid;
 *		string	de_name<AUTOFS_MAXPATHLEN>;
 *	}
 */

#ifdef nextdp
#undef nextdp
#endif
#define	nextdp(dp)	((struct dirent64 *)((char *)(dp) + (dp)->d_reclen))

/*
 * ENCODE ONLY
 */
bool_t
xdr_autofs_putrddirres(xdrs, rddir, reqsize)
	XDR *xdrs;
	struct autofsrddir *rddir;
	uint_t reqsize;			/* requested size */
{
	struct dirent64 *dp;
	char *name;
	int size;
	uint_t namlen;
	bool_t true = TRUE;
	bool_t false = FALSE;
	int entrysz;
	int tofit;
	int bufsize;
	uint_t ino, off;

	bufsize = 1 * BYTES_PER_XDR_UNIT;
	for (size = rddir->rddir_size, dp = rddir->rddir_entries;
		size > 0;
		/* LINTED pointer alignment */
		size -= dp->d_reclen, dp = nextdp(dp)) {
		if (dp->d_reclen == 0 /* || DIRSIZ(dp) > dp->d_reclen */) {
			return (FALSE);
		}
		if (dp->d_ino == 0) {
			continue;
		}
		name = dp->d_name;
		namlen = strlen(name);
		ino = (uint_t)dp->d_ino;
		off = (uint_t)dp->d_off;
		entrysz = (1 + 1 + 1 + 1) * BYTES_PER_XDR_UNIT +
		    roundup(namlen, BYTES_PER_XDR_UNIT);
		tofit = entrysz + 2 * BYTES_PER_XDR_UNIT;
		if (bufsize + tofit > reqsize) {
			rddir->rddir_eof = FALSE;
			break;
		}
		if (!xdr_bool(xdrs, &true) ||
		    !xdr_u_int(xdrs, &ino) ||
		    !xdr_bytes(xdrs, &name, &namlen, AUTOFS_MAXPATHLEN) ||
		    !xdr_u_int(xdrs, &off)) {
			return (FALSE);
		}
		bufsize += entrysz;
	}
	if (!xdr_bool(xdrs, &false)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &rddir->rddir_eof)) {
		return (FALSE);
	}
	return (TRUE);
}

#define	DIRENT64_RECLEN(namelen)	\
	((offsetof(dirent64_t, d_name) + 1 + (namelen) + 7) & ~ 7)
#define	reclen(namlen)	DIRENT64_RECLEN((namlen))

/*
 * DECODE ONLY
 */
bool_t
xdr_autofs_getrddirres(xdrs, rddir)
	XDR *xdrs;
	struct autofsrddir *rddir;
{
	struct dirent64 *dp;
	uint_t namlen;
	int size;
	bool_t valid;
	int offset = -1;
	uint_t fileid;

	size = rddir->rddir_size;
	dp = rddir->rddir_entries;
	for (;;) {
		if (!xdr_bool(xdrs, &valid)) {
			return (FALSE);
		}
		if (!valid) {
			break;
		}
		if (!xdr_u_int(xdrs, &fileid) ||
		    !xdr_u_int(xdrs, &namlen)) {
			return (FALSE);
		}
		if (reclen(namlen) > size) {
			rddir->rddir_eof = FALSE;
			goto bufovflw;
		}
		if (!xdr_opaque(xdrs, dp->d_name, namlen)||
		    !xdr_int(xdrs, &offset)) {
			return (FALSE);
		}
		dp->d_ino = fileid;
		dp->d_reclen = reclen(namlen);
		dp->d_name[namlen] = '\0';
		dp->d_off = offset;
		size -= dp->d_reclen;
		/* LINTED pointer alignment */
		dp = nextdp(dp);
	}
	if (!xdr_bool(xdrs, &rddir->rddir_eof)) {
		return (FALSE);
	}
bufovflw:
	rddir->rddir_size = (char *)dp - (char *)(rddir->rddir_entries);
	rddir->rddir_offset = offset;
	return (TRUE);
}

bool_t
xdr_autofs_rddirres(register XDR *xdrs, autofs_rddirres *objp)
{
	if (!xdr_enum(xdrs, (enum_t *)&objp->rd_status))
		return (FALSE);
	if (objp->rd_status != AUTOFS_OK)
		return (TRUE);
	if (xdrs->x_op == XDR_ENCODE)
		return (xdr_autofs_putrddirres(
		    xdrs, (struct autofsrddir *)&objp->rd_rddir,
		    objp->rd_bufsize));
	else if (xdrs->x_op == XDR_DECODE)
		return (xdr_autofs_getrddirres(xdrs,
		    (struct autofsrddir *)&objp->rd_rddir));
	else return (FALSE);
}
