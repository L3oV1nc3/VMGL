# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

include $(TOP)/options.mk
#ARCH=Linux
#ARCH=SunOS
#ARCH=FreeBSD
ARCH=$(shell uname | sed -e 's/-//g')
MACHTYPE=$(shell uname -m)
ECHO := echo
include $(TOP)/$(ARCH).mk

LDFLAGS += -lpthread

###########################
# LEAVE THESE THINGS ALONE!
###########################

CR_CC = $(CC)
CR_CXX = $(CXX)

ifeq ($(USE_OSMESA), 1)
CFLAGS +=-DUSE_OSMESA
CXXFLAGS +=-DUSE_OSMESA
endif

ifdef VTK
VTK_STRING = (VTK)
error VTK on non-windows platform?
endif

ifdef GLUT
LDFLAGS += -L/usr/X11R6/lib -lglut
OPENGL = 1
endif

ifdef OPENGL
ifdef SYSTEM_OPENGL_LIBRARY
LDFLAGS += -L/usr/X11R6/lib -lGLU $(SYSTEM_OPENGL_LIBRARY) -lXmu -lXi -lX11
else
LDFLAGS += -L/usr/X11R6/lib -lGLU -lGL -lXmu -lXi -lX11
endif
endif

ifdef PROGRAM
OBJDIR := $(TOP)/built/$(PROGRAM)
else
ifdef LIBRARY
OBJDIR := $(TOP)/built/$(LIBRARY)
else
OBJDIR := dummy_objdir
endif
endif

BINDIR := $(TOP)/dist/bin/
DSO_DIR := $(TOP)/dist/lib/

define MAKE_OBJDIR
        if test ! -d $(OBJDIR); then $(MKDIR) $(OBJDIR); fi
endef
        
define MAKE_BINDIR
	if test ! -d $(BINDIR); then $(MKDIR) $(BINDIR); fi
endef

define MAKE_DSODIR
	if test ! -d $(DSO_DIR); then $(MKDIR) $(DSO_DIR); fi
endef

ifdef TEST
FILES := $(TEST)
endif

OBJS    := $(addprefix $(OBJDIR)/, $(FILES))
OBJS    := $(addsuffix $(OBJSUFFIX), $(OBJS))
TEMPFILES := *~ \\\#*\\\# so_locations *.pyc tmpAnyDX.a tmpAnyDX.exp load.map shr.o
APIFILES = $(TOP)/glapi_parser/apiutil.py $(TOP)/glapi_parser/APIspec.txt

ifdef LIBRARY
SHORT_TARGET_NAME = $(LIBRARY)
ifdef SHARED
	TARGET := $(LIBPREFIX)$(LIBRARY)$(DLLSUFFIX)
#	LIBNAME := $(addprefix $(OBJDIR)/, $(LIBPREFIX)$(LIBRARY)$(DLLSUFFIX))
else
	TARGET := $(LIBPREFIX)$(LIBRARY)$(LIBSUFFIX)
#	LIBNAME := $(addprefix $(OBJDIR)/, $(LIBPREFIX)$(LIBRARY)$(LIBSUFFIX))
endif
LIBNAME := $(addprefix $(OBJDIR)/, $(TARGET)) 
#TARGET := $(LIBNAME)
else
	LIBNAME := dummy_libname
endif

ifdef PROGRAM
PROG_TARGET := $(BINDIR)/$(PROGRAM)
TARGET := $(PROGRAM)
SHORT_TARGET_NAME = $(PROGRAM)
else
PROG_TARGET := dummy_prog_target
endif

ifndef TARGET
TARGET := NOTHING
endif

INCLUDE_DIRS += -I$(TOP)/include -I.
ifdef PLAYSTATION2
INCLUDE_DIRS += -I/usr/X11R6/include
endif

CFLAGS += -D$(ARCH) $(INCLUDE_DIRS)
CXXFLAGS += -D$(ARCH) $(INCLUDE_DIRS)

ifdef LESSWARN
WARN_STRING = (NOWARN)
else
CFLAGS += $(FULLWARN)
endif

