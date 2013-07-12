# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

TOP = ..

#Hack. To be punished harshly
include $(TOP)/options.mk
#ARCH=Linux
#ARCH=SunOS
#ARCH=FreeBSD
ARCH=$(shell uname | sed -e 's/-//g')
MACHTYPE=$(shell uname -m)
include $(TOP)/$(ARCH).mk

SHARED = 1
LIBRARY = vmgl
FILES = getprocaddress load stub context NULLfuncs
LIBRARIES = crutil spuload

ifeq ($(ARCH), Linux)
LDFLAGS += -lX11
ifeq ($(MACHTYPE), i386)
FILES += $(ARCH)_$(MACHTYPE)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_$(MACHTYPE)_exports.py
ARCH_EXPORTS = $(ARCH)_$(MACHTYPE)_exports.s
else
ifeq ($(MACHTYPE),x86_64)
FILES += $(ARCH)_$(MACHTYPE)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_$(MACHTYPE)_exports.py
ARCH_EXPORTS = $(ARCH)_$(MACHTYPE)_exports.c
else
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c 
endif
endif
endif

ifeq ($(ARCH), SunOS)
LDFLAGS += -lX11
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c
endif

ifeq ($(ARCH), FreeBSD)
LDFLAGS += -lX11
FILES += $(ARCH)_exports glx xfont
ARCH_EXPORTS_PY = $(ARCH)_exports.py
ARCH_EXPORTS = $(ARCH)_exports.c
endif

PRECOMP = $(ARCH_EXPORTS) getprocaddress.c opengl.def NULLfuncs.c tsfuncs.c

ifeq ($(THREADSAFE), 1)
FILES += tsfuncs
PRECOMP += tsfuncs.c
endif

SLOP = $(PRECOMP)

LIB_DEFS = opengl.def

include $(TOP)/cr.mk

opengl.def: defs.py $(APIFILES)
	@$(ECHO) "Creating the defs file..."
	@$(PYTHON) defs.py > opengl.def

getprocaddress.c: getprocaddress.py $(APIFILES)
	@$(ECHO) "Creating crGetProcAddress"
	@$(PYTHON) getprocaddress.py > getprocaddress.c

$(ARCH_EXPORTS): $(ARCH_EXPORTS_PY) $(APIFILES) entrypoints.py
	@$(ECHO) "Creating the OpenGL DLL export functions (for $(ARCH))"
	@$(PYTHON) $(ARCH_EXPORTS_PY) > $(ARCH_EXPORTS)

NULLfuncs.c: NULLfuncs.py $(APIFILES)
	@$(ECHO) "Creating the NULL dispatch table functions"
	@$(PYTHON) NULLfuncs.py > NULLfuncs.c

tsfuncs.c: tsfuncs.py $(APIFILES)
	@$(ECHO) "Building thread-safe dispatch code"
	@$(PYTHON) tsfuncs.py > tsfuncs.c
