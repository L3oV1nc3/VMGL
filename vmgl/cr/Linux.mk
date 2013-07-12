# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

#C and flags

G++-INCLUDE-DIR = /usr/include/g++
CXX = g++
CC = gcc

CXXFLAGS          += -DLINUX -Wall -Werror
CXX_RELEASE_FLAGS += -O3 -DNDEBUG -fno-strict-aliasing -fexpensive-optimizations -funroll-loops -fprefetch-loop-arrays
CXX_DEBUG_FLAGS   += -g -Werror

CFLAGS            += -DLINUX -Wall -Wmissing-prototypes -Wsign-compare
C_RELEASE_FLAGS   += -O3 -DNDEBUG -fno-strict-aliasing -fexpensive-optimizations -funroll-loops -fprefetch-loop-arrays
C_DEBUG_FLAGS     += -g -Werror

PROFILEFLAGS = -pg -a

#MACHTYPE specifics
ifeq ($(MACHTYPE),x86_64)
ifeq ($(FORCE_32BIT_ABI),1)
LDFLAGS           += -m32 -L/usr/X11R6/lib
CFLAGS            += -m32 -fPIC
else
LDFLAGS           += -L/usr/X11R6/lib64
CFLAGS            += -fPIC
endif
else
LDFLAGS           += -L/usr/X11R6/lib
endif

ifeq ($(MACHTYPE), ia64)
CFLAGS            += -fPIC
endif

ifeq ($(MACHTYPE), alpha)
CXXFLAGS          += -fPIC -mieee 
CFLAGS            += -fPIC -mieee
endif

ifeq ($(MACHTYPE), mips)
ifeq ($(shell ls /proc/ps2pad), /proc/ps2pad)
PLAYSTATION2      =   1
CXXFLAGS          += -DPLAYSTATION2
CFLAGS            += -DPLAYSTATION2
endif
endif

ifeq ($(MACHTYPE), i686)
CXX_RELEASE_FLAGS += -march=pentium4 -msse2 -mfpmath=sse 
C_RELEASE_FLAGS += -march=pentium4 -msse2 -mfpmath=sse 
#Hackish
MACHTYPE=i386
endif

ifeq ($(MACHTYPE),i586)
MACHTYPE=i386
endif

ifeq ($(MACHTYPE),i486)
MACHTYPE=i386
endif

#Miscellaneous

CAT = cat
AS = as
LEX = flex -t
LEXLIB = -ll
YACC = bison -y -d
LD = $(CXX)
AR = ar
ARCREATEFLAGS = cr
RANLIB = true
LN = ln -s
MKDIR = mkdir -p
RM = rm -f
CP = cp
MAKE = gmake -s
NOWEB = noweb
LATEX = latex
BIBTEX = bibtex
DVIPS = dvips -t letter
GHOSTSCRIPT = gs
LIBPREFIX = lib
DLLSUFFIX = .so
LIBSUFFIX = .a
OBJSUFFIX = .o
MV = mv
SHARED_LDFLAGS += -shared -Wl,-Bsymbolic
PERL = perl
PYTHON = python -t -t
JGRAPH = /u/eldridge/bin/IRIX/jgraph
PS2TIFF = pstotiff.pl
PS2TIFFOPTIONS = -alpha -mag 2
PS2PDF = ps2pdf