ifeq ($(RELEASE), 1)
CFLAGS += $(C_RELEASE_FLAGS)
CXXFLAGS += $(CXX_RELEASE_FLAGS)
LDFLAGS += $(LD_RELEASE_FLAGS)
RELEASE_STRING = (RELEASE)
RELEASE_FLAGS = "RELEASE=1"
else
ifdef PROFILE
CFLAGS += $(C_DEBUG_FLAGS) $(PROFILE_FLAGS)
CXXFLAGS += $(CXX_DEBUG_FLAGS) $(PROFILE_FLAGS)
LDFLAGS += $(LD_DEBUG_FLAGS) $(PROFILE_LAGS)
RELEASE_STRING = (PROFILE)
else
CFLAGS += $(C_DEBUG_FLAGS)
CXXFLAGS += $(CXX_DEBUG_FLAGS)
LDFLAGS += $(LD_DEBUG_FLAGS)
RELEASE_STRING = (DEBUG)
endif
endif

ifeq ($(THREADSAFE), 1)
CFLAGS += -DCHROMIUM_THREADSAFE=1
CXXFLAGS += -DCHROMIUM_THREADSAFE=1
RELEASE_STRING += (THREADSAFE)
endif

ifndef SUBDIRS
all: arch $(PRECOMP) 
	@$(MAKE) $(PARALLELMAKEFLAGS) recurse
	
recurse: $(PROG_TARGET) $(LIBNAME) copies done
else
SUBDIRS_ALL = $(foreach dir, $(SUBDIRS), $(dir).subdir)

subdirs: $(SUBDIRS_ALL)

$(SUBDIRS_ALL):
	@$(MAKE) -C $(basename $@) $(RELEASE_FLAGS)
endif

release:
	@$(MAKE) RELEASE=1

profile:
	@$(MAKE) PROFILE=1

done:
	@$(ECHO) "  Done!"
	@$(ECHO) ""

arch: 
	@$(ECHO) "-------------------------------------------------------------------------------"
ifdef BANNER
	@$(ECHO) "              $(BANNER)"
else
ifdef PROGRAM
	@$(ECHO) "              Building $(TARGET) for $(ARCH) $(RELEASE_STRING) $(STATE_STRING) $(PACK_STRING) $(UNPACK_STRING) $(VTK_STRING) $(WARN_STRING)"
	@$(MAKE_BINDIR)
endif
ifdef LIBRARY
	@$(ECHO) "              Building $(TARGET) for $(ARCH) $(RELEASE_STRING) $(STATE_STRING) $(PACK_STRING) $(UNPACK_STRING) $(VTK_STRING) $(WARN_STRING)"
	@$(MAKE_DSODIR)
endif
endif
	@$(ECHO) "-------------------------------------------------------------------------------"
	@$(MAKE_OBJDIR)

LDFLAGS += -L$(DSO_DIR)
STATICLIBRARIES := $(foreach lib,$(LIBRARIES),$(wildcard $(TOP)/dist/lib/$(LIBPREFIX)$(lib)$(LIBSUFFIX)))
LIBRARIES := $(foreach lib,$(LIBRARIES),-l$(lib))

# XXX this target should also have a dependency on all static Cr libraries.
$(PROG_TARGET): $(OBJS) $(STATICLIBRARIES)
ifdef PROGRAM
	@$(ECHO) "Linking $(PROGRAM) for $(ARCH)"
	@$(CR_CXX) $(OBJS) -o $(PROG_TARGET)$(EXESUFFIX) $(LDFLAGS) $(LIBRARIES)
endif

$(LIBNAME): $(OBJS) $(LIB_DEFS) $(STATICLIBRARIES)
ifdef LIBRARY
	@$(ECHO) "Linking $@: "
ifdef SHARED
	@$(LD) $(SHARED_LDFLAGS) -o $(LIBNAME) $(OBJS) $(LDFLAGS) $(LIBRARIES)
else #shared
	@$(ECHO) "Static lib: $@"
	@$(AR) $(ARCREATEFLAGS) $@ $(OBJS) 
	@$(RANLIB) $@
endif #shared
	@$(MV) $(LIBNAME) $(DSO_DIR)
endif #library

