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
# uts/aarch64/rpcmod/Makefile
#
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Copyright (c) 2018, Joyent, Inc.

#
#	This makefile drives the production of the rpcmod streams kernel
#	module.
#
#	aarch64 architecture dependent
#

#
#	Path to the base of the uts directory tree (usually /usr/src/uts).
#
UTSBASE	= ../..

#
#	Define the module and object file sets.
#
MODULE		= rpcmod
OBJECTS		= $(RPCMOD_OBJS:%=$(OBJS_DIR)/%)
ROOTMODULE	= $(ROOT_STRMOD_DIR)/$(MODULE)
ROOTLINK	= $(ROOT_SYS_DIR)/$(MODULE)

#
#	Include common rules.
#
include $(UTSBASE)/aarch64/Makefile.aarch64

#
#	Define targets
#
ALL_TARGET	= $(BINARY)
INSTALL_TARGET	= $(BINARY) $(ROOTMODULE) $(ROOTLINK)

#
#	Overrides.
#
MODSTUBS_DIR	 = $(OBJS_DIR)
$(MODSTUBS_O)	:= AS_CPPFLAGS += -DRPCMOD_MODULE
CLEANFILES	+= $(MODSTUBS_O)

#
#	depends_on
#
LDFLAGS		+= -Nmisc/tlimod

#
# For now, disable these warnings; maintainers should endeavor
# to investigate and remove these for maximum coverage.
# Please do not carry these forward to new Makefiles.
#

CERRWARN	+= -_gcc=-Wno-parentheses
CERRWARN	+= $(CNOWARN_UNINIT)
CERRWARN	+= -_gcc=-Wno-switch
CERRWARN	+= -_gcc=-Wno-unused-variable
CERRWARN	+= -_gcc=-Wno-unused-label
CERRWARN	+= -_gcc=-Wno-unused-function

# needs work
SMOFF += all_func_returns,indenting,no_if_block,deref_check

#
#	Default build targets.
#
.KEEP_STATE:

def:		$(DEF_DEPS)

all:		$(ALL_DEPS)

clean:		$(CLEAN_DEPS)

clobber:	$(CLOBBER_DEPS)

install:	$(INSTALL_DEPS)

$(ROOTLINK):	$(ROOT_SYS_DIR) $(ROOTMODULE)
		-$(RM) $@; ln $(ROOTMODULE) $@

#
#	Include common targets.
#
include $(UTSBASE)/aarch64/Makefile.targ
