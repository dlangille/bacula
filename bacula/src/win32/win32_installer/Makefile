#
# Copyright (C) 2000-2018 Kern Sibbald
# License: BSD 2-Clause; see file LICENSE-FOSS
#
#
# Makefile for win32 bacula executables
# Using MinGW cross-compiler on GNU/Linux
#
#  Written by Robert Nelson, June 2006
#

include ../Makefile.inc

VERSION := $(shell sed -ne 's/^.define[ \t]VERSION[ \t][ \t]*"\(.*\)"/\1/p' < ../../version.h)
RELEASE ?= $(shell awk '/define RELEASE [0-9]+/ { print $$3 }' ../../version.h)

DEFINES := \
	-DVERSION=$(VERSION) \
	-DOUT_DIR=$(BUILDDIR)/release32 \
	-DSRC_DIR=release32 \
	-DBUILD_TOOLS=MinGW32 \
	-DWINVER=32 \
	-DBUILD_BAT=$(BUILD_BAT)

INSTALL_EXE := $(BUILDDIR)/release32/winbacula-$(VERSION).exe

BACULABINARIES := \
	bacula.dll \
	alldrives-fd.dll \
	exchange-fd.dll \
	bacula-fd.exe \
	bconsole.exe \
	bsleep.exe \
	bsmtp.exe \
	bacula-sd.exe \
	bextract.exe \
	bls.exe \
	bcopy.exe

ifeq ($(bat),no)
	BACULA_BINARIES=$(BACULABINARIES)
	HELP=
else
	BACULA_BINARIES=$(BACULABINARIES) bat.exe #bacula-tray-monitor.exe
	HELP=help
endif

EXTRA=

#	bacula-sd.exe \


DEPKGS_BINARIES := \
	libeay32.dll \
	pthreadGCE2.dll \
	zlib1.dll \
	ssleay32.dll \
	openssl.exe \
	sed.exe \
	expr64.exe \
	snooze.exe \
	QtCore4.dll \
	QtGui4.dll  \
	libwinpthread-1.dll \
	libgcc_s_sjlj-1.dll \
	libstdc++-6.dll
#	libgcc_s_dw2-1.dll


NONGCC_BINARIES := 

NONGCC_LIBRARIES := 

MINGW_BINARIES := 

SCRIPT_FILES := 

CAT_FILES := 

DIRD_FILES := 

SSL_FILES := \
	openssl.cnf

LICENSE_FILES := \
	LICENSE

##########################################################################

# Targets

.PHONY: all clean installer distclean

all:	      $(HELP) docs $(INSTALL_EXE) $(EXTRA)

installer:    $(HELP) docs $(INSTALL_EXE)

distclean: clean

clean:
	@echo "Cleaning `pwd`"
	$(CMD_ECHO)-rm -f $(INSTALL_EXE)
	$(CMD_ECHO)-rm -rf release32

help:
	rm -rf release32/help
	mkdir -p release32/help
	cp -f $(BINDIR)/help/* release32/help/

docs:
	rm -rf release32/docs
	mkdir -p release32/docs/manuals/en/console
	mkdir -p release32/docs/manuals/en/main
	mkdir -p release32/docs/manuals/en/misc
	mkdir -p release32/docs/manuals/en/problems
	mkdir -p release32/docs/manuals/en/utility
	cp -f $(DOCDIR)/docs/manuals/en/pdf-and-html/console/console.pdf release32/docs/manuals/en/console/
	cp -f $(DOCDIR)/docs/manuals/en/pdf-and-html/main/main.pdf release32/docs/manuals/en/main/
	cp -f $(DOCDIR)/docs/manuals/en/pdf-and-html/misc/misc.pdf release32/docs/manuals/en/misc/
	cp -f $(DOCDIR)/docs/manuals/en/pdf-and-html/problems/problems.pdf release32/docs/manuals/en/problems/
	cp -f $(DOCDIR)/docs/manuals/en/pdf-and-html/utility/utility.pdf release32/docs/manuals/en/utility/

#
# Rules
#

define Convert_Binary
release32/$$(notdir $(1)): $(1)
	$$(call checkdir,$$@)
	$(ECHO_CMD)cp -f $$^ $$@ ; \
	$(STAB2CV) $$@
endef

define Copy_Binary
release32/$$(notdir $(1)): $(1)
	$$(call checkdir,$$@)
	$(ECHO_CMD)cp -f $$^ $$@
endef

define Copy_Docs
release32/$(1): $(DOCDIR)/$(1)
	$$(call checkdir,$$@)
	cp -f $$^ $$(dir $$@)
	$(ECHO_CMD)cp -f $$^ $$(dir $$@)
endef

define Copy_Licenses
release32/$$(notdir $(1)): $(1)
	$$(call checkdir,$$@)
	$(ECHO_CMD)cp -f $$^ $$(dir $$@)
endef

$(foreach file,$(addprefix $(DEPKGS)/bin/, $(DEPKGS_BINARIES)),$(eval $(call Convert_Binary,$(file))))

$(foreach file,$(addprefix $(DEPKGS)/bin/, $(NONGCC_BINARIES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix $(DEPKGS)/lib/, $(NONGCC_LIBRARIES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix $(BINDIR)/, $(BACULA_BINARIES)),$(eval $(call Convert_Binary,$(file))))

$(foreach file,$(addprefix $(DEPKGS)/ssl/, $(SSL_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix ../scripts/, $(SCRIPT_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix ../cats/, $(CAT_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix ../../dird/, $(DIRD_FILES)),$(eval $(call Copy_Binary,$(file))))

$(foreach file,$(addprefix $(MAINDIR)/, $(LICENSE_FILES)),$(eval $(call Copy_Licenses,$(file))))

#$(foreach file,$(DOC_FILES),$(eval $(call Copy_Docs,$(file))))


$(INSTALL_EXE): winbacula.nsi $(addprefix release32/,$(BACULA_BINARIES) $(SCRIPT_FILES) $(CAT_FILES) $(DEPKGS_BINARIES) $(NONGCC_BINARIES) $(NONGCC_LIBRARIES) $(MINGW_BINARIES) $(SSL_FILES) $(DIRD_FILES) $(LICENSE_FILES) )
	echo "makensis -V3 $(DEFINES) winbacula.nsi"
	makensis -V3 $(DEFINES) winbacula.nsi

include $(BUILDDIR)/Makefile.rules
