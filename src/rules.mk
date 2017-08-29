# Standard things

sp 		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

# Subdirectories, in random order

#dir	:= $(d)/test
#include		$(dir)/rules.mk

# Local variables

HEADERS_$(d)	:= $(shell find $(d) -type f -iname '*.h')
SRCS_$(d)	:= $(shell find $(d) -type f -iname '*.cpp')
OBJS_$(d)	:= $(patsubst $(d)/%.cpp,$(BUILDDIR)/$(d)/%.o, $(filter %.cpp,$(SRCS_$(d))))

CLEAN		:= $(CLEAN) $(OBJS_$(d))

# Local rules

$(BUILDDIR)/$(d):
		mkdir -p $@

$(OBJS_$(d)):	CXXFLAGS_TGT = $(INCLUDES_systemc) $(CXXFLAGS_systemc)
$(OBJS_$(d)):	$(HEADERS_$(d)) | $(BUILDDIR)/$(d)

$(BUILDDIR)/$(d)/lib$(PACKAGE).so:	LDFLAGS_TGT = $(LINKFLAGS_systemc)
$(BUILDDIR)/$(d)/lib$(PACKAGE).so:	LDLIBS_TGT = $(LIBPATH_systemc) $(LIB_systemc)
$(BUILDDIR)/$(d)/lib$(PACKAGE).so: $(OBJS_$(d))
	$(SHAREDLIB)

$(BUILDDIR)/$(d)/lib$(PACKAGE).a:	ARFLAGS_TGT :=
$(BUILDDIR)/$(d)/lib$(PACKAGE).a: $(OBJS_$(d))
	$(STATICLIB)

TGT_LIB		:= $(TGT_LIB) $(BUILDDIR)/$(d)/lib$(PACKAGE).so $(BUILDDIR)/$(d)/lib$(PACKAGE).a
CLEAN		:= $(CLEAN) $(TGT_LIB)
TGT_INCLUDE	:= $(HEADERS_$(d))

INCLUDES_$(PACKAGE) := -I $(d) $(CXXFLAGS_$(PACKAGE))
PATH_$(PACKAGE) := $(BUILDDIR)/$(d)/
LIBPATH_$(PACKAGE) := -L $(PATH_$(PACKAGE))
LIB_$(PACKAGE) := -l $(PACKAGE)
SHAREDLIB_$(PACKAGE) := $(BUILDDIR)/$(d)/lib$(PACKAGE).so
STATICLIB_$(PACKAGE) := $(BUILDDIR)/$(d)/lib$(PACKAGE).a

# Standard things

-include	$(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
