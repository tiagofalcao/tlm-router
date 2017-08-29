# Standard things

sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)


# Local variables

HEADERS_$(d)	:= $(shell find $(d) -type f -iname '*.h')
SRCS_$(d)	:= $(shell find $(d) -type f -iname '*.cpp')
OBJS_$(d)	:= $(patsubst $(d)/%.cpp,$(BUILDDIR)/$(d)/%.o, $(filter %.cpp,$(SRCS_$(d))))

CLEAN		:= $(CLEAN) $(OBJS_$(d))

# Local rules

$(BUILDDIR)/$(d):
	mkdir -p $@

OPTIONALS := tlm-dummy $(OPTIONALS)

$(OBJS_$(d)):	CXXFLAGS_TGT = $(INCLUDES_$(PACKAGE)) $(CXXFLAGS_$(PACKAGE)) $(INCLUDES_systemc) $(CXXFLAGS_systemc)  $(INCLUDES_tlm-dummy) $(CXXFLAGS_tlm-dummy)
$(OBJS_$(d)):	systemc tlm-dummy $(HEADERS_$(d)) | $(BUILDDIR)/$(d)

TGT_$(d) := $(BUILDDIR)/$(d)/test
CLEAN		:= $(CLEAN) $(TGT_$(d))

$(TGT_$(d)):	LINKFLAGS_TGT = $(LINKFLAGS_$(PACKAGE)) $(LINKFLAGS_systemc) $(LINKFLAGS_tlm-dummy)
$(TGT_$(d)):	LDLIBS_TGT = $(LIBPATH_$(PACKAGE)) $(LIB_$(PACKAGE)) $(LIBPATH_systemc)  $(LIB_systemc) $(LIBPATH_tlm-dummy)  $(LIB_tlm-dummy)
$(TGT_$(d)): $(OBJS_$(d))
	$(LINK)

TESTS := TEST_$(d) $(TESTS)
TEST_$(d): $(TGT_$(d))
	@echo "Running $< ..."
	$(TGT_$(d))

# Standard things

-include	$(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
