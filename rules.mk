# Standard things
.SUFFIXES:
.SUFFIXES:	.c .o

# Version
VERSION := $(VERSION)-$(shell git describe --always --dirty --long)
DEFINES += -DVERSION=$(VERSION)

CFLAGS_$(PACKAGE) :=
CXXFLAGS_$(PACKAGE) :=
INCLUDES_$(PACKAGE) :=
LIBPATH_$(PACKAGE) :=
LIB_$(PACKAGE) :=
LINKFLAGS_$(PACKAGE) :=

# Subdirectories, in random order
dir	:= src
include		$(dir)/rules.mk
dir	:= tests
include		$(dir)/rules.mk

# General directory independent rules
COMP            = $(CXX) $(CXXFLAGS) $(CXXFLAGS_TGT) -I $(shell dirname $<) -o $@ -c $<
LINK            = $(CXX) $(LDFLAGS_TGT) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(LDLIBS_TGT)
COMPLINK        = $(CXX) $(CXXFLAGS) $(CXXFLAGS_TGT) -I $(shell dirname $<) $(LDFLAGS_TGT) $(LDFLAGS) -o $@ $< $(LDLIBS) $(LDLIBS_TGT)
SHAREDLIB       = $(CXX) --shared $(LDFLAGS_TGT) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(LDLIBS_TGT)
STATICLIB       = $(AR) $(ARFLAGS_TGT) $(ARFLAGS) $@ $^

$(BUILDDIR)/%.o: %.cpp
		$(COMP)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

PKGCONFIG_FILE := $(BUILDDIR)/$(PACKAGE).pc
$(PKGCONFIG_FILE): Makefile rules.mk
		@echo "Writing $(PKGCONFIG_FILE)"
		@echo "prefix=$(PREFIX)"  >$(PKGCONFIG_FILE)
		@echo "exec_prefix=$(EXEC_PREFIX)"  >>$(PKGCONFIG_FILE)
		@echo "libdir=$(LIBDIR)"  >>$(PKGCONFIG_FILE)
		@echo "includedir=$(INCLUDEDIR)"  >>$(PKGCONFIG_FILE)
		@echo "" >>$(PKGCONFIG_FILE)
		@echo "Name: $(PACKAGE)"  >>$(PKGCONFIG_FILE)
		@echo "Description: $(DESCRIPTION)"  >>$(PKGCONFIG_FILE)
		@echo "Requires: $(REQUIREMENTS)"  >>$(PKGCONFIG_FILE)
		@echo "Requires.private: $(OPTIONALS)"  >>$(PKGCONFIG_FILE)
		@echo "Version: $(VERSION)"  >>$(PKGCONFIG_FILE)
		@echo "Libs: -L\$${libdir} -l$(PACKAGE)"  >>$(PKGCONFIG_FILE)
		@echo "Libs.private:"  >>$(PKGCONFIG_FILE)
		@echo "Cflags: -I\$${includedir}"  >>$(PKGCONFIG_FILE)

# The variables TGT_*, CLEAN and CMD_INST* may be added to by the Makefile
# fragments in the various subdirectories.

.PHONY: all
all: requires $(BUILDDIR) $(TGT_BIN) $(TGT_SBIN) $(TGT_ETC) $(TGT_LIB) $(TGT_PC) $(PKGCONFIG_FILE)

.PHONY: clean
clean:
	rm -f $(CLEAN) $(PKGCONFIG_FILE)

.PHONY: requires $(REQUIREMENTS) $(OPTIONALS)
requires: $(REQUIREMENTS) $(OPTIONALS)

$(REQUIREMENTS) $(OPTIONALS):
	@echo -n "Checking $@... "
ifneq (,$(findstring $@,$(REQUIREMENTS)))
	@if ! pkg-config --exists $@ ; then echo "no"; exit 1; fi
else
	@if ! pkg-config --exists $@ ; then echo -n "no"; fi
endif
	$(eval HAVE_$@ := $(shell pkg-config --exists $@ 2>/dev/null; test $$? != 0 ; echo $$?))
	$(eval VERSION_$@ := $(shell pkg-config --modversion $@ 2>/dev/null))
	$(eval CFLAGS_$@ := $(shell pkg-config --cflags-only-other $@ 2>/dev/null))
	$(eval CXXFLAGS_$@ := $(CFLAGS_$@))
	$(eval INCLUDES_$@ := $(shell pkg-config --cflags-only-I $@ 2>/dev/null))
	$(eval LIBPATH_$@ := $(shell pkg-config --libs-only-L $@ 2>/dev/null))
	$(eval LIB_$@ := $(shell pkg-config --libs-only-l $@ 2>/dev/null))
	$(eval LINKFLAGS_$@ := $(shell pkg-config --libs-only-other $@ 2>/dev/null))
	$(eval DEFINES := $(DEFINES) -D$(shell echo 'HAVE_$@=$(HAVE_$@)' | tr '[:lower:]' '[:upper:]' | sed 's/-/_/'))
	@echo $(VERSION_$@)

INSTALL = install
.PHONY: install
install: all
	[ -z "$(TGT_BIN)" ] || $(INSTALL) -m 644 -D -t $(BINDIR) $(TGT_BIN)
	[ -z "$(TGT_SBIN)" ] || $(INSTALL) -m 755 -D -t $(SBINDIR) $(TGT_SBIN)
	[ -z "$(TGT_LIB)" ] || $(INSTALL) -m 644 -D -t $(LIBDIR) $(TGT_LIB)
	[ -z "$(TGT_INCLUDE)" ] || $(INSTALL) -m 644 -D -t $(INCLUDEDIR) $(TGT_INCLUDE)
	$(INSTALL) -m 644 -D -t $(LIBDIR)/pkgconfig $(PKGCONFIG_FILE)

.PHONY: check test $(TESTS)
test: all $(TESTS)
check: test

# Prevent make from removing any build targets, including intermediate ones

.SECONDARY:	$(CLEAN)