# Dummy
copies:



.SUFFIXES: .cpp .c .cxx .cc .C .s .l

%.cpp: %.l
	@$(ECHO) "Creating $@"
	@$(LEX) $< > $@

%.cpp: %.y
	@$(ECHO) "Creating $@"
	@$(YACC) $<
	@$(MV) y.tab.c $@

$(OBJDIR)/%.obj: %.cpp Makefile
	@$(ECHO) -n "Compiling "
	@$(CR_CXX) /Fo$@ /c $(CXXFLAGS) $<

$(OBJDIR)/%.obj: %.c Makefile
	@$(ECHO) -n "Compiling "
	@$(CR_CC) /Fo$@ /c $(CFLAGS) $<

$(OBJDIR)/%.o: %.cpp Makefile
	@$(ECHO) "Compiling $<"
	@$(CR_CXX) -o $@ -c $(CXXFLAGS) $<

$(OBJDIR)/%.o: %.cxx Makefile
	@$(ECHO) "Compiling $<"
	@$(CR_CXX) -o $@ -c $(CXXFLAGS) $<

$(OBJDIR)/%.o: %.cc Makefile
	@$(ECHO) "Compiling $<"
	@$(CR_CXX) -o $@ -c $(CXXFLAGS) $<

$(OBJDIR)/%.o: %.C Makefile
	@$(ECHO) "Compiling $<"
	@$(CR_CXX) -o $@ -c $(CXXFLAGS) $<

$(OBJDIR)/%.o: %.c Makefile
	@$(ECHO) "Compiling $<"
	@$(CR_CC) -o $@ -c $(CFLAGS) $<

$(OBJDIR)/%.o: %.s Makefile
	@$(ECHO) "Assembling $<"
	@$(AS) -o $@ $<

###############
# Other targets
###############

clean:
ifdef SUBDIRS
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done
	@$(RM) glapi_parser/apiutil.pyc
	@$(RM) -rf built/
else
ifdef LIBRARY
	@$(ECHO) "Removing all $(ARCH) object files for $(TARGET)."
else
ifdef PROGRAM
	@$(ECHO) "Removing all $(ARCH) object files for $(PROGRAM)."
endif
endif
endif
	@$(RM) $(OBJS) $(TEMPFILES) $(TARGET)
ifneq ($(SLOP)HACK, HACK)
	@$(ECHO) "Also blowing away:    $(SLOP)"
	@$(RM) $(SLOP)
endif


ifdef SUBDIRS
clobber:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clobber; done
else
clobber: clean
ifdef LIBRARY
	@$(ECHO) "Removing $(LIBNAME) for $(ARCH)."
	@$(RM) $(LIBNAME)
	@$(ECHO) "Also removing $(DSO_DIR)/$(TARGET)."
	@$(RM) $(DSO_DIR)/$(TARGET)
ifdef COPY_TARGETS
	@$(ECHO) "Also removing library copies."
	@$(RM) $(COPY_TARGETS)
	@$(RM) $(addprefix $(DSO_DIR)/,$(notdir $(COPY_TARGETS)))
endif
else
ifdef PROGRAM
	@$(ECHO) "Removing $(PROGRAM) for $(ARCH)."
	@$(RM) $(PROGRAM)
	@$(RM) $(BINDIR)/$(PROGRAM)
endif
endif
endif


# Make CRNAME.tar.gz and CRNAME.zip files
CRNAME = cr-lean
tarballs:
	@echo "Removing generated files"
	make clean
	rm -rf ../$(CRNAME)/dist
	
#	remove old archive files
	-rm -f ../$(CRNAME).tar.gz
	-rm -f ../$(CRNAME).zip
	-rm -rf ../$(CRNAME)
	
#	make copy of cr directory
	cp -r ../cr ../$(CRNAME)
	
#	remove CVS files and other unneeded files
	-find ../$(CRNAME) -name CVS -exec rm -rf '{}' \;
	
#	make tarball and zip file
	cd .. ; tar cvf $(CRNAME).tar $(CRNAME) ; gzip $(CRNAME).tar
	cd .. ; zip -r $(CRNAME).zip $(CRNAME)
