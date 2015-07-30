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
LINK            = $(CXX) $(LDFLAGS_TGT) $(LDFLAGS) -o $@ $^ $(LIBS) $(LIBS_TGT)
COMPLINK        = $(CXX) $(CXXFLAGS) $(CXXFLAGS_TGT) -I $(shell dirname $<) $(LDFLAGS_TGT) $(LDFLAGS) -o $@ $< $(LIBS) $(LIBS_TGT)
SHAREDLIB       = $(CXX) --shared $(LDFLAGS_TGT) $(LDFLAGS) -o $@ $^ $(LIBS) $(LIBS_TGT)
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
requires: $(REQUIREMENTS)

$(REQUIREMENTS) $(OPTIONALS):
	@echo -n "Checking $@... "
	@if ! pkg-config $@ ; then echo "no"; exit 1; fi
	$(eval VERSION_$@ := $(shell pkg-config --modversion $@ 2>/dev/null))
	$(eval CFLAGS_$@ := $(shell pkg-config --cflags-only-other $@ 2>/dev/null))
	$(eval CXXFLAGS_$@ := $(CFLAGS_$@))
	$(eval INCLUDES_$@ := $(shell pkg-config --cflags-only-I $@ 2>/dev/null))
	$(eval LIBPATH_$@ := $(shell pkg-config --libs-only-L $@ 2>/dev/null))
	$(eval LIB_$@ := $(shell pkg-config --libs-only-l $@ 2>/dev/null))
	$(eval LINKFLAGS_$@ := $(shell pkg-config --libs-only-other $@ 2>/dev/null))
	$(eval HAVE_$@ := 1)
	$(eval DEFINES := $(DEFINES) $(shell echo '\-DHAVE_$@=1' | tr '[:lower:]' '[:upper:]'))
	@echo $(VERSION_$@)

INSTALL = install
.PHONY: install
install: all
	[ -z "$(TGT_BIN)" ] || $(INSTALL) -m 644 -t $(BINDIR) $(TGT_BIN)
	[ -z "$(TGT_SBIN)" ] || $(INSTALL) -m 755 -t $(SBINDIR) $(TGT_SBIN)
	[ -z "$(TGT_LIB)" ] || $(INSTALL) -m 644 -t $(LIBDIR) $(TGT_LIB)
	[ -z "$(TGT_INCLUDE)" ] || $(INSTALL) -m 644 -t $(INCLUDEDIR) $(TGT_INCLUDE)
	$(INSTALL) -m 644 -t $(LIBDIR)/pkgconfig $(PKGCONFIG_FILE)

.PHONY: test $(TESTS)
test: all $(TESTS)

# Prevent make from removing any build targets, including intermediate ones

.SECONDARY:	$(CLEAN)
