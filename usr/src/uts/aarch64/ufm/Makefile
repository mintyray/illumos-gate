#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#

#
# Copyright 2019 Joyent, Inc.
#

UTSBASE	= ../..

MODULE = ufm
OBJECTS	= $(OBJS_DIR)/ufm.o
ROOTMODULE = $(ROOT_DRV_DIR)/$(MODULE)
CONF_SRCDIR = $(UTSBASE)/common/io

include $(UTSBASE)/aarch64/Makefile.aarch64

ALL_TARGET = $(BINARY) $(SRC_CONFILE)
INSTALL_TARGET = $(BINARY) $(ROOTMODULE) $(ROOT_CONFFILE)

.KEEP_STATE:

def: $(DEF_DEPS)

all: $(ALL_DEPS)

clean: $(CLEAN_DEPS)

clobber: $(CLOBBER_DEPS)

install: $(INSTALL_DEPS)

include $(UTSBASE)/aarch64/Makefile.targ
