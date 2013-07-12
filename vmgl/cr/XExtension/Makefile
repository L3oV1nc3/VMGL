# Copyright (c) H. Andres Lagar-Cavilla, University of Toronto, 2006-2007
## vmgl project
## See the file LICENSE.txt for information on redistributing this software.

TOP = ..

#SHARED = 1
LIBRARY = crXxtn
FILES = CrXExtension
LIB_DEFS = XExtension.def
X_ROOT = ../../vnc_unixsrc/Xvnc

C_RELEASE_FLAGS	+= -fno-strength-reduce -ansi -pedantic
CFLAGS	 	+= -D_POSIX_C_SOURCE=199309L -D_POSIX_SOURCE -D_XOPEN_SOURCE=500L -D_BSD_SOURCE -D_SVID_SOURCE \
		   -D_REENTRANT -DFUNCPROTO=15 -DNARROWPROTO -DXTHREADS -DXUSE_MTSAFE_API -DMALLOC_0_RETURNS_NULL \
		   -fPIC -I$(X_ROOT)/lib/X11 -I$(X_ROOT)/include/extensions -I$(X_ROOT)/exports/include
SHARED_LDFLAGS	+= -Wl,-soname,lib$(LIBRARY).so

include $(TOP)/cr.mk
