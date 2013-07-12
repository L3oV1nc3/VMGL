# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

SPU = 1

SHARED = 1
LIBRARY = packspu
FILES = packspu \
	packspu_beginend \
	packspu_bufferobject \
	packspu_client \
	packspu_config \
	packspu_context \
	packspu_get \
	packspu_getstring \
	packspu_init \
	packspu_misc \
	packspu_net \
	packspu_swapbuf \
	packspu_pixel \
	packspu_diff \
	packspu_state \
	packspu_lists \
	packspu_resume \
	packspu_resume_autogen
#	packspu_flush \

LIBRARIES = crutil spuload crstate crpacker crXxtn crdlm

PRECOMP = packspu.c packspu_get.c packspu_beginend.c packspu_proto.h packspu_diff.c packspu_state.c packspu_resume_autogen.c
#packspu_flush.c 
SLOP = $(PRECOMP)

LIB_DEFS += pack.def

STATE_STRING = (STATE)
PACK_STRING = (PACK)

include $(TOP)/cr.mk

packspu.c: pack.py packspu_special packspu_unimplemented_special packspu_proto.h $(APIFILES)
	@$(ECHO) Building the Pack SPU
	@$(PYTHON) pack.py > $@

packspu_diff.c: diffgen.py packspu_diff_special packspu_diffpixel_special $(APIFILES)
	@$(ECHO) Generating Pack Diff
	@$(PYTHON) diffgen.py > $@

packspu_state.c: stategen.py $(APIFILES)
	@$(ECHO) Generating Pack State
	@$(PYTHON) stategen.py > $@

packspu_resume_autogen.c: packspu_resume_autogen.py packspu_special packspu_flush_special packspu_proto.h $(APIFILES)
	@$(ECHO) Generating Pack Resume Stubs
	@$(PYTHON) packspu_resume_autogen.py > $@

packspu_get.c: packspu_get.py packspu_special ../glstub/get_sizes.py ../glstub/get_components.py packspu_proto.h $(APIFILES)
	@$(ECHO) Building the Pack SPU Get functions
	@$(PYTHON) packspu_get.py > $@	

#packspu_flush.c: packspu_flush.py packspu_flush_special $(APIFILES)
#	@$(ECHO) Building the Pack SPU Auto-flushing functions
#	@$(PYTHON) packspu_flush.py > $@

packspu_beginend.c: packspu_beginend.py packspu_vertex_special packspu_proto.h $(APIFILES)
	@$(ECHO) Building the Pack SPU Vertex functions
	@$(PYTHON) packspu_beginend.py > $@

packspu_proto.h: packspu_proto.py packspu_special packspu_unimplemented_special packspu_vertex_special packspu_flush_special $(APIFILES)
	@$(ECHO) Building the Pack SPU prototypes header
	@$(PYTHON) packspu_proto.py > $@
