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
# Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Copyright 2016 RackTop Systems.
#

LIBRARY=	pam_smb_passwd.a
VERS=		.1
OBJECTS=	smb_passwd.o

include		../../Makefile.pam_modules

# These are in LDLIBS32/64 so they come before -lsmb
LDLIBS32 += -L$(ROOT)/usr/lib/smbsrv -L$(ADJUNCT_PROTO)/usr/lib/mps
LDLIBS64 += -L$(ROOT)/usr/lib/smbsrv/$(MACH64) -L$(ADJUNCT_PROTO)/usr/lib/mps/64

LDLIBS		+= -lsmb -lpam -lc

all:	$(LIBS)


include	$(SRC)/lib/Makefile.targ
