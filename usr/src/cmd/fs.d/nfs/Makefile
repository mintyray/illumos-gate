#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
#
#
# Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Copyright 2018 Nexenta Systems, Inc.  All rights reserved.
#
# cmd/fs.d/nfs/Makefile
#
#  cmd/fs.d/nfs is the directory of all nfs specific commands
#  whose executable reside in $(INSDIR1) and $(INSDIR2).
#

include $(SRC)/Makefile.master

SUBDIR1=		\
	dfshares	\
	dtrace		\
	exportfs	\
	mountd		\
	nfs4cbd		\
	nfsd		\
	nfsfind		\
	nfsstat		\
	rquotad		\
	share		\
	statd		\
	tests

# These do "make catalog"
SUBDIR2=		\
	clear_locks	\
	dfmounts	\
	lockd		\
	mount		\
	nfslog		\
	nfsmapid	\
	nfsref		\
	rp_basic	\
	showmount	\
	umount

SUBDIR3=	etc svc
SUBDIRS=	$(SUBDIR1) $(SUBDIR2) $(SUBDIR3)

# for messaging catalog files
#
POFILES=        $(SUBDIR2:%=%/%.po)
POFILE=         nfs.po

all:=		TARGET= all
install:=	TARGET= install
clean:=		TARGET= clean
clobber:=	TARGET= clobber
catalog:=       TARGET= catalog

.KEEP_STATE:

.PARALLEL:	$(SUBDIRS)

all install clean clobber: $(SUBDIRS)

catalog: $(SUBDIR2)
	$(RM) $(POFILE)
	cat $(POFILES) > $(POFILE)

$(SUBDIRS): FRC
	@cd $@; pwd; $(MAKE) $(TARGET)

FRC:
