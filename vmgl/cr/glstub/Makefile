# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

PROGRAM = glstub

FILES = server_main \
	server_boundsinfo \
	server_bufferobject \
	server_clear \
	server_clip \
	server_config \
	server_context \
	server_dispatch \
	server_gentextures \
	server_get \
	server_getmap \
	server_getstring \
	server_getpointer \
	server_getpixelmap \
	server_getteximage \
	server_lists \
	server_misc \
	server_occlude \
	server_papi \
	server_projmatrix \
	server_readpixels \
	server_retval \
	server_simpleget \
	server_stream \
	server_tiles \
	server_viewport \
	server_window \
	server_winpos \
	server_writeback \
	server_XWindowing

LIBRARIES = spuload crstate crutil crunpacker

#ifeq ($(ARCH), SunOS)
LIBRARIES += crpacker
#endif

PRECOMP = server_dispatch.h server_dispatch.c server_get.c server_retval.c server_simpleget.c
SLOP = $(PRECOMP)

UNPACK_STRING = (UNPACK)

X_ROOT = ../../vnc_unixsrc/Xvnc

include $(TOP)/cr.mk

CFLAGS += -I$(X_ROOT)/include/extensions

LDFLAGS += -lX11 -lXext -ldl 

ifeq ($(ARCH), SunOS)
LDFLAGS += -lsocket -lnsl
endif

server_dispatch.h: server_dispatch_header.py server_special $(APIFILES)
	@$(ECHO) Building the server dispatch header
	@$(PYTHON) server_dispatch_header.py > $@

server_dispatch.c: server_dispatch.py server_special $(APIFILES)
	@$(ECHO) Building the server dispatch table
	@$(PYTHON) server_dispatch.py > $@

server_retval.c: server_retval.py server_special $(APIFILES)
	@$(ECHO) Building the server retval functions
	@$(PYTHON) server_retval.py > $@

server_get.c: server_get.py server_special get_components.py $(APIFILES)
	@$(ECHO) Building the server get functions
	@$(PYTHON) server_get.py > $@

server_simpleget.c: server_simpleget.py get_sizes.py $(APIFILES)
	@$(ECHO) Building the simple server get functions
	@$(PYTHON) server_simpleget.py > $@
