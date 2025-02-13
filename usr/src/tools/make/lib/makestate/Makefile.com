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

# Copyright 2015, Richard Lowe.

include $(SRC)/tools/Makefile.tools

LIBRARY = libmakestate.a
VERS = .1
OBJECTS = ld_file.o lock.o

include $(SRC)/lib/Makefile.lib
include $(SRC)/tools/Makefile.tools
include ../../../Makefile.com

LIBS = $(DYNLIB)
SRCDIR = $(SRC)/cmd/make/lib/makestate
MAPFILES = $(SRCDIR)/mapfile-vers
LDLIBS += -lc
ADJUNCT_LIBS += libc.so

FILEMODE= 755

all: $(LIBS)

lint:

$(ROOTONBLDLIBMACH)/%: %
	$(INS.file)

$(ROOTONBLDLIBMACH64)/%: %
	$(INS.file)

# We can't create CTF in the tools build because of a bootstrap bug with the new CTF
$(DYNLIB) := CTFMERGE_POST= :
CTFCONVERT_O= :

include $(SRC)/lib/Makefile.targ
